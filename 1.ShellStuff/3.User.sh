#!/bin/bash
date=`date +%Y-%m-%d__%H:%M:%S`;

UserNum=`cat /etc/passwd |tr ":" " "| awk '$3 >= 1000 { printf("%d\n", $3)  }' | wc -l`

RecentUser=`last -w | uniq -w 1 | head -n 3 | cut -d " " -f 1 | tr "\n" " " | awk '{printf("%s,%s,%s", $1, $2, $3)}'`

RootUser=`cat /etc/group | grep sudo | cut -d ":" -f 4`

NowUser=`w -h |  awk '{if(cnt!=0) printf(","); printf("%s_%s_%s", $1, $3, $2);cnt++;}'`

echo "$date $UserNum [$RecentUser] [$RootUser] [$NowUser]"

