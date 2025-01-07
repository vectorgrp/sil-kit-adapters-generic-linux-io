#!/bin/bash
set -e 

## This script creates the character devices for the chardev demo

## Create the chip folder
mkdir ./chardevs

## Create two fifos and redirect read/write
## You can adapt file descriptors 3 and 4 if needed
(cd chardevs && mkfifo fifo1 && mkfifo fifo2)
exec 3<>./chardevs/fifo1 && echo message1 > ./chardevs/fifo1
exec 4<>./chardevs/fifo2 && echo message2 > ./chardevs/fifo2
