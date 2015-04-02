/*
      * Copyright (C) 2014-2015 VMware, Inc. All rights reserved.
      *
      * Module   : checklocal.c
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
#include "includes.h"

uint32_t
TDNFCheckLocalPackages(
    PTDNF pTdnf,
    const char* pszLocalPath
    )
{
    uint32_t dwError = 0;
    int i = 0;
    char* pszRPMPath = NULL;
    const char* pszFile = NULL;
    GDir* pDir = NULL;
    HySack hSack = NULL;
    HyPackage hPkg = NULL;
    HyGoal hGoal = NULL;
    HyPackageList hPkgList = NULL;

    if(!pTdnf || !pszLocalPath)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pDir = g_dir_open(pszLocalPath, 0, NULL);
    if(!pDir)
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }
    fprintf(stdout, "Checking all packages from: %s\n", pszLocalPath);

    hSack = hy_sack_create(NULL, NULL, NULL, 0);
    if(!hSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    hy_sack_create_cmdline_repo(hSack);
    hPkgList = hy_packagelist_create();
    if(!hPkgList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    while ((pszFile = g_dir_read_name (pDir)) != NULL)
    {
        if (!g_str_has_suffix (pszFile, TDNF_RPM_EXT))
        {
            continue;
        }
        pszRPMPath = g_build_filename(pszLocalPath, pszFile, NULL);
        hPkg = hy_sack_add_cmdline_package(hSack, pszRPMPath);

        g_free(pszRPMPath);
        pszRPMPath = NULL;

        if(!hPkg)
        {
            dwError = ERROR_TDNF_INVALID_PARAMETER; 
            BAIL_ON_TDNF_ERROR(dwError);
        }
        hy_packagelist_push(hPkgList, hPkg);
        hPkg = NULL;
    }

    fprintf(stdout, "Found %d packages\n", hy_packagelist_count(hPkgList));

    hGoal = hy_goal_create(hSack);
    if(!hGoal)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    FOR_PACKAGELIST(hPkg, hPkgList, i)
    {
        dwError = hy_goal_install(hGoal, hPkg);
        BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    }
    
    dwError = hy_goal_run_flags(hGoal, HY_ALLOW_UNINSTALL);
    if(dwError)
    {
        TDNFGoalReportProblems(hGoal);
        BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    }

cleanup:
    if(pDir)
    {
        g_dir_close(pDir);
    }
    if(pszRPMPath)
    {
        g_free(pszRPMPath);
    }
    if(hGoal)
    {
        hy_goal_free(hGoal);
    } 
    if(hPkgList)
    {
        hy_packagelist_free(hPkgList);
    } 
    if(hSack)
    {
        hy_sack_free(hSack);
    } 
    return dwError;

error:
    goto cleanup;
}
