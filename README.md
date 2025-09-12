# Notify

A simple cross-platform command-line notification tool ⏰🔔, plus a lightweight single-header library [`notify.h`](./notify.h) for integration in your own projects.

### Features
* **Desktop notifications**
* **Timed reminders** with flexible duration syntax
* **Optional audio playback** (custom file or system bell)
* **Cross-platform**:
  * Linux (via `notify-send`)
  * macOS (via `osascript`)
  * Windows (via `MessageBox`)

Perfect for break timers, lightweight reminders, or automating alerts directly from the terminal.

## 🚀 Build
```sh
gcc build.c -o build
./build

# Example output:
CMD: mkdir bin
CMD: gcc -O2 main.c -o bin/notify
MSG: Saved binary to './bin'
```
This compiles `notify` and saves the binary to the `bin/` folder.

## ▶️ Usage
### Basic reminder
```sh
# Notify in 45 minutes
./bin/notify 45m "Rest, it is time to rest my friend."
```
**NOTE:** Quotes around the message are optional.

### Reminder with custom audio
```sh
# Notify in 1 hour 5 minutes and play audio.wav
./bin/notify audio.wav 1h5m "Rest, it is time to rest my friend."
```

### Time format
* Supports `s` (seconds), `m` (minutes), `h` (hours), or combinations:
  * `1h5m15s`
  * `90m`
  * `45s`

### Behavior
* **Message** → displayed in a native notification popup.
* **Audio file** → plays if provided (`.wav` recommended).
* **No file found** → falls back to system bell sound.

## 📦 Install (optional)
For easier access, copy the binary into your `$PATH`:
```sh
sudo cp bin/notify /usr/local/bin/
```

Now you can simply run:
```sh
notify 30m "Stretch your legs!"
```

## 🛠️ API (C Header)
This project also includes [`notify.h`](./notify.h) — a single-header library for embedding notifications in your own C projects.

### Create a notification
```c
/**
 * Cross-platform notification function. Uses system notification center to push messages.
 * @param source notification source. Default: "App".
 * @param title title of notification. Default: "Notification".
 * @param message notification description. Default: "".
 * @returns 0 on success, -1 on failure
 */
extern int ntf_notify(const char *source, const char *title, const char *message);
```

### Play a custom alert sound
```c
/**
 * Cross-platform bell function that plays system notification sound or custom WAV file.
 * @param audio_file Path to WAV audio file. If NULL or file doesn't exist, plays default system bell.
 * @returns 0 on success, -1 on failure
 */
extern int ntf_beep(const char *audio_file);
```

👉 Command-line utility source code is in [`main.c`](./main.c).

## 📄 License
MIT.
