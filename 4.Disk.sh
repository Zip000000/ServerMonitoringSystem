#!/bin/bash

date=`date +%Y-%m-%d__%H:%M:%S`;

df -m | df -T -x tmpfs -x devtmpfs | tail -n +2| awk -v d=$date '{printf("%s 1 %s %s %s %s\n", d, $7, $3, $5, $6); a1+=$3; a2+=$5;} END{printf("%s 0 disk %s %s %d%%\n", d, a1, a2, (a1-a2)/a1*100)}'

