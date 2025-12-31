dashboard -output /dev/pts/2
target remote localhost:3333
set architecture armv4t
monitor reset halt
monitor reset init
load
dashboard
set $pc = 0x30000000
b main
c
