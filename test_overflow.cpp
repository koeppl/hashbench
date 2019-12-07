#include <tudocomp/util/compact_hash/map/typedefs.hpp>
#include <gtest/gtest.h>
#include <cmath>
#include <separate/separate_chaining_table.hpp>
#include <separate/compact_chaining_map.hpp>
#include <separate/group_chaining.hpp>
#include <separate/keysplit_adapter.hpp>
#include <separate/bijective_hash.hpp>
#include <separate/bucket_table.hpp>
#include "cht_overflow.hpp"

template<class T>
T random_int(const T& maxvalue) {
   return static_cast<T>(std::rand() * (1.0 / (RAND_MAX + 1.0 )) * maxvalue);
}

template<class T>
void test_map_random(T& map) {
   using key_type = typename T::key_type;
   using value_type = typename T::value_type;
   const uint64_t max_key = map.max_key();
   const uint64_t max_value = map.max_value();
   for(size_t reps = 0; reps < 3; ++reps) {

      map.clear();
      std::map<typename T::key_type, typename T::value_type> rev;
      for(size_t i = 0; i < 3000; ++i) {
	 const key_type key = random_int<key_type>(max_key);
	 const value_type val = random_int<value_type>(max_value);
	 map[key] = rev[key] = val;
	 ASSERT_EQ(map.size(), rev.size());
	 if(! (i % 13)) map.shrink_to_fit();
      }
      for(auto el : rev) {
	 auto it = map.find(el.first);
	 ASSERT_NE(it, map.end());
	 ASSERT_EQ( it->second, el.second);
      }
      for(size_t i = 0; i < 100; ++i) {
	 const key_type key = random_int<key_type>(max_key);
	 if(rev.find(key) == rev.end()) {
	    auto it = map.find(key);
	    ASSERT_EQ(it, map.end());
	 } else {
	    ASSERT_EQ(map.find(key)->first, rev.find(key)->first);
	    ASSERT_EQ(map.find(key)->second, rev.find(key)->second);
	 }
      }
   }
}


TEST(chtoverflow, random) { 
   using namespace separate_chaining;
   {
      using key_type = uint32_t;
      separate_chaining_map<plain_bucket<key_type>, plain_bucket<key_type> , hash_mapping_adapter<key_type, SplitMix>, incremental_resize, cht_overflow> map(sizeof(key_type)*8);
      test_map_random(map);
   }
   // {
   //    using key_type = uint32_t;
   //    separate_chaining_map<plain_bucket<key_type>, plain_bucket<key_type> , hash_mapping_adapter<key_type, SplitMix>, incremental_resize, cht_overflow> map(sizeof(key_type)*8);
   //    test_map_random_serialize(map);
   // }
   // {
   //    using key_type = uint64_t;
   //    separate_chaining_map<plain_bucket<key_type>, plain_bucket<key_type> , hash_mapping_adapter<key_type, SplitMix>, incremental_resize, cht_overflow> map(sizeof(key_type)*8);
   //    test_map_random_serialize(map);
   // }
}

TEST(chtoverflow, randomlow) { 
   using namespace separate_chaining;
   {
      using key_type = uint32_t;
      separate_chaining_map<plain_bucket<key_type>, plain_bucket<key_type> , hash_mapping_adapter<key_type, SplitMix>, incremental_resize, cht_overflow> map(15,3);
      test_map_random(map);
   }
}

TEST(grpoverflow, randomlow) { 
   using namespace separate_chaining;
   using namespace separate_chaining::group;
   {
      using key_type = uint32_t;
      group_chaining_table<hash_mapping_adapter<uint64_t, SplitMix>, cht_overflow<key_type, size_t>> map(23,3);
      test_map_random(map);
   }
}

constexpr int most_significant_bit(const uint64_t& x) {
    return x == 0 ? -1 : (sizeof(uint64_t)*8-1) - __builtin_clzll(x);
}

TEST(mapentry, random) { 
   using key_type = uint64_t;
   using value_type = uint64_t;
   constexpr size_t elements = 59000;
   constexpr float loadfactor = 0.95f;
   size_t reserve = std::ceil(elements/loadfactor);

   constexpr uint_fast8_t keywidth = 16;
   constexpr uint_fast8_t valwidth = 32;

   

   uint_fast8_t reserve_bits = most_significant_bit(reserve);
   if(1ULL<<reserve_bits != reserve) ++reserve_bits;
   reserve = 1ULL<<reserve_bits;

   using map_type = tdc::compact_hash::map::sparse_layered_hashmap_t<value_type>;
   map_type map(reserve, keywidth, valwidth);
   map.max_load_factor(loadfactor);

   const uint64_t max_key = 1ULL<<keywidth;
   constexpr uint64_t max_value = std::numeric_limits<value_type>::max();
   for(size_t i = 0; i < elements; ++i) {
	 const key_type key = random_int<key_type>(max_key);
	 value_type value = random_int<value_type>(max_value);
     const auto entry = map.insert_key_width(key, std::move(value), keywidth);
     const size_t position = entry.id();
     ASSERT_EQ(map.key_width(), keywidth);
     ASSERT_EQ(map.lookup_id(position).found(), true);
     ASSERT_EQ(map.lookup_id(position).id(), position);
     ASSERT_EQ(*entry.ptr().val_ptr(), value);
     ASSERT_EQ(map.needs_to_grow_capacity(map.size()+1), false);

     const size_t initial_address = (map.table_size() + position - map.placement().displacement_table().get(position)) % map.table_size();
     const size_t quotient = entry.ptr().get_quotient();
     ASSERT_EQ(map.compose_key(initial_address, quotient), key);
   } 
}




int main(int argc, char **argv) {

   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
