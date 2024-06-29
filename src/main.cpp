//--------------------------------------------------
// git-kudos
// main.cpp
// Date: 2024-06-29
// By Breno Cunha Queiroz
//--------------------------------------------------
#include "cmakeConfig.hpp"
#include <algorithm>
#include <array>
#include <filesystem>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// ANSI color codes
const std::string RESET = "\033[0m";
const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string BLUE = "\033[34m";
const std::string MAGENTA = "\033[35m";
const std::string CYAN = "\033[36m";
const std::string WHITE = "\033[37m";
const std::string BOLD = "\033[1m";

fs::path _repo;
std::vector<std::string> _authors;                            // Sorted author emails
std::map<std::string, std::vector<std::string>> _authorNames; // Map author email to each name

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

void processAuthors() {
    // List repo authors
    std::string gitAuthors = runCommand("git log --format='%aN <%aE>' | sort -u");
    std::istringstream stream(gitAuthors);
    std::string line;

    // Reset authors
    _authors.clear();
    _authorNames.clear();

    // For each name-email pair, calculate map from author email to author names
    while (std::getline(stream, line)) {
        // Find the position of the email within angle brackets
        size_t emailStart = line.find('<');
        size_t emailEnd = line.find('>');

        if (emailStart != std::string::npos && emailEnd != std::string::npos) {
            // Extract name and email
            std::string name = line.substr(0, emailStart - 1);
            std::string email = line.substr(emailStart + 1, emailEnd - emailStart - 1);

            // Trim whitespace from name
            name.erase(name.find_last_not_of(" \n\r\t") + 1);

            // Convert email to lowercase for comparison
            std::transform(email.begin(), email.end(), email.begin(), [](unsigned char c) { return std::tolower(c); });

            // Map the email to the name
            _authorNames[email].push_back(name);

            // Save author name (always using first one)
            if (_authorNames[email].size() == 1)
                _authors.push_back(email);
        }
    }

    // Sort authors by name
    std::sort(_authors.begin(), _authors.end(), [](std::string a0, std::string a1) { return _authorNames[a0][0] < _authorNames[a1][0]; });
}

void printListAuthors() {
    for (const auto author : _authors)
        std::cout << std::string() + BOLD + WHITE + _authorNames[author][0] + RESET + WHITE + " <" + BLUE + author + WHITE + ">" << std::endl;
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

    // Process repo authors (merge contributions from same author with different names)
    processAuthors();

    if (listAuthors)
        printListAuthors();

    return 0;
}
