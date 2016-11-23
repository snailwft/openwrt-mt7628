#!/bin/sh

num="hello"
echo $num

while true
do
	var=`ps |grep "telpo_lte"|grep -v grep|wc -l`

	if [ $var -eq 0 ]
	then
		telpo_lte &
		#echo "not running"
		#echo "var=$var"		
	fi
	sleep 3
done

