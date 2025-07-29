#!/bin/bash
# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

localScriptDir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

## Create the chardev folder
chardevs_dir="$localScriptDir/chardevs"

clean_chardevs() {
  ## This function closes file descriptors from current process if they are open
  close_file_descriptors() {
    for fd in "$@"; do
      if [ -e "/proc/$$/fd/$fd" ]; then
        eval "exec $fd>&-"
      fi
    done
  }

  # Close file descriptors if they are open
  close_file_descriptors 3 4

  # Remove the chardevs directory
  if [ -d "$chardevs_dir" ]; then
    rm -rf "$chardevs_dir"
  else
    echo "[info] Chardevs directory does not exist"
  fi
  echo "[info] Workspace cleaned successfully"
}

# This handler is called when the script exits to clean up any leftovers in the workspace
exit_handler() {
  echo "[info] Cleaning up workspace before exiting"
  clean_chardevs
}

# $1 : error message if command failed
check_result() {
  if [ $? -ne 0 ]; then
    echo "[error] $1"
    exit 1
  fi
}

# Set up the trap to call exit_handler on EXIT signal, which will clean up workspace in case of script exit 
trap 'exit_handler' SIGHUP SIGTERM

# Clean up any already-spawned demo fifos if any
clean_chardevs

# Create the chardevs folder
mkdir -p "$chardevs_dir"

# Create two fifos and redirect read/write
# You can adapt file descriptors 3 and 4 if needed
mkfifo "$chardevs_dir/fifo1"
check_result "Failed to create fifo1"

exec 3<>"$chardevs_dir/fifo1"
check_result "Unable to open fifo1 for read/write operations"

mkfifo "$chardevs_dir/fifo2"
check_result "Failed to create fifo2"

exec 4<>"$chardevs_dir/fifo2"
check_result "Unable to open fifo2 for read/write operations"

echo message1 > "$chardevs_dir/fifo1"
echo message2 > "$chardevs_dir/fifo2"

# Print out the created files and folders
echo "[info] Created chardevs:"
find "$chardevs_dir"
