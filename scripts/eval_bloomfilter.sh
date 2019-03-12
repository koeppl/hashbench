#!/bin/zsh
jsonfile=$(tempfile)
for dataset in ~/data/trie/*.distinct; do
	../build/bloomfilter "$dataset" "$dataset" > "$jsonfile"
	./readjson.sh "$jsonfile" |
		while read -r line; do
			echo "$line filename=$(basename $dataset)"
		done 
done
rm $jsonfile
