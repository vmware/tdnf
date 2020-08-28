#
# Copyright (C) 2020 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

install(DIRECTORY DESTINATION "share/bash-completion/completions")
install(FILES "tdnf-completion.bash"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/bash-completion/completions"
    RENAME "tdnf"
    COMPONENT etc
)
