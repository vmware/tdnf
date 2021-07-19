/*
 * Copyright (C) 2015-2017 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#pragma once

#include <curl/curl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    CONF_FLAG_IPV4,
    CONF_FLAG_IPV6,
    CONF_FLAG_ALLOWERASING,
    CONF_FLAG_ASSUMENO,
    CONF_FLAG_BEST,
    CONF_FLAG_CACHEONLY,
    CONF_FLAG_DEBUGSOLVER,
    CONF_FLAG_REFRESHMETADATA,
    CONF_FLAG_GPGCHECK,
    CONF_FLAG_QUIET,
    CONF_FLAG_SHOWDUPS,
    CONF_FLAG_VERBOSE
}TDNF_CONF_FLAG;

typedef enum
{
    CONF_TYPE_CONFIG_FILE,
    CONF_TYPE_DEBUG_LEVEL,
    CONF_TYPE_DISABLE_EXCLUDES,
    CONF_TYPE_DISABLE_REPO,
    CONF_TYPE_ENABLE_REPO,
    CONF_TYPE_EXCLUDE_PACKAGE,
    CONF_TYPE_INSTALL_ROOT,
    CONF_TYPE_RELEASE_VER,
    CONF_TYPE_RPM_VERBOSITY,
}TDNF_CONF_TYPE;

typedef enum
{
    ALTER_AUTOERASE,
    ALTER_DOWNGRADE,
    ALTER_DOWNGRADEALL,
    ALTER_ERASE,
    ALTER_INSTALL,
    ALTER_REINSTALL,
    ALTER_UPGRADE,
    ALTER_UPGRADEALL,
    ALTER_DISTRO_SYNC,
    ALTER_OBSOLETED
}TDNF_ALTERTYPE;

typedef enum
{
    TDNF_RPMLOG_EMERG,
    TDNF_RPMLOG_ALERT,
    TDNF_RPMLOG_CRIT,
    TDNF_RPMLOG_ERR,
    TDNF_RPMLOG_WARNING,
    TDNF_RPMLOG_NOTICE,
    TDNF_RPMLOG_INFO,
    TDNF_RPMLOG_DEBUG
}TDNF_RPMLOG;

//command scope
typedef enum
{
    SCOPE_NONE = -1,
    SCOPE_ALL,
    SCOPE_INSTALLED,
    SCOPE_AVAILABLE,
    SCOPE_EXTRAS,
    SCOPE_OBSOLETES,
    SCOPE_RECENT,
    SCOPE_UPGRADES,
    SCOPE_DOWNGRADES
}TDNF_SCOPE;

//availability - updateinfo
typedef enum
{
    AVAIL_AVAILABLE,
    AVAIL_INSTALLED,
    AVAIL_UPDATES
}TDNF_AVAIL;

//output mode - updateinfo
typedef enum
{
    OUTPUT_SUMMARY,
    OUTPUT_LIST,
    OUTPUT_INFO
}TDNF_UPDATEINFO_OUTPUT;

//updateinfo - type
typedef enum
{
    UPDATE_UNKNOWN,
    UPDATE_SECURITY,
    UPDATE_BUGFIX,
    UPDATE_ENHANCEMENT
}TDNF_UPDATEINFO_TYPE;

//Clean command type
typedef enum
{
    CLEANTYPE_NONE = -1,
    CLEANTYPE_PACKAGES,
    CLEANTYPE_METADATA,
    CLEANTYPE_DBCACHE,
    CLEANTYPE_PLUGINS,
    CLEANTYPE_EXPIRE_CACHE,
    CLEANTYPE_RPMDB,
    CLEANTYPE_ALL
}TDNF_CLEANTYPE;

//RepoList command filter
typedef enum
{
    REPOLISTFILTER_ALL,
    REPOLISTFILTER_ENABLED,
    REPOLISTFILTER_DISABLED
}TDNF_REPOLISTFILTER;

//CmdOpt Types
typedef enum
{
    CMDOPT_NONE = -1,
    CMDOPT_KEYVALUE,
    CMDOPT_ENABLEREPO,
    CMDOPT_DISABLEREPO,
    CMDOPT_CURL_INIT_CB,
    CMDOPT_ENABLEPLUGIN,
    CMDOPT_DISABLEPLUGIN,
}TDNF_CMDOPT_TYPE;

// skip problem type mask
typedef enum
{
    SKIPPROBLEM_NONE      = 0x00,
    SKIPPROBLEM_CONFLICTS = 0x01,
    SKIPPROBLEM_OBSOLETES = 0x02,
    SKIPPROBLEM_DISABLED  = 0x04,
} TDNF_SKIPPROBLEM_TYPE;

typedef struct _TDNF_ *PTDNF;

typedef struct _TDNF_PKG_INFO
{
    uint32_t dwEpoch;
    uint32_t dwInstallSizeBytes;
    char* pszName;
    char* pszRepoName;
    char* pszVersion;
    char* pszArch;
    char* pszEVR;
    char* pszSummary;
    char* pszURL;
    char* pszLicense;
    char* pszDescription;
    char* pszFormattedSize;
    char* pszRelease;
    char* pszLocation;
    char **ppszDependencies;
    char **ppszFileList;
    struct _TDNF_PKG_INFO* pNext;
}TDNF_PKG_INFO, *PTDNF_PKG_INFO;

typedef struct _TDNF_SOLVED_PKG_INFO
{
    int nNeedAction;
    int nNeedDownload;
    TDNF_ALTERTYPE nAlterType;
    PTDNF_PKG_INFO pPkgsNotAvailable;
    PTDNF_PKG_INFO pPkgsExisting;
    PTDNF_PKG_INFO pPkgsToInstall;
    PTDNF_PKG_INFO pPkgsToDowngrade;
    PTDNF_PKG_INFO pPkgsToUpgrade;
    PTDNF_PKG_INFO pPkgsToRemove;
    PTDNF_PKG_INFO pPkgsUnNeeded;
    PTDNF_PKG_INFO pPkgsToReinstall;
    PTDNF_PKG_INFO pPkgsObsoleted;
    PTDNF_PKG_INFO pPkgsRemovedByDowngrade;
    char** ppszPkgsNotResolved;
    char** ppszPkgsNotInstalled;
}TDNF_SOLVED_PKG_INFO, *PTDNF_SOLVED_PKG_INFO;

/*
 * api clients can set this callback via setopts
 * once set, tdnf curl calls will call this function
 * before any curl options are set. this can be used
 * to modify curl opts outside the ones tdnf uses.
*/
typedef uint32_t
(*PFN_CURL_CONFIG_CB)(
    CURL *pCurl,
    const char *pszUrl
    );

