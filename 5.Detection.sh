#!/bin/bash

date=`date +%Y-%m-%d__%H:%M:%S`;

dangerpid=(`ps aux | tail -n +2 |  awk '$3 > 50 || $4 > 50 {printf("%s ", $2);}'`)


if [[ ${#dangerpid[@]} -eq 0 ]]; then
    echo "[first detect] No Bad Pro! :)"
    exit 0
fi

sleep 5


cnt=0
for i in ${dangerpid[@]}
do
    #echo $i
    eval `ps aux | tail -n +2 | grep $i | grep -v grep |  awk -v cnt=$cnt '$3 > 50 || $4 > 50 {printf("danger[%d]=\"%s\"\n", cnt, $0);}'`
    cnt=$[$cnt+1]
done


if [[ ${#danger[@]} -eq 0 ]]; then
    echo "[second detect] No Bad Pro! :)"
    exit 0
fi


for line in "${danger[@]}"
do
    #echo $line
    echo $line | awk -v d=$date '{printf("%s %s %s %s %s%% %s%%\n", d, $11, $2, $1, $3, $4)}'
done


    

#ps aux | tail -n +2 |  awk -v c=$cnt '$3 > 2 || $4 > 2 {printf("danger[%d]=\"%s\"\n", NF, $0); c++;}'
