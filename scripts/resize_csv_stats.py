#/usr/bin/env python3
import json
import sys
import re

columns=['size','bucket_count', 'min_bucket_size', 'average_bucket_size', 'median_bucket_size', 'max_bucket_size', 'overflow_size', 'overflow_capacity']


filematch = re.search('log_(\w+)_(\d+)_([a-zA-Z_0-9.]+)\.json', sys.argv[1])
#filematch = re.search('log_(\w+)_(\d+)_([a-zA-Z_]+)([0-9.]*)\.json', sys.argv[1])
if not filematch:
    sys.stderr.write('invalid file name')
    sys.exit(1)

filecolumns=['hashfunction', 'bucketsize', 'overflow']

print('#', end = '')
print(','.join(filecolumns), end=',')
print(','.join(columns))

with open(sys.argv[1]) as json_file:
    data = json.load(json_file)
    for sub in data['sub']:
        d = dict()
        for stat in sub['stats']:
            d[stat['key']] = stat['value']
        print(','.join(filematch.groups()), end=',')
        print(','.join(str(d[x]) for x in columns))

