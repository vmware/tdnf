_tdnf__process_if_prev_is_option() {
    local prev opts
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    case $prev in
        -c|--config)
            COMPREPLY=( $(compgen -f -- $cur) )
            return 0
            ;;
        -d|--debuglevel)
            opts="emergency alert critical error warning notice info debug"
            COMPREPLY=( $(compgen -W "$opts" -- $cur) )
            return 0
            ;;
        --downloaddir|--download-path)
            COMPREPLY=( $(compgen -d -- $cur) )
            return 0
            ;;
        --enablerepo)
            opts=$(tdnf repolist disabled 2>/dev/null | awk '{if (NR > 1) print $1}')
            COMPREPLY=( $(compgen -W "$opts" -- $cur) )
            return 0
            ;;
        --disablerepo)
            opts=$(tdnf repolist enabled 2>/dev/null | awk '{if (NR > 1) print $1}')
            COMPREPLY=( $(compgen -W "$opts" -- $cur) )
            return 0
            ;;
        -i|--installroot)
            COMPREPLY=( $(compgen -d -- $cur) )
            return 0
            ;;
        --repo|--repoid)
            opts=$(tdnf repolist all 2>/dev/null | awk '{if (NR > 1) print $1}')
            COMPREPLY=( $(compgen -W "$opts" -- $cur) )
            return 0
            ;;
        --rpmverbosity)
            opts="emergency alert critical error warning notice info debug"
            COMPREPLY=( $(compgen -W "$opts" -- $cur) )
            return 0
            ;;
        --forcearch)
            opts="x86_64 aarch64"
            COMPREPLY=( $(compgen -W "$opts" -- $cur) )
            return 0
            ;;
        --releasever)
            COMPREPLY=( $(compgen -W "4.0 5.0" -- $cur) )
            return 0
            ;;
        --sec-severity)
            opts="Critical Important Moderate Low"
            COMPREPLY=( $(compgen -W "$opts" -- $cur) )
            return 0
            ;;
        --exclude|--disableplugin|--enableplugin)
            COMPREPLY=( $(compgen -W "" -- $cur) )
            return 0
            ;;
        --repofrompath|--repofromdir)
            COMPREPLY=( $(compgen -d -- $cur) )
            return 0
            ;;
        --file)
            COMPREPLY=( $(compgen -f -- $cur) )
            return 0
            ;;
        --arch)
            opts="x86_64 aarch64 noarch"
            COMPREPLY=( $(compgen -W "$opts" -- $cur) )
            return 0
            ;;
        --metadata-path)
            COMPREPLY=( $(compgen -d -- $cur) )
            return 0
            ;;
        --whatdepends|--whatrequires|--whatprovides|--whatobsoletes|--whatconflicts|--whatrecommends|--whatsuggests|--whatsupplements|--whatenhances)
            opts=$(tdnf repoquery --qf=%{name} 2>/dev/null)
            COMPREPLY=( $(compgen -W "$opts" -- $cur) )
            return 0
            ;;
        --qf)
            COMPREPLY=( $(compgen -W "" -- $cur) )
            return 0
            ;;
        --to|--from)
            opts=$(tdnf history list | awk '{if (NR > 1) print $1}')
            COMPREPLY=( $(compgen -W "$opts" -- $cur) )
            return 0
            ;;
        --setopt)
            COMPREPLY=( $(compgen -W "" -- $cur) )
            return 0
            ;;
        --rpmdefine)
            COMPREPLY=( $(compgen -W "" -- $cur) )
            return 0
            ;;
    esac
    return 1
}

