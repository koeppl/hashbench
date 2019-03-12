#!/bin/zsh
csvfile=$(tempfile)
../build/microbench -t $csvfile
columns=( $(head -n1 -q $csvfile | sed -e 's@ @_@g' -e 's@Problem_Space@problemspace@' -e 's@_(us)@@g'| tr '[:upper:]' '[:lower:]' | tr ',' '\n') )
tail -n+2 $csvfile | \
while read line; do
	((col=1))
	row=( $(echo $line | tr ',' '\n') )
	echo -n "RESULT "
	for entry in $row; do
		echo -n $columns[$col]
		echo -n "=$entry "
		((++col))
	done
	echo ""
done

