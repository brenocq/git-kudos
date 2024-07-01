cmake_minimum_required(VERSION 3.14)
project(git-kudos VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/cmakeConfig.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/src/cmakeConfig.hpp)

add_executable(git-kudos src/main.cpp)

#---------- Installation ----------#
include(GNUInstallDirs)
install(TARGETS git-kudos
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/cmakeConfig.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/git-kudos
)
