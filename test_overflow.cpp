#include <tudocomp/util/compact_hash/map/typedefs.hpp>
#include <gtest/gtest.h>
#include <cmath>

template<class T>
T random_int(const T& maxvalue) {
   return static_cast<T>(std::rand() * (1.0 / (RAND_MAX + 1.0 )) * maxvalue);
}

template<class T>
void test_map_random_serialize(T& map) {
   using key_type = typename T::key_type;
   using value_type = typename T::value_type;
   const uint64_t max_key = map.max_key();
   constexpr uint64_t max_value = std::numeric_limits<value_type>::max();
   for(size_t reps = 0; reps < 100; ++reps) {
      map.clear();
      for(size_t i = 0; i < 100; ++i) {
	 const key_type key = random_int<key_type>(max_key);
	 const value_type val = random_int<value_type>(max_value);
	 map[key] = val;
	 if(i % 13) { map.erase(random_int<key_type>(max_key)); }
      }
      std::stringstream ss(std::ios_base::in | std::ios_base::out | std::ios::binary);
      map.serialize(ss);
      T map2;
      ss.seekg(0);
      map2.deserialize(ss);
      DCHECK_EQ(map.key_bit_width(), map2.key_bit_width());
      DCHECK_EQ(map.size(), map2.size());
      DCHECK_EQ(map.bucket_count_log2(), map2.bucket_count_log2());

      for(size_t i = 0; i < map.bucket_count(); ++i) {
	 DCHECK_EQ(map.bucket_size(i), map2.bucket_size(i));
      }

      for(auto el : map2) {
	 auto it = map.find(el.first);
	 ASSERT_NE(it, map.end());
	 ASSERT_EQ( it->second, el.second);
      }
      for(auto el : map) {
	 auto it = map2.find(el.first);
	 ASSERT_NE(it, map.end());
	 ASSERT_EQ( it->second, el.second);
      }
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
