#!/bin/bash
set -e

scriptDir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# create the adchips - chardevs
$scriptDir/../../../advalues/demos/create_adchips.sh
source $scriptDir/../../../chardev/demos/create_chardevs.sh

# update the adchips paths into the DevicesConfig.yaml file
sed -i -E s#-\ path:\ \".*/adchips#"-\ path:\ \"$(pwd)"/adchips#g $scriptDir/../DevicesConfig.yaml
# update the chardevs paths into the DevicesConfig.yaml file
sed -i -E s#-\ path:\ \".*/chardevs#"-\ path:\ \"$(pwd)"/chardevs#g $scriptDir/../DevicesConfig.yaml

$scriptDir/../../../bin/sil-kit-adapter-generic-linux-io --adapter-configuration $scriptDir/../DevicesConfig.yaml --log Debug
