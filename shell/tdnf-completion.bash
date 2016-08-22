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
		--installroot) 
			COMPREPLY=( $(compgen -d -- $cur) )
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
			opts=`tdnf info downgrades | grep "^Name " | awk '{print $3}'`
			;;
		erase|reinstall|remove)
			opts=`tdnf info installed | grep "^Name " | awk '{print $3}'`
			;;
		install)
			opts=`tdnf info available | grep "^Name " | awk '{print $3}'`
			;;
		repolist)
			[ $1 -eq $(($COMP_CWORD - 1)) ] &&
				opts="all enabled disabled"
			;;
		upgrade)
			opts=`tdnf info upgrades | grep "^Name " | awk '{print $3}'`
			;;
	esac
	COMPREPLY=( $(compgen -W "$opts" -- $cur) )
	return 0
}

_tdnf()
{
	local c=0 cur __opts __cmds
	COMPREPLY=()
	__opts="--allowerasing --assumeno -y --assumeyes --best -C --cacheonly -c --config -d --debuglevel --debugsolver --disablerepo --enablerepo -e --errorlevel -h --help -i --installroot --nogpgcheck --refresh --rpmverbosity --setopt --showduplicates --version -v --verbose --releasever -4 -6"
	__cmds="autoremove autoupdate check-local check-update clean count distro-sync downgrade erase help info install list makecache provides whatprovides remove reinstall repolist search updateinfo upgrade upgrade-to update update-to"

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
