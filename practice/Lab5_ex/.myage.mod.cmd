savedcmd_myage.mod := printf '%s\n'   myage.o | awk '!x[$$0]++ { print("./"$$0) }' > myage.mod
