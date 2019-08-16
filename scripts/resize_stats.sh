#!/bin/zsh

function die {
	echo $1 >&2
	exit 1
}


overflow_types=(cht_overflow dummy_overflow)
#overflow_types=(cht_overflow map_overflow array_overflow dummy_overflow)
hash_types=(HASH_SPLITMIX HASH_XORSHIFT HASH_MULTIPLICATIVE)
bucket_sizes=(32 64 128 192 255)
cht_overflow_fractions=(0.001 0.2 0.0001 1.0 0.8 0.6 0.1 0.01)
# array_overflow_lengths=(512 256 4 16 64)
# array_overflow_length=16


function Loop {
cat > ../linear_scaling.h <<EOF
#define SEPARATE_MAX_BUCKET_SIZE ${bucket_size}
#define ${hash_type} 1
#define OVERFLOW_TABLE ${overflow_type}
//#define ARRAY_OVERFLOW_LENGTH ${array_overflow_length}
#define CHT_OVERFLOW_FRACTION ${cht_overflow_fraction}
EOF
		cat ../linear_scaling.h

		cmake -DCMAKE_BUILD_TYPE=Release ..
		prefixname="log_${hash_type}_${bucket_size}_${overflow_type}"
		if [[ "$overflow_type" = 'cht_overflow' ]]; then
			prefixname="$prefixname${cht_overflow_fraction}"
		fi
		jsonfile="${prefixname}.json"
		datfile="${prefixname}.dat"
		make linear_scaling || die "Could not compile!"
		# ./linear_scaling $(calc '2<<22') | tee "$jsonfile" || die "Could not run!"
		./linear_scaling $(calc '2<<26') > "$jsonfile" || die "Could not run!"
		python3 ../scripts/resize_stats.py "$jsonfile" > "$datfile" || die "Failed to execute python script"
}


for hash_type in $hash_types; do
		for overflow_type in $overflow_types; do
			for bucket_size in $bucket_sizes; do

			if [[ "$overflow_type" = 'cht_overflow' ]]; then
				for cht_overflow_fraction in $cht_overflow_fractions; do
					Loop
				done
			else
				Loop
			fi
	done
done
done
