#!/bin/bash

# For debugging: exit script on the first failure
set -e
set -o pipefail

size=1024
value_widths=(1 2 4 8)
jsonfile=$(mktemp)

while [[ $size -le 268435456 ]]; do
	for value_width in ${value_widths[@]}; do
		set -x
		../build/randomcopy "$size" "$value_width" > "$jsonfile"
		set +x
		../scripts/readjson.sh "$jsonfile" |
			while read -r line; do
				echo "$line size=$size value_width=$value_width"
			done
			((size=size*3/2))
	done
done
rm $jsonfile
