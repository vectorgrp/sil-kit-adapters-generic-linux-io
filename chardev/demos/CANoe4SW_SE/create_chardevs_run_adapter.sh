#!/bin/bash
# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

scriptDir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# check if user is root
if [[ $EUID -ne 0 ]]; then
    echo "This script must be run as root / via sudo!"
    exit 1
fi

# cleanup trap for child processes 
trap 'kill $(jobs -p); exit' EXIT SIGHUP;

echo "[info] Creating chardevs"
source $scriptDir/../create_chardevs.sh

# update the chardevs paths into the DevicesConfig.yaml file
sed -i -E s#-\ path:\ \".*/chardev/demos/chardevs#"-\ path:\ \"$(pwd)"/chardev/demos/chardevs#g $scriptDir/../DevicesConfig.yaml

echo "[info] Starting sil-kit-adapter-generic-linux-io in advalues mode"
$scriptDir/../../../bin/sil-kit-adapter-generic-linux-io --adapter-configuration $scriptDir/../DevicesConfig.yaml --log Debug
