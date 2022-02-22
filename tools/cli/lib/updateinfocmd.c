/*
 * Copyright (C) 2015-2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : updateinfocmd.c
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

#include "includes.h"


char*
TDNFGetUpdateInfoType(
    int nType
    )
{
    char* pszType = "Unknown";

    switch(nType)
    {
        case UPDATE_SECURITY:
            pszType = "Security";
            break;
        case UPDATE_BUGFIX:
            pszType = "Bugfix";
            break;
        case UPDATE_ENHANCEMENT:
            pszType = "Enhancement";
            break;
    }

    return pszType;
}

uint32_t
TDNFCliUpdateInfoCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;

    PTDNF_UPDATEINFO pUpdateInfo = NULL;
    PTDNF_UPDATEINFO_ARGS pInfoArgs = NULL;

    dwError = TDNFCliParseUpdateInfoArgs(pCmdArgs, &pInfoArgs);
    BAIL_ON_CLI_ERROR(dwError);

    if(pInfoArgs->nMode == OUTPUT_SUMMARY)
    {
        dwError = TDNFCliUpdateInfoSummary(pContext, pCmdArgs, pInfoArgs);
        BAIL_ON_CLI_ERROR(dwError);
    }
    else
    {
        dwError = pContext->pFnUpdateInfo(
                      pContext,
                      pInfoArgs,
                      &pUpdateInfo);
        if (dwError == ERROR_TDNF_NO_DATA)
        {
            dwError = 0;
        }
        BAIL_ON_CLI_ERROR(dwError);

        dwError = pCmdArgs->nJsonOutput ?
            TDNFCliUpdateInfoOutputJson(pUpdateInfo, pInfoArgs->nMode) :
            TDNFCliUpdateInfoOutput(pUpdateInfo, pInfoArgs->nMode);
        BAIL_ON_CLI_ERROR(dwError);
    }
cleanup:
    if(pInfoArgs)
    {
        TDNFCliFreeUpdateInfoArgs(pInfoArgs);
    }
    TDNFFreeUpdateInfo(pUpdateInfo);
    return dwError;

error:
    goto cleanup;
}


uint32_t
TDNFCliUpdateInfoSummary(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs,
    PTDNF_UPDATEINFO_ARGS pInfoArgs
    )
{
    uint32_t dwError = 0;
    int i = 0;
    PTDNF_UPDATEINFO_SUMMARY pSummary = NULL;
    int nCount = 0;

    if(!pContext || !pCmdArgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = pContext->pFnUpdateInfoSummary(
                  pContext,
                  AVAIL_AVAILABLE,
                  pInfoArgs,
                  &pSummary);
    BAIL_ON_CLI_ERROR(dwError);

    if (pCmdArgs->nJsonOutput)
    {
        struct json_dump *jd = jd_create(0);
        jd_map_start(jd);
        for(i = UPDATE_UNKNOWN; i <= UPDATE_ENHANCEMENT; ++i)
        {
            jd_map_add_int(jd, TDNFGetUpdateInfoType(pSummary[i].nType), pSummary[i].nCount);
        }
        pr_json(jd->buf);
        JD_SAFE_DESTROY(jd);
    }
    else
    {
        for(i = UPDATE_UNKNOWN; i <= UPDATE_ENHANCEMENT; i++)
        {
            if(pSummary[i].nCount > 0)
            {
                nCount++;
                pr_crit(
                    "%d %s notice(s)\n",
                    pSummary[i].nCount,
                    TDNFGetUpdateInfoType(pSummary[i].nType));
            }
        }
        if (nCount == 0)
        {
            pr_crit("\n%d updates.\n", nCount);
            dwError = ERROR_TDNF_NO_DATA;
            BAIL_ON_CLI_ERROR(dwError);
        }
    }

cleanup:
    if(pSummary)
    {
        TDNFFreeUpdateInfoSummary(pSummary);
    }
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliUpdateInfoOutput(
    PTDNF_UPDATEINFO pInfo,
    TDNF_UPDATEINFO_OUTPUT mode
    )
{
    uint32_t dwError = 0;
    PTDNF_UPDATEINFO_PKG pPkg = NULL;

    for(; pInfo; pInfo = pInfo->pNext)
    {
        for(pPkg = pInfo->pPackages; pPkg; pPkg = pPkg->pNext)
        {
            if (mode == OUTPUT_INFO)
            {
                pr_crit("       Name : %s\n"
                        "  Update ID : %s\n"
                        "       Type : %s\n"
                        "    Updated : %s\n"
                        "Needs Reboot: %d\n"
                        "Description : %s\n",
                            pPkg->pszFileName,
                            pInfo->pszID,
                            TDNFGetUpdateInfoType(pInfo->nType),
                            pInfo->pszDate,
                            pInfo->nRebootRequired,
                            pInfo->pszDescription);
            }
            else if (mode == OUTPUT_LIST)
            {
                pr_crit("%s %s %s\n", pInfo->pszID,
                        TDNFGetUpdateInfoType(pInfo->nType),
                        pPkg->pszFileName);
            }
        }
    }
    return dwError;
}

uint32_t
TDNFCliUpdateInfoOutputJson(
    PTDNF_UPDATEINFO pInfo,
    TDNF_UPDATEINFO_OUTPUT mode
    )
{
    uint32_t dwError = 0;
    PTDNF_UPDATEINFO_PKG pPkg = NULL;
    struct json_dump *jd = jd_create(0);
    struct json_dump *jd_info = NULL;
    struct json_dump *jd_pkgs = NULL;

    CHECK_JD_NULL(jd);
    CHECK_JD_RC(jd_list_start(jd));

    for(; pInfo; pInfo = pInfo->pNext)
    {
        jd_info = jd_create(0);
        CHECK_JD_NULL(jd_info);
        jd_pkgs = jd_create(0);
        CHECK_JD_NULL(jd_pkgs);

        CHECK_JD_RC(jd_map_start(jd_info));

        CHECK_JD_RC(jd_map_add_string(jd_info, "Type", TDNFGetUpdateInfoType(pInfo->nType)));
        CHECK_JD_RC(jd_map_add_string(jd_info, "UpdateID", pInfo->pszID));
        if (mode == OUTPUT_INFO)
        {
            CHECK_JD_RC(jd_map_add_string(jd_info, "Updated", pInfo->pszDate));
            CHECK_JD_RC(jd_map_add_bool(jd_info, "NeedsReboot", pInfo->nRebootRequired));
            CHECK_JD_RC(jd_map_add_string(jd_info, "Description", pInfo->pszDescription));
        }
        CHECK_JD_RC(jd_list_start(jd_pkgs));

        for(pPkg = pInfo->pPackages; pPkg; pPkg = pPkg->pNext)
        {
            CHECK_JD_RC(jd_list_add_string(jd_pkgs, pPkg->pszFileName));
        }
        CHECK_JD_RC(jd_map_add_child(jd_info, "Packages", jd_pkgs));
        JD_SAFE_DESTROY(jd_pkgs);
        CHECK_JD_RC(jd_list_add_child(jd, jd_info));
        JD_SAFE_DESTROY(jd_info);
    }
    pr_json(jd->buf);
    JD_SAFE_DESTROY(jd);

error:
    JD_SAFE_DESTROY(jd);
    JD_SAFE_DESTROY(jd_info);
    JD_SAFE_DESTROY(jd_pkgs);
    return dwError;
}
