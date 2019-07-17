#!/bin/bash

#!/bin/bash
date=`date +%Y-%m-%d__%H:%M:%S`;

#UserNum=`cat /etc/passwd | grep -v "root" | wc -l`

for i in `cat /etc/passwd | cut -d ":" -f 3`
do
    if [[ $i -ge 1000 ]]; then
        UserNum=$[${UserNum}+1];
    fi
done
RecentUser=`last | uniq -w 1 | head -n 3 | cut -d " " -f 1 | tr "\n" " "`



echo -n "$date $UserNum [$RecentUser] [] ["

cnt=0
w | tail -n +3 |tr -s " " | cut -d " " -f 1-3 | while read line;
do
    a=($line)
    if [[ $cnt -ne 0 ]]; then
        echo -n ",";
    fi

    echo -n "${a[0]}_${a[2]}_${a[1]}";
    cnt=1
done
echo "]"


