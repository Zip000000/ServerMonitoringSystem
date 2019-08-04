#!/bin/bash
cp -r 1.ShellStuff 8.ClntForPi
cp ClntList.c 8.ClntForPi
cp Common.c 8.ClntForPi
cp config 8.ClntForPi
cp Epoll.c 8.ClntForPi
cp Sock.c 8.ClntForPi
cp z.Client.c 8.ClntForPi

#for i in {2..10}; do
#    scp -r 8.ClntForPi zip@pi$i:~
#done

for i in {2..10}; do
    scppi 8.ClntForPi zip@pi$i:~
done
