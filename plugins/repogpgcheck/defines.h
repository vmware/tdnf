/*
 * Copyright (C) 2020 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#ifndef __PLUGINS_REPOGPGCHECK_DEFINES_H__
#define __PLUGINS_REPOGPGCHECK_DEFINES_H__

#define ERROR_TDNF_GPG_BASE_START        2000
#define ERROR_TDNF_GPG_ERROR             ERROR_TDNF_GPG_BASE_START + 1
#define ERROR_TDNF_GPG_VERSION_FAILED    ERROR_TDNF_GPG_BASE_START + 2
#define ERROR_TDNF_GPG_VERIFY_RESULT     ERROR_TDNF_GPG_BASE_START + 3
#define ERROR_TDNF_GPG_SIGNATURE_CHECK   ERROR_TDNF_GPG_BASE_START + 4

/* gpgme specific errors */
#define ERROR_TDNF_GPGME_START           2400

#define TDNF_REPO_CONFIG_REPO_GPGCHECK_KEY "repo_gpgcheck"
#define TDNF_REPO_METADATA_SIG_EXT         ".asc"

#define REPOGPGCHECK_PLUGIN_ERROR "repogpgcheck plugin error"
#define REPOGPGCHECK_ERROR_TABLE \
{ \
    {ERROR_TDNF_GPG_ERROR,           "ERROR_TDNF_GPG_ERROR",           "unknown error"}, \
    {ERROR_TDNF_GPG_VERSION_FAILED,  "ERROR_TDNF_GPG_VERSION_FAILED",  "version failed"}, \
    {ERROR_TDNF_GPG_VERIFY_RESULT,   "ERROR_TDNF_GPG_VERIFY_RESULT",   "failed to verify result"}, \
    {ERROR_TDNF_GPG_SIGNATURE_CHECK, "ERROR_TDNF_GPG_SIGNATURE_CHECK", "failed to verify signature"}, \
};
#endif /* __PLUGINS_REPOGPGCHECK_DEFINES_H__ */
