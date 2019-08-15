#/usr/bin/env python3
import json
import sys


print('# size\tbucket_count\tmin\tavg\tmedian\tmax\tmedian/max\toverflow_size\toverflow_capacity')

with open(sys.argv[1]) as json_file:
    data = json.load(json_file)
    for sub in data['sub']:
        d = dict()
        for stat in sub['stats']:
            d[stat['key']] = stat['value']
        # arr = [d['size'], d['bucket_count'], d['min_bucket_size'], d['average_bucket_size'], d['median_bucket_size'], d['max_bucket_size']]
        arr = [d['size'], d['bucket_count'], d['min_bucket_size'], d['average_bucket_size'], d['median_bucket_size'], d['max_bucket_size'], d['median_bucket_size']/d['max_bucket_size'], d['overflow_size'], d['overflow_capacity']]
        print('\t'.join(str(x) for x in arr))

