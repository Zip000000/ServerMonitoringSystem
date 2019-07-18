#!/bin/bash
#RootUser=`cat /etc/passwd | cut -d ":" -f 1 | xargs -n 1 id | grep sudo | tr "()" " " | cut -d " " -f 2 | awk '{if(cnt!=0) printf(" ");printf("%s", $1); cnt++;}'`

