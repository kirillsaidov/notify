#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define mkdir(path, mode) _mkdir(path)
#else // unix & macos
    #include <unistd.h>
    #include <sys/types.h>
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
#define COMPILER_NAME "gcc"
#define SOURCE_FILES "main.c"

// define build command
#ifdef _WIN32
    #define CMD COMPILER_NAME " -O2 " SOURCE_FILES " -o bin/notify.exe -lwinmm -lole32 -lcomctl32 -Wno-unused-result"
#elif defined(__APPLE__)
    #define CMD COMPILER_NAME " -O2 " SOURCE_FILES " -o bin/notify -framework AudioToolbox -framework CoreFoundation -Wno-unused-result"
#else // unix
    #define CMD COMPILER_NAME " -O2 " SOURCE_FILES " -o bin/notify -Wno-unused-result"
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

int main(void) {
    // enable windows support for terminal colors
    init_term_colors();

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
