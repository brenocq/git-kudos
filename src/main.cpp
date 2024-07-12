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

// Progress bar
constexpr size_t BAR_WIDTH = 50;

// Author information
std::string _thisAuthor;                                      // Email of current git user
std::vector<std::string> _authors;                            // Sorted author emails
std::map<std::string, std::vector<std::string>> _authorNames; // Map author email to each name

// Kudo options
std::vector<std::string> _extensions; // Extensions to parse (parse all is empty)
std::vector<fs::path> _excludePaths;  // Paths to exclude
bool _printFileBreakdown;             // Print file breakdown for each author

struct Kudos {
    fs::path path;
    size_t totalLines = 0;
    std::map<std::string, size_t> authorLines;
    std::map<std::string, std::map<fs::path, size_t>> authorFileLines;

    std::vector<std::string> calcSortedAuthors() const {
        // Sort authors by descending number of lines
        std::vector<std::pair<std::string, size_t>> sortedAuthors(authorLines.begin(), authorLines.end());
        std::sort(sortedAuthors.begin(), sortedAuthors.end(), [](const auto& a, const auto& b) { return b.second < a.second; });
        // Return authors
        std::vector<std::string> result(sortedAuthors.size());
        for (size_t i = 0; i < result.size(); i++)
            result[i] = sortedAuthors[i].first;
        return result;
    }

    std::vector<fs::path> calcSortedFiles(std::string author) const {
        // Sort files by descending number of lines
        std::vector<std::pair<fs::path, size_t>> sortedFiles(authorFileLines.at(author).begin(), authorFileLines.at(author).end());
        std::sort(sortedFiles.begin(), sortedFiles.end(), [](const auto& a, const auto& b) { return b.second < a.second; });
        // Return files
        std::vector<fs::path> result(sortedFiles.size());
        for (size_t i = 0; i < result.size(); i++)
            result[i] = sortedFiles[i].first;
        return result;
    }

    void operator+=(const Kudos& kudos) {
        totalLines += kudos.totalLines;
        for (const auto& [author, lines] : kudos.authorLines)
            authorLines[author] += lines;
        for (const auto& [author, fileLines] : kudos.authorFileLines)
            for (const auto& [file, lines] : fileLines)
                authorFileLines[author][file] += lines;
    }
};

