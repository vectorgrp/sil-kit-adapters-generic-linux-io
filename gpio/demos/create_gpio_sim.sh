#!/bin/bash

# check if user is root
if [[ $EUID -ne 0 ]]; then
  echo "This script must be run as root / via sudo!"
  exit 1
fi

modprobe gpio-sim

if [[ $? -ne 0 ]]; then
  exit 1
fi

dir="/sys/kernel/config/gpio-sim/gpio-demo"

if [ -d "$dir" ]; then
  echo "[error] $dir already exists. Use clean_gpio_sim.sh to clean the existing files."
  exit 1
fi

mkdir $dir

mkdir $dir/bank0
echo 5 > $dir/bank0/num_lines

mkdir $dir/bank1
echo 3 > $dir/bank1/num_lines

echo 1 > $dir/live
