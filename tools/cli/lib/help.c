/*
 * Copyright (C) 2015-2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

static const char *help_msg =
 "Usage: tdnf [options] COMMAND\n"
 "\nCommon Options\n"
 "--4, -4                    Resolve to IPv4 addresses only\n"
 "                           Example: tdnf -4 install package\n"

 "--6, -6                    Resolve to IPv6 addresses only\n"
 "                           Example: tdnf -6 install package\n"

 "--alldeps                  Download all dependencies (requires --downloadonly or --urls)\n"
 "                           Example: tdnf --downloadonly --alldeps install package\n"

 "--allowerasing             Allow erasing of installed packages to resolve dependencies\n"
 "                           Example: tdnf --allowerasing install pkg\n"

 "--assumeno                 Answer 'no' to all questions\n"
 "                           Example: tdnf --assumeno remove package\n"

 "-y, --assumeyes            Answer 'yes' to all questions\n"
 "                           Example: tdnf -y install package\n"

 "-b, --best                 Try the best available package versions\n"
 "                           Example: tdnf --best install package\n"

 "--builddeps                Install build dependencies\n"
 "                           Example: tdnf --builddeps install package\n"

 "-C, --cacheonly            Run entirely from cache, don't update\n"
 "                           Example: tdnf -C install package\n"

 "-c, --config <file>        Path to configuration file\n"
 "                           Example: tdnf -c /etc/tdnf/tdnf.conf install package\n"

 "-d, --debuglevel <level>   Debug output level (emergency|alert|critical|error|warning|notice|info|debug)\n"
 "                           Example: tdnf -d debug install package\n"

 "--debugsolver              Enable debug output for dependency solver\n"
 "                           Example: tdnf --debugsolver install pkg\n"

 "--disableexcludes          Disable excludes from tdnf.conf\n"
 "                           Example: tdnf --disableexcludes install\n"

 "--disableplugin <name>     Disable plugins by name\n"
 "                           Example: tdnf --disableplugin=versionlock install package\n"

 "--disablerepo <repoid>     Disable repositories by id\n"
 "                           Example: tdnf --disablerepo=updates install package\n"

 "--downloaddir <dir>        Directory to store downloaded packages (requires --downloadonly)\n"
 "                           Example: tdnf --downloadonly --downloaddir=/tmp/pkgs install\n"

 "--downloadonly             Download packages only, do not install\n"
 "                           Example: tdnf --downloadonly install pkg\n"
 "\n"
 "--urls                     Print package URLs only, do not download or install\n"
 "                           Example: tdnf --urls install pkg\n"

 "--enablerepo <repoid>      Enable repositories by id\n"
 "                           Example: tdnf --enablerepo=updates install\n"

 "--enableplugin <name>      Enable plugins by name\n"
 "                           Example: tdnf --enableplugin=versionlock install package\n"

 "--exclude <pkg1,pkg2,...>  Exclude packages by name\n"
 "                           Example: tdnf --exclude=linux* install\n"

 "--forcearch <arch>         Force architecture (x86_64|aarch64|noarch)\n"
 "                           Example: tdnf --forcearch=x86_64 install\n"

 "-h, --help                 Show this help message\n"
 "                           Example: tdnf --help\n"

 "-i, --installroot <path>   Set install root path (must be absolute)\n"
 "                           Example: tdnf -i /mnt/chroot install pkg\n"

 "-j, --json                 Output in JSON format\n"
 "                           Example: tdnf --json list installed\n"

 "--noautoremove             Do not remove automatically installed dependencies\n"
 "                           Example: tdnf --noautoremove remove pkg\n"

 "--nodeps                   Skip dependency checks (requires --downloadonly or --urls)\n"
 "                           Example: tdnf --downloadonly --nodeps install package\n"

 "--nogpgcheck               Skip GPG signature checks\n"
 "                           Example: tdnf --nogpgcheck install pkg\n"

 "--nocligpgcheck            Skip GPG signature checks for command line rpms\n"
 "                           Example: tdnf --nocligpgcheck install http://foo.bar.com/package.rpm\n"

 "--noplugins                Disable all plugins\n"
 "                           Example: tdnf --noplugins install package\n"

 "-q, --quiet                Quiet operation\n"
 "                           Example: tdnf -q install package\n"

 "--reboot-required          Check if reboot is required after transaction\n"
 "                           Example: tdnf --reboot-required install package\n"

 "--refresh                  Refresh repository metadata before operation\n"
 "                           Example: tdnf --refresh install package\n"

 "--releasever <version>     Set release version\n"
 "                           Example: tdnf --releasever=5.0 install pkg\n"

 "--repo <repoid>            Enable only this repository\n"
 "                           Example: tdnf --repo=base install pkg\n"

 "--repofromdir <id>,<dir>   Add repository from directory\n"
 "                           Example: tdnf --repofromdir=local,/path/to/repo install package\n"

 "--repofrompath <id>,<path> Add repository from path\n"
 "                           Example: tdnf --repofrompath=local,/mnt/repo install package\n"

 "--repoid <repoid>          Same as --repo\n"
 "                           Example: tdnf --repoid=base install pkg\n"

 "--rpmdefine <macro>        Define RPM macro\n"
 "                           Example: tdnf --rpmdefine='_dbpath <path>' install package\n"

 "--rpmverbosity <level>     RPM verbosity level (emergency|alert|critical|error|warning|notice|info|debug)\n"
 "                           Example: tdnf --rpmverbosity=debug install package\n"

 "--sec-severity <level>     Filter by security severity (Critical|Important|Moderate|Low)\n"
 "                           Example: tdnf updateinfo --sec-severity=Critical\n"

 "--security                 Only show security updates\n"
 "                           Example: tdnf --security update\n"

 "--setopt <option>=<value>  Set configuration option\n"
 "                           Example: tdnf --setopt=keepcache=1 install package\n"

 "--skip-broken              Skip packages with broken dependencies\n"
 "                           Example: tdnf --skip-broken install pkg\n"

 "--skipconflicts            Skip packages with conflicts\n"
 "                           Example: tdnf --skipconflicts install pkg\n"

 "--skipdigest               Skip package digest verification\n"
 "                           Example: tdnf --skipdigest install pkg\n"

 "--skipsignature            Skip package signature verification\n"
 "                           Example: tdnf --skipsignature install\n"

 "--skipobsoletes            Skip obsolete packages\n"
 "                           Example: tdnf --skipobsoletes install\n"

 "--source                   Operate on source packages\n"
 "                           Example: tdnf --source install package\n"

 "--testonly                 Run transaction in test mode only\n"
 "                           Example: tdnf --testonly install package\n"

 "-v, --verbose              Verbose operation\n"
 "                           Example: tdnf -v install package\n"

 "--version                  Show version information\n"
 "                           Example: tdnf --version\n"

 "\nRepoquery Select Options\n"
 "--available                Show only available packages\n"
 "                           Example: tdnf repoquery --available pkg\n"

 "--duplicates               Show duplicate packages\n"
 "                           Example: tdnf repoquery --duplicates\n"

 "--extras                   Show packages not in any repository\n"
 "                           Example: tdnf repoquery --extras\n"

 "--file <file>              Query packages that own the file\n"
 "                           Example: tdnf repoquery --file=/usr/bin/bash\n"

 "--installed                Show only installed packages\n"
 "                           Example: tdnf repoquery --installed pkg\n"

 "--userinstalled            Show only user-installed packages\n"
 "                           Example: tdnf repoquery --userinstalled\n"

 "--upgrades                 Show only upgradeable packages\n"
 "                           Example: tdnf repoquery --upgrades\n"

 "--downgrades               Show only downgradeable packages\n"
 "                           Example: tdnf repoquery --downgrades\n"

 "--whatconflicts <cap>      Find packages that conflict with capability\n"
 "                           Example: tdnf repoquery --whatconflicts=package\n"

 "--whatdepends <cap>        Find packages that depend on capability\n"
 "                           Example: tdnf repoquery --whatdepends=glibc\n"

 "--whatenhances <cap>       Find packages that enhance capability\n"
 "                           Example: tdnf repoquery --whatenhances=package\n"

 "--whatobsoletes <cap>      Find packages that obsolete capability\n"
 "                           Example: tdnf repoquery --whatobsoletes=package\n"

 "--whatprovides <cap>       Find packages that provide capability\n"
 "                           Example: tdnf repoquery --whatprovides=/usr/bin/bash\n"

 "--whatrecommends <cap>     Find packages that recommend capability\n"
 "                           Example: tdnf repoquery --whatrecommends=package\n"

 "--whatrequires <cap>       Find packages that require capability\n"
 "                           Example: tdnf repoquery --whatrequires=glibc\n"

 "--whatsuggests <cap>       Find packages that suggest capability\n"
 "                           Example: tdnf repoquery --whatsuggests=package\n"

 "--whatsupplements <cap>    Find packages that supplement capability\n"
 "                           Example: tdnf repoquery --whatsupplements=package\n"

 "\nRepoquery Query Options\n"
 "--changelogs               Show package changelogs\n"
 "                           Example: tdnf repoquery --changelogs pkg\n"

 "--conflicts                Show package conflicts\n"
 "                           Example: tdnf repoquery --conflicts pkg\n"

 "--depends                  Show package dependencies\n"
 "                           Example: tdnf repoquery --depends package\n"

 "--enhances                 Show packages enhanced by this package\n"
 "                           Example: tdnf repoquery --enhances pkg\n"

 "--list                     List files in package\n"
 "                           Example: tdnf repoquery --list package\n"

 "--location                 Show package location\n"
 "                           Example: tdnf repoquery --location pkg\n"

 "--obsoletes                Show packages obsoleted by this package\n"
 "                           Example: tdnf repoquery --obsoletes pkg\n"

 "--provides                 Show capabilities provided by package\n"
 "                           Example: tdnf repoquery --provides package\n"

 "--qf <fmt>                 Use custom query format\n"
 "                           Example: tdnf repoquery --qf='%{name}-%{version}' package\n"

 "--recommends               Show packages recommended by this package\n"
 "                           Example: tdnf repoquery --recommends pkg\n"

 "--requires                 Show package requirements\n"
 "                           Example: tdnf repoquery --requires pkg\n"

 "--requires-pre             Show pre-requisites\n"
 "                           Example: tdnf repoquery --requires-pre pkg\n"

 "--suggests                 Show packages suggested by this package\n"
 "                           Example: tdnf repoquery --suggests pkg\n"

 "--source                   Show source package\n"
 "                           Example: tdnf repoquery --source package\n"

 "--supplements              Show packages supplemented by this package\n"
 "                           Example: tdnf repoquery --supplements pkg\n"

 "\nReposync Options\n"
 "--arch <arch>              Sync packages for specific architecture (can be specified multiple times)\n"
 "                           Example: tdnf reposync --arch=x86_64 --arch=noarch\n"

 "--delete                   Delete local packages not in repository\n"
 "                           Example: tdnf reposync --delete\n"

 "--download-metadata        Download repository metadata\n"
 "                           Example: tdnf reposync --download-metadata\n"

 "--download-path <dir>      Directory to download packages to\n"
 "                           Example: tdnf reposync --download-path=/mnt/repo\n"

 "--gpgcheck                 Enable GPG signature checking\n"
 "                           Example: tdnf reposync --gpgcheck\n"

 "--metadata-path <dir>      Directory to store metadata\n"
 "                           Example: tdnf reposync --metadata-path=/mnt/repo/metadata\n"

 "--newest-only              Download only newest packages\n"
 "                           Example: tdnf reposync --newest-only\n"

 "--norepopath               Do not create repository directory structure\n"
 "                           Example: tdnf reposync --norepopath\n"

 "--source                   Sync source packages\n"
 "                           Example: tdnf reposync --source\n"

 "--urls                     Show URLs instead of downloading\n"
 "                           Example: tdnf reposync --urls\n"

 "\nList and Updateinfo Scope Options\n"
 "--all                      Show all packages/advisories\n"
 "                           Example: tdnf list --all\n"

 "--downgrades               Show downgradeable packages\n"
 "                           Example: tdnf list --downgrades\n"

 "--info                     Show detailed advisory information\n"
 "                           Example: tdnf updateinfo --info\n"

 "--recent                   Show recent packages/advisories\n"
 "                           Example: tdnf list --recent\n"

 "--summary                  Show advisory summary\n"
 "                           Example: tdnf updateinfo --summary\n"

 "--updates                  Show updateable packages\n"
 "                           Example: tdnf list --updates\n\n"

 "History Options\n"
 "--from <id>                Start from transaction ID\n"
 "                           Example: tdnf history list --from=10\n"

 "--reverse                  Show history in reverse order\n"
 "                           Example: tdnf history list --reverse\n"

 "--to <id>                  End at transaction ID\n"
 "                           Example: tdnf history list --to=20\n\n"

 "List of Main Commands\n"
 "autoerase                  Same as 'autoremove'\n"
 "                           Example: tdnf autoerase\n"

 "autoremove                 Remove automatically installed dependencies\n"
 "                           Example: tdnf autoremove\n"

 "check                      Check repositories for problems\n"
 "                           Example: tdnf check\n"

 "check-local <dir>          Check local RPM folder for problems\n"
 "                           Example: tdnf check-local /path/to/rpms\n"

 "check-update               Check for available package upgrades\n"
 "                           Example: tdnf check-update\n"

 "clean <type>               Remove cached data (packages|metadata|dbcache|plugins|expire-cache|all|expire-cache|all)\n"
 "                           Example: tdnf clean all\n"

 "count                      Count packages\n"
 "                           Example: tdnf count\n"

 "distro-sync                Synchronize installed packages to latest available versions\n"
 "                           Example: tdnf distro-sync\n"

 "downgrade <pkg>            Downgrade a package\n"
 "                           Example: tdnf downgrade package-1.0\n"

 "erase <pkg>                Remove a package or packages from your system\n"
 "                           Example: tdnf erase package\n"

 "help                       Display this help message\n"
 "                           Example: tdnf help\n"

 "history <cmd>              History commands (init|update|list|rollback|undo|redo)\n"
 "                           Example: tdnf history list\n"

 "info <pkg>                 Display details about a package or group of packages\n"
 "                           Example: tdnf info package\n"

 "install <pkg>              Install a package or packages on your system\n"
 "                           Example: tdnf install package\n"

 "list <pkg>                 List a package or groups of packages\n"
 "                           Example: tdnf list installed\n"

 "makecache                  Generate the metadata cache\n"
 "                           Example: tdnf makecache\n"

 "mark <action> <pkg>        Mark package(s) (install|remove)\n"
 "                           Example: tdnf mark install package\n"

 "provides <cap>             Same as 'whatprovides'\n"
 "                           Example: tdnf provides /usr/bin/bash\n"

 "whatprovides <cap>         Find what package provides the given value\n"
 "                           Example: tdnf whatprovides /usr/bin/bash\n"

 "reinstall <pkg>            Reinstall a package\n"
 "                           Example: tdnf reinstall package\n"

 "remove <pkg>               Remove a package or packages from your system\n"
 "                           Example: tdnf remove package\n"

 "repolist [filter]          Display the configured software repositories (all|enabled|disabled)\n"
 "                           Example: tdnf repolist all\n"

 "repoquery <pkg>            Query repositories\n"
 "                           Example: tdnf repoquery --list package\n"

 "reposync                   Download all packages from one or more repositories to a directory\n"
 "                           Example: tdnf reposync --download-path=/mnt/repo\n"

 "search <term>              Search package details for the given string\n"
 "                           Example: tdnf search bash\n"

 "update <pkg>               Upgrade a package or packages on your system (same as 'upgrade')\n"
 "                           Example: tdnf update package\n"

 "update-to <pkg-version>    Same as 'upgrade-to'\n"
 "                           Example: tdnf update-to package-1.0\n"

 "updateinfo [mode]          Display advisories about packages (all|info|summary)\n"
 "                           Example: tdnf updateinfo summary\n"

 "upgrade <pkg>              Upgrade a package or packages on your system\n"
 "                           Example: tdnf upgrade package\n"

 "upgrade-to <pkg-version>   Upgrade a package on your system to the specified version\n"
 "                           Example: tdnf upgrade-to package-1.0-1.ph5\n"
 "\nPlease refer to https://github.com/vmware/tdnf/wiki for documentation.";

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
