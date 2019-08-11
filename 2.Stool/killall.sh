#!/bin/bash

`ps -ef | grep ZIP | awk '{printf("kill %s\n", $2)}'`

