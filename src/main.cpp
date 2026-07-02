#include "Scanner.h"

int main() {

    std::string pathName("C:\\Program Files"); // Path for Indexing
    std::string dbName("search_db");           // Out DB name

    int maxLinesLimit{ 50 };                   // Output Lines Limit
    Scanner sc(dbName,pathName,maxLinesLimit);


    int queryType{ 1 };                        // 0 LIKE %target_name
                                               // 1 LIKE %target_name%
    sc.MakeQuery(queryType);


    return 0;
}