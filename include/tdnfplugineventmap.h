/*
 * Copyright (C) 2020-2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#ifndef _TDNF_PLUGIN_EVENT_MAP_H_
#define _TDNF_PLUGIN_EVENT_MAP_H_

#include "tdnfplugin.h"

#define TDNF_PLUGIN_EVENT_MAP_VERSION "1.0.0"

/*
 * get current plugin event map version. event maps
 * might add to existing definition or add new.
 * please see map below. Changes will be backward
 * compatible unless mentioned otherwise. So it
 * is enough to ensure >= a known good version.
*/
const char *
TDNFPluginGetEventMapVersion(
    );

#define TDNF_EVENT_ITEM_TDNF_HANDLE  "tdnf.handle"
#define TDNF_EVENT_ITEM_REPO_SECTION "repo.section"
#define TDNF_EVENT_ITEM_REPO_ID      "repo.id"
#define TDNF_EVENT_ITEM_REPO_DATADIR "repo.datadir"
#define TDNF_EVENT_ITEM_REPO_MD_URL  "repomd.url"
#define TDNF_EVENT_ITEM_REPO_MD_FILE "repomd.file"

typedef enum
{
    TDNF_EVENT_ITEM_TYPE_NONE,
    TDNF_EVENT_ITEM_TYPE_INT,
    TDNF_EVENT_ITEM_TYPE_STRING,
    TDNF_EVENT_ITEM_TYPE_PTR
}TDNF_EVENT_ITEM_TYPE;

typedef struct _TDNF_PLUGIN_EVENT_ITEM_
{
    TDNF_EVENT_ITEM_TYPE nType;
    const char *pcszName;
}TDNF_PLUGIN_EVENT_ITEM, *PTDNF_PLUGIN_EVENT_ITEM;

typedef struct _TDNF_PLUGIN_EVENT_MAP_
{
    int nItems;
    TDNF_PLUGIN_EVENT nEvent;
    PTDNF_PLUGIN_EVENT_ITEM pItems;
}TDNF_PLUGIN_EVENT_MAP, *PTDNF_PLUGIN_EVENT_MAP;

#define TDNF_PLUGIN_EVENT_MAPS \
{ \
    {\
        1,\
        MAKE_PLUGIN_EVENT(TDNF_PLUGIN_EVENT_TYPE_INIT, 0, 0),\
        {\
            {TDNF_EVENT_ITEM_TYPE_PTR, TDNF_EVENT_ITEM_TDNF_HANDLE}\
        }\
    }, \
    {\
        1,\
        MAKE_PLUGIN_EVENT(TDNF_PLUGIN_EVENT_TYPE_REPO,\
                          TDNF_PLUGIN_EVENT_STATE_READCONFIG,\
                          TDNF_PLUGIN_EVENT_PHASE_START),\
        {\
            {TDNF_EVENT_ITEM_TYPE_PTR,    TDNF_EVENT_ITEM_REPO_SECTION}\
        }\
    },\
    {\
        3,\
        MAKE_PLUGIN_EVENT(TDNF_PLUGIN_EVENT_TYPE_REPO_MD,\
                          TDNF_PLUGIN_EVENT_STATE_DOWNLOAD,\
                          TDNF_PLUGIN_EVENT_PHASE_END),\
        {\
            {TDNF_EVENT_ITEM_TYPE_STRING,    TDNF_EVENT_ITEM_REPO_ID},\
            {TDNF_EVENT_ITEM_TYPE_STRING,    TDNF_EVENT_ITEM_REPO_DATADIR},\
            {TDNF_EVENT_ITEM_TYPE_STRING,    TDNF_EVENT_ITEM_REPO_BASEURL},\
            {TDNF_EVENT_ITEM_TYPE_STRING,    TDNF_EVENT_ITEM_REPO_MD_FILE}\
        }\
    }\
};

#endif /* _TDNF_PLUGIN_EVENT_MAP_H_ */
