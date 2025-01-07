#!/bin/bash
set -e 

scriptDir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source $scriptDir/stop_processes.sh

$scriptDir/../../../advalues/demos/tasks_scripts/clean_up.sh
$scriptDir/../../../chardev/demos/tasks_scripts/clean_up.sh
$scriptDir/../../../gpio/demos/tasks_scripts/clean_up.sh

# stop processes
processes=("sil-kit-registry" )
stop_processes "${processes[@]}"
