#!/bin/bash
# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

set -e 

scriptDir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $scriptDir/stop_processes.sh

# stop processes
processes=("run_demos.sh" "update_chips.sh" "sil-kit-adapter-generic-linux-io")
stop_processes "${processes[@]}"

# delete created adchips
echo "[info] Deleting created adchips & chardevs directories"
rm -rf $scriptDir/../../../adchips $scriptDir/../../../chardevs
