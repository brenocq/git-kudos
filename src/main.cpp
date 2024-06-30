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
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <regex>
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
const std::string ITALIC = "\033[3m";
const std::string UNDERLINE = "\033[4m";

std::vector<std::string> _authors;                            // Sorted author emails
std::map<std::string, std::vector<std::string>> _authorNames; // Map author email to each name
std::vector<std::string> _extensions;                         // Extensions to parse (parse all is empty)

struct Kudos {
    fs::path path;
    size_t totalLines = 0;
    std::map<std::string, size_t> authorLines;

    std::vector<std::pair<std::string, size_t>> calcSortedAuthors() const {
        // Sort authors by descending number of lines
        std::vector<std::pair<std::string, size_t>> sortedAuthors(authorLines.begin(), authorLines.end());
        std::sort(sortedAuthors.begin(), sortedAuthors.end(), [](const auto& a, const auto& b) { return b.second < a.second; });
        // Remove authors without lines
        for (int i = sortedAuthors.size() - 1; i >= 0; i--)
            if (sortedAuthors[i].second == 0)
                sortedAuthors.erase(sortedAuthors.begin() + i);
        return sortedAuthors;
    }

    void operator+=(const Kudos& kudos) {
        totalLines += kudos.totalLines;
        for (const auto& [author, lines] : kudos.authorLines)
            authorLines[author] += lines;
    }
};

void printHelp() {
    std::cout << "Usage: git-kudos [options] [<repo>]\n";
    std::cout << "\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help                   Print this help message\n";
    std::cout << "  -v, --version                Print version\n";
    std::cout << "  --list-authors               List authors in repo\n";
    std::cout << "  --extensions=<extensions>    Filter kudos by file extensions (e.g., cpp,h,c)\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "  git-kudos\n";
    std::cout << "  git-kudos one/folder another/folder my/file.txt\n";
    std::cout << "  git-kudos --list-authors my/repo/path\n";
    std::cout << "  git-kudos --extensions=hpp,cpp,h,c\n";
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
        std::cout << std::string() + RESET + BOLD + BLUE + _authorNames[author][0] + RESET + WHITE + ITALIC + " <" + author + ">" << std::endl;
}

void parseExtensions(std::string extensions) {
    std::stringstream ss(extensions);
    std::string ext;
    while (std::getline(ss, ext, ','))
        _extensions.push_back("." + ext);
}

void printKudos(const Kudos& kudos) {
    if (kudos.path.empty())
        return;

    // Print path
    std::cout << BOLD + YELLOW + UNDERLINE + fs::absolute(kudos.path).string();
    std::cout << RESET + WHITE + " (" << kudos.totalLines << " lines)" + RESET;
    std::cout << std::endl;

    // Calculate sorted authors
    auto sortedAuthors = kudos.calcSortedAuthors();

    // Print author kudos
    for (const auto& [author, lines] : sortedAuthors) {
        std::cout << BOLD + BLUE + _authorNames[author][0];                               // Print name
        std::cout << RESET + WHITE + ": " << lines << " line" << (lines != 1 ? "s" : ""); // Print lines
        std::cout << RESET + GREEN + " (" << std::fixed << std::setprecision(2) << lines / (float)kudos.totalLines * 100 << "%)"
                  << std::endl; // Print percentage
    }
}

Kudos calcKudos(fs::path path) {
    Kudos kudos{};
    kudos.path = path;

    if (!fs::exists(path))
        return kudos;
    if (fs::is_directory(path)) {
        // Ignore .git folder
        if (path.string().find(".git") != std::string::npos)
            return kudos;

        for (const auto& entry : fs::directory_iterator(path))
            kudos += calcKudos(entry.path());
    } else {
        // Filter by extension if necessary
        if (!_extensions.empty())
            if (!std::any_of(_extensions.begin(), _extensions.end(), [&path](std::string ext) { return ext == path.extension(); }))
                return kudos;

        // Use git blame to get line-by-line author information
        std::string command = "git blame --line-porcelain " + path.string();
        std::string output = runCommand(command);

        std::istringstream stream(output);
        std::string line;
        std::string email;
        while (std::getline(stream, line)) {
            if (line.find("author-mail <") == 0) {
                std::regex emailRegex("<(.*?)>");
                std::smatch match;
                if (std::regex_search(line, match, emailRegex)) {
                    email = match[1].str();
                    std::transform(email.begin(), email.end(), email.begin(), [](unsigned char c) { return std::tolower(c); });
                    kudos.authorLines[email]++;
                    kudos.totalLines++;
                }
            }
        }
    }

    return kudos;
}

int main(int argc, char* argv[]) {
    std::vector<fs::path> paths;
    bool listAuthors = false;

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
        } else if (arg.find("--extensions=") != std::string::npos) {
            parseExtensions(arg.substr(13));
        } else {
            if (fs::exists(arg)) {
                paths.push_back(arg);
            } else
                std::cerr << "Unknown option " << arg << std::endl;
        }
    }

    // Add current path if no path specified
    if (paths.empty())
        paths.push_back(".");

    // Process repo authors (merge contributions from same author with different names)
    // TODO handle when paths are from different repos
    processAuthors();

    if (listAuthors)
        printListAuthors();
    else {
        for (const auto& path : paths) {
            // TODO check path is good
            Kudos kudos = calcKudos(path);
            printKudos(kudos);
        }
    }

    return 0;
}
