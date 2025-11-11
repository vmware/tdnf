#!/bin/bash

#
# Copyright (C) 2020 VMware, Inc. All Rights Reserved.
# Copyright (C) 2024-2025, Broadcom, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

path="/var/cache/tdnf/cached-updateinfo.txt"
ref_file="/tmp/updateinfo_timestamp.$$"
tdnf_command="tdnf -q --refresh updateinfo"

update_interval="1 hour ago"
update_timeout="15s"

if [[ "$PAM_TYPE" == "close_session" ]]; then
    exit 0
fi

touch -d "$update_interval" "$ref_file"
if [ ! -s "${path}" ] || [ "$path" -ot "$ref_file" ]; then
    # /var/cache/tdnf may not exist, but we pipe the output to a file there
    mkdir -p "$(dirname ${path})"
    timeout "$update_timeout" $tdnf_command | grep -vE '^Refreshing|^Disabling' > "${path}"
    tdnf_status=${PIPESTATUS[0]}
    if [[ ${tdnf_status} == 124 ]]; then
        echo "tdnf updateinfo timed out"
        # remove any partial data
        rm -f "${path}"
        exit 0
    fi
    if [[ ${tdnf_status} != 0 ]]; then
        echo "tdnf updateinfo failed with ${tdnf_status}"
    fi
fi
rm -f "$ref_file"

if [ -s "${path}" ]; then
    grep -qE 'Security|Bugfix|Enhancement' "${path}" || exit 0
    echo
    cat "${path}"
    echo "Run 'tdnf updateinfo info' to see the details."
else
    echo "tdnf update info not available yet!"
fi
