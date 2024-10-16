_tdnf__process_if_prev_is_option()
{
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
        --downloaddir)
            COMPREPLY=( $(compgen -d -- $cur) )
            return 0
            ;;
        --enablerepo)
            opts=`tdnf repolist disabled | awk '{if (NR > 1) print $1}'`
            COMPREPLY=( $(compgen -W "$opts" -- $cur) )
            return 0
            ;;
        --disablerepo)
            opts=`tdnf repolist enabled | awk '{if (NR > 1) print $1}'`
            COMPREPLY=( $(compgen -W "$opts" -- $cur) )
            return 0
            ;;
        --snapshotexcluderepos)
            opts=`tdnf repolist enabled | awk '{if (NR > 1) print $1}'`
            COMPREPLY=( $(compgen -W "$opts" -- $cur) )
            return 0
            ;;
        --installroot)
            COMPREPLY=( $(compgen -d -- $cur) )
            return 0
            ;;
        --repo|repoid)
            opts=`tdnf repolist all | awk '{if (NR > 1) print $1}'`
            COMPREPLY=( $(compgen -W "$opts" -- $cur) )
            return 0
            ;;
        --rpmverbosity)
            opts="emergency alert critical error warning notice info debug"
            COMPREPLY=( $(compgen -W "$opts" -- $cur) )
            return 0
            ;;
    esac
    return 1
}

_tdnf__process_if_cmd()
{
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
            [ $1 -eq $(($COMP_CWORD - 1)) ] &&
                opts="packages metadata dbcache plugins expire-cache all"
            ;;
        downgrade)
            opts=$(tdnf list --downgrades | awk '{print $1}' | cut -d'.' -f1)
            ;;
        autoerase|autoremove|erase|reinstall|remove)
            opts=$(tdnf list --installed | awk '{print $1}' | cut -d'.' -f1)
            ;;
        history)
            [ $1 -eq $(($COMP_CWORD - 1)) ] &&
                opts="init update list rollback undo redo"
            ;;
        install)
            opts="$(tdnf list --available | awk '{print $1}' | cut -d'.' -f1) $(compgen -d -G '*.rpm')"
            ;;
        mark)
            [ $1 -eq $(($COMP_CWORD - 1)) ] &&
                opts="install remove"
            ;;
        repolist)
            [ $1 -eq $(($COMP_CWORD - 1)) ] &&
                opts="all enabled disabled"
            ;;
        update|upgrade)
            opts=$(tdnf list --upgrades | awk '{print $1}' | cut -d'.' -f1)
            ;;
    esac
    COMPREPLY=( $(compgen -W "$opts" -- $cur) )
    return 0
}

_tdnf()
{
    local c=0 cur __opts __cmds
    COMPREPLY=()
    __opts="--assumeno --assumeyes --cacheonly --debugsolver --disableexcludes --disableplugin --disablerepo --downloaddir --downloadonly --enablerepo --enableplugin --exclude --installroot --noautoremove --nogpgcheck --noplugins --quiet --reboot --refresh --releasever --repo --repofrompath --repoid --rpmverbosity --security --sec --setopt --skip --skipconflicts --skipdigest --skipsignature --skipobsoletes --snapshotexcluderepos --snapshottime --testonly --version --available --duplicates --extras --file --installed --whatdepends --whatrequires --whatenhances --whatobsoletes --whatprovides --whatrecommends --whatrequires --whatsuggests --whatsupplements --depends --enhances --list --obsoletes --provides --recommends --requires --requires --suggests --source --supplements --arch --delete --download --download --gpgcheck --metadata --newest --norepopath --source --urls"
    __cmds="autoerase autoremove check check-local check-update clean distro-sync downgrade erase help history info install list makecache mark provides whatprovides reinstall remove repolist repoquery reposync search update update-to updateinfo upgrade upgrade-to"
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
