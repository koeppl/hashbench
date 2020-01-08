#pragma once
#include <cstdint>
#include <cstdlib>

//#define USE_BUCKET_TABLES 0 // use the bucket tables

using default_value_type = uint8_t;
constexpr uint8_t default_value_width = sizeof(default_value_type)*8;

#define USE_BONSAI_TABLES 1 // use the bonsai tables
#define USE_OVERFLOW_TABLES 1 // use the overflow approach
#define USE_STANDARD_TABLES 1 // use the overflow approach
#define USE_PLAIN_TABLES 1 // use the overflow approach
#define USE_CHMAP_TABLE 1 // use the overflow approach
// #ifdef __AVX2__
// #undef __AVX2__
// #endif//__AVX2__

#define INSTANCE_LENGTH  10 // length of the benchmark
#define CELERO_SAMPLING_COUNT 0 // number of total samples
#define CELERO_OPERATION_COUNT 3 // number of operations in one sample

#define MAX_LOAD_FACTOR 0.95

template<class T>
inline T random_int(const T& maxvalue) {
   return static_cast<T>(std::rand() * (1.0 / (RAND_MAX + 1.0 )) * maxvalue);
}

inline void random_int_reset() { std::srand(1); }
