#!/bin/bash

for i in {1..10}; do
    scp -r 8.ClntForPi zip@pi$i:~
done
