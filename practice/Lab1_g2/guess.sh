#!/bin/bash
# Q2: 猜數字進階版 —— 範圍由參數給、限制次數、統計次數
if [ $# -ne 3 ]; then
    echo "Usage: $0 min max limit"
    exit 1
fi

min=$1
max=$2
limit=$3
count=0
win=no

ask_guess() {
    echo "[$count/$limit] Please enter a number between $min and $max"
    read guess
}

echo "Please enter the target number between $min and $max:"
read target

while [ $count -lt $limit ]; do
    count=`expr $count + 1`
    ask_guess
    if [ "$guess" -gt "$target" ]; then
        echo "Too high!"
        max=`expr $guess - 1`
    elif [ "$guess" -lt "$target" ]; then
        echo "Too low"
        min=`expr $guess + 1`
    else
        win=yes
        break
    fi
done

if [ "$win" = "yes" ]; then
    echo "Bingo! You got it in $count tries."
else
    echo "Game over! The answer is $target."
fi
