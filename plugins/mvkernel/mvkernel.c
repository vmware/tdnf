/*
 * Copyright (C) 2020 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

static bool isKernMoved = false;

static int32_t _bindmount(const char *src, const char *dst,
                          unsigned long flags);

int32_t get_kern_version(char *buf, int32_t bufsize)
{
    int32_t dwError = 0;
    struct utsname st = {0};

    if (!buf || bufsize < _UTSNAME_VERSION_LENGTH)
    {
        return ERROR_TDNF_INVALID_PARAMETER;
    }

    dwError = uname(&st);
    BAIL_ON_TDNF_ERROR(dwError);

    snprintf(buf, bufsize, "%s", st.release);

    return dwError;

error:
    return ERR_TDNF_MVKERNEL_GET_KERN_VER;
}

static int32_t _bindmount(const char *src, const char *dst,
                          unsigned long flags)
{
    int32_t dwError = 0;
    const unsigned long mntflags = flags;

    if (IsNullOrEmptyString(src) || IsNullOrEmptyString(dst))
    {
        return ERROR_TDNF_INVALID_PARAMETER;
    }

    dwError = mkdir(dst, 0755);
    BAIL_ON_TDNF_ERROR((dwError && dwError != EEXIST));

    dwError = mount(src, dst, "", mntflags, "");
    printf("Mount %s at %s - %s\n", src, dst, !dwError ? "success" : "failed");
    if (dwError)
    {
        fprintf(stderr, "Reason: %s\n", strerror(errno));
        goto error;
    }

    return dwError;

error:
    return ERR_TDNF_MVKERNEL_MOUNT_FAILED;
}

static int32_t fix_dst_path(const char *src, char *dst, int32_t dstsize)
{
    char *s = NULL;
    char buf[dstsize];

    if (IsNullOrEmptyString(src) || IsNullOrEmptyString(dst) || dstsize <= 0)
    {
        return ERROR_TDNF_INVALID_PARAMETER;
    }

    BAIL_ON_TDNF_ERROR(!(s = strrchr(src, '/')));

    memset(buf, 0, dstsize);
    snprintf(buf, dstsize, "%s/%s", dst, ++s);
    strcpy(dst, buf);

    return 0;

error:
    return ERR_TDNF_MVKERNEL_UNKNWN;
}

int32_t removedir(const char *path)
{
    FTS *ftsp = NULL;
    FTSENT *ent = NULL;
    int32_t dwError = 0;
    char rpath[PATH_MAX] = {0};

    if (IsNullOrEmptyString(path))
    {
        return ERROR_TDNF_INVALID_PARAMETER;
    }

    if (!realpath(path, rpath))
    {
        return ERR_TDNF_MVKERNEL_WRNG_PATH;
    }

    char *paths[] = { rpath, NULL };

    ftsp = fts_open(paths, FTS_PHYSICAL, NULL);
    if (!ftsp)
    {
        return ERR_TDNF_MVKERNEL_FTS_OP_FAILED;
    }

    errno = 0;
    while ((ent = fts_read(ftsp)))
    {
        if (ent->fts_info & FTS_DP)
        {
            rmdir(ent->fts_path);
        }
        else if (ent->fts_info & FTS_F)
        {
            unlink(ent->fts_path);
        }

        if (errno)
        {
            dwError = ERR_TDNF_MVKERNEL_FOPS_FAILED;
            fprintf(stderr, "Failed to delete: %s\n", ent->fts_path);
            goto error;
        }
    }

error:
    if (fts_close(ftsp))
    {
         return ERR_TDNF_MVKERNEL_FTS_OP_FAILED;
    }

    return dwError;
}

int32_t copy_file(const char *src, const char *dst)
{
    size_t n = 0;
    FILE *in = NULL;
    FILE *out = NULL;
    char rbuf[BUFSIZ] = {0};
    int32_t dwError = ERR_TDNF_MVKERNEL_FOPS_FAILED;

    if (IsNullOrEmptyString(src) || IsNullOrEmptyString(dst))
    {
        return ERROR_TDNF_INVALID_PARAMETER;
    }

    in = fopen(src, "rb");
    out= fopen(dst, "wb");
    BAIL_ON_TDNF_ERROR((!in || !out));

    while ((n = fread(rbuf, 1, BUFSIZ, in)))
    {
        BAIL_ON_TDNF_ERROR((fwrite(rbuf, 1, n, out) != n));
    }

    BAIL_ON_TDNF_ERROR(!feof(in));

    dwError = 0;

error:
    if ((in && fclose(in)) || (out && fclose(out)))
    {
        dwError = ERR_TDNF_MVKERNEL_FOPS_FAILED;
    }

    return dwError;
}

int32_t mvdir(const char *src, const char *dst)
{
    FTS *ftsp = NULL;
    FTSENT *ent = NULL;
    int32_t dwError = 0;
    char srcpath[PATH_MAX] = {0};
    char dstpath[PATH_MAX] = {0};

    if (IsNullOrEmptyString(src) || IsNullOrEmptyString(dst))
    {
        return ERROR_TDNF_INVALID_PARAMETER;
    }

    if (!realpath(src, srcpath))
    {
        return ERR_TDNF_MVKERNEL_WRNG_PATH;
    }

    char *paths[] = { srcpath, NULL };

    ftsp = fts_open(paths, FTS_PHYSICAL, NULL);
    if (!ftsp)
    {
        return ERR_TDNF_MVKERNEL_FTS_OP_FAILED;
    }

    if (!realpath(dst, dstpath))
    {
        BAIL_ON_TDNF_ERROR((dwError = ERR_TDNF_MVKERNEL_WRNG_PATH));
    }

    while ((ent = fts_read(ftsp))) {
        char *s = NULL;
        char *cursrc = ent->fts_path;

        if (ent->fts_info & FTS_D)
        {
            struct stat fstat = {0};

            dwError = fix_dst_path(cursrc, dstpath, sizeof(dstpath));
            BAIL_ON_TDNF_ERROR(dwError);

            if (stat(srcpath, &fstat) || mkdir(dstpath, fstat.st_mode))
            {
                if (errno != EEXIST)
                {
                    BAIL_ON_TDNF_ERROR((dwError = ERR_TDNF_MVKERNEL_MKDIR_FAILED));
                }
            }
        }
        else if (ent->fts_info & FTS_DP)
        {
            s = strrchr(dstpath, '/');
            BAIL_ON_TDNF_ERROR(!s && (dwError = ERR_TDNF_MVKERNEL_UNKNWN));

            *s = '\0';
        }
        else if (ent->fts_info & FTS_F)
        {
            dwError = fix_dst_path(cursrc, dstpath, sizeof(dstpath));
            BAIL_ON_TDNF_ERROR(dwError);

            if (copy_file(cursrc, dstpath))
            {
                fprintf(stderr, "Unable to copy file: %s to destination %s\n",
                        cursrc, dstpath);
                goto error;
            }
            sync();

            s = strrchr(dstpath, '/');
            BAIL_ON_TDNF_ERROR(!s && (dwError = ERR_TDNF_MVKERNEL_UNKNWN));

            *s = '\0';
        }
        else
        {
            /* We shouldn't reach here */
            fprintf(stderr, "Other: %s\n", cursrc);
        }
    }

    dwError = removedir(src);
    if (dwError)
    {
        fprintf(stderr, "Deletion of %s failed\n", src);
        goto error;
    }

