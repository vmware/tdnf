/*
 * Copyright (C) 2021-2023 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

static int tdnfCreateLockFile(const char *lockPath);

static int tdnfCreateLockFile(const char *lockPath)
{
    mode_t oldmask;
    int lockFd;

    oldmask = umask(022);
    lockFd = open(lockPath, O_RDWR|O_CREAT|O_CLOEXEC, 0644);
    (void) umask(oldmask);

    if (lockFd < 0)
    {
        pr_err("%s: open failed for %s (%s)\n", __func__, lockPath,
               strerror(errno));
        return -1;
    }

    {
        /* Write out PID into lock file */
        char pidBuf[128] = {0};
        int rsnpf;

        rsnpf = snprintf(pidBuf, sizeof(pidBuf), "%ld\n", (long) getpid());

        if (rsnpf > 0)
        {
            if (!write(lockFd, pidBuf, rsnpf))
            {
              sync();
            }
        }
    }

    return lockFd;
}

int tdnfLockAcquire(const char *lockPath)
{
    int lockFd;

    if (IsNullOrEmptyString(lockPath))
    {
        pr_err("%s: lockPath is empty\n", __func__);
        return -1;
    }

    lockFd = tdnfCreateLockFile(lockPath);
    if (lockFd < 0)
    {
        pr_err("%s: tdnfCreateLockFile failed\n", __func__);
        return -1;
    }

    while (1)
    {
        if (!flock(lockFd, LOCK_EX|LOCK_NB))
        {
           break;
        }
        pr_err("WARNING: failed to acquire lock on: %s, retrying ...\n", lockPath);
        sleep(1);
    }

    return lockFd;
}

void tdnfLockFree(const char *lockPath, const int lockFd)
{
    if (lockFd >= 0)
    {
        if (flock(lockFd, LOCK_UN))
        {
          pr_err("ERROR: failed to unlock: '%s'\n", lockPath);
        }
        (void) close(lockFd);
    }

    if (!access(lockPath, F_OK) && remove(lockPath))
    {
        pr_err("WARNING: Unable to remove lockfile(%s)\n", lockPath);
    }
}
