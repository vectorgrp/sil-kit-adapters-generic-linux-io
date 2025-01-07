#!/bin/bash
set -e 

# cleanup trap for child processes 
trap 'kill $(jobs -p); exit' EXIT SIGHUP;

./advalues/demos/tasks_scripts/update_adchip.sh &
./chardev/demos/tasks_scripts/update_chardev.sh &
./gpio/demos/tasks_scripts/update_gpio.sh &

# Keep the script running indefinitely until closed by user or other script
tail -f /dev/null