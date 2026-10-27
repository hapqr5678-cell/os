#!/bin/bash
# Q3: 選單式計算機 (read + case + function + expr + while)
show_menu() {
    echo "===== Calculator ====="
    echo "a) add  s) sub  m) mul  d) div  q) quit"
    echo "Choose:"
}

calc() {
    case $op in
        a) echo "$x + $y = `expr $x + $y`";;
        s) echo "$x - $y = `expr $x - $y`";;
        m) echo "$x * $y = `expr $x \* $y`";;
        d) if [ "$y" -eq 0 ]; then
               echo "Error: divide by zero"
           else
               echo "$x / $y = `expr $x / $y`"
           fi;;
    esac
}

while true; do
    show_menu
    read op
    case $op in
        a|s|m|d)
            echo "Enter two numbers:"
            read x y
            calc;;
        q)
            echo "Bye!"
            break;;
        *)
            echo "Invalid option";;
    esac
done
