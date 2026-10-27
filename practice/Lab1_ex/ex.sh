#!/bin/sh
# LAB1: Number Guessing Game

# check that $1 is an integer between $2 and $3
in_range() {
	case "$1" in
		''|*[!0-9]*) return 1 ;;
	esac
	[ "$1" -ge "$2" ] && [ "$1" -le "$3" ]
}

ask_guess() {
	echo "Please guess a number between $min and $max"
	read guess
	while ! in_range "$guess" "$min" "$max"; do
		echo "Invalid input, please enter a number between $min and $max"
		read guess
	done
}

echo "Please enter the target number between 1 and 10"
read target
while ! in_range "$target" 1 10; do
	echo "Invalid input, please enter the target number between 1 and 10"
	read target
done

min=1
max=10

while true; do
	ask_guess
	if [ "$guess" -gt "$target" ]; then
		max=`expr $guess - 1`
		echo "Too high!"
	elif [ "$guess" -lt "$target" ]; then
		min=`expr $guess + 1`
		echo "Too low!"
	else
		echo "Bingo!"
		break
	fi
done
