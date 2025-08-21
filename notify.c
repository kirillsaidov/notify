#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
    #include <mmsystem.h>
    #include <io.h>         
    #define ACCESS _access
    #define F_OK 0
#elif defined(__APPLE__)
    #include <CoreFoundation/CoreFoundation.h>
    #include <AudioToolbox/AudioToolbox.h>
    #include <unistd.h>  // required for access()
    #define ACCESS access
#else // unix
    #include <unistd.h>
    #define ACCESS access
#endif

/**
 * Cross-platform bell function that plays system notification sound or custom WAV file.
 * @param audio_file Path to WAV audio file. If NULL or file doesn't exist, plays default system bell.
 * @returns 0 on success, -1 on failure
 */
int bell(const char *audio_file);

/**
 * Cross-platform notification function. Uses system notification center to push messages.
 * @param source notification source. Default: "App".
 * @param title title of notification. Default: "Notification".
 * @param message notification description. Default: "".
 */
void notify(const char *source, const char *title, const char *message);

/**
 * Function to parse time string and convert to total seconds.
 * @param time_str time string. Examples: 1h30m, 90m, 3600s, 2h15m10s.
 * @returns total seconds
 */
long parse_time(const char *time_str);

/**
 * Function to format seconds into "Xd Yh Zm Ws" format.
 * @param seconds total seconds
 * @param buffer buffer to save the result
 * @param buf_size buffer size
 */
void format_time(long seconds, char *const buffer, const size_t buf_size);


int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage:\n");
        printf("    %s <time> <message>\n", argv[0]);
        printf("    %s [audio] <time> <message>\n", argv[0]);
        printf("Time format examples: 1h30m, 90m, 3600s, 2h15m10s\n");
        printf("Example: %s 5m \"Take a break!\"\n", argv[0]);
        return 1;
    }

    // Parse the audio argument
    const char *audio_file = isdigit(argv[1][0]) ? NULL : argv[1];
    const char *total_seconds_arg = argv[audio_file ? 2 : 1];

    // Parse the time argument
    long total_seconds = parse_time(total_seconds_arg);
    if (total_seconds <= 0) {
        printf("Invalid time format: %s\n", total_seconds_arg);
        printf("Use format like: 1h30m, 90m, 3600s, etc.\n");
        return 1;
    }

    // Combine the message parts
    char message[4096] = {0};  // Reduced size for safety
    size_t remaining_space = sizeof(message) - 1;
    for (int i = 2 + (audio_file ? 1 : 0); i < argc && remaining_space > 0; i++) {
        const size_t arg_len = strlen(argv[i]);
        const size_t copy_len = (arg_len < remaining_space) ? arg_len : remaining_space;
        
        strncat(message, argv[i], copy_len);
        remaining_space -= copy_len;
        
        if (i < argc - 1 && remaining_space > 0) {
            strcat(message, " ");
            remaining_space--;
        }
    }

    // Print
    char time_str[64] = {0};
    format_time(total_seconds, time_str, sizeof(time_str));
    printf("Starting timer for: %s\n", time_str);
    printf("Message: %s\n", message);

    // Countdown loop
    long remaining = total_seconds;
    while (remaining >= 0) {
        format_time(remaining, time_str, sizeof(time_str));

        // Clear line and print countdown
        printf("\r%-20s", time_str);  // Fixed width for clean display
        fflush(stdout);

        if (remaining == 0) {
            // Timer completed - show notification
            printf("\nTime's up!\n");
            bell(audio_file);
            notify("Notify", "Timer Complete", message);
            break;
        }

        // Sleep for 1 second
        #ifdef _WIN32
            Sleep(1000);
        #else
            sleep(1);
        #endif

        remaining--;
    }

    return 0;
}

void format_time(long seconds, char *const buffer, const size_t buf_size) {
    if (buffer == NULL || buf_size == 0) return;
    
    long days = seconds / (24 * 3600);
    seconds %= (24 * 3600);
    long hours = seconds / 3600;
    seconds %= 3600;
    long minutes = seconds / 60;
    seconds %= 60;

    snprintf(buffer, buf_size, "%*ldd %*ldh %*ldm %*lds", 2, days, 2, hours, 2, minutes, 2, seconds);
}

long parse_time(const char* time_str) {
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
            // No valid number found or negative value
            break;
        }
        
        ptr = end_ptr;

        // Check for unit specifier
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
            default:
                // Assume seconds if no unit specified
                total_seconds += value;
                break;
        }

        // Skip any non-digit characters except unit specifiers
        while (*ptr && (*ptr < '0' || *ptr > '9')) ptr++;
    }

    return total_seconds;
}

