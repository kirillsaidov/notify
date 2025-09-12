#include <stdio.h>
#include <stdlib.h>

// mkdir
#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define mkdir(path, mode) _mkdir(path)
    #define STAT _stat
    #define ACCESS _access
    #define F_OK 0
#else // unix & macos
    #include <sys/stat.h>
    #include <sys/types.h>
    #define STAT stat
    #define ACCESS access
#endif

#include <sys/stat.h>

// colors
#define COLOR_BOLD_RED "\033[1;31m"
#define COLOR_BOLD_GREEN "\033[1;32m"
#define COLOR_BOLD_YELLOW "\033[1;33m"
#define COLOR_RESET "\033[0m"

// colored output
#define LOG_ERR(fmt, ...) printf(COLOR_BOLD_RED "ERR: " COLOR_RESET fmt "\n", ##__VA_ARGS__)
#define LOG_MSG(fmt, ...) printf(COLOR_BOLD_GREEN "MSG: " COLOR_RESET fmt "\n", ##__VA_ARGS__)
#define LOG_CMD(fmt, ...) printf(COLOR_BOLD_YELLOW "CMD: " COLOR_RESET fmt "\n", ##__VA_ARGS__)

// build metadata
#define SOURCE_FILES "main.c"
#ifdef _WIN32
    #define BUILDER_BINARY_NAME "build.exe"
    #define BUILDER_BINARY_NAME_TEMP "build_tmp.exe"
#else
    #define BUILDER_BINARY_NAME "build"
    #define BUILDER_BINARY_NAME_TEMP "build_tmp"
#endif
#define CMD_REBUILD_THIS "gcc build.c -o " BUILDER_BINARY_NAME_TEMP

// define GCC command
#ifdef _WIN32
    #define CMD "gcc -O2 " SOURCE_FILES " -o bin/notify.exe -lwinmm -lole32 -lcomctl32 -Wno-unused-result"
#elif defined(__APPLE__)
    #define CMD "gcc -O2 " SOURCE_FILES " -o bin/notify -framework AudioToolbox -framework CoreFoundation -Wno-unused-result"
#else // unix
    #define CMD "gcc -O2 " SOURCE_FILES " -o bin/notify -Wno-unused-result"
#endif

/// @brief Initializes windows support for terminal colors.
void init_term_colors(void) {
    #ifdef _WIN32
        // Only Windows needs special initialization
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode = 0;
        GetConsoleMode(hConsole, &mode);
        SetConsoleMode(hConsole, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    #endif
}

// /// @brief Performs a few checks upon rebuild.
// void init_this_build(void) {
//     if (ACCESS(BUILDER_BINARY_NAME_TEMP, F_OK) == 0 && rename(BUILDER_BINARY_NAME_TEMP, BUILDER_BINARY_NAME) != 0) {
        
//     }
// }

/// @brief Returns file modification time.
/// @param filename path to file
/// @return modification time upon success, otherwise -1 upon failure
time_t get_fmod_time(const char *filename) {
    struct STAT filestat;
    if (STAT(filename, &filestat) != 0) {
        return -1; // failed to access file
    }
    return filestat.st_mtime;
}

/// @brief Checks if it should rebuild the build script.
/// @param source source file
/// @param binary binary file
/// @return 1 - should build, 0 - no need to build, -1 - error reading files
int should_rebuild(const char *source, const char *binary) {
    if (ACCESS(binary, F_OK) != 0) return 1;  // binary does not exist, build it
    if (ACCESS(source, F_OK) != 0) return -1; // error: source does not exist, cannot build

    // get modification time
    const time_t smod_time = get_fmod_time(source);
    const time_t bmod_time = get_fmod_time(binary);
    if (smod_time < 0 || bmod_time < 0) return -1; // error: cannot read files

    LOG_ERR("diff: %d", smod_time - bmod_time);
    return smod_time > bmod_time + 10; // 1 second threshold
}

int main(void) {
    // enable windows support for terminal colors
    init_term_colors();

    // TODO: 
    // // check if we need to rebuild the build script
    // const int do_rebuild = should_rebuild(__FILE__, BUILDER_BINARY_NAME);
    // if (do_rebuild < 0) {
    //     LOG_ERR("Cannot read this builder files!");
    //     return 1;
    // } else if (do_rebuild) {
    //     LOG_MSG("Build script was modified. Rebuilding...");
    //     LOG_CMD(CMD_REBUILD_THIS);
    //     if (system(CMD_REBUILD_THIS) != 0) {
    //         LOG_ERR("Failed to rebuild this build script!");
    //         LOG_MSG("Maybe run manually: " CMD_REBUILD_THIS);
    //         return 1;
    //     } else {
    //         return system(BUILDER_BINARY_NAME_TEMP);
    //     }
    // }

    // make directory for binary output
    LOG_CMD("mkdir bin");
    mkdir("bin", 0755);

    // build project
    LOG_CMD(CMD);
    if (system(CMD) != 0) {
        LOG_ERR("Failed to build the project!");
        return 1;
    }
    
    LOG_MSG("Saved binary to './bin'");
    return 0;
}
