#!/bin/bash
while true; do
ps -ef | grep -v grep| grep ZIP
echo ""
sleep 1;
done
