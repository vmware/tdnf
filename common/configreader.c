/*
 * Copyright (C) 2015-2020 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

uint32_t TDNFConfSectionDefault(
    PCONF_DATA pData,
    const char *pszSection
    );

uint32_t TDNFConfKeyvalueDefault(
    PCONF_DATA pData,
    const char *psKey,
    const char *pszValue
    );

static PFN_CONF_SECTION_CB pfnConfSectionCB = TDNFConfSectionDefault;
static PFN_CONF_KEYVALUE_CB pfnConfKeyValueCB = TDNFConfKeyvalueDefault;

void
TDNFPrintConfigData(
    PCONF_DATA pData
    )
{
    PCONF_SECTION pSection = NULL;
    PKEYVALUE pKeyValue = NULL;
    if(!pData) return;

    pr_info("File: %s\n", pData->pszConfFile);

    pSection = pData->pSections;
    while(pSection)
    {
        pr_info("[%s]\n", pSection->pszName);
        pKeyValue = pSection->pKeyValues;
        while(pKeyValue)
        {
            pr_info("%s=%s\n", pKeyValue->pszKey, pKeyValue->pszValue);
            pKeyValue = pKeyValue->pNext;
        }
        pSection = pSection->pNext;
    }

}

uint32_t
TDNFGetSectionBoundaries(
    const char *pszLine,
    const char **ppszStart,
    const char **ppszEnd
    )
{
    uint32_t dwError = 0;
    const char *pszEnd = NULL;
    const char *pszStart = NULL;

    pszStart = strchr(pszLine, '[');
    if(!pszStart)
    {
        dwError = ENOENT;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pszEnd = strrchr(pszLine, ']');
    if(!pszEnd)
    {
        dwError = ENOENT;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(pszEnd < pszStart)
    {
        dwError = ENOENT;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppszStart = pszStart;
    *ppszEnd = pszEnd;

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFGetSection(
    const char *pszLine,
    char **ppszSection
    )
{
    uint32_t dwError = 0;
    char *pszSection = NULL;
    const char *pszStart = NULL;
    const char *pszEnd = NULL;

    dwError = TDNFGetSectionBoundaries(pszLine, &pszStart, &pszEnd);
    BAIL_ON_TDNF_ERROR(dwError);

    pszStart = TDNFLeftTrim(pszStart + 1);
    pszEnd = TDNFRightTrim(pszStart, pszEnd - 1);

    dwError = TDNFAllocateMemory(
                  pszEnd - pszStart + 2,
                  1,
                  (void**)&pszSection);
    BAIL_ON_TDNF_ERROR(dwError);
    memcpy(pszSection, pszStart, pszEnd - pszStart + 1);

    *ppszSection = pszSection;

cleanup:
    return dwError;

error:
    if(ppszSection)
    {
        *ppszSection = NULL;
    }
    goto cleanup;
}

uint32_t
TDNFIsSection(
    const char *pszLine,
    int *pnSection
    )
{
    uint32_t dwError = 0;
    const char *pszStart = NULL;
    const char *pszEnd = NULL;

    dwError = TDNFGetSectionBoundaries(pszLine, &pszStart, &pszEnd);
    BAIL_ON_TDNF_ERROR(dwError);

    *pnSection = 1;
cleanup:
    return dwError;

error:
    if(pnSection)
    {
        *pnSection = 0;
    }
    if(dwError == ENOENT)
    {
        dwError = 0;
    }
    goto cleanup;
}

void
TDNFFreeKeyValues(
    PKEYVALUE pKeyValue
    )
{
    if(!pKeyValue)
    {
        return;
    }
    while(pKeyValue)
    {
        PKEYVALUE pKeyValueTemp = pKeyValue->pNext;
        TDNF_SAFE_FREE_MEMORY(pKeyValue->pszKey);
        TDNF_SAFE_FREE_MEMORY(pKeyValue->pszValue);
        TDNF_SAFE_FREE_MEMORY(pKeyValue);
        pKeyValue = pKeyValueTemp;
    }
}

void
TdnfFreeConfigSections(
    PCONF_SECTION pSection
    )
{
    if(!pSection)
    {
        return;
    }
    while(pSection)
    {
        PCONF_SECTION pSectionTemp = pSection->pNext;
        TDNFFreeKeyValues(pSection->pKeyValues);
        TDNF_SAFE_FREE_MEMORY(pSection->pszName);
        TDNF_SAFE_FREE_MEMORY(pSection);
        pSection = pSectionTemp;
    }
}

void
TDNFFreeConfigData(
    PCONF_DATA pData
    )
{
    if(!pData)
    {
        return;
    }
    TdnfFreeConfigSections(pData->pSections);
    TDNF_SAFE_FREE_MEMORY(pData->pszConfFile);
    TDNF_SAFE_FREE_MEMORY(pData);
}

uint32_t
TDNFConfSectionDefault(
    PCONF_DATA pData,
    const char *pszSection
    )
{
    uint32_t dwError = 0;
    PCONF_SECTION pNewSection = NULL;
    PCONF_SECTION pSection = NULL;

    if(!pData || IsNullOrEmptyString(pszSection))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pSection = pData->pSections;
    while(pSection && pSection->pNext) pSection = pSection->pNext;

    dwError = TDNFAllocateMemory(1, sizeof(CONF_SECTION), (void **)&pNewSection);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateString(pszSection, &pNewSection->pszName);
    BAIL_ON_TDNF_ERROR(dwError);

    if(pSection)
    {
        pSection->pNext = pNewSection;
    }
    else
    {
        pData->pSections = pNewSection;
    }
    pNewSection = NULL;

cleanup:
    return dwError;

error:
    if(pNewSection)
    {
        TdnfFreeConfigSections(pNewSection);
    }
    goto cleanup;
}

uint32_t
TDNFConfKeyvalueDefault(
    PCONF_DATA pData,
    const char *pszKey,
    const char *pszValue
    )
{
    uint32_t dwError = 0;
    char *pszEq = NULL;
    PCONF_SECTION pSection = NULL;
    PKEYVALUE pNewKeyValue = NULL;
    PKEYVALUE pKeyValue = NULL;
    const char *pszTemp = NULL;
    const char *pszTempEnd = NULL;

    //Allow for empty values
    if(!pData || IsNullOrEmptyString(pszKey))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pszEq = strchr(pszKey, '=');
    if(!pszEq)
    {
        pr_err("keyvalue lines must be of format key=value\n");
        dwError = EDOM;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pSection = pData->pSections;
    for(;pSection && pSection->pNext; pSection = pSection->pNext);

    if(!pSection)
    {
        pr_err("conf file must start with a section");
        dwError = EINVAL;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pKeyValue = pSection->pKeyValues;
    for(;pKeyValue && pKeyValue->pNext; pKeyValue = pKeyValue->pNext);

    pszTemp = TDNFRightTrim(pszValue, pszEq);
    dwError = TDNFAllocateMemory(sizeof(KEYVALUE), 1, (void**)&pNewKeyValue);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateMemory(
                  pszTemp - pszValue + 1,
                  1,
                  (void**)&pNewKeyValue->pszKey);
    BAIL_ON_TDNF_ERROR(dwError);
    strncpy(pNewKeyValue->pszKey, pszValue, pszTemp - pszValue);

    pszTemp = TDNFLeftTrim(pszEq + 1);
    pszTempEnd = TDNFRightTrim(pszTemp, pszTemp + strlen(pszTemp) - 1);
    dwError = TDNFAllocateMemory(
                  pszTempEnd - pszTemp + 2,
                  1,
                  (void**)&pNewKeyValue->pszValue);
    BAIL_ON_TDNF_ERROR(dwError);
    strncpy(pNewKeyValue->pszValue, pszTemp, pszTempEnd - pszTemp + 1);

    if(pKeyValue)
    {
        pKeyValue->pNext = pNewKeyValue;
    }
    else
    {
        pSection->pKeyValues = pNewKeyValue;
    }
cleanup:
    return dwError;

error:
    TDNFFreeKeyValues(pNewKeyValue);
    goto cleanup;
}

uint32_t
TDNFProcessConfigLine(
    const char *pszLine,
    PCONF_DATA pData
    )
{
    uint32_t dwError = 0;
    int nSection = 0;

    if(IsNullOrEmptyString(pszLine) || !pData)
    {
        dwError = EINVAL;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFIsSection(pszLine, &nSection);
    BAIL_ON_TDNF_ERROR(dwError);

    if(nSection && pfnConfSectionCB)
    {
        char *pszSection = NULL;

        dwError = TDNFGetSection(pszLine, &pszSection);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = pfnConfSectionCB(pData, pszSection);
        BAIL_ON_TDNF_ERROR(dwError);
        TDNF_SAFE_FREE_MEMORY(pszSection);
    }
    else if(pfnConfKeyValueCB)
    {
        if(strchr(pszLine, '='))
        {
            dwError = pfnConfKeyValueCB(pData, pszLine, pszLine);
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFReadConfigFile(
    const char *pszFile,
    const int nLineLength,
    PCONF_DATA *ppData
    )
{
    uint32_t dwError = 0;
    FILE *fp = NULL;
    char *pszLine = NULL;
    PCONF_DATA pData = NULL;
    int nMaxLineLength = 0;

    if(IsNullOrEmptyString(pszFile) || !ppData)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    fp = fopen(pszFile, "r");
    if(!fp)
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(1, sizeof(CONF_DATA), (void **)&pData);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateString(pszFile, &pData->pszConfFile);
    BAIL_ON_TDNF_ERROR(dwError);

    nMaxLineLength = nLineLength > MAX_CONFIG_LINE_LENGTH ?
                  nLineLength : MAX_CONFIG_LINE_LENGTH;
    dwError = TDNFAllocateMemory(1, nMaxLineLength, (void **)&pszLine);
    BAIL_ON_TDNF_ERROR(dwError);

    if(fp)
    {
        while(fgets(pszLine, nMaxLineLength, fp) != NULL)
        {
            const char *pszTrimmedLine = TDNFLeftTrim(pszLine);

            //ignore empty lines, comments
            if(IsNullOrEmptyString(pszTrimmedLine) || *pszTrimmedLine == '#')
            {
                continue;
            }
            dwError = TDNFProcessConfigLine(pszTrimmedLine, pData);
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

    *ppData = pData;

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszLine);
    if(fp)
    {
        fclose(fp);
    }
    return dwError;

error:
    if(ppData)
    {
        *ppData = NULL;
    }
    TDNFFreeConfigData (pData);
    goto cleanup;
}

uint32_t
TDNFConfigGetSection(
    PCONF_DATA pData,
    const char *pszGroup,
    PCONF_SECTION *ppSection
    )
{
    uint32_t dwError = 0;
    PCONF_SECTION pSections = NULL;
    PCONF_SECTION pSection = NULL;

    if(!pData || IsNullOrEmptyString(pszGroup) || !ppSection)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pSections = pData->pSections;
    for(; pSections; pSections = pSections->pNext)
    {
        if(!strcmp(pszGroup, pSections->pszName))
        {
            pSection = pSections;
            break;
        }
    }

    if(!pSection)
    {
        dwError = ENOENT;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppSection = pSection;

cleanup:
    return dwError;

error:
    if(ppSection)
    {
        *ppSection = NULL;
    }
    goto cleanup;
}

uint32_t
TDNFReadKeyValueBoolean(
    PCONF_SECTION pSection,
    const char* pszKeyName,
    int nDefault,
    int* pnValue
    )
{
    uint32_t dwError = 0;
    char* pszValue = NULL;
    int nValue = 0;

    if(!pSection || !pszKeyName || !pnValue)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    dwError = TDNFReadKeyValue(
                  pSection,
                  pszKeyName,
                  NULL,
                  &pszValue);
    BAIL_ON_TDNF_ERROR(dwError);

    if(pszValue)
    {
        if(!strcmp(pszValue, "1") || !strcasecmp(pszValue, "true"))
        {
            nValue = 1;
        }
    }
    else
    {
        nValue = nDefault;
    }

    *pnValue = nValue;

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszValue);
    return dwError;

error:
    if(pnValue)
    {
        *pnValue = 0;
    }
    goto cleanup;
}

uint32_t
TDNFReadKeyValueInt(
    PCONF_SECTION pSection,
    const char* pszKeyName,
    int nDefault,
    int* pnValue
    )
{
    uint32_t dwError = 0;
    char* pszValue = NULL;
    int nValue = 0;

    if(!pSection || !pszKeyName || !pnValue)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    dwError = TDNFReadKeyValue(
                  pSection,
                  pszKeyName,
                  NULL,
                  &pszValue);
    BAIL_ON_TDNF_ERROR(dwError);

    if(pszValue)
    {

        nValue = atoi(pszValue);
    }
    else
    {
        nValue = nDefault;
    }

    *pnValue = nValue;

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszValue);
    return dwError;

error:
    if(pnValue)
    {
        *pnValue = 0;
    }
    goto cleanup;
}

uint32_t
TDNFReadKeyValueStringArray(
    PCONF_SECTION pSection,
    const char* pszKeyName,
    char*** pppszValueList
    )
{
    uint32_t dwError = 0;
    char** ppszValList = NULL;
    PKEYVALUE pKeyValues = NULL;

    if (!pSection || !pszKeyName || !pppszValueList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pKeyValues = pSection->pKeyValues;
    for (; pKeyValues; pKeyValues = pKeyValues->pNext)
    {
        if (strcmp(pszKeyName, pKeyValues->pszKey) == 0)
        {
            dwError = TDNFSplitStringToArray(pKeyValues->pszValue,
                                             " ", &ppszValList);
            BAIL_ON_TDNF_ERROR(dwError);
            *pppszValueList = ppszValList;
            break;
        }
    }

cleanup:
    return dwError;

error:
    if(pppszValueList)
    {
        *pppszValueList = NULL;
    }
    goto cleanup;
}

uint32_t
TDNFReadKeyValue(
    PCONF_SECTION pSection,
    const char* pszKeyName,
    const char* pszDefault,
    char** ppszValue
    )
{
    uint32_t dwError = 0;
    char* pszVal = NULL;
    char* pszValue = NULL;
    PKEYVALUE pKeyValues = NULL;

    if(!pSection || !pszKeyName || !ppszValue)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pKeyValues = pSection->pKeyValues;
    for(; pKeyValues; pKeyValues = pKeyValues->pNext)
    {
        if(strcmp(pszKeyName, pKeyValues->pszKey) == 0)
        {
            dwError = TDNFAllocateString(pKeyValues->pszValue, &pszVal);
            BAIL_ON_TDNF_ERROR(dwError);
            break;
        }
    }

    if(pszVal)
    {
        dwError = TDNFAllocateString(
                      pszVal,
                      &pszValue);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else if(pszDefault)
    {
        dwError = TDNFAllocateString(
                      pszDefault,
                      &pszValue);
        BAIL_ON_TDNF_ERROR(dwError);

    }

    *ppszValue = pszValue;

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszVal);
    return dwError;

error:
    if(ppszValue)
    {
        *ppszValue = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pszValue);
    goto cleanup;
}
