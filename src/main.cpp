//--------------------------------------------------
// git-kudos
// main.cpp
// Date: 2024-06-29
// By Breno Cunha Queiroz
//--------------------------------------------------
#include "cmakeConfig.hpp"
#include <algorithm>
#include <array>
#include <cstdio>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#define popen _popen
#define pclose _pclose
#define DEV_NULL "NUL"
#else
#define DEV_NULL "/dev/null"
#endif

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
const std::string CURSOR_UP = "\033[A";

// Progress bar
constexpr size_t BAR_WIDTH = 50;

// Author information
std::string _thisAuthor;                        // Email of current git user
std::set<std::string> _authors;                 // Author emails
std::map<std::string, std::string> _authorName; // Map author email to each name

// Kudo options
bool _printFileBreakdown; // Print file breakdown for each author

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
    std::cout << "Usage: git-kudos [-d | --detailed] [<paths>] [-x | --exclude <paths-to-exclude>]\n";
    std::cout << "\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help                        Print this help message\n";
    std::cout << "  -v, --version                     Print version\n";
    std::cout << "  -d, --detailed                    Output detailed list of files\n";
    std::cout << "  -x, --exclude <paths-to-exclude>  Exclude specified paths\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "  git-kudos                                   Kudos for current path\n";
    std::cout << "  git-kudos include/menu/ file.txt data/*.js  Kudos for specified files and folders\n";
    std::cout << "  git-kudos out[A-C].csv                      Kudos for outA.csv outB.csv outC.csv\n";
    std::cout << "  git-kudos alg[15].rs                        Kudos for alg1.rs alg5.rs\n";
    std::cout << "  git-kudos **.{h,c,hpp,cpp}                  Kudos for by C/C++ files\n";
    std::cout << "  git-kudos src/**/test.js                    Kudos for text.js files inside src/\n";
    std::cout << "  git-kudos src/**/test/*.cpp                 Kudos for .cpp files inside test folders\n";
    std::cout << "  git-kudos src/**Renderer*.*                 Kudos for files that contain \"Renderer\"\n";
    std::cout << "  git-kudos src/**.{h,cpp} -x src/*/test/     Kudos for C++ files and exclude test folders\n";
    std::cout << "  git-kudos **.py -d                          Detailed kudos for .py files\n";
}

void printVersion() { std::cout << "git-kudos version v" << KUDOS_VERSION << std::endl; }

std::string runCommand(const std::string& command) {
    std::array<char, 512> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen((command + " 2> " + DEV_NULL).c_str(), "r"), pclose);

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

Kudos calcKudos(fs::path path) {
    Kudos kudos{};
    kudos.path = path;

    // Use git blame to get line-by-line author information
    std::string command = "cd " + path.parent_path().string() + " && git blame --line-porcelain " + path.filename().string();
    std::string output = runCommand(command);

    // Process git blame to update kudos
    std::istringstream stream(output);
    std::string line;
    std::string email;
    std::string authorName = "";
    while (std::getline(stream, line)) {
        // Get author name for next commit
        if (line.find("author ") == 0)
            authorName = line.substr(6);
        // Get author email for next commit
        if (line.find("author-mail <") == 0) {
            std::regex emailRegex("<(.*?)>");
            std::smatch match;
            if (std::regex_search(line, match, emailRegex)) {
                email = processEmail(match[1].str());
                kudos.authorFileLines[email][path]++;
                kudos.authorLines[email]++;
                kudos.totalLines++;

                if (_authors.find(email) == _authors.end()) {
                    _authors.insert(email);
                    _authorName[email] = authorName;
                }
            }
        }
    }

    return kudos;
}

size_t printProgressBar(size_t current, size_t total, fs::path file) {
    static size_t lastFileSize = 0;

    // Calculate progress
    float progress = static_cast<float>(current) / total;
    int pos = BAR_WIDTH * progress;

    // Prepare progress bar line
    std::ostringstream progressBar;
    progressBar << WHITE << "Processing [";
    for (int i = 0; i < BAR_WIDTH; ++i) {
        if (i <= pos)
            progressBar << GREEN << "+";
        else
            progressBar << RED << "-";
    }
    progressBar << WHITE << "] ";
    progressBar << CYAN << current << "/" << total << " files";
    progressBar << " " << GREEN << std::fixed << std::setprecision(2) << (progress * 100.0) << "%" << RESET;

    // Print progress bar
    std::cout << progressBar.str();

    // Clear previous file name
    std::cout << std::endl << "           ";
    for (size_t i = 0; i < lastFileSize; i++)
        std::cout << " ";
    // Print file name
    std::cout << "\r           " << WHITE << file.string() << "\r" << CURSOR_UP;

    // Calculate the number of spaces needed to clear both lines
    std::regex ansiRegex(R"(\x1b\[[0-9]*m)");
    size_t barLineWidth = std::regex_replace(progressBar.str(), ansiRegex, "").size();
    lastFileSize = file.string().size();
    size_t numSpaces = std::max(lastFileSize + 11, barLineWidth);

    std::cout.flush();

    return numSpaces;
}