void notify(const char* source, const char* title, const char* message) {
    // Handle NULL parameters safely
    const char* safe_source = (source && *source) ? source : "App";
    const char* safe_title = (title && *title) ? title : "Notification";
    const char* safe_message = (message && *message) ? message : "";

    #ifdef _WIN32
        // Windows implementation using balloon tooltip
        NOTIFYICONDATA nid = {0};
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = GetConsoleWindow();
        nid.uID = 1;                                    // Add unique ID
        nid.uFlags = NIF_INFO | NIF_TIP | NIF_MESSAGE;
        nid.uCallbackMessage = WM_USER + 1;             // Add callback message
        nid.dwInfoFlags = NIIF_INFO;                    // Use standard info icon
        nid.uTimeout = 5000;                            // 5 second timeout

        // Use safe string copy functions
        strncpy_s(nid.szInfoTitle, sizeof(nid.szInfoTitle), safe_title, _TRUNCATE);
        strncpy_s(nid.szInfo, sizeof(nid.szInfo), safe_message, _TRUNCATE);
        strncpy_s(nid.szTip, sizeof(nid.szTip), safe_source, _TRUNCATE);

        Shell_NotifyIcon(NIM_ADD, &nid);
        Sleep(100);  // Brief delay
        Shell_NotifyIcon(NIM_DELETE, &nid);
    #elif defined(__APPLE__)
        // macOS implementation using AppleScript
        char script[2048];
        
        // Escape quotes in the strings to prevent script injection
        snprintf(script, sizeof(script),
            "osascript -e 'display notification \"%s\" with title \"%s\" subtitle \"%s\"' 2>/dev/null",
            safe_message, safe_title, safe_source
        );
        system(script);
    #else
        // Linux implementation using notify-send
        char command[2048];
        if (ACCESS("/usr/bin/notify-send", F_OK) == 0) {
            snprintf(command, sizeof(command),
                "notify-send -a \"%s\" \"%s\" \"%s\" 2>/dev/null",
                safe_source, safe_title, safe_message
            );
            system(command);
        } else {
            // Fallback for systems without notify-send
            printf("[%s] %s: %s\n", safe_source, safe_title, safe_message);
        }
    #endif
}

int bell(const char *audio_file) {
    // Check if audio_file is NULL or if file doesn't exist
    // Fall back to default system bell behavior
    if (audio_file == NULL || ACCESS(audio_file, F_OK) != 0) {
        #ifdef _WIN32
            MessageBeep(MB_OK);  // Always succeeds on Windows            
        #elif defined(__APPLE__)
            AudioServicesPlaySystemSound(kSystemSoundID_UserPreferredAlert);
        #else // Unix
            printf("\a");
            fflush(stdout);
            
            // Try system beep commands (fail silently if not available)
            system("beep 2>/dev/null || pactl play-sample bell-terminal 2>/dev/null");
        #endif
        return 0;
    }
    
    // At this point, audio_file is valid and exists - try to play it
    #ifdef _WIN32
        // Windows: Play custom WAV file using PlaySound
        if (PlaySoundA(audio_file, NULL, SND_FILENAME | SND_ASYNC) != 0) {
            return 0;  // Success
        }
        // If PlaySound fails, fall back to system bell
        MessageBeep(MB_OK);
    #elif defined(__APPLE__)
        // macOS: Play custom WAV file using AudioServicesCreateSystemSoundID
        CFURLRef soundURL = CFURLCreateFromFileSystemRepresentation(
            kCFAllocatorDefault,
            (const UInt8*)audio_file,
            strlen(audio_file),
            false
        );
        
        if (soundURL) {
            SystemSoundID soundID;
            OSStatus status = AudioServicesCreateSystemSoundID(soundURL, &soundID);
            CFRelease(soundURL);
            
            if (status == noErr) {
                AudioServicesPlaySystemSound(soundID);
                AudioServicesDisposeSystemSoundID(soundID);
                return 0;
            }
        }
        
        // Fall back to system bell
        AudioServicesPlaySystemSound(kSystemSoundID_UserPreferredAlert);        
    #else // Unix
        // Try multiple audio players for WAV files
        char command[1024];  // Increased buffer size
        int result = -1;
        
        // Try aplay (ALSA) - most common on Linux
        snprintf(command, sizeof(command), "aplay \"%s\" 2>/dev/null", audio_file);
        result = system(command);

        if (result != 0) {
            // Try paplay (PulseAudio)
            snprintf(command, sizeof(command), "paplay \"%s\" 2>/dev/null", audio_file);
            result = system(command);
        }
        
        if (result != 0) {
            // Try sox's play command
            snprintf(command, sizeof(command), "play \"%s\" 2>/dev/null", audio_file);
            result = system(command);
        }
        
        if (result == 0) {
            return 0;  // Success
        }
        
        // All custom audio attempts failed, fall back to system bell
        printf("\a");
        fflush(stdout);
        system("beep 2>/dev/null || pactl play-sample bell-terminal 2>/dev/null");
    #endif

    // Indicate custom file failed
    return -1;  
}


