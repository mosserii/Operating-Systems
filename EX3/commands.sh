sudo rmmod message_slot
sudo mknod /dev/msgslot1 c 235 1
sudo chmod o+rw /dev/msgslot1

make
sudo apt-get update
sudo apt install python3-pip
pip3 install colorama

gcc -O3 -Wall -std=c11 message_sender.c message_slot.h -o message_sender.o
gcc -O3 -Wall -std=c11 message_reader.c message_slot.h -o message_reader.o

sudo insmod message_slot.ko
python3 ./tester.py

