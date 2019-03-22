#!/bin/sh

# For debugging: exit script on the first failure
set -e
set -o pipefail

size=33554432
jsonfile=$(mktemp)

while [[ $size -le 268435456 ]]; do
	set -x
	../build/reservecopy "$size" > "$jsonfile"
	set +x
	./readjson.sh "$jsonfile" |
		while read -r line; do
			echo "$line size=$size"
		done
	((size=size*3/2))
done
rm $jsonfile
