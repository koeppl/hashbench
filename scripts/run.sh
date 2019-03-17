#!/bin/sh
if [[ ! -d ../build ]]; then
	mkdir -p ../build &&
	cmake --build ../build .. &&
	make -C ../build
fi
mkdir -p eval
./eval_bloomfilter.sh | tee eval/bloomfilter.txt
./eval_randomcopy.sh | tee eval/random.txt
./eval_reservedcopy.sh | tee eval/reserve.txt
./microbench.sh | tee eval/microbench.txt
