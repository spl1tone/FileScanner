#include "Scanner.h"
#include <sstream>
#include <cstdlib>
#include <stdexcept>

Scanner::Scanner(const std::string& db_name, const std::string& path_name, int _maxLinesLimit)
    : dbName(db_name), pathName(path_name), maxLinesLimit(_maxLinesLimit)
{
    if (maxLinesLimit < 1) {
        throw std::invalid_argument("maxLinesLimit should be >= 1");
    }

    if (!Init()) {
        throw std::runtime_error("Failed to initialize Scanner");
    }
}

Scanner::~Scanner() {
    Close();
}

bool Scanner::CreateTable() {
    const char* createTableSQL =
        "CREATE TABLE IF NOT EXISTS files ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "file_name TEXT NOT NULL,"
        "full_path TEXT NOT NULL"
        ");"
        "CREATE INDEX IF NOT EXISTS idx_file_name ON files(file_name);";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL Error: " << errMsg << '\n';
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool Scanner::PrepareStatements() {

    std::stringstream ssPrefix, ssContains;
    ssPrefix << "SELECT full_path, file_name FROM files WHERE file_name LIKE ? LIMIT " << maxLinesLimit << ";";
    ssContains << "SELECT full_path, file_name FROM files WHERE file_name LIKE ? LIMIT " << maxLinesLimit << ";";

    if (sqlite3_prepare_v2(db, ssPrefix.str().c_str(), -1, &stmtLikePrefix, nullptr) != SQLITE_OK)     return false;
    if (sqlite3_prepare_v2(db, ssContains.str().c_str(), -1, &stmtLikeContains, nullptr) != SQLITE_OK) return false;

    return true;
}

void Scanner::SearchFile(const std::string& targetName, std::vector<std::string>& paths, int queryType, int maxLimit) {
    paths.clear();
    sqlite3_stmt* stmt = (queryType == 0) ? stmtLikePrefix : stmtLikeContains;

    std::string query = (queryType == 0) ? (targetName + "%") : ("%" + targetName + "%");

    sqlite3_bind_text(stmt, 1, query.c_str(), -1, SQLITE_TRANSIENT);

    if (queryType == 0) {
        std::cout << "\n--- Top " << maxLimit << " results starting with '" << targetName << "': --- \n";
    }
    else {
        std::cout << "\n--- Top " << maxLimit << " results containing '"    << targetName << "': ---\n";
    }

    int counter = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        counter++;

        const unsigned char* path = sqlite3_column_text(stmt, 0);
        const unsigned char* name = sqlite3_column_text(stmt, 1);

        paths.push_back(reinterpret_cast<const char*>(path));
        std::cout << counter << ". " << name << " : \n" << path << "\n\n";
    }

    if (counter == 0) {
        std::cout << "No files found.\n";
    }
    else {
        std::cout << "-------------------------------------------\n";
        std::cout << "Shown " << counter << " results.\n\n";
    }
    sqlite3_reset(stmt);
}

void Scanner::OpenFile(const std::vector<std::string>& paths) {
    if (paths.empty()) return;

    std::string input;
    std::getline(std::cin, input);

    int code;
    try {
        code = std::stoi(input);
    }
    catch (...) {
        std::cout << "Invalid input (enter a number)\n";
        return;
    }

    if (code == 0) return;

    if (code > static_cast<int>(paths.size()) || code < 0) {
        std::cout << "Wrong Option\n";
    }
    else {
        std::string path = paths[code - 1];
        std::cout << "Opening: " << path << '\n';

        std::string command = "explorer /select,\"" + path + "\"";
        system(command.c_str());

        ShellExecuteA(nullptr, "open", path.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    }
}

void Scanner::ScanPathAndSave(const std::string& pathName) {

    std::error_code ec;
    auto it = fs::recursive_directory_iterator(pathName, fs::directory_options::skip_permission_denied, ec);

    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    const char* insertSQL = "INSERT INTO files (file_name, full_path) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr);

    for (const auto& i : it) {
        if (ec) continue;

        if (fs::is_regular_file(i, ec)) {
            std::string fileName = i.path().filename().string();
            std::string fullPath = i.path().string();

            sqlite3_bind_text(stmt, 1, fileName.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, fullPath.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
            sqlite3_reset(stmt);
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
    std::cout << "Successfully indexed and saved to DB\n\n";
}

void Scanner::MakeQuery(int queryType) {
    if (queryType != 0 && queryType != 1) throw std::invalid_argument("Incorrect query type\n");

    std::string query;
    std::vector<std::string> paths;

    while (true) {
        std::cout << "\nEnter file name to search (or 'exit' to quit): ";
        std::getline(std::cin, query);

        if (query == "exit") break;
        if (query == "clear") {
            system("cls");
            continue;
        }
        if (query.empty()) continue;

        SearchFile(query, paths, queryType, maxLinesLimit);

        if (!paths.empty()) {
            std::cout << "\nSelect File To Open (0 - discard): ";
            OpenFile(paths);
        }
    }
}

bool Scanner::Init() {
    std::setlocale(LC_ALL, ".UTF-8");
    bool dbExists = fs::exists(dbName);

    if (sqlite3_open(dbName.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Can't open database\n";
        return false;
    }

    sqlite3_exec(db, "PRAGMA synchronous = OFF;", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "PRAGMA journal_mode = MEMORY;", nullptr, nullptr, nullptr);

    if (!dbExists) {
        std::cout << "Database not found. Starting initial indexing...\n\n";
        if (!CreateTable()) return false;

        std::cout << "Scanning " << pathName << "...\n";
        ScanPathAndSave(pathName);
    }
    else {
        std::cout << "Database loaded successfully (Contains cached index).\n\n";
    }

    return PrepareStatements();
}

void Scanner::Close() {
    if (stmtLikePrefix) sqlite3_finalize(stmtLikePrefix);
    if (stmtLikeContains) sqlite3_finalize(stmtLikeContains);
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}