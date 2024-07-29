/*
 * Copyright (C) 2015-2023 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#ifndef __CLIENT_INCLUDES_H__
#define __CLIENT_INCLUDES_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <ftw.h>
#include <time.h>
#include <utime.h>
#include <fnmatch.h>
#include <libgen.h>
#include <ctype.h>
#include <sys/file.h>
#include <time.h>
#include <sys/utsname.h>
#include <sys/vfs.h>
#include <sys/types.h>

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
#include <rpm/rpmcli.h>
#include <rpm/rpmtypes.h>

//libcurl
#include <curl/curl.h>

#include "../history/history.h"

#include <tdnf.h>
#include <tdnfplugin.h>
#include <tdnfplugineventmap.h>
#include <tdnf-common-defines.h>

#include "defines.h"
#include "structs.h"
#include "../common/config.h"
#include "../common/structs.h"
#include "../common/prototypes.h"
#include "prototypes.h"

#include "config.h"

#endif /* __CLIENT_INCLUDES_H__ */
