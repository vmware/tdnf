/*
 * Copyright (C) 2017 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Header   : includes.h
 *
 * Abstract :
 *
 *            tdnf
 *
 *            command line tool
 *
 * Authors  : Priyesh Padmavilasom (ppadmavilasom@vmware.com)
 *
 */

#ifndef __CLI_LIB_INCLUDES_H__
#define __CLI_LIB_INCLUDES_H__

/* Use this to get rid of variable/parameter unused warning */
#define UNUSED(var) ((void)(var))

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

#include <tdnf.h>
#include <tdnfcli.h>

#include "../defines.h"
#include "prototypes.h"
#include "../common/structs.h"
#include "../common/prototypes.h"

#endif /* __CLI_LIB_INCLUDES_H__ */
