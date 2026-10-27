#!/bin/bash
# Q4 consumer: 從 stdin 讀分數,判定等第,最後印統計
total=0
n=0
pass=0
while read s; do
    if [ "$s" -ge 90 ]; then
        g=A
    elif [ "$s" -ge 80 ]; then
        g=B
    elif [ "$s" -ge 60 ]; then
        g=C
    else
        g=F
    fi
    echo "$s -> $g"
    total=`expr $total + $s`
    n=`expr $n + 1`
    if [ $s -ge 60 ]; then
        pass=`expr $pass + 1`
    fi
done
if [ $n -eq 0 ]; then
    echo "No input"
else
    echo "count=$n pass=$pass avg=`expr $total / $n`"
fi
