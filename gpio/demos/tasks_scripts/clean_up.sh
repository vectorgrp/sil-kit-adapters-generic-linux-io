#!/bin/bash
set -e 

scriptDir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $scriptDir/../../../adapter/demos/tasks_scripts/stop_processes.sh

# stop processes
processes=("sil-kit-demo-glio-gpio-forward-device" "run_adapter.sh" "update_gpio.sh")
stop_processes "${processes[@]}"