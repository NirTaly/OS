#!/bin/bash

make bin
cp smash os1-tests-master
cd os1-tests-master
# echo "pipe tests:"
# ./test.py -smash="smash" -test="unit/pipe"

# echo "\ntimeout tests:"
# ./test.py -smash="smash" -test="unit/timeout"

./test.py -smash="smash" -test="unit"
./test.py -smash="smash" -test="tests_tal/built_ins"