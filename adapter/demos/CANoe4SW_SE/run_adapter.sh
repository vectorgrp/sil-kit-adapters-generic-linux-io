#!/bin/bash
# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

set -e

scriptDir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

advalues_demos_dir="$scriptDir/../../../advalues/demos"
chardev_demos_dir="$scriptDir/../../../chardev/demos"

# create the adchips - chardevs
$advalues_demos_dir/create_adchips.sh
source $chardev_demos_dir/create_chardevs.sh

# update the adchips paths into the DevicesConfig.yaml file
sed -i -E s#-\ path:\ \".*/advalues/demos/adchips#"-\ path:\ \"$(pwd)"/advalues/demos/adchips#g $scriptDir/../DevicesConfig.yaml
# update the chardevs paths into the DevicesConfig.yaml file
sed -i -E s#-\ path:\ \".*/chardev/demos/chardevs#"-\ path:\ \"$(pwd)"/chardev/demos/chardevs#g $scriptDir/../DevicesConfig.yaml

$scriptDir/../../../bin/sil-kit-adapter-generic-linux-io --adapter-configuration $scriptDir/../DevicesConfig.yaml --log Debug
