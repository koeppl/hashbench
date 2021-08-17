Compact Hash Table Benchmark
============================

A C++17 hash table benchmark, covering

  - [the compact separate chaining hash tables](https://github.com/koeppl/separate_chaining)
  - [the tudocomp sparse compact hash tables](https://github.com/tudocomp/compact_sparse_hash)
  - [Rigtorp's hash table](https://github.com/rigtorp/HashMap)
  - [Tessil's sparse hash table](https://github.com/Tessil/sparse-map)
  - [Google's sparse hash table](https://github.com/sparsehash/sparsehash)
  - [Gregory Popovitch's Sparsepp](https://github.com/greg7mdp/sparsepp])
  - C++ STL's `unordered_map`


## Compilation

The external requirements are available as git submodules.

```
git submodule init
git submodule update
```

## Dependencies

- Command line tools
  - cmake
  - make
  - a C++17 compiler like gcc or clang 
  - glog for the tudocomp sparse compact hash tables
  - [celero](https://github.com/DigitalInBlue/Celero) for benchmarking
  - gtest (optional) for tests
 
## Benchmarks

We use [tudostats](https://github.com/tudocomp/tudostats.git) for measuring time and memory usage.
The output is a JSON file.

### Global compile flags
Compile flags can be modified in the file `CMakeLists.txt`.
The compile flag `-DSTATS_DISABLED=1` disables tudostats.
When tudostats are enabled:
 - the flag `-DMALLOC_DISABLED=1` disables memory measurement, which can speed up the benchmarks.
 - the flag `-DPRINT_STATS`  lets tudostats print statistics of the separate chaining hash tables.

The hash tables to use can be enabled/disabled in `defs.hpp`.

###  `randomcopy` `n` `v`
Fills hash tables with `n` key-value pairs with 32-bit keys and `v` bit values.
`v` can range between 1 and 8. If you need larger value bit widths, you need to specify a larger integer type for the values (`default_value_type` in `defs.hpp`).

###  `reservecopy` `n` `v`
The same as `randomcopy` with the difference that the hash table know in advance the minimum size `2^{k-16}`, where `k` is the smallest power of two larger or equal to `n`.
This allows compact hash tables to allocate buckets. The `mock_key_type` in `reservecopy.cpp` is the type in which a quotient can be stored.

### `microbench`
Measures the time for insertion, deletions, successful and unsuccesful queries.
Each experiment is conducted multiple times, and the average time with deviation is returned.
Needs the celero library installed.


