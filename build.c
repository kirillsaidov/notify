#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define mkdir(path, mode) _mkdir(path)
    #define STAT _stat
    #define ACCESS _access
    #define F_OK 0
#else // unix & macos
    #include <unistd.h>
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
#define COMPILER_NAME "gcc"
#define SOURCE_FILES "main.c"
#define BUILDER_BINARY_NAME "build"
#define BUILDER_BINARY_NAME_OLD "build.old"
#define CMD_REBUILD_THIS COMPILER_NAME " build.c -o " BUILDER_BINARY_NAME

// define GCC command
#ifdef _WIN32
    #define CMD COMPILER_NAME " -O2 " SOURCE_FILES " -o bin/notify.exe -lwinmm -lole32 -lcomctl32 -Wno-unused-result"
#elif defined(__APPLE__)
    #define CMD COMPILER_NAME " -O2 " SOURCE_FILES " -o bin/notify -framework AudioToolbox -framework CoreFoundation -Wno-unused-result"
#else // unix
    #define CMD COMPILER_NAME " -O2 " SOURCE_FILES " -o bin/notify -Wno-unused-result"
#endif

/// @brief Rename file.
/// @param old_name ditto
/// @param new_name ditto
/// @return 0 upon success, -1 upon failure
/// @note Taken from tsoding/nob.h
int rename_file(const char *old_name, const char *new_name) {
    #ifdef _WIN32
        if (!MoveFileEx(old_name, new_name, MOVEFILE_REPLACE_EXISTING)) {
            return -1;
        }
    #else // unix & macos
        if (rename(old_name, new_name) != 0) {
            return -1;
        }
    #endif
    return 0;
}

/// @brief Returns file modification time.
/// @param filename path to file
/// @return modification time upon success, -1 upon failure
time_t get_fmod_time(const char *filename) {
    struct STAT filestat;
    if (STAT(filename, &filestat) != 0) {
        return -1; // failed to access file
    }
    return filestat.st_mtime;
}

/// @brief Checks if we need to rebuild. 
/// @param output_path binary path
/// @param input_paths source file paths
/// @param input_paths_count number of sources files to check
/// @return 1 - should build, 0 - no need to build, -1 - error reading files
/// @note Taken from tsoding/nob.h
int should_rebuild(const char *output_path, const char *input_paths[], const size_t input_paths_count)
{
    if (ACCESS(output_path, F_OK) != 0) return 1;  // output does not exist, build it

    #ifdef _WIN32
        BOOL bSuccess;

        // try opening output file
        HANDLE output_path_fd = CreateFile(output_path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
        if (output_path_fd == INVALID_HANDLE_VALUE) return -1; // error opening file

        // get output file times
        FILETIME output_path_time;
        bSuccess = GetFileTime(output_path_fd, NULL, NULL, &output_path_time);
        CloseHandle(output_path_fd);
        if (!bSuccess) return -1; // failed to read file times

        // compare output file times to source file times
        for (size_t i = 0; i < input_paths_count; ++i) {
            // open source file
            const char *input_path = input_paths[i];
            HANDLE input_path_fd = CreateFile(input_path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
            if (input_path_fd == INVALID_HANDLE_VALUE) return -1; // file does not exist
            
            // get source file times
            FILETIME input_path_time;
            bSuccess = GetFileTime(input_path_fd, NULL, NULL, &input_path_time);
            CloseHandle(input_path_fd);
            if (!bSuccess) return -1; // failed to read file times

            // NOTE: if even a single input_path is fresher than output_path that's 100% rebuild
            if (CompareFileTime(&input_path_time, &output_path_time) == 1) return 1;
        }
    #else
        const time_t output_path_time = get_fmod_time(output_path);
        for (size_t i = 0; i < input_paths_count; ++i) {
            const time_t input_path_time = get_fmod_time(input_paths[i]);
            // NOTE: if even a single input_path is fresher than output_path that's 100% rebuild
            if (input_path_time > output_path_time) return 1;
        }
    #endif

    return 0;
}

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
    
    // check if we need to rebuild the build script
    const int do_rebuild = should_rebuild(BUILDER_BINARY_NAME, (const char*[]){__FILE__}, 1);
    if (do_rebuild < 0) {
        LOG_ERR("Cannot read builder files!");
        return 1;
    } else if (do_rebuild) {
        LOG_MSG("Build script was modified. Rebuilding...");
        LOG_CMD("mv " BUILDER_BINARY_NAME " " BUILDER_BINARY_NAME_OLD);
        if (rename_file(BUILDER_BINARY_NAME, BUILDER_BINARY_NAME_OLD) != 0) {
            LOG_ERR("Failed to rename old binary.");
            LOG_ERR("Rebuild manually: " CMD_REBUILD_THIS);
            return 1;
        }

        // rebuild
        LOG_CMD(CMD_REBUILD_THIS);
        if (system(CMD_REBUILD_THIS) != 0) {
            LOG_ERR("Failed to rebuild this build script!");
            LOG_MSG("Maybe run manually: " CMD_REBUILD_THIS);
            return 1;
        }

        // start new build script
        return system(BUILDER_BINARY_NAME);
    }

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
