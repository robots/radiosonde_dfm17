#target extended-remote /dev/ttyACM0
#monitor frequency 2000000
#monitor swd_scan
#attach 1

file FLASH_RUN/dfm17/dfm17.elf

target remote localhost:3333
