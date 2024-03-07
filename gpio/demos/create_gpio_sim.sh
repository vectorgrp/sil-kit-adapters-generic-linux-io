#!/bin/bash

modprobe gpio-sim

mkdir /sys/kernel/config/gpio-sim/gpio-demo

mkdir /sys/kernel/config/gpio-sim/gpio-demo/bank0
echo 5 > /sys/kernel/config/gpio-sim/gpio-demo/bank0/num_lines

mkdir /sys/kernel/config/gpio-sim/gpio-demo/bank1
echo 3 > /sys/kernel/config/gpio-sim/gpio-demo/bank1/num_lines

echo 1 > /sys/kernel/config/gpio-sim/gpio-demo/live
