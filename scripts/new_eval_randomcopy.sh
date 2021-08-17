#!/bin/bash

# For debugging: exit script on the first failure
set -e
set -o pipefail

exponent=15
((size=1<<$exponent))
parity=0

value_widths=(1 8)
jsonfile=$(mktemp)

while [[ $size -le 589057696 ]]; do
	for value_width in ${value_widths[@]}; do
		set -x
		../build/randomcopy "$size" "$value_width" > "$jsonfile"
		set +x
		../scripts/readjson.sh "$jsonfile" |
			while read -r line; do
				echo "$line size=$size value_width=$value_width"
			done
	done
	if [[ $parity == '1' ]]; then
		((++exponent))
		parity=0
	else
		parity=1
	fi
	((size=(1<<$exponent)+($parity<<($exponent-1)) ))
done
rm $jsonfile
