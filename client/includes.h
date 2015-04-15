/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
//glib
#include <glib.h>
#include <glib/gstdio.h>
//hawkey
#include <hawkey/advisory.h>
#include <hawkey/advisorypkg.h>
#include <hawkey/advisoryref.h>
#include <hawkey/errno.h>
#include <hawkey/goal.h>
#include <hawkey/nevra.h>
#include <hawkey/package.h>
#include <hawkey/packagelist.h>
#include <hawkey/query.h>
#include <hawkey/reldep.h>
#include <hawkey/repo.h>
#include <hawkey/sack.h>
#include <hawkey/selector.h>
#include <hawkey/util.h>
//librepo
#include <librepo/librepo.h>

//librpm
#include <rpm/rpmlib.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmlog.h>
#include <rpm/rpmps.h>
#include <rpm/rpmts.h>
#include <rpm/rpmkeyring.h>

#include <tdnfclient.h>

#include "defines.h"
#include "structs.h"
#include "prototypes.h"
