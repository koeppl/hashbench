#!/bin/zsh

function printPhase {
	title=$(echo "$1" | jq ".[\"title\"]")
	time=$(echo "$1" | jq ".[\"timeDelta\"]")
	mem=$(echo "$1" | jq ".[\"memPeak\"]")
	echo "RESULT group=$title time=$time mem=$mem experiment=$2" | sed 's@"@@g'
}

entries=$(jq '.["sub"] | length' "$1")
((--entries))
for entryid in $(seq 0 $entries); do
	entry=$(jq ".[\"sub\"][$entryid]" "$1")
	experiment=$(echo "$entry" | jq ".[\"title\"]")
	printPhase "$(jq ".[\"sub\"][$entryid][\"sub\"][0]" "$1")" "$experiment"
	printPhase "$(jq ".[\"sub\"][$entryid][\"sub\"][1]" "$1")" "$experiment"
done
