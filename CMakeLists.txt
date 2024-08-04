cmake_minimum_required(VERSION 3.14)
project(git-kudos VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/cmakeConfig.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/src/cmakeConfig.hpp)

add_executable(git-kudos src/main.cpp)

#---------- Installation ----------#
include(GNUInstallDirs)

# Install executable
install(TARGETS git-kudos
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install include
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/cmakeConfig.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/git-kudos
)

# Install bash completion script
install(FILES ${CMAKE_SOURCE_DIR}/completion/git-kudos.sh
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/bash-completion/completions"
    RENAME git-kudos
)

# Install fish completion script
install(FILES ${CMAKE_SOURCE_DIR}/completion/git-kudos.fish
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/fish/completions"
    RENAME git-kudos.fish
)

# Install zsh completion script
install(FILES ${CMAKE_SOURCE_DIR}/completion/git-kudos.zsh
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/zsh/site-functions"
    RENAME _git-kudos
)