_tdnf__process_if_cmd() {
    local cmd opts
    cmd="${COMP_WORDS[$1]}"
    [[ " $__cmds " =~ " $cmd " ]] || return 1
    case $cmd in
        check-local)
            [ $1 -eq $(($COMP_CWORD - 1)) ] &&
                COMPREPLY=( $(compgen -d -- $cur) )
            return 0
            ;;
        clean)
            if [ $1 -eq $(($COMP_CWORD - 1)) ]; then
                opts="packages metadata dbcache plugins expire-cache all"
            else
                return 0
            fi
            ;;
        downgrade)
            opts=$(tdnf repoquery --downgrades --qf=%{name} 2>/dev/null)
            ;;
        autoerase|autoremove|erase|reinstall|remove)
            opts=$(tdnf repoquery --installed --qf=%{name} 2>/dev/null)
            ;;
        history)
            if [ $1 -eq $(($COMP_CWORD - 1)) ]; then
              opts="init update list rollback undo redo"
            else
              return 0
            fi
            ;;
        count)
            return 0
            ;;
        distro-sync)
            opts=$(tdnf repoquery --qf=%{name} 2>/dev/null)
            ;;
        install)
            opts=$(tdnf repoquery --qf=%{name})
            ;;
        mark)
            if [ $1 -eq $(($COMP_CWORD - 1)) ]; then
                opts="install remove"
            else
                return 0
            fi
            ;;
        repolist)
            if [ $1 -eq $(($COMP_CWORD - 1)) ]; then
                opts="all enabled disabled"
            else
                return 0
            fi
            ;;
        repoquery)
            # After repoquery, offer both repoquery-specific options and package names
            local repoquery_opts="--available --duplicates --extras --file --installed --userinstalled --upgrades --downgrades --whatconflicts --whatdepends --whatenhances --whatobsoletes --whatprovides --whatrecommends --whatrequires --whatsuggests --whatsupplements --changelogs --conflicts --depends --enhances --list --location --obsoletes --provides --qf --recommends --requires --requires-pre --suggests --supplements --source"
            local pkg_names=$(tdnf repoquery --qf=%{name} 2>/dev/null)
            opts="$repoquery_opts $pkg_names"
            ;;
        update|upgrade)
            opts=$(tdnf repoquery --upgrades --qf=%{name} 2>/dev/null)
            ;;
        check|help|makecache)
            # Commands that take no arguments
            return 0
            ;;
        check-update)
            # Optional package names
            opts="$(tdnf repoquery --qf=%{name} 2>/dev/null)"
            ;;
        info)
            # Package names or scope options
            local scope_opts="installed available updates downgrades recent all"
            local pkg_names="$(tdnf repoquery --qf=%{name} 2>/dev/null)"
            opts="$scope_opts $pkg_names"
            ;;
        list)
            # Scope options or package names
            local scope_opts="installed available updates downgrades recent all"
            local pkg_names="$(tdnf repoquery --qf=%{name} 2>/dev/null)"
            opts="$scope_opts $pkg_names"
            ;;
        provides|whatprovides)
            # Capability names (files, provides, etc.) - no easy way to list these
            # Just return 0 to allow free-form input
            return 0
            ;;
        reposync)
            # Optional repository names
            opts=$(tdnf repolist all 2>/dev/null | awk '{if (NR > 1) print $1}')
            ;;
        search)
            # Search terms - no easy way to list these, allow free-form input
            return 0
            ;;
        update-to|upgrade-to)
            # Package names (with optional version)
            opts="$(tdnf repoquery --qf=%{name} 2>/dev/null)"
            ;;
        updateinfo)
            # Mode options or package names
            local mode_opts="all info summary"
            local pkg_names="$(tdnf repoquery --qf=%{name} 2>/dev/null)"
            opts="$mode_opts $pkg_names"
            ;;
    esac
    COMPREPLY=( $(compgen -W "$opts" -- $cur) )
    return 0
}

_tdnf() {
    local c=0 cur __opts __cmds
    COMPREPLY=()
    __opts="--4 -4 --6 -6 --alldeps --allowerasing --assumeno -y --assumeyes -b --best --builddeps -C --cacheonly -c --config -d --debuglevel --debugsolver --disableexcludes --disableplugin --disablerepo --downloaddir --downloadonly --enablerepo --enableplugin --exclude --forcearch --help -h -i --installroot --json --noautoremove --nodeps --nogpgcheck --nocligpgcheck --noplugins -q --quiet --reboot-required --refresh --releasever --repo --repofromdir --repofrompath --repoid --rpmdefine --rpmverbosity --sec-severity --security --setopt --skip-broken --skipconflicts --skipdigest --skipsignature --skipobsoletes --source --testonly -v --verbose --version --available --duplicates --extras --file --installed --userinstalled --upgrades --downgrades --whatdepends --whatrequires --whatenhances --whatobsoletes --whatprovides --whatrecommends --whatsuggests --whatsupplements --whatconflicts --changelogs --conflicts --depends --enhances --list --location --obsoletes --provides --qf --recommends --requires --requires-pre --suggests --supplements --all --info --summary --recent --updates --downgrades --to --from --reverse --arch --delete --download-metadata --download-path --gpgcheck --metadata-path --newest-only --norepopath --urls"
    __cmds="autoerase autoremove check check-local check-update clean count distro-sync downgrade erase help history info install list makecache mark provides whatprovides reinstall remove repolist repoquery reposync search update update-to updateinfo upgrade upgrade-to"
    cur="${COMP_WORDS[COMP_CWORD]}"
    _tdnf__process_if_prev_is_option && return 0
    while [ $c -lt ${COMP_CWORD} ]; do
        _tdnf__process_if_cmd $((c++)) && return 0
    done

    # if command was not specified:
    # 1) time for [options], or
    # 2) command autocomplete?
    local opts
    [[ $cur == -* ]] && opts=$__opts || opts=$__cmds
    COMPREPLY=( $(compgen -W "$opts" -- $cur) )
    return 0
}
complete -F _tdnf -o default -o filenames tdnf

# vim: set et ts=4 sw=4 :
