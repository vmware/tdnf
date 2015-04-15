/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Header : structs.h
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

#pragma once

#ifdef __cplusplus
extern "C" {
#endif


//Map command name to client function
typedef struct _TDNF_CLI_CMD_MAP
{
    char* pszCmdName;
    PFN_CMD pFnCmd;
}TDNF_CLI_CMD_MAP, *PTDNF_CLI_CMD_MAP;

typedef struct _TDNF_LIST_ARGS
{
    TDNF_SCOPE nScope;
    char** ppszPackageNameSpecs;
}TDNF_LIST_ARGS, *PTDNF_LIST_ARGS;

typedef struct _TDNF_UPDATEINFO_ARGS
{
    TDNF_UPDATEINFO_OUTPUT nMode;
    TDNF_SCOPE nScope;
    TDNF_UPDATEINFO_TYPE nType;
    char** ppszPackageNameSpecs;
}TDNF_UPDATEINFO_ARGS, *PTDNF_UPDATEINFO_ARGS;


#ifdef __cplusplus
}
#endif
