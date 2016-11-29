#pragma once

typedef struct _KEYVALUE_
{
    char *pszKey;
    char *pszValue;
    struct _KEYVALUE_ *pNext;
}KEYVALUE, *PKEYVALUE;

typedef struct _CONF_SECTION_
{
    char *pszName;
    PKEYVALUE pKeyValues;
    struct _CONF_SECTION_ *pNext;
}CONF_SECTION, *PCONF_SECTION;

typedef struct _CONF_DATA_
{
    char *pszConfFile;
    PCONF_SECTION pSections;
}CONF_DATA, *PCONF_DATA;

typedef uint32_t
(*PFN_CONF_SECTION_CB)(
    PCONF_DATA pData,
    const char *pszSection
    );

typedef uint32_t
(*PFN_CONF_KEYVALUE_CB)(
    PCONF_DATA pData,
    const char *pszKey,
    const char *pszValue
    );
