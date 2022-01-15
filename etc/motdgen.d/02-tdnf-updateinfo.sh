#!/bin/bash

#
# Copyright (C) 2020-2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

path="/var/cache/tdnf/cached-updateinfo.txt"

if [ -s "${path}" ]; then
    grep -qE 'Security|Bugfix|Enhancement' "${path}" || exit 0
    echo; cat "${path}"; echo "Run 'tdnf updateinfo info' to see the details."
else
    echo "tdnf update info not available yet!"
fi
