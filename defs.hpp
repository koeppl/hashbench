#pragma once
#include <cstdint>

using default_value_type = uint8_t;
constexpr uint8_t default_value_width = sizeof(default_value_type)*8;
#define USE_BONSAI_TABLES 1 // use the bonsai tables
//#define USE_BUCKET_TABLES 0 // use the bucket tables
#define INSTANCE_LENGTH  21 // length of the benchmark
#define CELERO_SAMPLING_COUNT 0 // number of total samples
#define CELERO_OPERATION_COUNT 3 // number of operations in one sample
