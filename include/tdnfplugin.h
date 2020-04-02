/*
 * Copyright (C) 2020 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#ifndef _TDNF_PLUGIN_H_
#define _TDNF_PLUGIN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "tdnf.h"

typedef struct _TDNF_PLUGIN_HANDLE_ *PTDNF_PLUGIN_HANDLE;
typedef struct _TDNF_EVENT_DATA_    *PTDNF_EVENT_DATA;
typedef struct _TDNF_PLUGIN_        *PTDNF_PLUGIN;

/* function names */
#define TDNF_FN_NAME_PLUGIN_GET_VERSION       "TDNFPluginGetVersion"
#define TDNF_FN_NAME_PLUGIN_GET_NAME          "TDNFPluginGetName"
#define TDNF_FN_NAME_PLUGIN_LOAD_INTERFACE    "TDNFPluginLoadInterface"

typedef enum
{
    TDNF_PLUGIN_EVENT_TYPE_NONE     = 0x0,
    TDNF_PLUGIN_EVENT_TYPE_INIT     = 0x1, /* init is not maskable */
    TDNF_PLUGIN_EVENT_TYPE_REPO     = 0x2,
    TDNF_PLUGIN_EVENT_TYPE_REPO_MD  = 0x4,
    TDNF_PLUGIN_EVENT_TYPE_ALL      = 0xFF
} TDNF_PLUGIN_EVENT_TYPE;

typedef enum
{
    TDNF_PLUGIN_EVENT_STATE_NONE,
    TDNF_PLUGIN_EVENT_STATE_DOWNLOAD,
    TDNF_PLUGIN_EVENT_STATE_CREATE,
    TDNF_PLUGIN_EVENT_STATE_READCONFIG,
    TDNF_PLUGIN_EVENT_STATE_PROCESS,
    TDNF_PLUGIN_EVENT_STATE_ACCESS,
    TDNF_PLUGIN_EVENT_STATE_CLOSE,
} TDNF_PLUGIN_EVENT_STATE;

typedef enum
{
    TDNF_PLUGIN_EVENT_PHASE_NONE,
    TDNF_PLUGIN_EVENT_PHASE_START,
    TDNF_PLUGIN_EVENT_PHASE_END
} TDNF_PLUGIN_EVENT_PHASE;


/* plugin event layout (32 bit)
 * <   type   >  <  state  >   <  phase  >
 * <  24 bits >  <  6 bits >   <  2 bits >
 * Eg: TDNF_PLUGIN_EVENT_REPO_READCONFIG_START = TDNF_PLUGIN_EVENT_TYPE_REPO  << 8 |
 *                                         TDNF_PLUGIN_EVENT_STATE_READCONFIG << 2 |
 *                                         TDNF_PLUGIN_EVENT_PHASE_START
*/
typedef uint32_t TDNF_PLUGIN_EVENT;

#define PLUGIN_EVENT_TYPE(event)                ((event) >> 8)
#define PLUGIN_EVENT_PHASE(event)               ((event) & 0x3)
#define PLUGIN_EVENT_STATE(event)               (((event) >> 2) & 0x3F)
#define MAKE_PLUGIN_EVENT(type, state, phase)   (((type) << 8) | ((state) << 2) | ((phase)))

/*
 * pData is context sensitive.
*/
typedef struct _TDNF_EVENT_CONTEXT_
{
    TDNF_PLUGIN_EVENT nEvent;
    PTDNF_EVENT_DATA pData;
} TDNF_EVENT_CONTEXT, *PTDNF_EVENT_CONTEXT;

/* version of the plugin interface */
typedef const char *
(*PFN_TDNF_PLUGIN_GET_VERSION)(
    void
    );

/* name of the plugin interface */
typedef const char *
(*PFN_TDNF_PLUGIN_GET_NAME)(
    void
    );

/*
 * Return a bit mask of events required by a plugin.
 * Before calling back on specific events, tdnf will
 * call this function to check if the plugin is interested
 * in recieving events.
 * To continue recieving events, fill events required
 * and return 0.
 * To change events required, fill events required with
 * necessary values and return 0.
 * To stop recieving events, return ERROR_TDNF_PLUGIN_NO_MORE_EVENTS
 * if stopping, there is no need to fill events.
*/
typedef uint32_t
(*PFN_TDNF_PLUGIN_EVENTS_NEEDED)(
    PTDNF_PLUGIN_HANDLE pHandle,
    TDNF_PLUGIN_EVENT_TYPE *pnEventTypes
    );

/*
 * initialize. handle returned must be saved.
 * handle is used in all api calls.
 * when done, use pFnCloseHandle to close.
*/
typedef uint32_t
(*PFN_TDNF_PLUGIN_INITIALIZE)(
    const char *pszConfigFile,
    PTDNF_PLUGIN_HANDLE *ppHandle
    );

/*
 * Return error description for errors originating from this plugin.
 * Return human readable error strings. Do not include user supplied
 * data in error strings.
*/
typedef uint32_t
(*PFN_TDNF_PLUGIN_GET_ERROR_STRING)(
    PTDNF_PLUGIN_HANDLE pHandle,
    uint32_t dwError,
    char **ppszError
    );

/* event callback */
typedef uint32_t
(*PFN_TDNF_PLUGIN_EVENT)(
    PTDNF_PLUGIN_HANDLE pHandle,
    PTDNF_EVENT_CONTEXT pContext
    );

/* close handle */
typedef uint32_t
(*PFN_TDNF_PLUGIN_CLOSE_HANDLE)(
    PTDNF_PLUGIN_HANDLE pHandle
    );

typedef struct _TDNF_PLUGIN_INTERFACE_
{
    PFN_TDNF_PLUGIN_INITIALIZE          pFnInitialize;
    PFN_TDNF_PLUGIN_EVENTS_NEEDED       pFnEventsNeeded;
    PFN_TDNF_PLUGIN_GET_ERROR_STRING    pFnGetErrorString;
    PFN_TDNF_PLUGIN_EVENT               pFnEvent;
    PFN_TDNF_PLUGIN_CLOSE_HANDLE        pFnCloseHandle;
} TDNF_PLUGIN_INTERFACE, *PTDNF_PLUGIN_INTERFACE;

/*
 * Plugins should implement this function with
 * the exact function name "TDNFPluginLoadInterface"
*/
typedef uint32_t
(*PFN_TDNF_PLUGIN_LOAD_INTERFACE)(
    PTDNF_PLUGIN_INTERFACE pInterface
    );

/* operations on plugin context data */
uint32_t
TDNFEventContextGetItemInt(
    PTDNF_EVENT_CONTEXT pContext,
    const char *pcszName,
    int32_t *pnInt
    );

uint32_t
TDNFEventContextGetItemString(
    PTDNF_EVENT_CONTEXT pContext,
    const char *pcszName,
    const char **pcszString
    );

uint32_t
TDNFEventContextGetItemPtr(
    PTDNF_EVENT_CONTEXT pContext,
    const char *pcszName,
    const void **pPtr
    );

/* plugin error reporting */
uint32_t
TDNFGetPluginErrorString(
    PTDNF pTdnf,
    PTDNF_PLUGIN pPlugin,
    uint32_t nErrorCode,
    char **ppszError
    );

#ifdef __cplusplus
}
#endif

#endif//TDNF_PLUGIN_H_
