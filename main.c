#include <stdio.h>
#include <stdlib.h>

#define NOTIFY_IMPLEMENTATION
#include "notify.h"

// app version
#define NOTIFY_VERSION "1.0.3"

int main(const int argc, const char *argv[]) {
    if (argc < 3) {
        printf("notify v" NOTIFY_VERSION " -- command-line notification utility.\n");
        printf("Usage:\n");
        printf("    %s <time> <message>\n", argv[0]);
        printf("    %s [audio] <time> <message>\n", argv[0]);
        printf("Time format examples: 1h30m, 90m, 3600s, 2h15m10s\n");
        printf("Example: %s 5m \"Take a break!\"\n", argv[0]);
        return 1;
    }

    // parse the audio argument
    const char *audio_file = isdigit(argv[1][0]) ? NULL : argv[1];
    const char *total_seconds_arg = argv[audio_file ? 2 : 1];

    // parse the time argument
    long total_seconds = ntf_parse_time(total_seconds_arg);
    if (total_seconds <= 0) {
        printf("Invalid time format: %s\n", total_seconds_arg);
        printf("Use format like: 1h30m, 90m, 3600s, etc.\n");
        return 1;
    }

    // combine the message parts
    char message[4096] = {0};
    size_t offset = 0;
    for (int i = 2 + (audio_file ? 1 : 0); i < argc; i++) {
        const int written = snprintf(
            message + offset, 
            sizeof(message) - offset,
            "%s%s",
            argv[i],
            (i < argc - 1) ? " " : ""
        );
        if (written < 0 || (size_t)written >= sizeof(message) - offset) {
            break; // truncate
        }
        offset += written;
    }

    // print
    char time_str[64] = {0};
    ntf_format_time(total_seconds, time_str, sizeof(time_str));
    printf("Starting timer for: %s\n", time_str);
    printf("Message: %s\n", message);

    // countdown loop
    long remaining = total_seconds;
    while (remaining >= 0) {
        ntf_format_time(remaining, time_str, sizeof(time_str));

        // clear line and print countdown
        printf("\r%-20s", time_str);  // fixed-width for clean display
        fflush(stdout);

        // timer completed
        if (remaining == 0) {
            printf("\nTime's up!\n");
            ntf_beep(audio_file);
            ntf_notify("Notify", "Timer Complete", message);
            break;
        }

        // sleep for 1 second
        ntf_sleep(1000);
        remaining--;
    }

    return 0;
}

