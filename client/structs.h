/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Header : structs.h
 *
 * Abstract :
 *
 *            tdnfclientlib
 *
 *            client library
 *
 * Authors  : Priyesh Padmavilasom (ppadmavilasom@vmware.com)
 */

#pragma once

typedef struct _TDNF_
{
    HySack hSack;
    HyGoal hGoal;
    PTDNF_CMD_ARGS pArgs;
    PTDNF_CONF pConf;
    PTDNF_REPO_DATA pRepos;
}TDNF;

typedef struct _TDNF_RPM_TS_
{
  rpmts              pTS;
  rpmKeyring         pKeyring;
  rpmtransFlags      nTransFlags;
  rpmprobFilterFlags nProbFilterFlags;
  FD_t               pFD;
  GArray*            pCachedRpmsArray;
}TDNFRPMTS, *PTDNFRPMTS;

typedef struct _TDNF_ENV_
{
    pthread_mutex_t mutexInitialize;
    int nInitialized;
}TDNF_ENV, *PTDNF_ENV;
