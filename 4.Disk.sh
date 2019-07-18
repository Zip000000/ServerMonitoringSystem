#!/bin/bash

date=`date +%Y-%m-%d__%H:%M:%S`;

df -m | grep "^/dev" | awk -v d=$date '{printf("%s 1 %s %s %s %s\n", d, $6, $2, $4, $5); a1+=$2; a2+=$4;} END{printf("%s 0 disk %s %s %d%%\n", d, a1, a2, (a1-a2)/a1*100)}'

