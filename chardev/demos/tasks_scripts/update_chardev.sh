#!/bin/bash
# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

set -e 

scriptDir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

echo "[info] Updating chardevs"
# Infinite loop
while true; do
  # Generate a random integer between 0 and 100
  random_number=$((RANDOM % 101))
  echo "new message_$random_number" > $scriptDir/../../../chardevs/fifo1
  echo "new message_$random_number > chardevs/fifo1"
  # Wait for 1 second
  sleep 1
done