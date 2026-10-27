savedcmd_d2ts.mod := printf '%s\n'   d2ts.o | awk '!x[$$0]++ { print("./"$$0) }' > d2ts.mod
