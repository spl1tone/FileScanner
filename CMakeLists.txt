cmake_minimum_required(VERSION 3.15)
project(FileScanner VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)

add_executable(scanner_app 
    src/main.cpp 
    src/Scanner.cpp
    src/sqlite3.c
)

if(UNIX)
    target_link_libraries(scanner_app PRIVATE pthread dl)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(scanner_app PRIVATE stdc++fs)
endif()