#!/bin/bash
# Q6: 帳密檢查 (字串比較 = != -z, read 兩個變數, while 計次)
USER_OK="admin"
PASS_OK="1234"
tries=0
ok=no
while [ $tries -lt 3 ]; do
    echo "Enter username and password:"
    read u p
    tries=`expr $tries + 1`
    if [ -z "$u" ] || [ -z "$p" ]; then
        echo "Username and password must not be empty"
    elif [ "$u" != "$USER_OK" ]; then
        echo "No such user: $u"
    elif [ "$p" = "$PASS_OK" ]; then
        ok=yes
        break
    else
        echo "Wrong password"
    fi
done
if [ "$ok" = "yes" ]; then
    echo "Welcome, $u! (tries: $tries)"
else
    echo "Too many attempts, locked."
fi
