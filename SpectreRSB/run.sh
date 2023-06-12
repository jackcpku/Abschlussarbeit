#!/bin/bash

# Your expected string
expected_string="The Magic Words are Squeamish Ossifrage."
length=${#expected_string}

# Counters
total_chars=0
correct_chars=0

for ((test=0; test<10; test++)); do
    sleep 2
    tmp=$(./a.out)
done

for ((iteration=0; iteration<100; iteration++)); do
    # Pause for a moment to avoid running too quickly
    sleep 2

    # Run a.out and capture the output
    output=$(./a.out)

    RETURN=$?

    # Check if the output is unreliable. If it is, skip to the next iteration
    if [[ $RETURN -eq 1 ]]; then
        echo "Unreliable output detected, skipping this run."
        continue
    fi

    # Remove the first line (the 'putting in memory' line)
    output=$(echo "$output" | sed '1d')

    # Use awk to extract each character from the output, and save them into an array
    IFS=$'\n' read -rd '' -a chars <<< "$(echo "$output" | awk -F"'" '{print $2}')"

    # Check each character against the expected character
    for (( i=0; i<$length; i++ )); do
        total_chars=$((total_chars + 1))

        # echo ${chars[i]}
        # echo ${expected_string:i:1}

        if [ "${chars[i]}" == "${expected_string:i:1}" ]; then
            correct_chars=$((correct_chars + 1))
        fi
    done

    # Calculate and print the percentage of correct characters with higher precision
    correct_percentage=$(bc <<< "scale=4; $correct_chars/$total_chars * 100")
    correct_percentage=$(printf "%.2f" $correct_percentage)
    echo "Correct characters percentage: $correct_percentage%"
done