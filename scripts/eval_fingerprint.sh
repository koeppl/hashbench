#!/bin/sh

# For debugging: exit script on the first failure
set -e
set -o pipefail

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
	set -x
	../build/fingerprint "$dataset" "$dataset" > "$jsonfile"
	set +x
	./readjson.sh "$jsonfile" |
		while read -r line; do
			echo "$line filename=$(basename $dataset)"
		done
done
rm $jsonfile
