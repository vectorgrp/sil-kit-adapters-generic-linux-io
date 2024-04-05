#!/bin/bash

scriptDir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
silKitDir=/home/vector/SilKit/SilKit-4.0.43-ubuntu-18.04-x86_64-gcc/
# if "exported_full_path_to_silkit" environment variable is set (in pipeline script), use it. Otherwise, use default value
silKitDir="${exported_full_path_to_silkit:-$silKitDir}"

# cleanup trap for child processes 
trap 'kill $(jobs -p); exit' EXIT SIGHUP;

if [ ! -d "$silKitDir" ]; then
    echo "The var 'silKitDir' needs to be set to actual location of your SilKit"
    exit 1
fi

# check if user is root
if [[ $EUID -ne 0 ]]; then
    echo "This script must be run as root / via sudo!"
    exit 1
fi

# create the adchips - chardevs - GPIO chips
$scriptDir/../../../advalues/demos/create_adchips.sh
source $scriptDir/../../../chardev/demos/create_chardevs.sh
$scriptDir/../../../gpio/demos/create_gpio_sim.sh 2>&1 /dev/null

# update the adchips paths into the DevicesConfig.yaml file
sed -i -E s#-\ path:\ \".*/adchips#"-\ path:\ \"$(pwd)"/adchips#g $scriptDir/../DevicesConfig.yaml
# update the chardevs paths into the DevicesConfig.yaml file
sed -i -E s#-\ path:\ \".*/chardevs#"-\ path:\ \"$(pwd)"/chardevs#g $scriptDir/../DevicesConfig.yaml

$silKitDir/SilKit/bin/sil-kit-registry --listen-uri 'silkit://0.0.0.0:8501' -s &> $scriptDir/sil-kit-registry.out &
sleep 1 # wait 1 second for the creation/existense of the .out file
timeout 30s grep -q 'Registered signal handler' <(tail -f /$scriptDir/sil-kit-registry.out) || (echo "[error] Timeout reached while waiting for sil-kit-registry to start"; exit 1;)

$scriptDir/../../../bin/sil-kit-adapter-generic-linux-io --adapter-configuration $scriptDir/../DevicesConfig.yaml --log Debug &> $scriptDir/sil-kit-adapter-generic-linux-io.out &

$scriptDir/run.sh

#capture returned value of run.sh script
exit_status=$?

echo "sil-kit-registry.out:--------------------------------------------------------------------------------------" 
cat $scriptDir/sil-kit-registry.out
echo "-----------------------------------------------------------------------------------------------------------" 

echo "sil-kit-adapter-generic-linux-io.out:---------------------------------------------------------------------------" 
cat $scriptDir/sil-kit-adapter-generic-linux-io.out
echo "-----------------------------------------------------------------------------------------------------------" 

# clean the environment
rm -rf $(pwd)/adchips
rm -rf $(pwd)/chardevs
$scriptDir/../../../gpio/demos/clean_gpio_sim.sh

#exit run_all.sh with same exit_status
exit $exit_status
