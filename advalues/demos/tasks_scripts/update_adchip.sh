#!/bin/bash
# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

set -e 

echo "[info] Updating adchips"
# Infinite loop
while true; do
  # Generate a random integer between 0 and 100
  random_number=$((RANDOM % 101))
  
  # Write the random number to the specified file
  echo $random_number > adchips/adchip0/out_voltage32
  echo "$random_number > adchips/adchip0/out_voltage32"
  # Wait for 1 second
  sleep 1
done