/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Header   : prototypes.h
 *
 * Abstract :
 *
 *            commonlib 
 *
 *            common library
 *
 * Authors  : Priyesh Padmavilasom (ppadmavilasom@vmware.com)
 */

//memory.c
uint32_t
TDNFAllocateMemory(
    size_t nNumElements,
    size_t nSize,
    void** ppMemory
    );

uint32_t
TDNFAllocateString(
    const char* pszSrc,
    char** ppszDst
    );

void
TDNFFreeMemory(
    void* pMemory
    );

//strings.c
uint32_t
TDNFAllocateString(
    const char* pszSrc,
    char** ppszDst
    );

uint32_t
TDNFSafeAllocateString(
    const char* pszSrc,
    char** ppszDst
    );

uint32_t
TDNFAllocateStringPrintf(
    char** ppszDst,
    const char* pszFmt,
    ...
    );

uint32_t
TDNFAllocateStringN(
    const char* pszSrc,
    uint32_t dwNumElements,
    char** ppszDst
    );

uint32_t
TDNFReplaceString(
    const char* pszSource,
    const char* pszSearch,
    const char* pszReplace,
    char** ppszDst
    );

void
TDNFFreeStringArray(
    char** ppszArray
    );

void
TDNFFreeStringArrayWithCount(
    char **ppszArray,
    int nCount
    );

//configreader.c
void
print_config_data(
    PCONF_DATA pData
    );

uint32_t
read_config_file_custom(
    const char *pszFile,
    const int nMaxLineLength,
    PFN_CONF_SECTION_CB pfnSectionCB,
    PFN_CONF_KEYVALUE_CB pfnKeyValueCB,
    PCONF_DATA *ppData
    );

uint32_t
read_config_file(
    const char *pszFile,
    const int nMaxLineLength,
    PCONF_DATA *ppData
    );

uint32_t
config_get_section(
    PCONF_DATA pData,
    const char *pszGroup,
    PCONF_SECTION *ppSection
    );

void
free_config_data(
    PCONF_DATA pData
    );

//utils.c
uint32_t
dup_argv(
    int argc,
    char* const* argv,
    char*** argvDup
    );

uint32_t
PMDUtilsFormatSize(
    uint32_t unSize,
    char** ppszFormattedSize
    );

uint32_t
file_read_all_text(
    const char *pszFileName,
    char **ppszText
    );

const char *
ltrim(
    const char *pszStr
    );

const char *
rtrim(
    const char *pszStart,
    const char *pszEnd
    );

uint32_t
count_matches(
    const char *pszString,
    const char *pszFind,
    int *pnCount
    );

uint32_t
string_replace(
    const char *pszString,
    const char *pszFind,
    const char *pszReplace,
    char **ppszResult
    );

uint32_t
make_array_from_string(
    const char *pszString,
    const char *pszSeparator,
    char ***pppszArray,
    int *pnCount
    );
