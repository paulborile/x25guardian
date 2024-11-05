#!/bin/sh

kill `cat $PX25_UTMP_FILE`
while true
do
	proc=`ps -el | grep x25_star | wc -l`
	sleep 1
	if [ $proc -eq 0 ]
	then
		echo "PX25 SOFTWARE HAS BEEN STOPPED"
		rm $PX25_UTMP_FILE
		exit
	fi
done