void printHelp() {
    std::cout << "Usage: git-kudos [options] [<paths>]\n";
    std::cout << "\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help                   Print this help message\n";
    std::cout << "  -v, --version                Print version\n";
    std::cout << "  --extensions=<extensions>    Filter kudos by file extensions (e.g., cpp,h,c)\n";
    std::cout << "  -d, --detailed               Detailed list of files that each author contributed to\n";
    std::cout << "  --exclude=<paths>            Exclude specified files/folders (comma separated)\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "  git-kudos                                       Kudos for current path\n";
    std::cout << "  git-kudos some/folder/ file.txt                 Kudos for selected paths\n";
    std::cout << "  git-kudos --extensions=hpp,cpp,h,c              Filter by C/C++ files\n";
    std::cout << "  git-kudos --extensions=py --detailed            Show file breakdown list for python files\n";
    std::cout << "  git-kudos --exclude=folder1,folder2,file.cpp    Exclude paths from search\n";
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

std::string processEmail(std::string email) {
    std::transform(email.begin(), email.end(), email.begin(), [](unsigned char c) { return std::tolower(c); });
    if (email == "not.committed.yet")
        return _thisAuthor;
    return email;
}

void processAuthors() {
    // Get email of current git user
    _thisAuthor = runCommand("git config user.email");
    _thisAuthor.resize(_thisAuthor.size() - 1); // Remove \n

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
            email = processEmail(email);

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

void parseExtensions(std::string extensions) {
    std::stringstream ss(extensions);
    std::string ext;
    while (std::getline(ss, ext, ','))
        _extensions.push_back("." + ext);
}

void parseExclusions(std::string exclusions) {
    std::stringstream ss(exclusions);
    std::string path;
    while (std::getline(ss, path, ','))
        _excludePaths.push_back(path);
}

Kudos calcKudos(fs::path path) {
    Kudos kudos{};
    kudos.path = path;

    // Use git blame to get line-by-line author information
    std::string command = "git blame --line-porcelain " + path.string();
    std::string output = runCommand(command);

    // Process git blame to update kudos
    std::istringstream stream(output);
    std::string line;
    std::string email;
    while (std::getline(stream, line)) {
        if (line.find("author-mail <") == 0) {
            std::regex emailRegex("<(.*?)>");
            std::smatch match;
            if (std::regex_search(line, match, emailRegex)) {
                email = processEmail(match[1].str());
                kudos.authorFileLines[email][path]++;
                kudos.authorLines[email]++;
                kudos.totalLines++;
            }
        }
    }

    return kudos;
}

void printProgressBar(size_t current, size_t total) {
    std::cout << WHITE + "Processing [";
    float progress = (current / (float)total);
    int pos = BAR_WIDTH * progress;
    for (int i = 0; i < BAR_WIDTH; ++i) {
        if (i <= pos)
            std::cout << GREEN << "+";
        else
            std::cout << RED << "-";
    }
    std::cout << WHITE + "] " + CYAN << current << "/" << total;
    std::cout << " " + GREEN << int(progress * 100.0) << "%\r";
    std::cout.flush();
}

void printKudos(const Kudos& kudos) {
    for (size_t i = 0; i < BAR_WIDTH + 30; i++)
        std::cout << " ";
    std::cout << "\r";
    if (kudos.path.empty())
        return;

    // Print path
    std::cout << BOLD + YELLOW + UNDERLINE + fs::absolute(kudos.path).string();
    std::cout << RESET + WHITE + " " << kudos.totalLines << " lines" + RESET;
    std::cout << std::endl;

    // Calculate sorted authors
    auto sortedAuthors = kudos.calcSortedAuthors();

    // Print author kudos
    for (const std::string& author : sortedAuthors) {
        size_t lines = kudos.authorLines.at(author);
        std::cout << "    ";
        std::cout << BOLD + BLUE + _authorNames[author][0];                                                                       // Print name
        std::cout << RESET + WHITE + " " << lines << " lines";                                                                    // Print lines
        std::cout << RESET + GREEN + " (" << std::fixed << std::setprecision(2) << lines / (float)kudos.totalLines * 100 << "%)"; // Print percentage
        std::cout << std::endl;

        if (_printFileBreakdown) {
            auto sortedFiles = kudos.calcSortedFiles(author);
            for (const fs::path& file : sortedFiles) {
                std::cout << "        ";
                std::cout << RESET + CYAN + file.string();                                                  // Print file
                std::cout << RESET + WHITE << " " << kudos.authorFileLines.at(author).at(file) << " lines"; // Print lines
                std::cout << std::endl;
            }
        }
        std::cout << RESET;
    }
}

int main(int argc, char* argv[]) {
    std::vector<fs::path> paths;
    _printFileBreakdown = false;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            printHelp();
            return 0;
        } else if (arg == "--version" || arg == "-v") {
            printVersion();
            return 0;
        } else if (arg.find("--extensions=") != std::string::npos) {
            parseExtensions(arg.substr(13));
        } else if (arg == "--detailed" || arg == "-d") {
            _printFileBreakdown = true;
        } else if (arg.find("--exclude=") != std::string::npos) {
            parseExclusions(arg.substr(10));
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

    for (const auto& path : paths) {
        // Compute files to process
        std::vector<fs::path> files;
        if (fs::is_directory(path)) {
            for (const auto& entry : fs::recursive_directory_iterator(path)) {
                // Ignore folders
                if (fs::is_directory(entry.path()))
                    continue;
                // Filter by extension if necessary
                if (!_extensions.empty())
                    if (!std::any_of(_extensions.begin(), _extensions.end(), [&entry](std::string ext) { return ext == entry.path().extension(); }))
                        continue;
                // Ignore .git folder
                if (entry.path().string().find(".git") != std::string::npos)
                    continue;
                // Ignore excluded paths
                if (std::any_of(_excludePaths.begin(), _excludePaths.end(), [&entry](const fs::path& excludePath) {
                        return entry.path().string().find(excludePath.string()) != std::string::npos;
                    }))
                    continue;
                files.push_back(entry.path());
            }
        } else
            files.push_back(path);

        // Compute kudos
        Kudos kudos{};
        kudos.path = path;
        size_t progress = 0;
        for (const auto& file : files) {
            // Print progress
            printProgressBar(progress++, files.size());
            // Add file kudos
            kudos += calcKudos(file);
        }

        // Print kudos
        printKudos(kudos);
    }

    return 0;
}

