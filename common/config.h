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
#define TDNF_CONF_KEY_INSTALLONLYPKGS     "installonlypkgs"
#define TDNF_CONF_KEY_CLEAN_REQ_ON_REMOVE "clean_requirements_on_remove"
#define TDNF_CONF_KEY_REPODIR             "repodir" // typo, keep for back compatibility
#define TDNF_CONF_KEY_REPOSDIR            "reposdir"
#define TDNF_CONF_KEY_CACHEDIR            "cachedir"
#define TDNF_CONF_KEY_PERSISTDIR          "persistdir"
#define TDNF_CONF_KEY_PROXY               "proxy"
#define TDNF_CONF_KEY_PROXY_USER          "proxy_username"
#define TDNF_CONF_KEY_PROXY_PASS          "proxy_password"
#define TDNF_CONF_KEY_KEEP_CACHE          "keepcache"
#define TDNF_CONF_KEY_DISTROVERPKGS       "distroverpkg"
#define TDNF_CONF_KEY_DISTROARCHPKG       "distroarchpkg"
#define TDNF_CONF_KEY_MAX_STRING_LEN      "maxstringlen"
#define TDNF_CONF_KEY_PLUGINS             "plugins"
#define TDNF_CONF_KEY_NO_PLUGINS          "noplugins"
#define TDNF_CONF_KEY_PLUGIN_PATH         "pluginpath"
#define TDNF_CONF_KEY_PLUGIN_CONF_PATH    "pluginconfpath"
#define TDNF_CONF_KEY_SSL_VERIFY          "sslverify"
#define TDNF_PLUGIN_CONF_KEY_ENABLED      "enabled"
#define TDNF_CONF_KEY_EXCLUDE             "excludepkgs"
#define TDNF_CONF_KEY_MINVERSIONS         "minversions"
#define TDNF_CONF_KEY_OPENMAX             "openmax"
#define TDNF_CONF_KEY_VARS_DIRS           "varsdir"
#define TDNF_CONF_KEY_CHECK_UPDATE_COMPAT "dnf_check_update_compat"
#define TDNF_CONF_KEY_DISTROSYNC_REINSTALL_CHANGED "distrosync_reinstall_changed"

//Repo file key names
#define TDNF_REPO_KEY_BASEURL             "baseurl"
#define TDNF_REPO_KEY_ENABLED             "enabled"
#define TDNF_REPO_KEY_METALINK            "metalink"
#define TDNF_REPO_KEY_MIRRORLIST          "mirrorlist"
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
#define TDNF_REPO_KEY_SSL_VERIFY          TDNF_CONF_KEY_SSL_VERIFY
#define TDNF_REPO_KEY_SSL_CA_CERT         "sslcacert"
#define TDNF_REPO_KEY_SSL_CLI_CERT        "sslclientcert"
#define TDNF_REPO_KEY_SSL_CLI_KEY         "sslclientkey"
#define TDNF_REPO_KEY_SKIP_MD_FILELISTS   "skip_md_filelists"
#define TDNF_REPO_KEY_SKIP_MD_UPDATEINFO  "skip_md_updateinfo"
#define TDNF_REPO_KEY_SKIP_MD_OTHER       "skip_md_other"

//file names
#define TDNF_REPO_METADATA_MARKER         "lastrefresh"
#define TDNF_REPO_METADATA_MIRRORLIST     "mirrorlist"
#define TDNF_REPO_METADATA_FILE_PATH      "repodata/repomd.xml"
#define TDNF_REPO_METADATA_FILE_NAME      "repomd.xml"
#define TDNF_REPO_METALINK_FILE_NAME      "metalink"
#define TDNF_REPO_BASEURL_FILE_NAME       "baseurl"

#define TDNF_AUTOINSTALLED_FILE           "autoinstalled"
#define TDNF_HISTORY_DB_FILE              "history.db"
#define TDNF_DEFAULT_DATA_LOCATION        "/var/lib/tdnf"

// repo defaults
#define TDNF_DEFAULT_REPO_LOCATION        "/etc/yum.repos.d"
#define TDNF_DEFAULT_CACHE_LOCATION       "/var/cache/tdnf"
#define TDNF_DEFAULT_VARS_DIRS            "/etc/tdnf/vars /etc/dnf/vars /etc/yum/vars"

/* pszPersistDir - default is configurable at build time,
   and configurable with "persistdir" at run time */
#define TDNF_DEFAULT_DB_LOCATION          HISTORY_DB_DIR

#define TDNF_DEFAULT_DISTROVERPKGS        "system-release(releasever) system-release redhat-release"
#define TDNF_DEFAULT_DISTROARCHPKG        "x86_64"
#define TDNF_RPM_CACHE_DIR_NAME           "rpms"
#define TDNF_REPODATA_DIR_NAME            "repodata"
#define TDNF_SOLVCACHE_DIR_NAME           "solvcache"
#define TDNF_REPO_METADATA_EXPIRE_NEVER   "never"

#define TDNF_CONF_DEFAULT_OPENMAX            1024
#define TDNF_CONF_DEFAULT_INSTALLONLY_LIMIT  2
#define TDNF_CONF_DEFAULT_SSLVERIFY          1

// repo default settings
#define TDNF_REPO_DEFAULT_ENABLED            0
#define TDNF_REPO_DEFAULT_SKIP               0
#define TDNF_REPO_DEFAULT_GPGCHECK           1
#define TDNF_REPO_DEFAULT_MINRATE            0
#define TDNF_REPO_DEFAULT_THROTTLE           0
#define TDNF_REPO_DEFAULT_TIMEOUT            0
#define TDNF_REPO_DEFAULT_RETRIES            10
#define TDNF_REPO_DEFAULT_PRIORITY           50
#define TDNF_REPO_DEFAULT_METADATA_EXPIRE    172800 // 48 hours in seconds
#define TDNF_REPO_DEFAULT_METADATA_EXPIRE_STR STRINGIFYX(TDNF_REPO_DEFAULT_METADATA_EXPIRE)
#define TDNF_REPO_DEFAULT_SKIP_MD_FILELISTS  0
#define TDNF_REPO_DEFAULT_SKIP_MD_UPDATEINFO 0
#define TDNF_REPO_DEFAULT_SKIP_MD_OTHER      0

// var names
#define TDNF_VAR_RELEASEVER               "releasever"
#define TDNF_VAR_BASEARCH                 "basearch"
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