error:
    if (fts_close(ftsp))
    {
        dwError = ERR_TDNF_MVKERNEL_FTS_OP_FAILED;
    }

    printf("Move %s to %s - %s\n", srcpath, dstpath, !dwError ? "success" :
            "failed");
    if (!dwError)
    {
        isKernMoved = true;
    }

    return dwError;
}

int32_t mv_running_kernel(const char *pkgname)
{
    char src[PATH_MAX] = {0};
    char kver[_UTSNAME_VERSION_LENGTH] = {0};

    if (IsNullOrEmptyString(pkgname))
    {
        return ERROR_TDNF_INVALID_PARAMETER;
    }

    if (get_kern_version(kver, sizeof(kver)))
    {
        return ERR_TDNF_MVKERNEL_GET_KERN_VER;
    }

    if (!strstr(pkgname, kver))
    {
        return 0;
    }

    snprintf(src, sizeof(src), LIBMODULESDIR"/%s", kver);
    return mvdir(src, TMPDIR);
}

int32_t bindmount(void)
{
    char src[PATH_MAX] = {0};
    char dst[PATH_MAX] = {0};
    char kver[_UTSNAME_VERSION_LENGTH] = {0};

    if (!isKernMoved)
    {
        return ERR_TDNF_MVKERNEL_ERR;
    }
    isKernMoved = false;

    if (get_kern_version(kver, sizeof(kver)))
    {
        return ERR_TDNF_MVKERNEL_GET_KERN_VER;
    }

    snprintf(src, sizeof(src), TMPDIR"/%s", kver);
    snprintf(dst, sizeof(dst), LIBMODULESDIR"/%s", kver);

    return _bindmount(src, dst, MS_BIND | MS_REC);
}
