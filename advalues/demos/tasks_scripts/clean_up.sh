#!/bin/bash
# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

set -e 

scriptDir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $scriptDir/../../../adapter/demos/tasks_scripts/stop_processes.sh

# stop processes
processes=("sil-kit-demo-glio-advalues-forward-device" "update_adchip.sh" "sil-kit-adapter-generic-linux-io")
stop_processes "${processes[@]}"

# delete created adchips
echo "[info] Deleting created adchips directory"
rm -rf $scriptDir/../adchips