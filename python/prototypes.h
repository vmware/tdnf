/*
 * Copyright (C) 2019 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#pragma once

/* tdnfpyrepodata.c */
uint32_t
TDNFPyMakeRepoData(
   PTDNF_REPO_DATA pRepoData,
   PyObject **ppPyRepoData
   );

/* utils.c */
uint32_t
TDNFPyAddEnums(
    PyObject *pModule
    );

void
TDNFPyRaiseException(
    PyObject *self,
    uint32_t dwErrorCode
    );
