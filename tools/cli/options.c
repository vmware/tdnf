/*
      * Copyright (C) 2014-2015 VMware, Inc. All rights reserved.
      *
      * Module   : options.c
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

static
uint32_t
_TDNFCliGetOptionByName(
    const char* pszName,
    struct option* pKnownOptions,
    struct option** ppOption
    );


uint32_t
TDNFCliValidateOptionName(
    const char* pszName,
    struct option* pKnownOptions
    )
{
    uint32_t dwError = 0;
    struct option* pOption = NULL;

    if(IsNullOrEmptyString(pszName) || !pKnownOptions)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = _TDNFCliGetOptionByName(pszName, pKnownOptions, &pOption);
    BAIL_ON_CLI_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliValidateOptionArg(
    const char* pszName,
    const char* pszArg,
    struct option* pKnownOptions
    )
{
    uint32_t dwError = 0;
    struct option* pOption = NULL;

    if(IsNullOrEmptyString(pszName) || !pKnownOptions)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = _TDNFCliGetOptionByName(pszName, pKnownOptions, &pOption);
    BAIL_ON_CLI_ERROR(dwError);

    if(IsNullOrEmptyString(pszArg) && pOption->has_arg == required_argument)
    {
        dwError = ERROR_TDNF_CLI_OPTION_ARG_REQUIRED;
    }

    if(!IsNullOrEmptyString(pszArg) && pOption->has_arg == no_argument)
    {
        dwError = ERROR_TDNF_CLI_OPTION_ARG_UNEXPECTED;
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliValidateOptions(
    const char* pszName,
    const char* pszArg,
    struct option* pKnownOptions
    )
{
    uint32_t dwError = 0;

    //pszArg can be NULL
    if(IsNullOrEmptyString(pszName) ||
       !pKnownOptions)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = TDNFCliValidateOptionName(pszName, pKnownOptions);
    BAIL_ON_CLI_ERROR(dwError);

    dwError = TDNFCliValidateOptionArg(pszName, pszArg, pKnownOptions);
    BAIL_ON_CLI_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
_TDNFCliGetOptionByName(
    const char* pszName,
    struct option* pKnownOptions,
    struct option** ppOption
    )
{
    uint32_t dwError = 0;
    struct option* pOption = NULL;

    if(IsNullOrEmptyString(pszName) || !pKnownOptions || !ppOption)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    while(pKnownOptions->name)
    {
        if(!strcmp(pszName, pKnownOptions->name))
        {
            pOption = pKnownOptions;
            break;
        } 
        ++pKnownOptions;
    }
    if(!pOption)
    {
        dwError = ERROR_TDNF_CLI_OPTION_NAME_INVALID;
        BAIL_ON_CLI_ERROR(dwError);
    }

    *ppOption = pOption;

cleanup:
    return dwError;

error:
    if(ppOption)
    {
        *ppOption = NULL;
    }
    goto cleanup;
}
