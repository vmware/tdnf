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
    do {                                                           \
        if (dwError)                                               \
        {                                                          \
            goto error;                                            \
        }                                                          \
    } while(0)

#define BAIL_ON_TDNF_SYSTEM_ERROR(dwError) \
    do {                                                           \
        if (dwError)                                               \
        {                                                          \
            dwError = ERROR_TDNF_SYSTEM_BASE + dwError;            \
            goto error;                                            \
        }                                                          \
    } while(0)

#define BAIL_ON_TDNF_HAWKEY_ERROR(dwError) \
    do {                                                           \
        if (dwError)                                               \
        {                                                          \
            dwError = ERROR_TDNF_HAWKEY_BASE + dwError;            \
            goto error;                                            \
        }                                                          \
    } while(0)

#define BAIL_ON_TDNF_RPM_ERROR(dwError) \
    do {                                                           \
        if (dwError)                                               \
        {                                                          \
            dwError = ERROR_TDNF_RPM_BASE + dwError;               \
            goto error;                                            \
        }                                                          \
    } while(0)

#define BAIL_ON_TDNF_CURL_ERROR(dwError) \
    do {                                                           \
        if (dwError)                                               \
        {                                                          \
            dwError = ERROR_TDNF_CURL_BASE + dwError;               \
            goto error;                                            \
        }                                                          \
    } while(0)

#define TDNF_SAFE_FREE_MEMORY(pMemory) \
    do {                                                           \
        if (pMemory) {                                             \
            TDNFFreeMemory(pMemory);                               \
        }                                                          \
    } while(0)

#define TDNF_SAFE_FREE_PKGLIST(hPkgList) \
    do {                                                           \
        if (hPkgList) {                                            \
            hy_packagelist_free(hPkgList);                         \
        }                                                          \
    } while(0)

#define TDNF_SAFE_FREE_STRINGARRAY(ppArray) \
    do {                                                           \
        if (ppArray) {                                             \
            TDNFFreeStringArray(ppArray);                          \
        }                                                          \
    } while(0)

#define TDNF_SAFE_FREE_PKGINFO(pPkgInfo) \
    do {                                                           \
        if (pPkgInfo) {                                            \
            TDNFFreePackageInfo(pPkgInfo);                         \
        }                                                          \
    } while(0)
//Misc
#define TDNF_RPM_EXT                      ".rpm"
#define TDNF_NAME                         "tdnf"
#define DIR_SEPARATOR                     '/'

//repomd type
#define TDNF_REPOMD_TYPE_PRIMARY          "primary"
#define TDNF_REPOMD_TYPE_FILELISTS        "filelists"
#define TDNF_REPOMD_TYPE_UPDATEINFO       "updateinfo"

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
#define TDNF_CONF_KEY_PROXY               "proxy"
#define TDNF_CONF_KEY_PROXY_USER          "proxy_username"
#define TDNF_CONF_KEY_PROXY_PASS          "proxy_password"
#define TDNF_CONF_KEY_KEEP_CACHE          "keepcache"
#define TDNF_CONF_KEY_DISTROVERPKG        "distroverpkg"
#define TDNF_CONF_KEY_DISTROARCHPKG       "distroarchpkg"
#define TDNF_CONF_KEY_MAX_STRING_LEN      "maxstringlen"
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
#define TDNF_REPO_KEY_METADATA_EXPIRE     "metadata_expire"

//file names
#define TDNF_REPO_METADATA_MARKER         "lastrefresh"
#define TDNF_REPO_METADATA_FILE_PATH      "repodata/repomd.xml"
#define TDNF_REPO_METADATA_FILE_NAME      "repomd.xml"

//Repo defaults
#define TDNF_DEFAULT_REPO_LOCATION        "/etc/yum.repos.d"
#define TDNF_DEFAULT_CACHE_LOCATION       "/var/cache/tdnf"
#define TDNF_DEFAULT_DISTROVERPKG         "photon-release"
#define TDNF_DEFAULT_DISTROARCHPKG        "x86_64"
#define TDNF_RPM_CACHE_DIR_NAME           "rpms"
#define TDNF_REPODATA_DIR_NAME            "repodata"
#define TDNF_REPO_DEFAULT_METADATA_EXPIRE "8294400"//48 hours in seconds
#define TDNF_REPO_METADATA_EXPIRE_NEVER   "never"
//var names
#define TDNF_VAR_RELEASEVER               "$releasever"
#define TDNF_VAR_BASEARCH                 "$basearch"

