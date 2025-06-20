#!/bin/bash
# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

scriptDir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# cleanup trap for child processes 
trap 'kill $(jobs -p); exit' EXIT SIGHUP;

$scriptDir/../../../bin/sil-kit-adapter-generic-linux-io --adapter-configuration $scriptDir/../DevicesConfig.yaml --log Debug
