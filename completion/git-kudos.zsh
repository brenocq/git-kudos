#compdef git-kudos

_git-kudos() {
  local -a commands
  commands=(
    '-h[Print help message]'
    '--help[Print help message]'
    '-v[Print version]'
    '--version[Print version]'
    '-d[Output detailed list of files]'
    '--detailed[Output detailed list of files]'
    '-x[Exclude specified paths]:exclude paths'
    '--exclude[Exclude specified paths]:exclude paths'
  )
  _arguments -s $commands
}

compdef _git-kudos git-kudos
