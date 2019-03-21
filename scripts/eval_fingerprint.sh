#!/bin/zsh

lpwd=$(pwd)
if [[ ! -d ~/data/trie/ ]]; then
	echo "Downloading data sets to ~/data/trie/..."
	echo "Press [Enter] to continue"
	read line
	mkdir -p ~/data/trie/
	cd ~/data/
	wget --recursive --no-parent -R "index.html*" -nH  http://dolomit.cs.tu-dortmund.de/trie/
fi

cd "$lpwd"

jsonfile=$(mktemp)
for dataset in ~/data/trie/*; do
	../build/bloomfilter "$dataset" "$dataset" > "$jsonfile"
	./readjson.sh "$jsonfile" |
		while read -r line; do
			echo "$line filename=$(basename $dataset)"
		done
done
rm $jsonfile
