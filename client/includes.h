/*
 * Copyright (C) 2015-2020 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Header : includes.h
 *
 * Abstract :
 *
 *            tdnfclientlib
 *
 *            client library
 *
 * Authors  : Priyesh Padmavilasom (ppadmavilasom@vmware.com)
 */

#ifndef __CLIENT_INCLUDES_H__
#define __CLIENT_INCLUDES_H__

/* Use this to get rid of variable/parameter unused warning */
#define UNUSED(var) ((void)(var))

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <utime.h>
#include <fnmatch.h>
#include <libgen.h>
#include <ctype.h>
//
#include <sys/utsname.h>

#include <dirent.h>
#include <pthread.h>

#include "../solv/includes.h"

//librpm
#include <rpm/rpmlib.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmlog.h>
#include <rpm/rpmps.h>
#include <rpm/rpmts.h>
#include <rpm/rpmkeyring.h>
#include <rpm/header.h>

//libcurl
#include <curl/curl.h>

#include <tdnf.h>
#include <tdnfplugin.h>
#include <tdnfplugineventmap.h>

#include "defines.h"
#include "structs.h"
#include "../common/structs.h"
#include "../common/prototypes.h"
#include "prototypes.h"

#include <metalink/metalink_parser.h>

// Enum in order of preference
enum {
    TDNF_HASH_MD5 = 0,
    TDNF_HASH_SHA1,
    TDNF_HASH_SHA256,
    TDNF_HASH_SENTINEL
};

#endif /* __CLIENT_INCLUDES_H__ */
