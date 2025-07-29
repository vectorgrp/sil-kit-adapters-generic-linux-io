#!/bin/bash
# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

set -e

scriptDir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# cleanup function for trap command
cleanup() {
    kill $(jobs -p) >/dev/null 2>&1 || true
    mv $scriptDir/../DevicesConfig.yaml.bak $scriptDir/../DevicesConfig.yaml
}

# cleanup trap for child processes
trap 'cleanup; exit' EXIT SIGHUP SIGTERM

advalues_demos_dir="$scriptDir/../../../advalues/demos"
chardev_demos_dir="$scriptDir/../../../chardev/demos"

# create the adchips - chardevs
$advalues_demos_dir/create_adchips.sh
source $chardev_demos_dir/create_chardevs.sh

# copy the DevicesConfig file to avoid overwritting it
cp $scriptDir/../DevicesConfig.yaml $scriptDir/../DevicesConfig.yaml.bak

# update the adchips paths into the DevicesConfig.yaml file
sed -i -E "s#- path: \".*/advalues/demos/adchips#- path: \"$advalues_demos_dir/adchips#g" "$scriptDir/../DevicesConfig.yaml"
# update the chardevs paths into the DevicesConfig.yaml file
sed -i -E "s#- path: \".*/chardev/demos/chardevs#- path: \"$chardev_demos_dir/chardevs#g" "$scriptDir/../DevicesConfig.yaml"

$scriptDir/../../../bin/sil-kit-adapter-generic-linux-io --adapter-configuration $scriptDir/../DevicesConfig.yaml --log Debug
