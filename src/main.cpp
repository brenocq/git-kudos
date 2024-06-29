//--------------------------------------------------
// git-kudos
// main.cpp
// Date: 2024-06-29
// By Breno Cunha Queiroz
//--------------------------------------------------
#include "cmakeConfig.hpp"
#include <array>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace fs = std::filesystem;

fs::path _repo = {};

void printHelp() {
    std::cout << "Usage: git-kudos [-h | --help] [-v | --version] [<repo>]\n";
    std::cout << "\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help           Print this help message\n";
    std::cout << "  -v, --version        Print version\n";
    std::cout << "  --list-authors       List authors in repo\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "  git-kudos --list-authors <repo>\n";
}

void printVersion() { std::cout << "git-kudos version " << KUDOS_VERSION << std::endl; }

std::string runCommand(const std::string& command) {
    std::array<char, 512> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(("sh -c \"" + command + " 2>/dev/null\"").c_str(), "r"), pclose);

    if (!pipe) {
        std::cerr << "popen() failed!" << std::endl;
        return "";
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        result += buffer.data();

    return result;
}

void printListAuthors() {
    std::string gitAuthors = runCommand("git log --format='%aN <%aE>' | sort -u");
    std::cout << gitAuthors << std::endl;
}

int main(int argc, char* argv[]) {
    bool listAuthors = false;

    // Get current git repository
    std::string currRepo = runCommand("git rev-parse --show-toplevel");
    if (!currRepo.empty())
        _repo = currRepo;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            printHelp();
            return 0;
        } else if (arg == "--version" || arg == "-v") {
            printVersion();
            return 0;
        } else if (arg == "--list-authors") {
            listAuthors = true;
        } else if (i == argc - 1) {
            // Check if last provided command is a git repo
            fs::path repo = arg;
            if (fs::exists(repo) && fs::is_directory(repo) && fs::exists(repo / ".git")) {
                _repo = repo;
                // Change to specified git repo
                fs::current_path(_repo);
            } else {
                std::cerr << "Provided path " << repo << " is not a git repo" << std::endl;
                return 0;
            }
        } else {
            std::cerr << "Unknown option " << arg << std::endl;
        }
    }

    // Check if set to valid repo
    if (_repo.empty()) {
        std::cerr << "No git repo was detected or specified. You should run from inside a git repo or specify a repo" << std::endl;
        return 0;
    }

    if (listAuthors)
        printListAuthors();

    return 0;
}
