#!/bin/bash
# SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
# SPDX-License-Identifier: MIT

set -e

scriptDir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

## This script creates the chip folders containing the files for the advalues demo
adchips_dir="$scriptDir/adchips"
mkdir -p "$adchips_dir/adchip0"
mkdir -p "$adchips_dir/adchip1/PIN12"

## Create adchip0 files
(cd "$adchips_dir/adchip0" &&
touch in_voltage103 && touch in_voltage15 && touch out_voltage32 &&
echo 1 > in_voltage103 && echo 4 > in_voltage15 && echo 3 > out_voltage32)

## Create adchip1 files
(cd "$adchips_dir/adchip1/PIN12" && touch value && touch direction && echo 1 > value && echo 0 > direction)
(cd "$adchips_dir/adchip1" && touch out_voltage5 && echo 1.2 > out_voltage5)

## Print out the created files and folders
echo "[info] Created adchips:"
find "$adchips_dir"