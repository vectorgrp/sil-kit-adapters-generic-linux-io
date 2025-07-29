#!/bin/bash
# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

scriptDir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# cleanup function for trap command
cleanup() {
    kill $(jobs -p) >/dev/null 2>&1 || true
    mv $scriptDir/../DevicesConfig.yaml.bak $scriptDir/../DevicesConfig.yaml
}

# cleanup trap for child processes 
trap 'cleanup; exit' EXIT SIGHUP SIGTERM

echo "[info] Creating chardevs"
source $scriptDir/../create_chardevs.sh

# copy the DevicesConfig file to avoid overwritting it
cp $scriptDir/../DevicesConfig.yaml $scriptDir/../DevicesConfig.yaml.bak

# update the chardevs paths into the DevicesConfig.yaml file
sed -i -E "s#- path: \".*/chardev/demos/chardevs#- path: \"$scriptDir/../chardevs#g" "$scriptDir/../DevicesConfig.yaml"

echo "[info] Starting sil-kit-adapter-generic-linux-io in advalues mode"
$scriptDir/../../../bin/sil-kit-adapter-generic-linux-io --adapter-configuration $scriptDir/../DevicesConfig.yaml --log Debug