void printKudos(const Kudos& kudos, size_t numFiles, bool detailedPrint) {
    // Pretty number print
    auto print = [](size_t num, std::string label) -> std::string { return std::to_string(num) + " " + label + (num != 1 ? "s" : ""); };

    // Print path
    std::cout << BOLD + YELLOW + UNDERLINE << "Kudos for " << print(numFiles, "file");
    std::cout << RESET + WHITE + " " << print(kudos.totalLines, "line") + RESET;
    std::cout << std::endl;

    // Calculate sorted authors
    auto sortedAuthors = kudos.calcSortedAuthors();

    // Print author kudos
    for (const std::string& author : sortedAuthors) {
        size_t lines = kudos.authorLines.at(author);
        std::cout << "    ";
        std::cout << BOLD + BLUE + _authorName[author];                                                                           // Print name
        std::cout << RESET + WHITE + " " << print(lines, "line");                                                                 // Print lines
        std::cout << RESET + GREEN + " (" << std::fixed << std::setprecision(2) << lines / (float)kudos.totalLines * 100 << "%)"; // Print percentage
        std::cout << std::endl;

        if (detailedPrint) {
            auto sortedFiles = kudos.calcSortedFiles(author);
            for (const fs::path& file : sortedFiles) {
                size_t numLines = kudos.authorFileLines.at(author).at(file);
                std::cout << "        ";
                std::cout << RESET + CYAN + file.string();                    // Print file
                std::cout << RESET + WHITE << " " << print(numLines, "line"); // Print lines
                std::cout << std::endl;
            }
        }
        std::cout << RESET;
    }
}

int main(int argc, char* argv[]) {
    std::vector<fs::path> paths;         // Paths to process
    std::vector<fs::path> excludedPaths; // Paths to exclude
    bool isDetailedPrint = false;
    bool hasExcludeFlag = false;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.empty())
            continue;
        if (arg == "--help" || arg == "-h") {
            printHelp();
            return 0;
        } else if (arg == "--version" || arg == "-v") {
            printVersion();
            return 0;
        } else if (arg == "--detailed" || arg == "-d") {
            isDetailedPrint = true;
        } else if (arg.find("--exclude") != std::string::npos || arg.find("-x") != std::string::npos) {
            hasExcludeFlag = true;
        } else {
            if (arg[0] == '-') {
                // Warn unknown option
                std::cerr << "Unknown option " << arg << std::endl;
            } else if (fs::exists(arg)) {
                // Add path to be processed
                if (!hasExcludeFlag)
                    paths.push_back(arg);
                else
                    excludedPaths.push_back(arg);
            } else {
                // Warn unknown path
                std::cerr << "Unknown path " << arg << std::endl;
            }
        }
    }

    // Add current path if no path specified
    if (paths.empty())
        paths.push_back(".");

    // List all input files
    std::vector<fs::path> allFiles;
    for (const auto& path : paths) {
        if (fs::is_directory(path)) {
            // Recursively add files
            for (const auto& entry : fs::recursive_directory_iterator(path))
                if (!fs::is_directory(entry.path()))
                    allFiles.push_back(entry.path());
        } else
            allFiles.push_back(path);
    }

    // Exclude files
    std::vector<fs::path> filesToProcess;
    for (const auto& file : allFiles) {
        // Ignore .git folder
        if (file.string().find(".git") != std::string::npos)
            continue;
        // Skip excluded paths
        if (std::any_of(excludedPaths.begin(), excludedPaths.end(),
                        [&file](const fs::path& excludedPath) { return file.string().find(excludedPath.string()) != std::string::npos; }))
            continue;
        filesToProcess.push_back(file);
    }
    std::sort(filesToProcess.begin(), filesToProcess.end());

    // Get email of current git user (necessary if change not commited yet)
    _thisAuthor = runCommand("git config user.email");
    if (!_thisAuthor.empty())
        _thisAuthor.erase(_thisAuthor.find_last_not_of(" \n\r\t") + 1);

    // Compute kudos
    Kudos kudos{};
    size_t progress = 1;
    size_t clearWidth = 0;
    for (const auto& file : filesToProcess) {
        // Print progress
        clearWidth = printProgressBar(progress++, filesToProcess.size(), file);
        // Add file kudos
        kudos += calcKudos(file);
    }

    // Clear progress bar
    for (size_t l = 0; l < 2; l++) {
        for (size_t i = 0; i < clearWidth; i++)
            std::cout << " ";
        std::cout << std::endl;
    }
    std::cout << "\r" << CURSOR_UP << CURSOR_UP;

    // Print kudos
    printKudos(kudos, filesToProcess.size(), isDetailedPrint);

    return 0;
}
