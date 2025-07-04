#!/bin/bash
# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

set -e 

scriptDir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
# Set a default path for canoe4sw-se installation directory
default_canoe4sw_se_install_dir="/opt/vector/canoe4sw-se"

# Check if the executable exists at the default path
if [[ -x "$default_canoe4sw_se_install_dir/canoe4sw-se" ]]; then
    canoe4sw_se_install_dir="$default_canoe4sw_se_install_dir"
else
    # If not found at the default path, search for the executable
	canoe4sw_se_install_dir=$(dirname $(find / -name canoe4sw-se -type f -executable -print -quit 2>/dev/null))
fi

if [[ -n "$canoe4sw_se_install_dir" ]]; then
	echo "canoe4sw-se found at location : $canoe4sw_se_install_dir"
	#run tests
	$canoe4sw_se_install_dir/canoe4sw-se "$scriptDir/Default.venvironment" -d "$scriptDir/working-dir" --verbosity-level "2" --test-unit "$scriptDir/testForwardGpio.vtestunit"  --show-progress "tree-element"
    exit_status=$?
else
    echo "canoe4sw-se executable not found"
    exit_status=-1
fi

exit $exit_status
