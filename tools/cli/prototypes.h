/*
      * Copyright (C) 2014-2015 VMware, Inc. All rights reserved.
      *
      * Header : prototypes.h
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

uint32_t
TDNFCliAutoEraseCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliCleanCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliCountCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliEraseCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliListCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliInfoCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliInstallCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliSearchCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliRepoListCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliUpgradeCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliDowngradeCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliReinstallCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliAlterCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs,
    TDNF_ALTERTYPE nType
    );

uint32_t
TDNFCliCheckLocalCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliCheckUpdateCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliMakeCacheCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliProvidesCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliUpdateInfoCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );
//help.c
void
TDNFCliShowUsage(
    );

void
TDNFCliShowHelp(
    );

void
TDNFCliShowOptionsUsage(
    );

void
TDNFCliShowNoSuchCommand(
    const char* pszCmd
    );

void
TDNFCliShowNoSuchOption(
    const char* pszOption
    );

uint32_t
TDNFCliHelpCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

//options.c
uint32_t
TDNFCliValidateOptionName(
    const char* pszOptionName,
    struct option* pKnownOptions
    );

uint32_t
TDNFCliValidateOptionArg(
    const char* pszOption,
    const char* pszArg,
    struct option* pKnownOptions
    );

uint32_t
TDNFCliValidateOptions(
    const char* pszOption,
    const char* pszArg,
    struct option* pKnownOptions
    );

//output.c
void
ShowConsoleProps(
    );

//parseargs.c
uint32_t
TDNFCliParseArgs(
    int argc,
    char* const* argv,
    PTDNF_CMD_ARGS* ppCmdArgs
    );

uint32_t
TDNFCliParsePackageArgs(
    PTDNF_CMD_ARGS pCmdArgs,
    char*** pppszPackageArgs,
    int* pnPackageCount
    );

//parsecleanargs.c
uint32_t
TDNFCliParseCleanArgs(
    PTDNF_CMD_ARGS pCmdArgs,
    TDNF_CLEANTYPE* pnCleanType
    );

//parselistargs.c
uint32_t
ParseScope(
    const char* pszScope,
    TDNF_SCOPE* pnScope
    );

uint32_t
TDNFCliParseListArgs(
    PTDNF_CMD_ARGS pCmdArgs,
    PTDNF_LIST_ARGS* ppListArgs
    );

uint32_t
TDNFCliParseInfoArgs(
    PTDNF_CMD_ARGS pCmdArgs,
    PTDNF_LIST_ARGS* ppListArgs
    );

uint32_t
TDNFCliParseRepoListArgs(
    PTDNF_CMD_ARGS pCmdArgs,
    TDNF_REPOLISTFILTER* pnFilter
    );

uint32_t
TDNFCliParseUpdateInfoArgs(
    PTDNF_CMD_ARGS pCmdArgs,
    PTDNF_UPDATEINFO_ARGS* ppUpdateInfoArgs
    );

void
TDNFFreeUpdateInfoArgs(
    PTDNF_UPDATEINFO_ARGS pUpdateInfoArgs
    );

void
TDNFFreeListArgs(
    PTDNF_LIST_ARGS pListArgs
    );
