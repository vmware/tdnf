/*
 * Copyright (C) 2022-2023 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

#define ASSERT_ARG(x) { \
    if (!(x)) { \
        dwError = ERROR_TDNF_INVALID_PARAMETER; \
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError); \
    } \
}

#define ASSERT_MEM(x) { \
    if (!(x)) { \
        dwError = ERROR_TDNF_OUT_OF_MEMORY; \
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError); \
    } \
}

/* Split nevra into name, ever, arch. This modifies its input,
   and returns pointers to the respective components */
static
int split_nevra(char *nevra, char **name, char **evr, char **arch)
{
    char *p = nevra + strlen(nevra) - 1;

    while (*p != '.' && p > nevra) p--;
    if (p <= nevra) {
        return -1;
    }
    *p = 0;
    *arch = p+1;
    p--;

    while (*p != '-' && p > nevra) p--;
    if (p <= nevra){
        return -2;
    }
    p--;
    while (*p != '-' && p > nevra) p--;
    if (p <= nevra) {
        return -3;
    }
    *p = 0;
    *evr = p+1;

    *name = nevra;

    return 0;
}

/* split string like "pkg-name=1.2.4-1.ph5"
   into "pkg-name" and "1.2.4-1.ph5"
   There is no arch
*/
static
int split_name_equals_evr(char *nevr, char **name, char **evr)
{
    char *p = nevr;
    while(*p && *p != '=')
        p++;
    if (*p)
        *p = 0;
    else
        return -1;
    *evr = p+1;
    *name = nevr;

    return 0;
}

/* Find packages by nevra as specified with ids. Must be either installed
   or not as set by the 'installed' flag. Adds result to qresult, can
   be multiples if package is in multiple repos. */
static uint32_t
SolvFindSolvablesByNevraId(
    Pool *pool,
    Id name,
    Id evr,
    Id arch,
    Queue* qresult,
    int installed
    )
{
    uint32_t dwError = 0;
    Id p;

    ASSERT_ARG(pool);
    ASSERT_ARG(qresult);

    FOR_POOL_SOLVABLES(p)
    {
        const Solvable *s = &pool->solvables[p];
        if (pool->considered && !MAPTST(pool->considered, p))
            continue;
        if (installed == (s->repo == pool->installed)) {
            if (s->name == name && s->evr == evr && s->arch == arch) {
                queue_push(qresult, p);
            }
        }
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

/* Find packages specfied by nevr (no arch),
   from specified repository */
static uint32_t
SolvFindSolvablesByNevrIdFromRepo(
    Pool *pool,
    Repo *repo,
    Id name,
    Id evr,
    Queue* qresult
    )
{
    uint32_t dwError = 0;
    Id p;

    ASSERT_ARG(pool);
    ASSERT_ARG(qresult);

    for (p = repo->start; p < repo->end; p++) {
        Solvable *s = pool_id2solvable(pool, p);
        if (s->name == name && s->evr == evr) {
            queue_push(qresult, p);
        }
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

/* Same as SolvFindSolvablesByNevraId but takes nevra as a string. */
uint32_t
SolvFindSolvablesByNevraStr(
    Pool *pool,
    const char *nevra,
    Queue* qresult,
    int installed
    )
{
    uint32_t dwError = 0;
    char *n = NULL, *name, *evr, *arch;
    Id id_name, id_evr, id_arch;

    ASSERT_ARG(pool);
    ASSERT_ARG(qresult);

    n = strdup(nevra);
    ASSERT_MEM(n);

    if (split_nevra(n, &name, &evr, &arch) != 0)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    id_name = pool_str2id(pool, name, 0);
    id_evr = pool_str2id(pool, evr, 0);
    id_arch = pool_str2id(pool, arch, 0);

    if (id_name && id_evr && id_arch)
    {
        dwError = SolvFindSolvablesByNevraId(pool, id_name, id_evr, id_arch, qresult, installed);
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

cleanup:
    if (n)
        free(n);
    return dwError;

error:
    goto cleanup;
}

/* Find packages in repo matching string of the
   form "pkg-name=1.2-4.ph5" */
uint32_t
SolvFindSolvablesByNEqualsEvrFromRepo(
    Pool *pool,
    Repo *repo,
    const char *nevr,
    Queue* qresult
)
{
    uint32_t dwError = 0;
    char *n = NULL, *name, *evr;
    Id id_name, id_evr;

    ASSERT_ARG(pool);
    ASSERT_ARG(qresult);

    n = strdup(nevr);
    ASSERT_MEM(n);

    if (split_name_equals_evr(n, &name, &evr) != 0)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    id_name = pool_str2id(pool, name, 0);
    id_evr = pool_str2id(pool, evr, 0);
    if (id_name && id_evr)
    {
        dwError = SolvFindSolvablesByNevrIdFromRepo(pool, repo, id_name, id_evr, qresult);
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

cleanup:
    if (n)
        free(n);
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvRequiresFromQueue(
    Pool *pool,
    Queue *pq_pkgs,  /* solvable ids */
    Queue *pq_deps   /* string ids */
)
{
    uint32_t dwError = 0;
    int i,j;

    for (i = 0; i < pq_pkgs->count; i++) {
        Queue q_tmp = {0};
        Solvable *p_solv = pool_id2solvable(pool, pq_pkgs->elements[i]);
        solvable_lookup_deparray(p_solv, SOLVABLE_REQUIRES, &q_tmp, -1);
        for(j = 0; j < q_tmp.count; j++) {
            queue_pushunique(pq_deps, q_tmp.elements[j]);
        }
        queue_free(&q_tmp);
    }
    return dwError;
}
