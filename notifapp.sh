#!/usr/bin/env bash

# define messages in an array
messages=(
    "Time to stretch your legs: 21 sit-ups."
    "Time to stretch your legs: horse stance."
    "Time to stretch your neck."
    "Time to stretch your spine."
    "Do eye exercises."
    "Exercise your waist."
)
num_options=${#messages[@]}                                # get array length
default_audio="/home/kirillsaidov/workfiles/tmp/audio.wav" # default audio
default_duration="50m"                                     # default duration

# display menu
echo ">> Which timer to run?"
for i in "${!messages[@]}"; do
    echo "$((i + 1)). ${messages[$i]}"
done

# read user choices (space-separated)
read -p ">> " -a choices

# validate all choices
for choice in "${choices[@]}"; do
	if [[ ! "$choice" =~ ^[0-9]+$ ]] || [ "$choice" -lt 1 ] || [ "$choice" -gt "$num_options" ]; then
		echo ">> Invalid choice: $choice. Please select between 1-$num_options."
        exit 1
    fi
done

# read timer durations (space-separated, optional)
read -p ">> Enter timer duration (default: $default_duration): " -a durations

# execute commands for each choice
for i in "${!choices[@]}"; do
    choice="${choices[$i]}"
    
    # get selected message (subtract 1 for zero-based array index)
    selected_message="${messages[$((choice - 1))]}"
    
    # get duration for this choice, or use default
    if [ -n "${durations[$i]}" ]; then
        duration="${durations[$i]}"
    else
        duration="$default_duration"
    fi
    
    # execute the command
    echo ">> Running timer $((i + 1))/${#choices[@]}: notify $duration \"$selected_message\""
    notify "$default_audio" "$duration" "$selected_message"
done

echo ">> All timers completed!"


