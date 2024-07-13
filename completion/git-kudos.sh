#!/bin/bash

_git_kudos() {
	local cur OPTS
	COMPREPLY=()
	cur="${COMP_WORDS[COMP_CWORD]}"
	OPTS="-h --help -v --version -d --detailed -x --exclude"
	compopt -o bashdefault -o default
	COMPREPLY=( $(compgen -W "${OPTS[*]}" -- $cur) )
	return 0
}

complete -F _git_kudos git-kudos
