# Notify
Simple command-line notification utility ⏰🔔. 

Send yourself reminders that pop up after a specified delay, with optional audio playback. 
Great for breaks, reminders, or timed tasks right from the terminal.

## Build
```sh
gcc build.c -o build
./build

# Example output:
CMD: mkdir bin
CMD: build notify.c
Saved to 'bin/notify'
```
This will compile `notify.c` and place the executable in the `bin/` directory

## Run
#### Basic reminder
```sh
# Notify in 45 minutes
./bin/notify 45m "Rest, it is time to rest my friend."
```
Double quotes `"msg"` are optional.

#### Reminder with custom audio
```sh
# Notify in 1 hour 5 minutes and play audio.wav
./bin/notify audio.wav 1h5m "Rest, it is time to rest my friend."
```

* The **time format** supports minutes (`m`), hours (`h`), seconds (`s`), or a combination (`1h5m15s`, `90m`, etc.).
* The **message** is any string you want displayed when time is up.
* If an audio file is provided, `notify` will attempt to play it.
* If no file is found, it falls back to the system **bell** sound.

## Install (optional)
You can copy the binary into your $PATH for easier access:
```sh
sudo cp bin/notify /usr/local/bin/
```

Then you can just run:
```sh
notify 30m "Stretch your legs!"
```

## LICENSE
MIT.
