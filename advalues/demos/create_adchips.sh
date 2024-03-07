#!/bin/bash

## This script creates the chip folders containing the files for the advalues demo
mkdir -p ./adchips/adchip0
mkdir -p ./adchips/adchip1/PIN12

## Create adchip0 files
(cd ./adchips/adchip0 &&
touch in_voltage103 && touch in_voltage15 && touch out_voltage32 &&
echo 1 > in_voltage103 && echo 4 > in_voltage15 && echo 3 > out_voltage32)

## Create adchip1 files
(cd ./adchips/adchip1/PIN12 && touch value && touch direction && echo 1 > value && echo 0 > direction)
(cd ./adchips/adchip1 && touch out_voltage5 && echo 1.2 > out_voltage5)
