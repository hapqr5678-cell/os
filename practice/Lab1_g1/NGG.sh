#!/bin/bash
# Q1: Number Guessing Game (原題標準版)
min=1
max=10

# 用 function 詢問猜測(題目明確要求)
ask_guess() {
    echo "Please enter a number between $min and $max"
    read guess
}

echo "Please enter the target number between 1 and 10:"
read target
while [ "$target" -lt 1 ] || [ "$target" -gt 10 ]; do
    echo "Out of range! Please enter the target number between 1 and 10:"
    read target
done

while true; do
    ask_guess
    if [ "$guess" -lt "$min" ] || [ "$guess" -gt "$max" ]; then
        echo "Out of range!"
    elif [ "$guess" -gt "$target" ]; then
        echo "Too high!"
        max=`expr $guess - 1`
    elif [ "$guess" -lt "$target" ]; then
        echo "Too low"
        min=`expr $guess + 1`
    else
        echo "Bingo!"
        break
    fi
done
