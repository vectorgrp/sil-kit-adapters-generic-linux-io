#!/bin/bash
# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

set -e 

scriptDir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
silKitDir=/home/vector/SilKit/SilKit-4.0.56-ubuntu-18.04-x86_64-gcc/
# if "exported_full_path_to_silkit" environment variable is set (in pipeline script), use it. Otherwise, use default value
silKitDir="${exported_full_path_to_silkit:-$silKitDir}"

logDir=$scriptDir/logs # define a directory for .out files
mkdir -p $logDir # if it does not exist, create it

# create a timestamp for log files
timestamp=$(date +"%Y%m%d_%H%M%S")

# cleanup trap for child processes 
trap 'kill $(jobs -p); mv $scriptDir/../DevicesConfig.yaml.bak $scriptDir/../DevicesConfig.yaml; exit' EXIT SIGHUP;

if [ ! -d "$silKitDir" ]; then
    echo "The var 'silKitDir' needs to be set to actual location of your SilKit"
    exit 1
fi

# check if user is root
if [[ $EUID -ne 0 ]]; then
    echo "This script must be run as root / via sudo!"
    exit 1
fi

gpio_demos_dir="$scriptDir/../../../gpio/demos"
advalues_demos_dir="$scriptDir/../../../advalues/demos"
chardev_demos_dir="$scriptDir/../../../chardev/demos"

# create the GPIO chips only if not in the pipeline
if [[ $CI_RUN -ne "1" ]] ; then
  $gpio_demos_dir/create_gpio_sim.sh 2>&1 /dev/null
fi

if [ ! -e /dev/gpiochip0 ] || [ ! -e /dev/gpiochip1 ]; then
    echo "One or both GPIO chip devices are missing: /dev/gpiochip0, /dev/gpiochip1"
    exit 1
fi

# copy the DevicesConfig file to avoid overwritting it
cp $scriptDir/../DevicesConfig.yaml $scriptDir/../DevicesConfig.yaml.bak

$silKitDir/SilKit/bin/sil-kit-registry --listen-uri 'silkit://0.0.0.0:8501' &> $logDir/sil-kit-registry_${timestamp}.out &
sleep 1 # wait 1 second for the creation/existense of the .out file
timeout 30s grep -q 'Press Ctrl-C to terminate...' <(tail -f /$logDir/sil-kit-registry_${timestamp}.out) || (echo "[error] Timeout reached while waiting for sil-kit-registry to start"; exit 1;)

$scriptDir/run_adapter.sh $scriptDir/../DevicesConfig.yaml --log Debug &> $logDir/run_adapter_adapter_${timestamp}.out &
sleep 1 # wait 1 second for the creation/existence of the .out file
timeout 30s grep -q 'Press CTRL + C to stop the process...' <(tail -f $logDir/run_adapter_adapter_${timestamp}.out -n +1) || { echo "[error] Timeout reached while waiting for sil-kit-adapter-generic-linux-io to start"; exit 1; }
echo "[info] sil-kit-adapter-generic-linux-io has been started"

$scriptDir/run.sh

#capture returned value of run.sh script
exit_status=$?

# clean the environment
rm -rf $advalues_demos_dir/adchips
rm -rf $chardev_demos_dir/chardevs
# clean the GPIO chips only if not in the pipeline
if [[ $CI_RUN -ne "1" ]] ; then
  $gpio_demos_dir/clean_gpio_sim.sh
fi

#exit run_all.sh with same exit_status
exit $exit_status
