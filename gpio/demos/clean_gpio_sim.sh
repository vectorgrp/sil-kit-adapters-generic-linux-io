#!/bin/bash

echo 0 > /sys/kernel/config/gpio-sim/gpio-demo/live
rmdir /sys/kernel/config/gpio-sim/gpio-demo/bank0
rmdir /sys/kernel/config/gpio-sim/gpio-demo/bank1
rmdir /sys/kernel/config/gpio-sim/gpio-demo
