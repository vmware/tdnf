/*
 * Copyright (C) 2017-2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#ifndef __SOLV_INCLUDES_H__
#define __SOLV_INCLUDES_H__

#include <stdio.h>
#include <stdint.h>
#include <sys/utsname.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>

// libsolv
#include <solv/evr.h>
#include <solv/pool.h>
#include <solv/poolarch.h>
#include <solv/repo.h>
#include <solv/repo_deltainfoxml.h>
#include <solv/repo_repomdxml.h>
#include <solv/repo_updateinfoxml.h>
#include <solv/repo_rpmmd.h>
#include <solv/repo_rpmdb.h>
#include <solv/repo_solv.h>
#include <solv/repo_write.h>
#include <solv/solv_xfopen.h>
#include <solv/solver.h>
#include <solv/selection.h>
#include <solv/solverdebug.h>
#include <solv/testcase.h>
#include <solv/chksum.h>

#include <tdnf.h>
#include <tdnf-common-defines.h>

#include "defines.h"
#include "tdnferror.h"
#include "../common/defines.h"
#include "../common/structs.h"
#include "../common/prototypes.h"
#include "prototypes.h"

#endif /* __SOLV_INCLUDES_H__ */
