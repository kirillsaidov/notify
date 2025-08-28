#include <stdio.h>
#include <stdlib.h>

// mkdir
#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define mkdir(path, mode) _mkdir(path)
#else
    #include <sys/stat.h>
    #include <sys/types.h>
#endif

#define NOTIFY_SOURCE_FILE "main.c"

// Define GCC command
#ifdef _WIN32
    #define CMD "gcc -O2 -lwinmm " NOTIFY_SOURCE_FILE " -o bin/notify.exe -Wno-unused-result"
#elif defined(__APPLE__)
    #define CMD "gcc -O2 " NOTIFY_SOURCE_FILE " -o bin/notify -framework AudioToolbox -framework CoreFoundation -Wno-unused-result"
#else // unix
    #define CMD "gcc -O2 " NOTIFY_SOURCE_FILE " -o bin/notify -Wno-unused-result"
#endif

int main(void) {
    printf("CMD: mkdir bin\n");
    mkdir("bin", 0755);

    printf("CMD: build " NOTIFY_SOURCE_FILE "\n");
    system(CMD);
    printf("Saved to 'bin/notify'\n");
    return 0;
}
