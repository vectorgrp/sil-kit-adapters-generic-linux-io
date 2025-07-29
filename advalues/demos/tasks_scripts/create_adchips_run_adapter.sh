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

echo "[info] Creating adchips"
# create the adchips
$scriptDir/../create_adchips.sh

# copy the DevicesConfig file to avoid overwritting it
cp $scriptDir/../DevicesConfig.yaml $scriptDir/../DevicesConfig.yaml.bak

# update the adchips paths into the DevicesConfig.yaml file
sed -i -E "s#- path: \".*/advalues/demos/adchips#- path: \"$scriptDir/../adchips#g" "$scriptDir/../DevicesConfig.yaml"

echo "[info] Starting sil-kit-adapter-generic-linux-io in advalues mode"
$scriptDir/../../../bin/sil-kit-adapter-generic-linux-io --adapter-configuration $scriptDir/../DevicesConfig.yaml --log Debug