
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


