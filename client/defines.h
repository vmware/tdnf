/*
 * Copyright (C) 2015-2023 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#pragma once

#include "config.h"

typedef enum
{
    /* this should be a bitmask */
    DETAIL_LIST,
    DETAIL_INFO,
    DETAIL_CHANGELOG,
    DETAIL_SOURCEPKG
}TDNF_PKG_DETAIL;

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
            dwError = ERROR_TDNF_CURL_BASE + dwError;              \
            goto error;                                            \
        }                                                          \
    } while(0)

#define STR_IS_TRUE(s) ((s) && (!strcmp((s), "1") || !strcasecmp((s), "true")))

//Misc
#define TDNF_RPM_EXT                      ".rpm"
#define TDNF_NAME                         "tdnf"
#define DIR_SEPARATOR                     '/'
#define SOLV_PATCH_MARKER                 "patch:"

//repomd type
#define TDNF_REPOMD_TYPE_PRIMARY          "primary"
#define TDNF_REPOMD_TYPE_FILELISTS        "filelists"
#define TDNF_REPOMD_TYPE_UPDATEINFO       "updateinfo"
#define TDNF_REPOMD_TYPE_OTHER            "other"

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
#define TDNF_CONF_KEY_PERSISTDIR          "persistdir"
#define TDNF_CONF_KEY_PROXY               "proxy"
#define TDNF_CONF_KEY_PROXY_USER          "proxy_username"
#define TDNF_CONF_KEY_PROXY_PASS          "proxy_password"
#define TDNF_CONF_KEY_KEEP_CACHE          "keepcache"
#define TDNF_CONF_KEY_DISTROVERPKG        "distroverpkg"
#define TDNF_CONF_KEY_DISTROARCHPKG       "distroarchpkg"
#define TDNF_CONF_KEY_MAX_STRING_LEN      "maxstringlen"
#define TDNF_CONF_KEY_PLUGINS             "plugins"
#define TDNF_CONF_KEY_NO_PLUGINS          "noplugins"
#define TDNF_CONF_KEY_PLUGIN_PATH         "pluginpath"
#define TDNF_CONF_KEY_PLUGIN_CONF_PATH    "pluginconfpath"
#define TDNF_PLUGIN_CONF_KEY_ENABLED      "enabled"
#define TDNF_CONF_KEY_EXCLUDE             "excludepkgs"
#define TDNF_CONF_KEY_MINVERSIONS         "minversions"
#define TDNF_CONF_KEY_OPENMAX             "openmax"
#define TDNF_CONF_KEY_CHECK_UPDATE_COMPAT "dnf_check_update_compat"
#define TDNF_CONF_KEY_DISTROSYNC_REINSTALL_CHANGED "distrosync_reinstall_changed"

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
#define TDNF_REPO_KEY_PRIORITY            "priority"
#define TDNF_REPO_KEY_METADATA_EXPIRE     "metadata_expire"
#define TDNF_REPO_KEY_TIMEOUT             "timeout"
#define TDNF_REPO_KEY_RETRIES             "retries"
#define TDNF_REPO_KEY_MINRATE             "minrate"
#define TDNF_REPO_KEY_THROTTLE            "throttle"
#define TDNF_REPO_KEY_SSL_VERIFY          "sslverify"
#define TDNF_REPO_KEY_SSL_CA_CERT         "sslcacert"
#define TDNF_REPO_KEY_SSL_CLI_CERT        "sslclientcert"
#define TDNF_REPO_KEY_SSL_CLI_KEY         "sslclientkey"
#define TDNF_REPO_KEY_SKIP_MD_FILELISTS   "skip_md_filelists"
#define TDNF_REPO_KEY_SKIP_MD_UPDATEINFO  "skip_md_updateinfo"
#define TDNF_REPO_KEY_SKIP_MD_OTHER       "skip_md_other"

//setopt keys
#define TDNF_SETOPT_KEY_REPOSDIR          "reposdir"

