/*
 * Copyright (C) 2022 VMware, Inc. All Rights Reserved.
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

/* Find packages by nevra as specified with ids. Must be either installed
   or not as set by the 'installed' flag. Returns results in a queue, can
   be multiples if package is in multiple repos. */
uint32_t
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

    ASSERT_ARG(pool);
    ASSERT_ARG(qresult);

    Id p;
    FOR_POOL_SOLVABLES(p)
    {
        Solvable *s = &pool->solvables[p];
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

    ASSERT_ARG(pool);
    ASSERT_ARG(qresult);

    n = strdup(nevra);
    ASSERT_MEM(n);

    if (split_nevra(n, &name, &evr, &arch) != 0)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    Id id_name = pool_str2id(pool, name, 0);
    Id id_evr = pool_str2id(pool, evr, 0);
    Id id_arch = pool_str2id(pool, arch, 0);

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
