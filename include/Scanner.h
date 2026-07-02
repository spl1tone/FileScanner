#ifndef SCANNER_H
#define SCANNER_H

#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <Windows.h>
#include "sqlite3.h"

namespace fs = std::filesystem;

class Scanner {
public:
    Scanner(const std::string& db_name, const std::string& path_name, int maxLinesLimit = 50);
    Scanner(const Scanner&) = delete;
    Scanner& operator=(const Scanner&) = delete;
    ~Scanner();

    void MakeQuery(int queryType = 0);

private:
    sqlite3* db{ nullptr };
    std::string dbName;
    std::string pathName;
    int maxLinesLimit;

    sqlite3_stmt* stmtLikePrefix{ nullptr };
    sqlite3_stmt* stmtLikeContains{ nullptr };

    bool Init();
    bool CreateTable();
    bool PrepareStatements();
    void ScanPathAndSave(const std::string& pathName);
    void SearchFile(const std::string& targetName, std::vector<std::string>& paths, int queryType, int maxLimit);
    void OpenFile(const std::vector<std::string>& paths);
    void Close();
};

#endif // !SCANNER_H