//file names
#define TDNF_REPO_METADATA_MARKER         "lastrefresh"
#define TDNF_REPO_METADATA_FILE_PATH      "repodata/repomd.xml"
#define TDNF_REPO_METADATA_FILE_NAME      "repomd.xml"
#define TDNF_REPO_METALINK_FILE_NAME      "metalink"
#define TDNF_REPO_BASEURL_FILE_NAME       "baseurl"

#define TDNF_AUTOINSTALLED_FILE           "autoinstalled"
#define TDNF_HISTORY_DB_FILE              "history.db"

// repo defaults
#define TDNF_DEFAULT_REPO_LOCATION        "/etc/yum.repos.d"
#define TDNF_DEFAULT_CACHE_LOCATION       "/var/cache/tdnf"

/* pszPersistDir - default is configurable at build time,
   and configurable with "persistdir" at run time */
#define TDNF_DEFAULT_DB_LOCATION          HISTORY_DB_DIR

#define TDNF_DEFAULT_DISTROVERPKG         "system-release"
#define TDNF_DEFAULT_DISTROARCHPKG        "x86_64"
#define TDNF_RPM_CACHE_DIR_NAME           "rpms"
#define TDNF_REPODATA_DIR_NAME            "repodata"
#define TDNF_SOLVCACHE_DIR_NAME           "solvcache"
#define TDNF_REPO_METADATA_EXPIRE_NEVER   "never"

#define TDNF_DEFAULT_OPENMAX              1024

// repo default settings
#define TDNF_REPO_DEFAULT_ENABLED            0
#define TDNF_REPO_DEFAULT_SKIP               0
#define TDNF_REPO_DEFAULT_GPGCHECK           1
#define TDNF_REPO_DEFAULT_MINRATE            0
#define TDNF_REPO_DEFAULT_THROTTLE           0
#define TDNF_REPO_DEFAULT_TIMEOUT            0
#define TDNF_REPO_DEFAULT_SSLVERIFY          1
#define TDNF_REPO_DEFAULT_RETRIES            10
#define TDNF_REPO_DEFAULT_PRIORITY           50
#define TDNF_REPO_DEFAULT_METADATA_EXPIRE    172800 // 48 hours in seconds
#define TDNF_REPO_DEFAULT_METADATA_EXPIRE_STR STRINGIFYX(TDNF_REPO_DEFAULT_METADATA_EXPIRE)
#define TDNF_REPO_DEFAULT_SKIP_MD_FILELISTS  0
#define TDNF_REPO_DEFAULT_SKIP_MD_UPDATEINFO 0
#define TDNF_REPO_DEFAULT_SKIP_MD_OTHER      0

