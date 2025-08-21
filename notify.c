#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifdef _WIN32
    #include <windows.h>
    #include <wchar.h>
#elif defined(__APPLE__)
    #include <CoreFoundation/CoreFoundation.h>
    #include <AudioToolbox/AudioToolbox.h>
    #include <objc/objc.h>
    #include <objc/objc-runtime.h>
#else
    #include <unistd.h>
#endif

/**
 * Cross-platform bell function that plays system notification sound
 * Returns 0 on success, -1 on failure
 */
int bell() {
    #ifdef _WIN32
        // Windows: Use MessageBeep for system sound
        if (MessageBeep(MB_OK) == 0) {
            // If MessageBeep fails, try console bell as fallback
            printf("\a");
            fflush(stdout);
        }
        return 0;
        
    #elif defined(__APPLE__)
        // macOS: Use AudioServicesPlaySystemSound
        AudioServicesPlaySystemSound(kSystemSoundID_UserPreferredAlert);
        return 0;
        
    #elif defined(__linux__)
        // Linux: Try multiple approaches
        
        // Method 1: Try console bell character
        printf("\a");
        fflush(stdout);
        
        // Method 2: Try system beep command as fallback
        // This will work if beep is installed, fail silently otherwise
        int result = system("beep 2>/dev/null");
        if (result == -1) {
            // If system() fails, try pactl for PulseAudio
            system("pactl play-sample bell-terminal 2>/dev/null");
        }
        
        return 0;
        
    #else
        // Fallback for other Unix-like systems
        printf("\a");
        fflush(stdout);
        return 0;
    #endif
}

void notify(const char* source, const char* title, const char* message) {
    if (title == NULL) title = "";
    if (message == NULL) message = "";
    if (source == NULL) source = "App";

    #ifdef _WIN32
        // Windows implementation using WinAPI
        NOTIFYICONDATA nid = {0};
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = GetConsoleWindow();
        nid.uFlags = NIF_INFO | NIF_TIP;
        nid.dwInfoFlags = NIIF_USER | NIIF_LARGE_ICON;

        // Format the message with source
        char fullTitle[256];
        snprintf(fullTitle, sizeof(fullTitle), "%s: %s", source, title);

        // Convert to wide characters for Windows API
        wchar_t wTitle[256];
        wchar_t wMessage[1024];
        mbstowcs(wTitle, fullTitle, sizeof(wTitle)/sizeof(wTitle[0]));
        mbstowcs(wMessage, message, sizeof(wMessage)/sizeof(wMessage[0]));

        wcscpy(nid.szInfoTitle, wTitle);
        wcscpy(nid.szInfo, wMessage);
        wcscpy(nid.szTip, L"Notification");

        Shell_NotifyIcon(NIM_ADD, &nid);
        Sleep(100);  // Brief delay to ensure notification is shown
        Shell_NotifyIcon(NIM_DELETE, &nid);

    #elif defined(__APPLE__)
        // macOS implementation using Apple's Notification Center
        char script[2048];
        if (source && strlen(source) > 0) {
            snprintf(script, sizeof(script),
                "osascript -e 'display notification \"%s\" with title \"%s\" subtitle \"%s\"'",
                message, title, source);
        } else {
            snprintf(script, sizeof(script),
                "osascript -e 'display notification \"%s\" with title \"%s\"'",
                message, title);
        }
        system(script);

    #else
        // Linux implementation using notify-send
        char command[2048];
        if (access("/usr/bin/notify-send", F_OK) != -1) {
            if (source && strlen(source) > 0) {
                snprintf(command, sizeof(command),
                    "notify-send -a \"%s\" \"%s\" \"%s\"",
                    source, title, message);
            } else {
                snprintf(command, sizeof(command),
                    "notify-send \"%s\" \"%s\"",
                    title, message);
            }
            system(command);
        } else {
            // Fallback for systems without notify-send
            printf("%s: %s - %s\n", source, title, message);
        }
    #endif
}

// Function to parse time string and convert to total seconds
long parse_time(const char* time_str) {
    long total_seconds = 0;
    char buffer[64];
    strncpy(buffer, time_str, sizeof(buffer)-1);
    buffer[sizeof(buffer)-1] = '\0';

    char* ptr = buffer;
    while (*ptr) {
        long value = strtol(ptr, &ptr, 10);
        if (value <= 0) break;

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

        // Skip any non-digit characters
        while (*ptr && (*ptr < '0' || *ptr > '9')) ptr++;
    }

    return total_seconds;
}

// Function to format seconds into "Xd Yh Zm Ws" format
void format_time(long seconds, char* buffer, size_t buf_size) {
    long days = seconds / (24 * 3600);
    seconds %= (24 * 3600);
    long hours = seconds / 3600;
    seconds %= 3600;
    long minutes = seconds / 60;
    seconds %= 60;

    if (days > 0) {
        snprintf(buffer, buf_size, "%ldd %ldh %ldm %lds", days, hours, minutes, seconds);
    } else if (hours > 0) {
        snprintf(buffer, buf_size, "%ldh %ldm %lds", hours, minutes, seconds);
    } else if (minutes > 0) {
        snprintf(buffer, buf_size, "%ldm %lds", minutes, seconds);
    } else {
        snprintf(buffer, buf_size, "%lds", seconds);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <time> <message>\n", argv[0]);
        printf("Time format examples: 1h30m, 90m, 3600s, 2h15m10s\n");
        return 1;
    }

    // Parse the time argument
    long total_seconds = parse_time(argv[1]);
    if (total_seconds <= 0) {
        printf("Invalid time format: %s\n", argv[1]);
        return 1;
    }

    // Combine the message parts
    char message[4096] = {0};
    for (int i = 2; i < argc; i++) {
        strncat(message, argv[i], sizeof(message) - strlen(message) - 1);
        if (i < argc - 1) {
            strncat(message, " ", sizeof(message) - strlen(message) - 1);
        }
    }

    printf("Starting timer for: %s\n", argv[1]);
    printf("Message: %s\n", message);

    // Countdown loop
    long remaining = total_seconds;
    char time_str[64];

    while (remaining >= 0) {
        format_time(remaining, time_str, sizeof(time_str));

        // Clear line and print countdown
        printf("\r%s\t\t\t", time_str);
        fflush(stdout);

        if (remaining == 0) {
            // Timer completed - show notification
            printf("\nTime's up!\n");
            bell();
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

