#!/bin/bash
# Q5: 命令列參數 $0 $1 $2 $3 $# —— 加總、最大值、階乘
echo "Welcome to use $0"
if [ $# -ne 3 ]; then
    echo "Usage: $0 n1 n2 n3"
    exit 1
fi
echo "Hi, we have $# argument(s): $1 $2 $3"

sum=`expr $1 + $2 + $3`
echo "sum = $sum"

max=$1
if [ $2 -gt $max ]; then
    max=$2
fi
if [ $3 -gt $max ]; then
    max=$3
fi
echo "max = $max"

# 階乘:function + while + expr \*
fact() {
    f=1
    i=1
    while [ $i -le $1 ]; do
        f=`expr $f \* $i`
        i=`expr $i + 1`
    done
    echo "$1! = $f"
}
fact $1
