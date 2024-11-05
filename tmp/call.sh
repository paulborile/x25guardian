#
# make calls out of 6200 PAD
#
/usr/5bin/echo "9000111Dpadasy6\c" >> /dev/cua2
sleep 2

/usr/5bin/echo "The quick brown fox jumps over the lazy dog 0123456789" >> /dev/cua2
/usr/5bin/echo "\c"  >> /dev/cua2


sleep 1
/usr/5bin/echo "CLR\c" >> /dev/cua2
