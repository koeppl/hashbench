#pragma once
#include <cstdint>

//#define USE_BUCKET_TABLES 0 // use the bucket tables

using default_value_type = uint8_t;
constexpr uint8_t default_value_width = sizeof(default_value_type)*8;
#define USE_BONSAI_TABLES 1 // use the bonsai tables
#define USE_OVERFLOW_TABLES 1 // use the overflow approach
#define USE_STANDARD_TABLES 1 // use the overflow approach
#define USE_PLAIN_TABLES 1 // use the overflow approach

#define INSTANCE_LENGTH  21 // length of the benchmark
#define CELERO_SAMPLING_COUNT 0 // number of total samples
#define CELERO_OPERATION_COUNT 3 // number of operations in one sample

#define MAX_LOAD_FACTOR 0.95
