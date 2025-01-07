#!/bin/bash
set -e 

# check if user is root
if [[ $EUID -ne 0 ]]; then
  echo "This script must be run as root / via sudo!"
  exit 1
fi

dir="/sys/kernel/config/gpio-sim/gpio-demo"

if [ ! -d "$dir" ]; then
  echo "[error] $dir does not exist."
  exit 1
fi

echo 0 > $dir/live
rmdir $dir/bank0 $dir/bank1 $dir
