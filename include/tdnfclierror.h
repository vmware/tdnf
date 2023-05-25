/*
 * Copyright (C) 2017-2023 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#ifndef __TDNF_CLI_ERR_H__
#define __TDNF_CLI_ERR_H__

#define ERROR_TDNF_CLI_CHECK_UPDATES_AVAILABLE           100
#define ERROR_TDNF_CLI_BASE                              900
#define ERROR_TDNF_CLI_NO_MATCH                          (ERROR_TDNF_CLI_BASE + 1)
#define ERROR_TDNF_CLI_INVALID_ARGUMENT                  (ERROR_TDNF_CLI_BASE + 2)
#define ERROR_TDNF_CLI_CLEAN_REQUIRES_OPTION             (ERROR_TDNF_CLI_BASE + 3)
#define ERROR_TDNF_CLI_NOT_ENOUGH_ARGS                   (ERROR_TDNF_CLI_BASE + 4)
#define ERROR_TDNF_CLI_NOTHING_TO_DO                     (ERROR_TDNF_CLI_BASE + 5)
#define ERROR_TDNF_CLI_CHECKLOCAL_EXPECT_DIR             (ERROR_TDNF_CLI_BASE + 6)
#define ERROR_TDNF_CLI_PROVIDES_EXPECT_ARG               (ERROR_TDNF_CLI_BASE + 7)
#define ERROR_TDNF_CLI_OPTION_NAME_INVALID               (ERROR_TDNF_CLI_BASE + 8)
#define ERROR_TDNF_CLI_OPTION_ARG_REQUIRED               (ERROR_TDNF_CLI_BASE + 9)
#define ERROR_TDNF_CLI_OPTION_ARG_UNEXPECTED             (ERROR_TDNF_CLI_BASE + 10)
#define ERROR_TDNF_CLI_SETOPT_NO_EQUALS                  (ERROR_TDNF_CLI_BASE + 11)
#define ERROR_TDNF_CLI_NO_SUCH_CMD                       (ERROR_TDNF_CLI_BASE + 12)
#define ERROR_TDNF_CLI_DOWNLOADDIR_REQUIRES_DOWNLOADONLY (ERROR_TDNF_CLI_BASE + 13)
#define ERROR_TDNF_CLI_ONE_DEP_ONLY                      (ERROR_TDNF_CLI_BASE + 14)
#define ERROR_TDNF_CLI_ALLDEPS_REQUIRES_DOWNLOADONLY     (ERROR_TDNF_CLI_BASE + 15)
#define ERROR_TDNF_CLI_NODEPS_REQUIRES_DOWNLOADONLY      (ERROR_TDNF_CLI_BASE + 16)
#define ERROR_TDNF_CLI_INVALID_MIXED_QUERY_QUERYFORMAT   (ERROR_TDNF_CLI_BASE + 17)

#endif /* __TDNF_CLI_ERR_H__ */
