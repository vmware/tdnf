/*
 * Copyright (C) 2024 Broadcom, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>

#include "../llconf/nodes.h"

struct cnfnode *parse_varsdirs(char *dirs[])
{
    struct cnfnode *cn_top = NULL;
    DIR *fdir = NULL;
    FILE *fptr = NULL;
    int i;

    cn_top = create_cnfnode("(root)");

    for (i = 0; dirs[i]; i++) {
        fdir = opendir(dirs[i]);
        if (!fdir) {
            if (errno == ENOENT)
                /* ignore non-existing directories */
                continue;
            else
                goto error;
        }

        struct dirent *dent;
        while((dent = readdir(fdir))) {
            char buf[256], path[256];
            char *p;

            p = dent->d_name;
            /* skip disallowed filenames */
            /* See https://dnf.readthedocs.io/en/latest/conf_ref.html#varfiles-label :
               "Filenames may contain only alphanumeric characters and underscores and be in lowercase." */
            while(*p && ((*p == '_') || isdigit(*p) || islower(*p)))
                p++;
            if (*p) {
                continue;
            }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
            snprintf(path, sizeof(path), "%s/%s", dirs[i], dent->d_name);
#pragma GCC diagnostic pop
            fptr = fopen(path, "rt");
            if (fptr == NULL) {
                goto error;
            }
            if (fgets(buf, sizeof(buf), fptr) == NULL) {
                /* might be EOF which is fine */
                if (errno)
                    goto error;
            }
            /* strip trailing spaces and newlines */
            p = buf + strlen(buf)-1;
            while (p >= buf && isspace(*p)) p--;
            p[1] = 0;

            struct cnfnode *cn_var = create_cnfnode(dent->d_name);
            cnfnode_setval(cn_var, buf);
            append_node(cn_top, cn_var);

            fclose(fptr);
        }
        closedir(fdir);
    }
    return(cn_top);

error:
    if (fdir) closedir(fdir);
    if (fptr) fclose(fptr);
    if (cn_top) destroy_cnftree(cn_top);
    return NULL;
}

char *replace_vars(struct cnfnode *cn_vars, const char *source)
{
    char dest[256];
    char *p, *d;
    const char *q, *s;
    struct cnfnode *cn_var;

    d = dest;
    s = source;
    while (*s && d < dest + sizeof(dest)-1) {
        char name[256];
        while(*s && *s != '$' && (d < dest + sizeof(dest)-1))
            *d++ = *s++;
        if (!*s)
            break;

        s++;
        p = name;
        while(*s && (p < name + sizeof(name)-1) && (isdigit(*s) || islower(*s)))
            *p++ = *s++;
        *p = 0;

        cn_var = find_child(cn_vars, name);
        if (cn_var) {
            q = cnfnode_getval(cn_var);
            while(*q && (d < dest + sizeof(dest)-1))
                *d++ = *q++;
        } /* handle not found vars as empty */
    }
    *d = 0;

    return strdup(dest);
}
