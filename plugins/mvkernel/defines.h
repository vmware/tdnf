/*
 * Copyright (C) 2020 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#ifndef __PLUGINS_MVKERNEL_DEFINES_H__
#define __PLUGINS_MVKERNEL_DEFINES_H__

#define UNUSED(var) ((void)(var))

typedef struct _TDNF_PLUGIN_HANDLE_
{
    PTDNF pTdnf;
    uint32_t nError; /* last error set by this plugin */
} TDNF_PLUGIN_HANDLE, *PTDNF_PLUGIN_HANDLE;

#define ERR_TDNF_MVKERNEL_BASE_START        2000
#define ERR_TDNF_MVKERNEL_MKDIR_FAILED      ERR_TDNF_MVKERNEL_BASE_START + 1
#define ERR_TDNF_MVKERNEL_CP_FAILED         ERR_TDNF_MVKERNEL_BASE_START + 2
#define ERR_TDNF_MVKERNEL_MOUNT_FAILED      ERR_TDNF_MVKERNEL_BASE_START + 3
#define ERR_TDNF_MVKERNEL_GET_KERN_VER      ERR_TDNF_MVKERNEL_BASE_START + 4
#define ERR_TDNF_MVKERNEL_WRNG_PATH         ERR_TDNF_MVKERNEL_BASE_START + 5
#define ERR_TDNF_MVKERNEL_FTS_OP_FAILED     ERR_TDNF_MVKERNEL_BASE_START + 6
#define ERR_TDNF_MVKERNEL_FOPS_FAILED       ERR_TDNF_MVKERNEL_BASE_START + 7
#define ERR_TDNF_MVKERNEL_UNKNWN            ERR_TDNF_MVKERNEL_BASE_START + 8
#define ERR_TDNF_MVKERNEL_ERR               ERR_TDNF_MVKERNEL_BASE_START + 9

#define MVKERNEL_PLUGIN_ERR                 "mvkernel plugin error"

#define MVKERNEL_ERR_TABLE \
{ \
    {ERR_TDNF_MVKERNEL_MKDIR_FAILED,    "ERR_TDNF_MVKERNEL_MKDIR_FAILED",   "failed to create directory"},      \
    {ERR_TDNF_MVKERNEL_CP_FAILED,       "ERR_TDNF_MVKERNEL_CP_FAILED",      "failed to copy file"},             \
    {ERR_TDNF_MVKERNEL_MOUNT_FAILED,    "ERR_TDNF_MVKERNEL_MOUNT_FAILED",   "mount operation failure"},         \
    {ERR_TDNF_MVKERNEL_GET_KERN_VER,    "ERR_TDNF_MVKERNEL_GET_KERN_VER",   "failed to get kernel version"},    \
    {ERR_TDNF_MVKERNEL_WRNG_PATH,       "ERR_TDNF_MVKERNEL_WRNG_PATH",      "provided path is wrong"},          \
    {ERR_TDNF_MVKERNEL_FTS_OP_FAILED,   "ERR_TDNF_MVKERNEL_FTS_OP_FAILED",  "fts operation failed"},            \
    {ERR_TDNF_MVKERNEL_FOPS_FAILED,     "ERR_TDNF_MVKERNEL_FOPS_FAILED",    "file system operation failed"},    \
    {ERR_TDNF_MVKERNEL_UNKNWN,          "ERR_TDNF_MVKERNEL_UNKNWN",         "unknown error"},                   \
    {ERR_TDNF_MVKERNEL_ERR,             "ERR_TDNF_MVKERNEL_ERR",            "failed to move kernel"},           \
};

#endif /* __PLUGINS_MVKERNEL_DEFINES_H__ */
