#! /bin/bash

COL=$1
VAL=`cat - | sed -e '/ *820000:/!d' -e 's/^.*:[ \t]*//' | cut -d ' ' -f ${COL}`

if [ .${VAL}. == .. ] 
then
	cat /dev/null
else
	echo ${VAL}
fi
