/*
 * Copyright (C) 2015-2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

static const char *help_msg =
 "Usage: tdnf [options] COMMAND\n\n"
 "common options:\n"
 "           [--assumeno]\n"
 "           [-y, --assumeyes]\n"
 "           [-C, --cacheonly]\n"
 "           [-c [config file]]\n"
 "           [--debugsolver]\n"
 "           [--disableexcludes]\n"
 "           [--disableplugin=<plugin_name>]\n"
 "           [--disablerepo=<repoid>]\n"
 "           [--downloaddir=<directory>]\n"
 "           [--downloadonly]\n"
 "           [--enablerepo=<repoid>]\n"
 "           [--enableplugin=<plugin_name>]\n"
 "           [--exclude [file1,file2,...]]\n"
 "           [--installroot [path]]\n"
 "           [--noautoremove]\n"
 "           [--nogpgcheck]\n"
 "           [--noplugins]\n"
 "           [-q, --quiet]\n"
 "           [--reboot-required]\n"
 "           [--refresh]\n"
 "           [--releasever RELEASEVER]\n"
 "           [--repo=<repoid>]\n"
 "           [--repofrompath=<repoid>,<path>]\n"
 "           [--repoid=<repoid>]\n"
 "           [--rpmverbosity [debug level name]]\n"
 "           [--security]\n"
 "           [--sec-severity CVSS_v3.0_Severity]\n"
 "           [--setopt SETOPTS]\n"
 "           [--skip-broken]\n"
 "           [--skipconflicts]\n"
 "           [--skipdigest]\n"
 "           [--skipsignature]\n"
 "           [--skipobsoletes]\n"
 "           [--snapshotexcluderepos=<repoid>[,<repoid2>,...]\n"
 "           [--snapshottime=<POSIX_time>]\n"
 "           [--testonly]\n"
 "           [--version]\n\n"
 "repoquery select options:\n"
 "           [--available]\n"
 "           [--duplicates]\n"
 "           [--extras]\n"
 "           [--file <file>]\n"
 "           [--installed]\n"
 "           [--whatdepends <capability1>[,<capability2>[..]]]\n"
 "           [--whatrequires <capability1>[,<capability2>[..]]]\n"
 "           [--whatenhances <capability1>[,<capability2>[..]]]\n"
 "           [--whatobsoletes <capability1>[,<capability2>[..]]]\n"
 "           [--whatprovides <capability1>[,<capability2>[..]]]\n"
 "           [--whatrecommends <capability1>[,<capability2>[..]]]\n"
 "           [--whatrequires <capability1>[,<capability2>[..]]]\n"
 "           [--whatsuggests <capability1>[,<capability2>[..]]]\n"
 "           [--whatsupplements <capability1>[,<capability2>[..]]]\n\n"
 "repoquery query options:\n"
 "           [--depends]\n"
 "           [--enhances]\n"
 "           [--list]\n"
 "           [--obsoletes]\n"
 "           [--provides]\n"
 "           [--recommends]\n"
 "           [--requires]\n"
 "           [--requires-pre]\n"
 "           [--suggests]\n"
 "           [--source]\n"
 "           [--supplements]\n\n"
 "reposync options:\n"
 "           [--arch=<arch> [--arch=<arch> [..]]\n"
 "           [--delete]\n"
 "           [--download-path=<directory>]\n"
 "           [--download-metadata]\n"
 "           [--gpgcheck]\n"
 "           [--metadata-path=<directory>]\n"
 "           [--newest-only]\n"
 "           [--norepopath]\n"
 "           [--source]\n"
 "           [--urls]\n\n"
 "List of Main Commands\n\n"
 "autoerase          same as 'autoremove'\n"
 "autoremove         Remove a package and its automatic dependencies or all auto installed packages\n"
 "check              Checks repositories for problems\n"
 "check-local        Checks local rpm folder for problems\n"
 "check-update       Check for available package upgrades\n"
 "clean              Remove cached data\n"
 "distro-sync        Synchronize installed packages to the latest available versions\n"
 "downgrade          Downgrade a package\n"
 "erase              Remove a package or packages from your system\n"
 "help               Display a helpful usage message\n"
 "history            History Commands\n"
 "info               Display details about a package or group of packages\n"
 "install            Install a package or packages on your system\n"
 "list               List a package or groups of packages\n"
 "makecache          Generate the metadata cache\n"
 "mark               Mark package(s)\n"
 "provides           same as 'whatprovides'\n"
 "whatprovides       Find what package provides the given value\n"
 "reinstall          Reinstall a package\n"
 "remove             Remove a package or packages from your system\n"
 "repolist           Display the configured software repositories\n"
 "repoquery          Query repositories\n"
 "reposync           Download all packages from one or more repositories to a directory\n"
 "search             Search package details for the given string\n"
 "update             Upgrade a package or packages on your system (same as 'upgrade')\n"
 "update-to          same as 'upgrade-to'\n"
 "updateinfo         Display advisories about packages\n"
 "upgrade            Upgrade a package or packages on your system\n"
 "upgrade-to         Upgrade a package on your system to the specified version\n"
 "\n"
 "Please refer to https://github.com/vmware/tdnf/wiki for documentation.";

void
TDNFCliShowUsage(
    void
    )
{
    pr_crit("You need to give some command\n");
    TDNFCliShowHelp();
}

void
TDNFCliShowHelp(
    void
    )
{
    pr_crit("%s\n", help_msg);
}

void
TDNFCliShowNoSuchCommand(
    const char *pszCmd
    )
{
    pr_crit("No such command: %s. Please use /usr/bin/tdnf --help\n",
            pszCmd ? pszCmd : "");
}

void
TDNFCliShowNoSuchOption(
    const char *pszOption
    )
{
    pr_crit("No such option: %s. Please use /usr/bin/tdnf --help\n",
            pszOption ? pszOption : "");
}

uint32_t
TDNFCliHelpCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    if (!pCmdArgs || !pContext)
    {
        return ERROR_TDNF_INVALID_PARAMETER;
    }

    TDNFCliShowHelp();

    return 0;
}
