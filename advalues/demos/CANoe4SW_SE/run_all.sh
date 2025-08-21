#!/bin/bash
# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

set -e

scriptDir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
silKitDir=/home/vector/SilKit/SilKit-5.0.1-ubuntu-22.04-x86_64-gcc/
# if "exported_full_path_to_silkit" environment variable is set (in pipeline script), use it. Otherwise, use default value
silKitDir="${exported_full_path_to_silkit:-$silKitDir}"

logDir=$scriptDir/logs # define a directory for .out files
mkdir -p $logDir # if it does not exist, create it

# create a timestamp for log files
timestamp=$(date +"%Y%m%d_%H%M%S")

# cleanup function for trap command
cleanup() {
    echo "[info] Cleaning up..."
    kill $(jobs -p) >/dev/null 2>&1 || true
    sleep 1 # wait for processes to terminate

    # remove any remaining child processes
    kill -9 $(jobs -p) >/dev/null 2>&1 || true

    # file cleanup
    rm -rf $scriptDir/../adchips
    mv $scriptDir/../DevicesConfig.yaml.bak $scriptDir/../DevicesConfig.yaml
}

# cleanup trap for child processes 
trap "exit_status=$?; cleanup; exit $exit_status" EXIT SIGHUP SIGTERM

if [ ! -d "$silKitDir" ]; then
    echo "[error] The var 'silKitDir' needs to be set to actual location of your SilKit"
    exit 1
fi

# check if user is root
if [[ $EUID -ne 0 ]]; then
    echo "[error] This script must be run as root / via sudo!"
    exit 1
fi

# copy the DevicesConfig file to avoid overwritting it
cp $scriptDir/../DevicesConfig.yaml $scriptDir/../DevicesConfig.yaml.bak

$silKitDir/SilKit/bin/sil-kit-registry --listen-uri 'silkit://0.0.0.0:8501' &> $logDir/sil-kit-registry_${timestamp}.out &
sleep 1 # wait 1 second for the creation/existense of the .out file
timeout 30s grep -q 'Press Ctrl-C to terminate...' <(tail -f $logDir/sil-kit-registry_${timestamp}.out) || (echo "[error] Timeout reached while waiting for sil-kit-registry to start"; exit 1;)

$scriptDir/../create_adchips.sh

# update the adchips paths into the DevicesConfig.yaml file
sed -i -E "s#- path: \".*/advalues/demos/adchips#- path: \"$scriptDir/../adchips#g" "$scriptDir/../DevicesConfig.yaml"

echo "[info] Starting sil-kit-adapter-generic-linux-io in advalues mode"
$scriptDir/../../../bin/sil-kit-adapter-generic-linux-io --adapter-configuration $scriptDir/../DevicesConfig.yaml --log Debug &> $logDir/create_adchips_run_adapter_${timestamp}.out &
sleep 1 # wait 1 second for the creation/existence of the .out file
timeout 30s grep -q 'Press CTRL + C to stop the process...' <(tail -f $logDir/create_adchips_run_adapter_${timestamp}.out -n +1) || { echo "[error] Timeout reached while waiting for sil-kit-adapter-generic-linux-io to start"; exit 1; }
echo "[info] sil-kit-adapter-generic-linux-io has been started"

$scriptDir/run.sh

exit $?
