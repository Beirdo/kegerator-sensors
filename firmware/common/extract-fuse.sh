#! /bin/bash

COL=$1
cat - | sed -e '/ *820000:/!d' -e 's/^.*:[ \t]*//' | cut -d ' ' -f ${COL}
