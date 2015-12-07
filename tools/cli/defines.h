/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Header : defines.h
 *
 * Abstract :
 *
 *            tdnf
 *
 *            command line tool
 *
 * Authors  : Priyesh Padmavilasom (ppadmavilasom@vmware.com)
 *
 */

#pragma once

#define ERROR_TDNF_CLI_BASE                    900
#define ERROR_TDNF_CLI_NO_MATCH                (ERROR_TDNF_CLI_BASE + 1)
#define ERROR_TDNF_CLI_INVALID_ARGUMENT        (ERROR_TDNF_CLI_BASE + 2)
#define ERROR_TDNF_CLI_CLEAN_REQUIRES_OPTION   (ERROR_TDNF_CLI_BASE + 3)
#define ERROR_TDNF_CLI_NOT_ENOUGH_ARGS         (ERROR_TDNF_CLI_BASE + 4)
#define ERROR_TDNF_CLI_NOTHING_TO_DO           (ERROR_TDNF_CLI_BASE + 5)
#define ERROR_TDNF_CLI_CHECKLOCAL_EXPECT_DIR   (ERROR_TDNF_CLI_BASE + 6)
#define ERROR_TDNF_CLI_PROVIDES_EXPECT_ARG     (ERROR_TDNF_CLI_BASE + 7)
#define ERROR_TDNF_CLI_OPTION_NAME_INVALID     (ERROR_TDNF_CLI_BASE + 8)
#define ERROR_TDNF_CLI_OPTION_ARG_REQUIRED     (ERROR_TDNF_CLI_BASE + 9)
#define ERROR_TDNF_CLI_OPTION_ARG_UNEXPECTED   (ERROR_TDNF_CLI_BASE + 10)
#define ERROR_TDNF_CLI_SETOPT_NO_EQUALS        (ERROR_TDNF_CLI_BASE + 11)


#define IsNullOrEmptyString(str) (!(str) || !(*str))

#define BAIL_ON_CLI_ERROR(unError) \
    if (unError)                                                   \
    {                                                              \
        goto error;                                                \
    }

#define TDNF_CLI_SAFE_FREE_MEMORY(pMemory) \
    if (pMemory) { \
    TDNFFreeMemory(pMemory); \
    }

#define TDNF_CLI_SAFE_FREE_STRINGARRAY(ppArray) \
    if (ppArray) { \
    TDNFFreeStringArray(ppArray); \
    }

typedef uint32_t (*PFN_CMD)(PTDNF, PTDNF_CMD_ARGS);

#define TDNF_CLI_ERROR_TABLE \
{ \
    {ERROR_TDNF_CLI_BASE,                    "ERROR_TDNF_CLI_BASE",                   "Generic base error."}, \
    {ERROR_TDNF_CLI_NO_MATCH,                "ERROR_TDNF_CLI_NO_MATCH",               "There was no match for the search."}, \
    {ERROR_TDNF_CLI_INVALID_ARGUMENT,        "ERROR_TDNF_CLI_INVALID_ARGUMENT",       "Invalid argument."}, \
    {ERROR_TDNF_CLI_CLEAN_REQUIRES_OPTION,   "ERROR_TDNF_CLI_CLEAN_REQUIRES_OPTION",  "Clean requires an option: packages, metadata, dbcache, plugins, expire-cache, all"}, \
    {ERROR_TDNF_CLI_NOT_ENOUGH_ARGS,         "ERROR_TDNF_CLI_NOT_ENOUGH_ARGS",        "The command line parser could not continue. Expected at least one argument."}, \
    {ERROR_TDNF_CLI_NOTHING_TO_DO,           "ERROR_TDNF_CLI_NOTHING_TO_DO",          "Nothing to do."}, \
    {ERROR_TDNF_CLI_OPTION_NAME_INVALID,     "ERROR_TDNF_CLI_OPTION_NAME_INVALID",    "Command line error: option is invalid."}, \
    {ERROR_TDNF_CLI_OPTION_ARG_REQUIRED,     "ERROR_TDNF_CLI_OPTION_ARG_REQUIRED",    "Command line error: expected one argument."}, \
    {ERROR_TDNF_CLI_OPTION_ARG_UNEXPECTED,   "ERROR_TDNF_CLI_OPTION_ARG_UNEXPECTED",  "Command line error: argument was unexpected."}, \
    {ERROR_TDNF_CLI_CHECKLOCAL_EXPECT_DIR,   "ERROR_TDNF_CLI_CHECKLOCAL_EXPECT_DIR",  "check-local requires path to rpm directory as a parameter"}, \
    {ERROR_TDNF_CLI_PROVIDES_EXPECT_ARG,     "ERROR_TDNF_CLI_PROVIDES_EXPECT_ARG",    "Need an item to match."}, \
    {ERROR_TDNF_CLI_SETOPT_NO_EQUALS,        "ERROR_TDNF_CLI_SETOPT_NO_EQUALS",       "Missing equal sign in setopt argument. setopt requires an argument of the form key=value."}, \
};
