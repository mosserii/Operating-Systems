make
sudo insmod message_slot.ko
sudo mknod /dev/ops1 c 235 1
sudo mknod /dev/ops2 c 235 2
sudo mknod /dev/ops3 c 235 3
sudo chmod 0777 /dev/ops1
sudo chmod 0777 /dev/ops2
sudo chmod 0777 /dev/ops3
gcc -O3 -Wall -std=c11 message_reader.c -o READER
gcc -O3 -Wall -std=c11 message_sender.c -o SENDER
gcc -O3 -Wall -std=c11 tester.c -o testerC
gcc -O3 -Wall -std=c11 tester2.c -o testerC2

sudo mknod /dev/test0 c 235 0
sudo mknod /dev/test1 c 235 1
sudo chmod 0777 /dev/test0
sudo chmod 0777 /dev/test1
gcc -O3 -Wall -std=c11 ex3_tester.c -o tester
./tester

sudo rmmod message_slot
sudo rm /dev/test0
sudo rm /dev/test1
sudo rm /dev/ops1
sudo rm /dev/ops2
sudo rm /dev/ops3
sudo insmod message_slot.ko
sudo mknod /dev/msgslot1 c 235 1
sudo chmod o+rw /dev/msgslot1
./testerC /dev/msgslot1

sudo rmmod message_slot
sudo rm /dev/msgslot1
sudo insmod message_slot.ko
sudo mknod /dev/msgslot1 c 235 1
sudo chmod o+rw /dev/msgslot1
./testerC2 /dev/msgslot1



