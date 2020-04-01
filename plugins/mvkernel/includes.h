/*
 * Copyright (C) 2020 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#ifndef __PLUGINS_MVKERNEL_INCLUDES_H__
#define __PLUGINS_MVKERNEL_INCLUDES_H__

#define TMPDIR        "/tmp"
#define LIBMODULESDIR "/lib/modules"

#include <fts.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/mount.h>
#include <sys/utsname.h>

#include <tdnf.h>
#include <tdnfplugin.h>
#include <tdnfplugineventmap.h>

#include "config.h"
#include "defines.h"
#include "prototypes.h"

#include "../../common/defines.h"
#include "../../common/structs.h"
#include "../../common/prototypes.h"

#endif /* __PLUGINS_MVKERNEL_INCLUDES_H__ */
