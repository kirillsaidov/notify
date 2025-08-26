#ifndef NOTIFY_H
#define NOTIFY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#if defined(_WIN32) || defined(_WIN64)
    #error "Unimplemented for Windows!"
    #define ACCESS _access
    #define F_OK 0
#elif defined(__APPLE__) || defined(__MACH__)
    #error "Unimplemented for MacOS!"
    #define ACCESS access
#else // unix
    #include <unistd.h>
    #define ACCESS access
#endif

#include <threads.h>

/**
 * Cross-platform bell function that plays system notification sound.
 */
extern int ntf_beep_system(void);

/**
 * Cross-platform bell function that plays system notification sound or custom WAV file.
 * @param audio_file Path to WAV audio file. If NULL or file doesn't exist, plays default system bell.
 * @returns 0 on success, -1 on failure
 */
extern int ntf_beep(const char *audio_file);

/**
 * Cross-platform notification function. Uses system notification center to push messages.
 * @param source notification source. Default: "App".
 * @param title title of notification. Default: "Notification".
 * @param message notification description. Default: "".
 * @returns 0 on success, -1 on failure
 */
extern int ntf_notify(const char *source, const char *title, const char *message);

/**
 * Function to parse time string and convert to total seconds.
 * @param time_str time string. Examples: 1h30m, 90m, 3600s, 2h15m10s.
 * @returns total seconds
 */
extern long ntf_parse_time(const char *time_str);

/**
 * Function to format seconds into "Xd Yh Zm Ws" format.
 * @param seconds total seconds
 * @param buffer buffer to save the result
 * @param buf_size buffer size
 */
extern void ntf_format_time(long seconds, char *const buffer, const size_t buf_size);

/**
 * Sleep function. Using threads.
 * @param msecs milliseconds
 */
extern void ntf_sleep(const long msecs);

/* ------------------------- */

#ifdef NOTIFY_IMPLEMENTATION

int ntf_beep_system(void) {
    #if defined(_WIN32) || defined(_WIN64)
    {
        #error "Unimplemented for Windows!"
    }
    #elif defined(__APPLE__) || defined(__MACH__)
    {
        #error "Unimplemented for MacOS!"
    }
    #else // unix
    {
        printf("\a");
        fflush(stdout);
    }
    #endif
    return 0;
}

int ntf_beep(const char *audio_file) {
    // play default bell character
    if (!audio_file || ACCESS(audio_file, F_OK) != 0) return ntf_beep_system();

    #if defined(_WIN32) || defined(_WIN64)
    {
        #error "Unimplemented for Windows!"
    }
    #elif defined(__APPLE__) || defined(__MACH__)
    {
        #error "Unimplemented for MacOS!"
    }
    #else // unix
    {
        // play custom file
        int result = -1;
        char command[1024] = {0};
        
        // try aplay first (ALSA)
        snprintf(command, sizeof(command), "aplay \"%s\" >/dev/null 2>&1 &", audio_file);
        result = system(command);
        
        // if failed, try paplay (PulseAudio)
        if (result != 0) {
            snprintf(command, sizeof(command), "paplay \"%s\" >/dev/null 2>&1 &", audio_file);
            result = system(command);
        }

        // if failed, try sox's play command
        if (result != 0) {
            snprintf(command, sizeof(command), "play \"%s\" 2>/dev/null", audio_file);
            result = system(command);
        }
        
        // if failed, try mplayer (if available)
        if (result != 0) {
            snprintf(command, sizeof(command), "mplayer \"%s\" >/dev/null 2>&1 &", audio_file);
            result = system(command);
        }
        
        // if all failed, try cvlc (VLC media player)
        if (result != 0) {
            snprintf(command, sizeof(command), "cvlc --play-and-exit \"%s\" >/dev/null 2>&1 &", audio_file);
            result = system(command);
        }
        
        // if all audio players failed, fall back to system beep
        if (result != 0) return ntf_beep_system();
        return 0;
    }
    #endif

    // indicate custom file failed
    return -1;  
}

int ntf_notify(const char *source, const char *title, const char *message) {
    // Handle NULL parameters with default values
    const char* safe_source = (source && *source) ? source : "Notify";
    const char* safe_title = (title && *title) ? title : "Notification";
    const char* safe_message = (message && *message) ? message : "";

    #if defined(_WIN32) || defined(_WIN64)
    {
        #error "Unimplemented for Windows!"
    }
    #elif defined(__APPLE__) || defined(__MACH__)
    {
        #error "Unimplemented for MacOS!"
    }
    #else // unix
    {
        // notify using notify-send
        char command[4096] = {0};
        int result = -1;
        if (ACCESS("/usr/bin/notify-send", F_OK) == 0) {
            snprintf(command, sizeof(command),
                "notify-send -a \"%s\" \"%s\" \"%s\" 2>/dev/null",
                safe_source, safe_title, safe_message
            );
            result = system(command); // unused return value
        }

        // check for error and fallback to printf
        if (result != 0) {
            printf("[%s] %s: %s\n", safe_source, safe_title, safe_message);
        }

        return result;
    }
    #endif
    return 0;
}

long ntf_parse_time(const char* time_str) {
    long total_seconds = 0;
    if (time_str == NULL || *time_str == '\0') {
        return total_seconds;
    }
    
    // copy to buffer to ensure zero-termination
    char buffer[64] = {0};
    strncpy(buffer, time_str, sizeof(buffer)-1);
    buffer[sizeof(buffer)-1] = '\0';

    char* ptr = buffer;
    while (*ptr) {
        char* end_ptr;
        long value = strtol(ptr, &end_ptr, 10);
        
        if (end_ptr == ptr || value < 0) {
            break; // no valid number found or negative value
        }
        
        // check for unit specifier
        ptr = end_ptr;
        switch (*ptr) {
            case 'h': case 'H':
                total_seconds += value * 3600;
                ptr++;
                break;
            case 'm': case 'M':
                total_seconds += value * 60;
                ptr++;
                break;
            case 's': case 'S':
                total_seconds += value;
                ptr++;
                break;
            default: // assume seconds if no unit specified
                total_seconds += value;
                break;
        }

        // skip any non-digit characters except unit specifiers
        while (*ptr && (*ptr < '0' || *ptr > '9')) ptr++;
    }

    return total_seconds;
}

void ntf_format_time(long seconds, char *const buffer, const size_t buf_size) {
    if (buffer == NULL || buf_size == 0) return;
    
    long days = seconds / (24 * 3600);
    seconds %= (24 * 3600);
    long hours = seconds / 3600;
    seconds %= 3600;
    long minutes = seconds / 60;
    seconds %= 60;

    snprintf(buffer, buf_size, "%*ldd %*ldh %*ldm %*lds", 2, days, 2, hours, 2, minutes, 2, seconds);
}

void ntf_sleep(const long msecs) {
    struct timespec ts = { .tv_sec = msecs / 1000, .tv_nsec = 0 };
    thrd_sleep(&ts, NULL);
}

#endif // NOTIFY_IMPLEMENTATION

#endif // NOTIFY_H