/*
 * Copyright (C) 2021-2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

static void tdnflock_free(tdnflock lock);
static void tdnflock_release(tdnflock lock);
static int tdnflock_acquire(tdnflock lock, int mode);
static tdnflock tdnflock_new(const char *lock_path, const char *descr);

static tdnflock tdnflock_new(const char *lock_path, const char *descr)
{
	mode_t oldmask;
	uint32_t dwErr = 0;
	tdnflock lock = NULL;

	if (IsNullOrEmptyString(lock_path) || IsNullOrEmptyString(descr))
	{
		goto error;
	}

	dwErr = TDNFAllocateMemory(1, sizeof(*lock), (void *)&lock);
	if (dwErr)
	{
		goto error;
	}

	oldmask = umask(022);
	lock->fd = open(lock_path, O_RDWR|O_CREAT, 0644);
	(void) umask(oldmask);

	if (lock->fd < 0)
	{
		if (errno == EACCES)
		{
			lock->fd = open(lock_path, O_RDONLY);
		}

		if (lock->fd < 0)
		{
			TDNF_SAFE_FREE_MEMORY(lock);
			return NULL;
		}
		else
		{
			lock->openmode = TDNFLOCK_READ;
		}
	}
	else
	{
		lock->openmode = TDNFLOCK_WRITE | TDNFLOCK_READ;
	}

	lock->fdrefs = 1;
	dwErr = TDNFAllocateString(lock_path, &lock->path);
	BAIL_ON_TDNF_ERROR(dwErr);
	dwErr = TDNFAllocateString(descr, &lock->descr);
	BAIL_ON_TDNF_ERROR(dwErr);

	return lock;

error:
	tdnflock_free(lock);

	return lock;
}

static void tdnflock_free(tdnflock lock)
{
	if (lock && --lock->fdrefs == 0)
	{
		TDNF_SAFE_FREE_MEMORY(lock->path);
		TDNF_SAFE_FREE_MEMORY(lock->descr);
		if (lock->fd >= 0)
		{
			(void) close(lock->fd);
		}
		TDNF_SAFE_FREE_MEMORY(lock);
	}
}

static int tdnflock_acquire(tdnflock lock, int mode)
{
	int res = 0;

	if (!lock || mode < 0)
	{
		goto end;
	}

	if (!(mode & lock->openmode))
		return res;

	if (lock->fdrefs > 1) {
		res = 1;
	} else {
		int cmd;
		struct flock info;

		if (mode & TDNFLOCK_WAIT)
		{
			cmd = F_SETLKW;
		}
		else
		{
			cmd = F_SETLK;
		}
		if (mode & TDNFLOCK_READ)
		{
			info.l_type = F_RDLCK;
		}
		else
		{
			info.l_type = F_WRLCK;
		}
		info.l_whence = SEEK_SET;
		info.l_start = 0;
		info.l_len = 0;
		info.l_pid = 0;

		if (fcntl(lock->fd, cmd, &info) != -1)
		{
			res = 1;
		}
	}

	lock->fdrefs += res;

end:
	return res;
}

static void tdnflock_release(tdnflock lock)
{
	if (!lock || lock->fdrefs <= 1)
	{
		return;
	}

	if (--lock->fdrefs == 1)
	{
		struct flock info;
		info.l_type = F_UNLCK;
		info.l_whence = SEEK_SET;
		info.l_start = 0;
		info.l_len = 0;
		info.l_pid = 0;
		(void) fcntl(lock->fd, F_SETLK, &info);
	}
}

/* External interface */
tdnflock tdnflockNew(const char *lock_path, const char *descr)
{
	tdnflock lock = NULL;

	if (IsNullOrEmptyString(lock_path) || IsNullOrEmptyString(descr))
	{
		goto end;
	}

	lock = tdnflock_new(lock_path, descr);
	if (!lock)
	{
		pr_err("can't create %s lock on %s (%s)\n",
				descr, lock_path, strerror(errno));
	}

end:
	return lock;
}

int tdnflockAcquire(tdnflock lock)
{
	int locked = 0; /* assume failure */

	if (!lock)
	{
		return locked;
	}

	locked = tdnflock_acquire(lock, TDNFLOCK_WRITE);
	if (!locked && (lock->openmode & TDNFLOCK_WRITE))
	{
		pr_crit("waiting for %s lock on %s\n", lock->descr, lock->path);
		locked = tdnflock_acquire(lock, (TDNFLOCK_WRITE|TDNFLOCK_WAIT));
	}

	if (!locked)
	{
		pr_err("can't create %s lock on %s (%s)\n", lock->descr, lock->path,
													strerror(errno));
	}

	return locked;
}

void tdnflockRelease(tdnflock lock)
{
	if (lock)
	{
		tdnflock_release(lock);
	}
}

tdnflock tdnflockNewAcquire(const char *lock_path, const char *descr)
{
	tdnflock lock = NULL;

	if (IsNullOrEmptyString(lock_path) || IsNullOrEmptyString(descr))
	{
		goto end;
	}

	lock = tdnflockNew(lock_path, descr);

	if (!tdnflockAcquire(lock))
	{
		lock = tdnflockFree(lock);
	}

end:
	return lock;
}

tdnflock tdnflockFree(tdnflock lock)
{
	if (lock)
	{
		tdnflock_release(lock);
		tdnflock_free(lock);
	}

	return NULL;
}