typedef struct _TDNF_CMD_OPT
{
    int nType;
    char* pszOptName;
    union
    {
        char* pszOptValue;
        PFN_CURL_CONFIG_CB pfnCurlConfigCB;
    };
    struct _TDNF_CMD_OPT* pNext;
}TDNF_CMD_OPT, *PTDNF_CMD_OPT;

typedef struct _TDNF_CMD_ARGS
{
    //Represent options in the dnf cmd line.
    //All options are one to one maps to dnf command line
    //options (incomplete)
    int nAllowErasing;     //allow erasures when solving
    int nAssumeNo;         //assume no for all questions
    int nAssumeYes;        //assume yes for all questions
    int nBest;             //resolve packages to latest version
    int nCacheOnly;        //operate entirely from cache
    int nDebugSolver;      //dump solv debug info
    int nShowHelp;         //Show help
    int nRefresh;          //expire metadata before running commands
    int nRpmVerbosity;     //set to rpm verbosity level
    int nShowDuplicates;   //show dups in list/search
    int nShowVersion;      //show version and exit
    int nNoGPGCheck;       //skip gpg check
    int nNoOutput;         //if quiet and assumeyes are provided
    int nQuiet;            //quiet option
    int nVerbose;          //print debug info
    int nIPv4;             //resolve to IPv4 addresses only
    int nIPv6;             //resolve to IPv6 addresses only
    int nDisableExcludes;  //disable excludes from tdnf.conf
    int nDownloadOnly;     //download packages only, no install
    char* pszDownloadDir;  //directory for download, if nDownloadOnly is set
    char* pszInstallRoot;  //set install root
    char* pszConfFile;     //set conf file location
    char* pszReleaseVer;   //Release version

    //Commands and args that do not fall in options
    char** ppszCmds;
    int nCmdCount;
    PTDNF_CMD_OPT pSetOpt;
}TDNF_CMD_ARGS, *PTDNF_CMD_ARGS;

