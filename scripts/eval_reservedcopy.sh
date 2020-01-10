#!/bin/bash

# For debugging: exit script on the first failure
set -e
set -o pipefail

size=33554432
value_widths=(1 8)
jsonfile=$(mktemp)

while [[ $size -le 268435456 ]]; do
	for value_width in ${value_widths[@]}; do
		set -x
		../build/reservecopy "$size" "$value_width" > "$jsonfile"
		set +x
		./readjson.sh "$jsonfile" |
			while read -r line; do
				echo "$line size=$size"
			done
	done
	((size=size*3/2))
done
rm $jsonfile