#define TDNF_UNKNOWN_ERROR_STRING "Unknown error"
#define TDNF_ERROR_TABLE \
{ \
    {ERROR_TDNF_BASE,                "ERROR_TDNF_EBASE",               "Generic base error"}, \
    {ERROR_TDNF_PACKAGE_REQUIRED,    "ERROR_TDNF_PACKAGE_REQUIRED",    "Package name expected but was not provided"}, \
    {ERROR_TDNF_CONF_FILE_LOAD,      "ERROR_TDNF_CONF_FILE_LOAD",      "Error loading tdnf conf (/etc/tdnf/tdnf.conf)"}, \
    {ERROR_TDNF_REPO_FILE_LOAD,      "ERROR_TDNF_REPO_FILE_LOAD",      "Error loading tdnf repo (normally under /etc/yum.repos.d/)"}, \
    {ERROR_TDNF_INVALID_REPO_FILE,   "ERROR_TDNF_INVALID_REPO_FILE",   "Encountered an invalid repo file"}, \
    {ERROR_TDNF_REPO_DIR_OPEN,       "ERROR_TDNF_REPO_DIR_OPEN",       "Error opening repo dir. Check if the repodir configured in tdnf.conf exists (usually /etc/yum.repos.d)"}, \
    {ERROR_TDNF_NO_MATCH,            "ERROR_TDNF_NO_MATCH",            "No matching packages"}, \
    {ERROR_TDNF_SET_PROXY,           "ERROR_TDNF_SET_PROXY",           "There was an error setting the proxy server."}, \
    {ERROR_TDNF_SET_PROXY_USERPASS,  "ERROR_TDNF_SET_PROXY_USERPASS",  "There was an error setting the proxy server user and pass"}, \
    {ERROR_TDNF_NO_DISTROVERPKG,     "ERROR_TDNF_NO_DISTROVERPKG",     "distroverpkg config entry is set to a package that is not installed. Check /etc/tdnf/tdnf.conf"}, \
    {ERROR_TDNF_DISTROVERPKG_READ,   "ERROR_TDNF_DISTROVERPKG_READ",   "There was an error reading version of distroverpkg"}, \
    {ERROR_TDNF_INVALID_ALLOCSIZE,   "ERROR_TDNF_INVALID_ALLOCSIZE",   "A memory allocation was requested with an invalid size"}, \
    {ERROR_TDNF_STRING_TOO_LONG,     "ERROR_TDNF_STRING_TOO_LONG",     "Requested string allocation size was too long."}, \
    {ERROR_TDNF_NO_ENABLED_REPOS,    "ERROR_TDNF_NO_ENABLED_REPOS",    "There are no enabled repos.\n Run ""tdnf repolist all"" to see the repos you have.\n You can enable repos by\n 1. by passing in --enablerepo <reponame>\n 2. editing repo files in your repodir(usually /etc/yum.repos.d)"}, \
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
    {ERROR_TDNF_RPMRC_NOTFOUND,      "ERROR_TDNF_RPMRC_NOTFOUND",      "rpm generic error - not found"}, \
    {ERROR_TDNF_RPMRC_FAIL,          "ERROR_TDNF_RPMRC_FAIL",          "rpm generic failure"}, \
    {ERROR_TDNF_RPMRC_NOTTRUSTED,    "ERROR_TDNF_RPMRC_NOTTRUSTED",    "rpm signature is OK, but key is not trusted"}, \
    {ERROR_TDNF_RPMRC_NOKEY,         "ERROR_TDNF_RPMRC_NOKEY",         "public key is unavailable. install public key using rpm --import or use --nogpgcheck to ignore."}, \
    {ERROR_TDNF_INVALID_PUBKEY_FILE, "ERROR_TDNF_INVALID_PUBKEY_FILE", "public key file is invalid or corrupted"}, \
    {ERROR_TDNF_KEYURL_UNSUPPORTED,  "ERROR_TDNF_KEYURL_UNSUPPORTED",  "GpgKey Url schemes other than file are not supported"}, \
    {ERROR_TDNF_KEYURL_INVALID,      "ERROR_TDNF_KEYURL_INVALID",      "GpgKey Url is invalid"}, \
    {ERROR_TDNF_RPM_NOT_SIGNED,      "ERROR_TDNF_RPM_NOT_SIGNED",      "RPM not signed. Use --nogpgcheck to ignore."}, \
    {ERROR_TDNF_RPMTD_CREATE_FAILED, "ERROR_TDNF_RPMTD_CREATE_FAILED", "RPM data container could not be created. Use --nogpgcheck to ignore."}, \
    {ERROR_TDNF_RPM_GET_RSAHEADER_FAILED,"ERROR_TDNF_RPM_GET_RSAHEADER_FAILED","RPM not signed. Use --nogpgcheck to ignore."}, \
    {ERROR_TDNF_RPM_GPG_PARSE_FAILED,"ERROR_TDNF_RPM_GPG_PARSE_FAILED","RPM failed to parse gpg key. Use --nogpgcheck to ignore."}, \
    {ERROR_TDNF_RPM_GPG_NO_MATCH,   "ERROR_TDNF_RPM_GPG_NO_MATCH",     "RPM is signed but failed to match with known keys. Use --nogpgcheck to ignore."}, \
    {ERROR_TDNF_AUTOERASE_UNSUPPORTED,"ERROR_TDNF_AUTOERASE_UNSUPPORTED","autoerase / autoremove is not supported."}, \
    {ERROR_TDNF_RPM_CHECK,           "ERROR_TDNF_RPM_CHECK",           "rpm check reported errors"}, \
    {ERROR_TDNF_METADATA_EXPIRE_PARSE, "ERROR_TDNF_METADATA_EXPIRE_PARSE", "metadata_expire value could not be parsed. Check your repo files."},\
    {ERROR_TDNF_SELF_ERASE, "ERROR_TDNF_SELF_ERASE", "The operation would result in removing the protected package : tdnf"},\
};
