//--------------------------------------------------
// git-kudos
// main.cpp
// Date: 2024-06-29
// By Breno Cunha Queiroz
//--------------------------------------------------
#include "cmakeConfig.hpp"
#include <iostream>
#include <vector>

void printHelp() {
    // clang-format off
    std::cout << "Usage: git-kudos [-h | --help] [-v | --version]" << std::endl;
    std::cout                                                      << std::endl;
    std::cout << "Options:"                                        << std::endl;
    std::cout << "  -h, --help         Print this help message"    << std::endl;
    std::cout << "  -v, --version      Print version"              << std::endl;
    // clang-format on
}

void printVersion() { std::cout << "git-kudos version " << KUDOS_VERSION << std::endl; }

int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            printHelp();
        } else if (arg == "--version" || arg == "-v") {
            printVersion();
        } else {
            std::cerr << "Unknown option " << arg << std::endl;
        }
    }

    return 0;
}