typedef struct _TDNF_CONF
{
    int nGPGCheck;
    int nInstallOnlyLimit;
    int nCleanRequirementsOnRemove;
    int nKeepCache;
    char* pszRepoDir;
    char* pszCacheDir;
    char* pszProxy;
    char* pszProxyUserPass;
    char* pszDistroVerPkg;
    char* pszBaseArch;
    char* pszVarReleaseVer;
    char* pszVarBaseArch;
    char** ppszExcludes;
    char** ppszMinVersions;
}TDNF_CONF, *PTDNF_CONF;

typedef struct _TDNF_REPO_DATA
{
    int nEnabled;
    int nSkipIfUnavailable;
    int nGPGCheck;
    long lMetadataExpire;
    char* pszId;
    char* pszName;
    char* pszBaseUrl;
    char* pszMetaLink;
    char** ppszUrlGPGKeys;

    struct _TDNF_REPO_DATA* pNext;
}TDNF_REPO_DATA, *PTDNF_REPO_DATA;

typedef struct _TDNF_CLEAN_INFO
{
    int nCleanAll;
    char** ppszReposUsed;
    int nRpmDbFilesRemoved;
    int nMetadataFilesRemoved;
    int nDbCacheFilesRemoved;
    int nPackageFilesRemoved;
}TDNF_CLEAN_INFO, *PTDNF_CLEAN_INFO;

typedef struct _TDNF_ERROR_DESC
{
    int nCode;
    char* pszName;
    char* pszDesc;
}TDNF_ERROR_DESC, *PTDNF_ERROR_DESC;

typedef struct _TDNF_UPDATEINFO_REF
{
    char* pszID;
    char* pszLink;
    char* pszTitle;
    char* pszType;
    struct _TDNF_UPDATEINFO_REF* pNext;
}TDNF_UPDATEINFO_REF, *PTDNF_UPDATEINFO_REF;

typedef struct _TDNF_UPDATEINFO_PKG
{
    char* pszName;
    char* pszFileName;
    char* pszEVR;
    char* pszArch;
    struct _TDNF_UPDATEINFO_PKG* pNext;
}TDNF_UPDATEINFO_PKG, *PTDNF_UPDATEINFO_PKG;

typedef struct _TDNF_UPDATEINFO
{
    int nType;
    char* pszID;
    char* pszDate;
    char* pszDescription;
    int nRebootRequired;
    PTDNF_UPDATEINFO_REF pReferences;
    PTDNF_UPDATEINFO_PKG pPackages;
    struct _TDNF_UPDATEINFO* pNext;
}TDNF_UPDATEINFO, *PTDNF_UPDATEINFO;

typedef struct _TDNF_UPDATEINFO_SUMMARY
{
    int nCount;
    int nType;
}TDNF_UPDATEINFO_SUMMARY, *PTDNF_UPDATEINFO_SUMMARY;

#define TDNF_REPOSYNC_MAXARCHS 10

typedef struct _TDNF_REPOSYNC_ARGS
{
    int nDelete;
    int nDownloadMetadata;
    int nGPGCheck;
    int nNewestOnly;
    int nPrintUrlsOnly;
    int nNoRepoPath;
    int nSourceOnly;
    char *pszDownloadPath;
    char *pszMetaDataPath;
    char **ppszArchs;
}TDNF_REPOSYNC_ARGS, *PTDNF_REPOSYNC_ARGS;

typedef enum {
    REPOQUERY_KEY_PROVIDES,
    REPOQUERY_KEY_OBSOLETES,
    REPOQUERY_KEY_CONFLICTS,
    REPOQUERY_KEY_REQUIRES,
    REPOQUERY_KEY_RECOMMENDS,
    REPOQUERY_KEY_SUGGESTS,
    REPOQUERY_KEY_SUPPLEMENTS,
    REPOQUERY_KEY_ENHANCES,
    REPOQUERY_KEY_COUNT
} REPOQUERY_WHAT_KEY;

typedef struct _TDNF_REPOQUERY_ARGS
{
    char *pszSpec;

    /* select options */
    char **ppszWhatDepends;
    char ***pppszWhatKeys;

    /* query options */
    int nInstalled;
    int nAvailable;
    int nDuplicates;
    int nExtras;
    int nDepends;
    int anDeps[REPOQUERY_KEY_COUNT];
    int nRequiresPre;
    int nList;
    int nUpgrades;
}TDNF_REPOQUERY_ARGS, *PTDNF_REPOQUERY_ARGS;

#ifdef __cplusplus
}
#endif
