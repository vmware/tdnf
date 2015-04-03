/*
      * Copyright (C) 2014-2015 VMware, Inc. All rights reserved.
      *
      * Header : structs.h
      *
      * Abstract :
      *
      *            tdnfclientlib
      *
      *            client library
      *
      * Authors  : Priyesh Padmavilasom (ppadmavilasom@vmware.com)
      *
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
}TDNFRPMTS, *PTDNFRPMTS;
