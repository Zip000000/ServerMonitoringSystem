#!/bin/bash

#!/bin/bash

date=`date +%Y-%m-%d__%H:%M:%S`;

Host=`hostname`

OSVersion=`cat /etc/issue | cut -d " " -f 1-3 | tr " " "_"`

KerVersion=`cat /proc/version | cut -d " " -f 3`

TimeNLoad=`uptime | cut -d " " -f 3-6,12-14 | tr ":," " " | awk '{printf("%s_%s_%s,_%s_hours,_%s_minutes %s %s %s", $1,$2,$3,$4,$5,$6,$7,$8)}'`

Disk=(`df -m | grep "^/dev" | awk '{a1+=$2; a2+=$4;} END{printf("%s %d\n", a1, (a1-a2)/a1*100)}'`)

Mem=(`free -m | grep Mem | awk '{printf("%s %d", $2, ($3/$2)*100)}'`)

temp=`cat /sys/class/thermal/thermal_zone0/temp | awk '{printf("%.2f\n",$1/1000)}'`
tempforcmp=`echo $temp | awk '{printf("%d\n",$1*1000)}'`

if [[ $tempforcmp -lt 50000 ]]; then
    TempWarningLevel="normal";
elif [[ $tempforcmp -lt 70000 ]]; then
    TempWarningLevel="note";
else
    TempWarningLevel="warning";
fi

if [[ ${Disk[1]} -lt 80 ]]; then
    DiskWarningLevel="normal";
elif [[ ${Disk[1]} -lt 90 ]]; then
    DiskWarningLevel="note";
else
    DiskWarningLevel="warning";
fi

if [[ ${Mem[1]} -lt 70 ]]; then
    MemWarningLevel="normal";
elif [[ ${Mem[1]} -lt 80 ]]; then
    MemWarningLevel="note";
else
    MemWarningLevel="warning";
fi

echo "$date $Host $OSVersion $KerVersion $TimeNLoad ${Disk[0]} ${Disk[1]}% ${Mem[0]} ${Mem[1]}% $temp $DiskWarningLevel $MemWarningLevel $TempWarningLevel"





