#
# Copyright (C) 2020 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

SET (systemd_FILES
     tdnf-automatic.service
     tdnf-automatic.timer
     tdnf-automatic-notifyonly.service
     tdnf-automatic-notifyonly.timer
     tdnf-automatic-install.service
     tdnf-automatic-install.service
     tdnf-automatic-install.timer
     tdnf-cache-updateinfo.service
     tdnf-cache-updateinfo.timer)

INSTALL (FILES ${systemd_FILES} DESTINATION ${SYSTEMD_DIR})
