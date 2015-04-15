/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Header : defines.h
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

typedef
uint32_t
(*TDNFQueryTermsFunction)(
    HyPackageList,
    HyQuery,
    const char*);

typedef enum
{
    DETAIL_LIST,
    DETAIL_INFO
}TDNF_PKG_DETAIL;


#define IsNullOrEmptyString(str) (!(str) || !(*str))

#define BAIL_ON_TDNF_ERROR(dwError) \
    if (dwError)                                                   \
    {                                                              \
        goto error;                                                \
    }

#define BAIL_ON_TDNF_SYSTEM_ERROR(dwError) \
    if (dwError)                                                   \
    {                                                              \
        dwError = ERROR_TDNF_SYSTEM_BASE + dwError;                \
        goto error;                                                \
    }

#define BAIL_ON_TDNF_HAWKEY_ERROR(dwError) \
    if (dwError)                                                   \
    {                                                              \
        dwError = ERROR_TDNF_HAWKEY_BASE + dwError;                \
        goto error;                                                \
    }

#define BAIL_ON_TDNF_RPM_ERROR(dwError) \
    if (dwError)                                                   \
    {                                                              \
        dwError = ERROR_TDNF_RPM_BASE + dwError;                   \
        goto error;                                                \
    }

#define TDNF_SAFE_FREE_MEMORY(pMemory) \
    if (pMemory) { \
    TDNFFreeMemory(pMemory); \
    }

#define TDNF_SAFE_FREE_PKGLIST(hPkgList) \
    if (hPkgList) { \
    hy_packagelist_free(hPkgList); \
    }

#define TDNF_SAFE_FREE_STRINGARRAY(ppArray) \
    if (ppArray) { \
    TDNFFreeStringArray(ppArray); \
    }

#define TDNF_SAFE_FREE_PKGINFO(pPkgInfo) \
    if (pPkgInfo) { \
    TDNFFreePackageInfo(pPkgInfo); \
    }
//Misc
#define TDNF_RPM_EXT                      ".rpm"

//Repo defines
#define TDNF_REPO_EXT                     ".repo"
#define TDNF_CONF_FILE                    "/etc/tdnf/tdnf.conf"
#define TDNF_CONF_GROUP                   "main"
//Conf file key names
#define TDNF_CONF_KEY_GPGCHECK            "gpgcheck"
#define TDNF_CONF_KEY_INSTALLONLY_LIMIT   "installonly_limit"
#define TDNF_CONF_KEY_CLEAN_REQ_ON_REMOVE "clean_requirements_on_remove"
#define TDNF_CONF_KEY_REPODIR             "repodir"
#define TDNF_CONF_KEY_CACHEDIR            "cachedir"
//Repo file key names
#define TDNF_REPO_KEY_BASEURL             "baseurl"
#define TDNF_REPO_KEY_ENABLED             "enabled"
#define TDNF_REPO_KEY_METALINK            "metalink"
#define TDNF_REPO_KEY_NAME                "name"
#define TDNF_REPO_KEY_SKIP                "skip_if_unavailable"
#define TDNF_REPO_KEY_GPGCHECK            "gpgcheck"
#define TDNF_REPO_KEY_GPGKEY              "gpgkey"
#define TDNF_REPO_KEY_USERNAME            "username"
#define TDNF_REPO_KEY_PASSWORD            "password"

//Repo defaults
#define TDNF_DEFAULT_REPO_LOCATION        "/etc/yum.repos.d"
#define TDNF_DEFAULT_CACHE_LOCATION       "/var/cache/tdnf"
#define TDNF_RPM_CACHE_DIR_NAME           "rpms"
#define TDNF_REPODATA_DIR_NAME            "repodata"

