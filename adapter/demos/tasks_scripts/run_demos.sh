#!/bin/bash
set -e 

scriptDir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

logDir=$scriptDir/logs # define a directory for .out files
mkdir -p $logDir # if it does not exist, create it

# cleanup trap for child processes 
trap 'kill $(jobs -p); exit' EXIT SIGHUP;

./bin/sil-kit-demo-glio-advalues-forward-device --log Debug &> $logDir/sil-kit-demo-glio-advalues-forward-device.out &
timeout 30s grep -q 'Press CTRL + C to stop the process...' <(tail -f $logDir/sil-kit-demo-glio-advalues-forward-device.out -n +1) || { echo "[error] Timeout reached while waiting for sil-kit-demo-glio-advalues-forward-device to start"; exit 1; }
echo "[info] sil-kit-demo-glio-advalues-forward-device has been started"

./bin/sil-kit-demo-glio-chardev-forward-device --log Debug &> $logDir/sil-kit-demo-glio-chardev-forward-device.out &
timeout 30s grep -q 'Press CTRL + C to stop the process...' <(tail -f $logDir/sil-kit-demo-glio-chardev-forward-device.out -n +1) || { echo "[error] Timeout reached while waiting for sil-kit-demo-glio-chardev-forward-device to start"; exit 1; }
echo "[info] sil-kit-demo-glio-chardev-forward-device has been started"

./bin/sil-kit-demo-glio-gpio-forward-device --log Debug &> $logDir/sil-kit-demo-glio-gpio-forward-device.out &
timeout 30s grep -q 'Press CTRL + C to stop the process...' <(tail -f $logDir/sil-kit-demo-glio-gpio-forward-device.out -n +1) || { echo "[error] Timeout reached while waiting for sil-kit-demo-glio-gpio-forward-device to start"; exit 1; }
echo "[info] sil-kit-demo-glio-gpio-forward-device has been started"

# Keep the script running indefinitely until closed by user or other script
tail -f /dev/null