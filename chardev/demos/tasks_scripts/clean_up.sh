#!/bin/bash
set -e 

scriptDir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $scriptDir/../../../adapter/demos/tasks_scripts/stop_processes.sh

# stop processes
processes=("sil-kit-demo-glio-chardev-forward-device" "update_chardev.sh" "create_chardevs_run_adapter.sh")
stop_processes "${processes[@]}"

# delete created adchips
echo "[info] Deleting created chardevs directory"
rm -rf $scriptDir/../../../chardevs