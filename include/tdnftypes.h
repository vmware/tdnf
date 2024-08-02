/*
 * Copyright (C) 2015-2023 VMware, Inc. All Rights Reserved.
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
    ALTER_AUTOERASEALL,
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
    SCOPE_DOWNGRADES,
    SCOPE_SOURCE
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

#define CLEANTYPE_NONE         0x00
#define CLEANTYPE_PACKAGES     0x01
#define CLEANTYPE_METADATA     0x02
#define CLEANTYPE_DBCACHE      0x04
#define CLEANTYPE_PLUGINS      0x08
#define CLEANTYPE_EXPIRE_CACHE 0x10
#define CLEANTYPE_KEYS         0x20
#define CLEANTYPE_ALL          0xff


//RepoList command filter
typedef enum
{
    REPOLISTFILTER_ALL,
    REPOLISTFILTER_ENABLED,
    REPOLISTFILTER_DISABLED
}TDNF_REPOLISTFILTER;

// skip problem type mask
typedef enum
{
    SKIPPROBLEM_NONE      = 0x00,
    SKIPPROBLEM_CONFLICTS = 0x01,
    SKIPPROBLEM_OBSOLETES = 0x02,
    SKIPPROBLEM_DISABLED  = 0x04,
    SKIPPROBLEM_BROKEN    = 0x08
} TDNF_SKIPPROBLEM_TYPE;

typedef struct _TDNF_ *PTDNF;

typedef struct _TDNF_PKG_CHANGELOG_ENTRY
{
    time_t timeTime;
    char *pszAuthor;
    char *pszText;
    struct _TDNF_PKG_CHANGELOG_ENTRY *pNext;
} TDNF_PKG_CHANGELOG_ENTRY, *PTDNF_PKG_CHANGELOG_ENTRY;

typedef struct _TDNF_PKG_INFO
{
    uint32_t dwEpoch;
    uint32_t dwInstallSizeBytes;
    uint32_t dwDownloadSizeBytes;
    int nChecksumType;
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
    char* pszFormattedDownloadSize;
    char* pszRelease;
    char* pszLocation;
    char ***pppszDependencies;
    char **ppszFileList;
    char *pszSourcePkg;
    unsigned char* pbChecksum;
    PTDNF_PKG_CHANGELOG_ENTRY pChangeLogEntries;
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
    char** ppszPkgsUserInstall;
}TDNF_SOLVED_PKG_INFO, *PTDNF_SOLVED_PKG_INFO;

typedef struct _TDNF_CMD_OPT
{
    char* pszOptName;
    char* pszOptValue;
    struct _TDNF_CMD_OPT* pNext;
}TDNF_CMD_OPT, *PTDNF_CMD_OPT;

typedef struct _TDNF_CMD_ARGS
{
    //Represent options in the dnf cmd line.
    //All options are one to one maps to dnf command line
    //options (incomplete)
    int nAllDeps;          //download all dependencies even if already installed
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
    int nNoDeps;           //download no dependencies
    int nNoGPGCheck;       //skip gpg check
    int nNoOutput;         //if quiet and assumeyes are provided
    int nQuiet;            //quiet option
    int nVerbose;          //print debug info
    int nIPv4;             //resolve to IPv4 addresses only
    int nIPv6;             //resolve to IPv6 addresses only
    int nDisableExcludes;  //disable excludes from tdnf.conf
    int nDownloadOnly;     //download packages only, no install
    int nNoAutoRemove;     //overide clean_requirements_on_remove config option
    int nJsonOutput;       //output in json format
    int nTestOnly;         //run test transaction only
    int nSkipBroken;
    int nSource;
    int nBuildDeps;
    char* pszDownloadDir;  //directory for download, if nDownloadOnly is set
    char* pszInstallRoot;  //set install root
    char* pszConfFile;     //set conf file location
    char* pszReleaseVer;   //Release version

    //Commands and args that do not fall in options
    char** ppszCmds;
    int nCmdCount;
    PTDNF_CMD_OPT pSetOpt;

    int nArgc;
    char **ppszArgv;
}TDNF_CMD_ARGS, *PTDNF_CMD_ARGS;

typedef struct _TDNF_CONF
{
    int nGPGCheck;
    int nSSLVerify;
    int nInstallOnlyLimit;
    int nCleanRequirementsOnRemove;
    int nKeepCache;
    int nOpenMax;          //set max number of open files
    int nCheckUpdateCompat;
    int nDistroSyncReinstallChanged;
    int nPluginsEnabled;
    char* pszRepoDir;
    char* pszCacheDir;
    char* pszPersistDir;
    char* pszProxy;
    char* pszProxyUserPass;
    char** ppszDistroVerPkgs;
    char* pszBaseArch;
    char* pszVarReleaseVer;
    char* pszVarBaseArch;
    char* pszUserAgentHeader;
    char* pszOSName;
    char* pszOSVersion;
    char** ppszExcludes;
    char** ppszMinVersions;
    char** ppszPkgLocks;
    char** ppszProtectedPkgs;
    char **ppszInstallOnlyPkgs;
    char **ppszVarsDirs;
    char *pszPluginPath;
    char *pszPluginConfPath;
}TDNF_CONF, *PTDNF_CONF;

typedef struct _TDNF_REPO_DATA
{
    int nEnabled;
    int nSkipIfUnavailable;
    int nGPGCheck;
    int nHasMetaData;
    long lMetadataExpire;
    char* pszId;
    char* pszName;
    char** ppszBaseUrls;
    char* pszMetaLink;
    char* pszMirrorList;
    char** ppszUrlGPGKeys;
    int nSSLVerify;
    char* pszSSLCaCert;
    char* pszSSLClientCert;
    char* pszSSLClientKey;
    char* pszUser;
    char* pszPass;
    int nPriority;
    int nTimeout;
    int nMinrate;
    int nThrottle;
    int nRetries;
    int nSkipMDFileLists;
    int nSkipMDUpdateInfo;
    int nSkipMDOther;
    char *pszCacheName;

    struct _TDNF_REPO_DATA* pNext;
}TDNF_REPO_DATA, *PTDNF_REPO_DATA;

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
    REPOQUERY_WHAT_KEY_PROVIDES,
    REPOQUERY_WHAT_KEY_OBSOLETES,
    REPOQUERY_WHAT_KEY_CONFLICTS,
    REPOQUERY_WHAT_KEY_REQUIRES,
    REPOQUERY_WHAT_KEY_RECOMMENDS,
    REPOQUERY_WHAT_KEY_SUGGESTS,
    REPOQUERY_WHAT_KEY_SUPPLEMENTS,
    REPOQUERY_WHAT_KEY_ENHANCES,
    REPOQUERY_WHAT_KEY_DEPENDS,
    REPOQUERY_WHAT_KEY_COUNT
} REPOQUERY_WHAT_KEY;

typedef enum {
    REPOQUERY_DEP_KEY_PROVIDES = 0,
    REPOQUERY_DEP_KEY_OBSOLETES,
    REPOQUERY_DEP_KEY_CONFLICTS,
    REPOQUERY_DEP_KEY_REQUIRES,
    REPOQUERY_DEP_KEY_RECOMMENDS,
    REPOQUERY_DEP_KEY_SUGGESTS,
    REPOQUERY_DEP_KEY_SUPPLEMENTS,
    REPOQUERY_DEP_KEY_ENHANCES,
    REPOQUERY_DEP_KEY_DEPENDS,
    REPOQUERY_DEP_KEY_REQUIRES_PRE,
    REPOQUERY_DEP_KEY_COUNT
}REPOQUERY_DEP_KEY;

#define TDNF_REPOQUERY_MAXARCHS 10

typedef struct _TDNF_REPOQUERY_ARGS
{
    char *pszSpec;

    /* select options */
    int nAvailable;        /* list what's available (default) */
    int nDuplicates;
    int nExtras;           /* packages that are installed but not available */
    int nInstalled;
    int nUpgrades;
    int nUserInstalled;
    char *pszFile;         /* packages that own this file */
    char ***pppszWhatKeys;
    char **ppszArchs;

    /* query options */
    int nChangeLogs;
    unsigned int depKeySet;               /* list dependencies of this type, 0 => unset */
    int nList;                /* list files of packages(s) */
    char *pszQueryFormat;
    int nSource;              /* show source packages */
}TDNF_REPOQUERY_ARGS, *PTDNF_REPOQUERY_ARGS;

typedef enum {
    HISTORY_CMD_LIST = 0,
    HISTORY_CMD_INIT,
    HISTORY_CMD_ROLLBACK,
    HISTORY_CMD_UNDO,
    HISTORY_CMD_REDO
} HISTORY_CMD;

typedef struct _TDNF_HISTORY_ARGS
{
    HISTORY_CMD nCommand;
    int nInfo;
    int nFrom;
    int nTo;
    int nReverse;
    char *pszSpec;
} TDNF_HISTORY_ARGS, *PTDNF_HISTORY_ARGS;

typedef struct _TDNF_HISTORY_INFO_ITEM
{
    int nId;
    int nType;
    char *pszCmdLine;
    time_t timeStamp;
    int nAddedCount;
    int nRemovedCount;
    char **ppszAddedPkgs;
    char **ppszRemovedPkgs;
} TDNF_HISTORY_INFO_ITEM, *PTDNF_HISTORY_INFO_ITEM;

typedef struct _TDNF_HISTORY_INFO
{
    int nItemCount;
    PTDNF_HISTORY_INFO_ITEM pItems;
} TDNF_HISTORY_INFO, *PTDNF_HISTORY_INFO;

#ifdef __cplusplus
}
#endif
