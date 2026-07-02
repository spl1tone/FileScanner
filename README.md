# High-Performance File Scanner & Indexer

A fast, lightweight command-line utility designed for instant file discovery across your operating system, operating on principles similar to macOS Spotlight. The application performs a deep, recursive scan of a target directory, caches the results into a local SQLite database, and offers a real-time, interactive search interface.

## Key Features
* **Zero External Dependencies**: SQLite (`Amalgamation`) is embedded directly into the source tree, making the application fully autonomous and self-contained.
* **Optimized Indexing**: Features high-speed disk scanning optimized via atomic SQLite transactions (`BEGIN TRANSACTION` / `COMMIT`) and optimized `PRAGMA` directives, minimizing I/O bottlenecks.
* **Instant Queries**: Leverages precompiled SQLite statements (`sqlite3_stmt`) and robust B-Tree database indexing to retrieve matching entries from hundreds of thousands of records within microseconds.
* **Cross-Platform**: Full build and runtime support for both Windows (MSVC) and Linux (GCC/Clang).
* **OS Integration**: Allows you to launch selected files instantly via the host OS default handler or open the native file manager with the file pre-selected (`explorer /select` on Windows / `xdg-open` on Linux).
