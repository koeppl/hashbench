#!/bin/zsh

function die {
	echo $1 >&2
	exit 1
}


overflow_types=(dummy_overflow array_overflow map_overflow)
hash_types=(HASH_SPLITMIX HASH_XORSHIFT HASH_MULTIPLICATIVE)
bucket_sizes=(32 64 128 192 255)


array_overflow_length=16
for hash_type in $hash_types; do
	for bucket_size in $bucket_sizes; do
		for overflow_type in $overflow_types; do

cat > ../linear_scaling.h <<EOF
#define SEPARATE_MAX_BUCKET_SIZE ${bucket_size}
#define ${hash_type} 1
#define OVERFLOW_TABLE ${overflow_type}
#define ARRAY_OVERFLOW_LENGTH ${array_overflow_length}
EOF
		cat ../linear_scaling.h

		cmake -DCMAKE_BUILD_TYPE=Release ..
		jsonfile="log_${hash_type}_${bucket_size}.json"
		datfile="log_${hash_type}_${bucket_size}.dat"
		make linear_scaling || die "Could not compile!"
		./linear_scaling $(calc '2<<26') | tee "$jsonfile"
		python3 ../scripts/resize_stats.py "$jsonfile" > "$datfile"
	done
done
done