// var names
#define TDNF_VAR_RELEASEVER               "$releasever"
#define TDNF_VAR_BASEARCH                 "$basearch"
/* dummy setopt values */
#define TDNF_SETOPT_NAME_DUMMY             "opt.dummy.name"
#define TDNF_SETOPT_VALUE_DUMMY            "opt.dummy.value"
/* plugin defines */
#define TDNF_DEFAULT_PLUGINS_ENABLED      0
#define TDNF_DEFAULT_PLUGIN_PATH          SYSTEM_LIBDIR"/tdnf-plugins"
#define TDNF_DEFAULT_PLUGIN_CONF_PATH     "/etc/tdnf/pluginconf.d"
#define TDNF_PLUGIN_CONF_EXT              ".conf"
#define TDNF_PLUGIN_CONF_EXT_LEN          5
#define TDNF_PLUGIN_CONF_MAIN_SECTION     "main"

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
    {ERROR_TDNF_SOLV_BASE,           "ERROR_TDNF_SOLV_BASE",           "Solv base error"}, \
    {ERROR_TDNF_SOLV_FAILED,         "ERROR_TDNF_SOLV_FAILED",         "Solv general runtime error"}, \
    {ERROR_TDNF_SOLV_OP,             "ERROR_TDNF_SOLV_OP",             "Solv client programming error"}, \
    {ERROR_TDNF_SOLV_LIBSOLV,        "ERROR_TDNF_SOLV_LIBSOLV",        "Solv error propagted from libsolv"}, \
    {ERROR_TDNF_SOLV_IO,             "ERROR_TDNF_SOLV_IO",             "Solv - I/O error"}, \
    {ERROR_TDNF_SOLV_CACHE_WRITE,    "ERROR_TDNF_SOLV_CACHE_WRITE",    "Solv - cache write error"}, \
    {ERROR_TDNF_SOLV_QUERY,          "ERROR_TDNF_SOLV_QUERY",          "Solv - ill formed query"}, \
    {ERROR_TDNF_SOLV_ARCH,           "ERROR_TDNF_SOLV_ARCH",           "Solv - unknown arch"}, \
    {ERROR_TDNF_SOLV_VALIDATION,     "ERROR_TDNF_SOLV_VALIDATION",     "Solv - validation check failed"}, \
    {ERROR_TDNF_SOLV_NO_SOLUTION,    "ERROR_TDNF_SOLV_NO_SOLUTION",    "Solv - goal found no solutions"}, \
    {ERROR_TDNF_SOLV_NO_CAPABILITY,  "ERROR_TDNF_SOLV_NO_CAPABILITY",  "Solv - the capability was not available"}, \
    {ERROR_TDNF_SOLV_CHKSUM,         "ERROR_TDNF_SOLV_CHKSUM",         "Solv - Checksum creation failed"}, \
    {ERROR_TDNF_REPO_WRITE,          "ERROR_TDNF_REPO_WRITE",          "Solv - Failed to write repo"}, \
    {ERROR_TDNF_SOLV_CACHE_NOT_CREATED, "ERROR_TDNF_SOLV_CACHE_NOT_CREATED", "Solv - Solv cache not found"}, \
    {ERROR_TDNF_ADD_SOLV,            "ERROR_TDNF_ADD_SOLV",            "Solv - Failed to add solv"}, \
    {ERROR_TDNF_REPO_BASE,           "ERROR_TDNF_REPO_BASE",           "Repo error base"}, \
    {ERROR_TDNF_SET_SSL_SETTINGS,    "ERROR_TDNF_SET_SSL_SETTINGS",    "There was an error while setting SSL settings for the repo."}, \
    {ERROR_TDNF_REPO_PERFORM,        "ERROR_TDNF_REPO_PERFORM",        "Error during repo handle execution"}, \
    {ERROR_TDNF_REPO_GETINFO,        "ERROR_TDNF_REPO_GETINFO",        "Repo during repo result getinfo"}, \
    {ERROR_TDNF_TRANSACTION_FAILED,  "ERROR_TDNF_TRANSACTION_FAILED",  "rpm transaction failed"}, \
    {ERROR_TDNF_NO_SEARCH_RESULTS,   "ERROR_TDNF_NO_SEARCH_RESULTS",   "No matches found"}, \
    {ERROR_TDNF_RPMRC_NOTFOUND,      "ERROR_TDNF_RPMRC_NOTFOUND",      "rpm generic error - not found (possible corrupt rpm file)"}, \
    {ERROR_TDNF_RPMRC_FAIL,          "ERROR_TDNF_RPMRC_FAIL",          "rpm generic failure"}, \
    {ERROR_TDNF_RPMRC_NOTTRUSTED,    "ERROR_TDNF_RPMRC_NOTTRUSTED",    "rpm signature is OK, but key is not trusted"}, \
    {ERROR_TDNF_RPMRC_NOKEY,         "ERROR_TDNF_RPMRC_NOKEY",         "public key is unavailable. install public key using rpm --import or use --nogpgcheck to ignore."}, \
    {ERROR_TDNF_INVALID_PUBKEY_FILE, "ERROR_TDNF_INVALID_PUBKEY_FILE", "public key file is invalid or corrupted"}, \
    {ERROR_TDNF_KEYURL_UNSUPPORTED,  "ERROR_TDNF_KEYURL_UNSUPPORTED",  "GpgKey Url schemes other than file are not supported"}, \
    {ERROR_TDNF_KEYURL_INVALID,      "ERROR_TDNF_KEYURL_INVALID",      "GpgKey Url is invalid"}, \
    {ERROR_TDNF_RPM_NOT_SIGNED,      "ERROR_TDNF_RPM_NOT_SIGNED",      "RPM not signed. Use --nogpgcheck to ignore."}, \
    {ERROR_TDNF_RPMTD_CREATE_FAILED, "ERROR_TDNF_RPMTD_CREATE_FAILED", "RPM data container could not be created. Use --nogpgcheck to ignore."}, \
    {ERROR_TDNF_RPM_GET_RSAHEADER_FAILED,"ERROR_TDNF_RPM_GET_RSAHEADER_FAILED","RPM not signed. Use --skipsignature or --nogpgcheck to ignore."}, \
    {ERROR_TDNF_RPM_GPG_PARSE_FAILED,"ERROR_TDNF_RPM_GPG_PARSE_FAILED","RPM failed to parse gpg key. Use --nogpgcheck to ignore."}, \
    {ERROR_TDNF_RPM_GPG_NO_MATCH,   "ERROR_TDNF_RPM_GPG_NO_MATCH",     "RPM is signed but failed to match with known keys. Use --nogpgcheck to ignore."}, \
    {ERROR_TDNF_AUTOERASE_UNSUPPORTED,"ERROR_TDNF_AUTOERASE_UNSUPPORTED","autoerase / autoremove is not supported."}, \
    {ERROR_TDNF_RPM_CHECK,           "ERROR_TDNF_RPM_CHECK",           "rpm check reported errors"}, \
    {ERROR_TDNF_RPMTS_BAD_ROOT_DIR,  "ERROR_TDNF_RPMTS_BAD_ROOT_DIR",  "Bad root directory"}, \
    {ERROR_TDNF_METADATA_EXPIRE_PARSE, "ERROR_TDNF_METADATA_EXPIRE_PARSE", "metadata_expire value could not be parsed. Check your repo files."},\
    {ERROR_TDNF_PROTECTED,           "ERROR_TDNF_PROTECTED",           "The operation would result in removing a protected package."},\
    {ERROR_TDNF_DOWNGRADE_NOT_ALLOWED,\
        "ERROR_TDNF_DOWNGRADE_NOT_ALLOWED",\
        "a downgrade is not allowed below the minimal version. Check 'minversions' in the configuration."},\
    {ERROR_TDNF_PERM, "ERROR_TDNF_PERM", "Operation not permitted. You have to be root."},\
    {ERROR_TDNF_OPT_NOT_FOUND, "ERROR_TDNF_OPT_NOT_FOUND", "A required option was not found"},\
    {ERROR_TDNF_OPERATION_ABORTED, "ERROR_TDNF_OPERATION_ABORTED", "Operation aborted."},\
    {ERROR_TDNF_INVALID_INPUT, "ERROR_TDNF_INVALID_INPUT", "Invalid input."},\
    {ERROR_TDNF_CACHE_DISABLED, "ERROR_TDNF_CACHE_DISABLED", "cache only is set, but no repo data found"},\
    {ERROR_TDNF_CACHE_DIR_OUT_OF_DISK_SPACE, "ERROR_TDNF_CACHE_DIR_OUT_OF_DISK_SPACE", "Insufficient disk space at cache directory /var/cache/tdnf (unless specified differently in config). Try freeing space first."},\
    {ERROR_TDNF_DUPLICATE_REPO_ID,         "ERROR_TDNF_DUPLICATE_REPO_ID",         "Duplicate repo id"}, \
    {ERROR_TDNF_EVENT_CTXT_ITEM_NOT_FOUND, "ERROR_TDNF_EVENT_CTXT_ITEM_NOT_FOUND", "An event context item was not found. This is usually related to plugin events. Try --noplugins to deactivate all plugins or --disableplugin=<plugin> to deactivate a specific one. You can permanently deactivate an offending plugin by setting enable=0 in the plugin config file."},\
    {ERROR_TDNF_EVENT_CTXT_ITEM_INVALID_TYPE, "ERROR_TDNF_EVENT_CTXT_ITEM_INVALID_TYPE", "An event item type had a mismatch. This is usually related to plugin events. Try --noplugins to deactivate all plugins or --disableplugin=<plugin> to deactivate a specific one. You can permanently deactivate an offending plugin by setting enable=0 in the plugin config file."},\
    {ERROR_TDNF_NO_GPGKEY_CONF_ENTRY,         "ERROR_TDNF_NO_GPGKEY_CONF_ENTRY",         "gpgkey entry is missing for this repo. please add gpgkey in repo file or use --nogpgcheck to ignore."}, \
    {ERROR_TDNF_URL_INVALID,                          "ERROR_TDNF_URL_INVALID",          "URL is invalid."}, \
    {ERROR_TDNF_SIZE_MISMATCH,                       "ERROR_TDNF_SIZE_MISMATCH",                       "File size does not match."}, \
    {ERROR_TDNF_CHECKSUM_MISMATCH,                   "ERROR_TDNF_CHECKSUM_MISMATCH",                   "File checksum does not match."}, \
    {ERROR_TDNF_BASEURL_DOES_NOT_EXISTS,             "ERROR_TDNF_BASEURL_DOES_NOT_EXISTS",             "Base URL and Metalink URL not found in the repo file"},\
    {ERROR_TDNF_CHECKSUM_VALIDATION_FAILED,          "ERROR_TDNF_CHECKSUM_VALIDATION_FAILED",          "Checksum Validation failed for the repomd.xml downloaded using URL from metalink"},\
    {ERROR_TDNF_METALINK_RESOURCE_VALIDATION_FAILED, "ERROR_TDNF_METALINK_RESOURCE_VALIDATION_FAILED", "No Resource present in metalink file for file download"},\
    {ERROR_TDNF_FIPS_MODE_FORBIDDEN,                 "ERROR_TDNF_FIPS_MODE_FORBIDDEN",                 "API call to digest API forbidden in FIPS mode!"},\
    {ERROR_TDNF_CURLE_UNSUPPORTED_PROTOCOL,          "ERROR_TDNF_CURLE_UNSUPPORTED_PROTOCOL",          "Curl doesn't Support this protocol"},\
    {ERROR_TDNF_CURLE_FAILED_INIT,                   "ERROR_TDNF_CURLE_FAILED_INIT",                   "Curl Init Failed"},\
    {ERROR_TDNF_CURLE_URL_MALFORMAT,                 "ERROR_TDNF_CURLE_URL_MALFORMAT",                 "URL seems to be corrupted. Please clean all and makecache"},\
    {ERROR_TDNF_SYSTEM_BASE,                         "ERROR_TDNF_SYSTEM_BASE",                         "unknown system error"},\
    {ERROR_TDNF_HISTORY_NODB,                        "ERROR_TDNF_HISTORY_ERROR",                       "History database error"},\
    {ERROR_TDNF_HISTORY_NODB,                        "ERROR_TDNF_HISTORY_NODB",                        "History database does not exist"},\
};


//remoterepo.c
#define sizeOfStruct(ARRAY) (sizeof(ARRAY)/sizeof(*ARRAY))

//metalink.c
typedef void (*TDNF_ML_FREE_FUNC) (void* data);

#define TAG_NAME_FILE "file"
#define TAG_NAME_SIZE "size"
#define TAG_NAME_HASH "hash"
#define TAG_NAME_URL  "url"

