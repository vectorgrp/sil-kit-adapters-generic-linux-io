#!/bin/bash
set -e 

echo "[info] Updating GPIO"
# Infinite loop
while true; do
  # Set the GPIO pin to pull-up
  echo "pull-up" > /sys/devices/platform/gpio-sim.0/gpiochip0/sim_gpio4/pull
  echo "\"pull-up\" >  /sys/devices/platform/gpio-sim.0/gpiochip0/sim_gpio4/pull"
  # Wait for 1 second
  sleep 0.5
  
  # Set the GPIO pin to pull-down
  echo "pull-down" > /sys/devices/platform/gpio-sim.0/gpiochip0/sim_gpio4/pull
  echo "\"pull-down\" > /sys/devices/platform/gpio-sim.0/gpiochip0/sim_gpio4/pull"
  # Wait for 1 second
  sleep 0.5
done
