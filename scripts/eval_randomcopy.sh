#!/bin/zsh
size=1024
jsonfile=$(tempfile)

while [[ $size -lt 1073741824 ]]; do
	../build/randomcopy "$size" > "$jsonfile"
	./readjson.sh "$jsonfile" |
		while read -r line; do
			echo "$line size=$size"
		done 
	((size*=2))
done
rm $jsonfile
