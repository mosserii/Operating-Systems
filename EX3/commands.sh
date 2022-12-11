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




