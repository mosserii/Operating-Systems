make
sudo insmod message_slot.ko
sudo mknod /dev/test0 c 235 0
sudo mknod /dev/test1 c 235 1
sudo chmod 0777 /dev/test0
sudo chmod 0777 /dev/test1
gcc -O3 -Wall -std=c11 ex3_tester.c -o tester
./tester




