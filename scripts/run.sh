#!/bin/bash

# For debugging: exit script on the first failure
set -e
set -o pipefail

if [[ ! -d ../build ]]; then
	mkdir -p ../build &&
	cmake --build ../build &&
	make -C ../build
fi
mkdir -p eval

./eval_fingerprint.sh | tee eval/fingerprint.txt
./eval_randomcopy.sh | tee eval/random.txt
./eval_reservedcopy.sh | tee eval/reserve.txt
./microbench.sh | tee eval/microbench.txt
