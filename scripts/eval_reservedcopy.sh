#!/bin/zsh
size=33554432
jsonfile=$(tempfile)

while [[ $size -le 268435456 ]]; do
	../build/randomcopy "$size" > "$jsonfile"
	./readjson.sh "$jsonfile" |
		while read -r line; do
			echo "$line size=$size"
		done 
	((size=size*3/2))
done
rm $jsonfile
