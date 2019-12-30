#pragma once
#include <cstdint>

using default_value_type = uint8_t;
constexpr uint8_t default_value_width = sizeof(default_value_type)*8;
#define USE_BONSAI_TABLES 1 // use the bonsai tables
//#define USE_BUCKET_TABLES 0 // use the bucket tables
#define INSTANCE_LENGTH  2 // length of the benchmark