#define TDNF_UNKNOWN_ERROR_STRING "Unknown error"
#define TDNF_ERROR_TABLE \
{ \
    {ERROR_TDNF_BASE,                "ERROR_TDNF_EBASE",               "Generic base error"}, \
    {ERROR_TDNF_PACKAGE_REQUIRED,    "ERROR_TDNF_PACKAGE_REQUIRED",    "Package name expected but was not provided"}, \
    {ERROR_TDNF_CONF_FILE_LOAD,      "ERROR_TDNF_CONF_FILE_LOAD",      "Error loading tdnf conf (/etc/tdnf/tdnf.conf)"}, \
    {ERROR_TDNF_REPO_FILE_LOAD,      "ERROR_TDNF_REPO_FILE_LOAD",      "Error loading tdnf repo (normally under /etc/yum.repos.d/)"}, \
    {ERROR_TDNF_INVALID_REPO_FILE,   "ERROR_TDNF_INVALID_REPO_FILE",   "Encountered an invalid repo file"}, \
    {ERROR_TDNF_REPO_DIR_OPEN,       "ERROR_TDNF_REPO_DIR_OPEN",       "Error opening repo dir. Check if the repodir configured in tdnf.conf exists (usually /etc/yum.repos.d)"}, \
    {ERROR_TDNF_NO_MATCH,            "ERROR_TDNF_NO_MATCH",            "No matching packages to list"}, \
    {ERROR_TDNF_NO_ENABLED_REPOS,    "ERROR_TDNF_NO_ENABLED_REPOS",    "There are no enabled repos.\n Run ""tdnf repolist all"" to see the repos you have.\n You can enable repos by editing repo files in your repodir(usually /etc/yum.repos.d)"}, \
    {ERROR_TDNF_PACKAGELIST_EMPTY,   "ERROR_TDNF_PACKAGELIST_EMPTY",   "Packagelist was empty"}, \
    {ERROR_TDNF_GOAL_CREATE,         "ERROR_TDNF_GOAL_CREATE",         "Error creating goal"}, \
    {ERROR_TDNF_INVALID_RESOLVE_ARG, "ERROR_TDNF_INVALID_RESOLVE_ARG", "Invalid argument in resolve"}, \
    {ERROR_TDNF_CLEAN_UNSUPPORTED,   "ERROR_TDNF_CLEAN_UNSUPPORTED",   "Clean type specified is not supported in this release. Please try clean all."}, \
    {ERROR_TDNF_HAWKEY_BASE,         "ERROR_TDNF_HAWKEY_BASE",         "Hawkey base error"}, \
    {ERROR_TDNF_HAWKEY_FAILED,       "ERROR_TDNF_HAWKEY_FAILED",       "Hawkey general runtime error"}, \
    {ERROR_TDNF_HAWKEY_OP,           "ERROR_TDNF_HAWKEY_OP",           "Hawkey client programming error"}, \
    {ERROR_TDNF_HAWKEY_LIBSOLV,      "ERROR_TDNF_HAWKEY_LIBSOLV",      "Hawkey error propagted from libsolv"}, \
    {ERROR_TDNF_HAWKEY_IO,           "ERROR_TDNF_HAWKEY_IO",           "Hawkey - I/O error"}, \
    {ERROR_TDNF_HAWKEY_CACHE_WRITE,  "ERROR_TDNF_HAWKEY_CACHE_WRITE",  "Hawkey - cache write error"}, \
    {ERROR_TDNF_HAWKEY_QUERY,        "ERROR_TDNF_HAWKEY_QUERY",        "Hawkey - ill formed query"}, \
    {ERROR_TDNF_HAWKEY_ARCH,         "ERROR_TDNF_HAWKEY_ARCH",         "Hawkey - unknown arch"}, \
    {ERROR_TDNF_HAWKEY_VALIDATION,   "ERROR_TDNF_HAWKEY_VALIDATION",   "Hawkey - validation check failed"}, \
    {ERROR_TDNF_HAWKEY_NO_SOLUTION,  "ERROR_TDNF_HAWKEY_NO_SOLUTION",  "Hawkey - goal found no solutions"}, \
    {ERROR_TDNF_HAWKEY_NO_CAPABILITY,"ERROR_TDNF_HAWKEY_NO_CAPABILITY","Hawkey - the capability was not available"}, \
    {ERROR_TDNF_REPO_BASE,           "ERROR_TDNF_REPO_BASE",           "Repo error base"}, \
    {ERROR_TDNF_REPO_PERFORM,        "ERROR_TDNF_REPO_PERFORM",        "Error during repo handle execution"}, \
    {ERROR_TDNF_REPO_GETINFO,        "ERROR_TDNF_REPO_GETINFO",        "Repo during repo result getinfo"}, \
    {ERROR_TDNF_TRANS_INCOMPLETE,    "ERROR_TDNF_TRANS_INCOMPLETE",    "Incomplete rpm transaction"}, \
    {ERROR_TDNF_TRANS_PKG_NOT_FOUND, "ERROR_TDNF_TRANS_PKG_NOT_FOUND", "Failed to find rpm package"}, \
    {ERROR_TDNF_NO_SEARCH_RESULTS,   "ERROR_TDNF_NO_SEARCH_RESULTS",   "No matches found"}, \
    {ERROR_TDNF_KEYURL_UNSUPPORTED,  "ERROR_TDNF_KEYURL_UNSUPPORTED",  "GpgKey Url schemes other than file are not supported"}, \
    {ERROR_TDNF_KEYURL_INVALID,      "ERROR_TDNF_KEYURL_INVALID",      "GpgKey Url is invalid"}, \
    {ERROR_TDNF_RPM_NOT_SIGNED,      "ERROR_TDNF_RPM_NOT_SIGNED",      "RPM not signed. Use --nogpgcheck to ignore."}, \
    {ERROR_TDNF_RPMTD_CREATE_FAILED, "ERROR_TDNF_RPMTD_CREATE_FAILED", "RPM data container could not be created. Use --nogpgcheck to ignore."}, \
    {ERROR_TDNF_RPM_GET_RSAHEADER_FAILED,"ERROR_TDNF_RPM_GET_RSAHEADER_FAILED","RPM not signed. Use --nogpgcheck to ignore."}, \
    {ERROR_TDNF_RPM_GPG_PARSE_FAILED,"ERROR_TDNF_RPM_GPG_PARSE_FAILED","RPM failed to parse gpg key. Use --nogpgcheck to ignore."}, \
    {ERROR_TDNF_RPM_GPG_NO_MATCH,   "ERROR_TDNF_RPM_GPG_NO_MATCH",     "RPM is signed but failed to match with known keys. Use --nogpgcheck to ignore."}, \
};
