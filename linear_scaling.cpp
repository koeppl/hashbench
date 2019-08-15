#include "linear_scaling.h"
#include <iostream>
#include <map>
#include <algorithm>
#include <separate/separate_chaining_table.hpp>
#include <separate/compact_chaining_map.hpp>
#include <separate/keysplit_adapter.hpp>
#include <separate/bijective_hash.hpp>
#include <separate/bucket_table.hpp>
#include <random>
#include <tudocomp/util/compact_hash/map/typedefs.hpp>


#ifndef STATS_ENABLED
#error need stats enabled!
#endif//STATS_ENABLED


#include <tudocomp_stat/json.hpp>
using namespace nlohmann;
#include <tudocomp_stat/StatPhase.hpp>

using namespace separate_chaining;

namespace separate_chaining {
  template<class key_t, class value_t>
    class cht_overflow {
        public:

        using key_type = key_t;
        using value_type = value_t;
        using class_type = cht_overflow<key_type, value_type>;
        using map_type = tdc::compact_hash::map::sparse_layered_hashmap_t<value_type>;

        private:
        map_type m_map;
        tdc::BitVector m_positions; // marks the positions in the CHT to which values are hashed
        tdc::BitVector m_bucketfull;
        // uint_fast8_t m_keywidth = sizeof(key_type)*8;
        //
        // class_type& operator=(class_type&& other) {
        //   m_map = std::move(other.m_map);
        //   m_positions = std::move(other.m_positions);
        //   m_bucketfull = std::move(other.m_bucketfull);
        // }

        void initialize(size_t elements) {
           const size_t reserve = std::ceil(elements/m_map.max_load_factor());
           uint_fast8_t reserve_bits = most_significant_bit(reserve);
           if(1ULL<<reserve_bits != reserve) ++reserve_bits;
           elements = 1ULL<<reserve_bits;

           m_map = map_type(elements, m_map.key_width());
           m_positions = tdc::BitVector(elements);
           DCHECK_EQ(m_map.table_size(), m_positions.size());
        }

        public:
        size_t size() const { return m_map.size(); }
        size_t capacity() const { return m_map.table_size(); }
        
        cht_overflow(uint_fast8_t keywidth) : m_map(2, keywidth) {
          m_map.max_load_factor(0.5);
        }

        bool valid_position(const size_t position) const { 
          return position < m_positions.size() && m_positions[position] == 1;
        }

        size_t first_position() const {  // TODO: major bottleneck!
          size_t position = 0;
          for(position = 0; position < m_positions.size() && m_positions[position] == 0; ++position) {}
          return position;
        }
        size_t next_position(size_t position) const { 
          DCHECK_LT(position,m_positions.size());
          do {
            ++position;
          } while(position < m_positions.size() && m_positions[position] == 0);
          return position;
        }
        size_t previous_position(size_t position) const { 
          DCHECK_GT(position,0);
          do {
            --position;
          } while(position > 0 && m_positions[position] == 0);
          return position;
        }

        void deserialize(std::istream& is) {
           size_t elements;
           is.read(reinterpret_cast<char*>(&elements), sizeof(decltype(elements)));
           initialize(elements);
           for(size_t i = 0; i < elements; ++i) {
              key_type key;
              value_type value;
              is.read(reinterpret_cast<char*>(&key), sizeof(decltype(key)));
              is.read(reinterpret_cast<char*>(&value), sizeof(decltype(value)));
              m_positions[m_map.insert(key, value).id()] = true;
           }
        }
        void serialize(std::ostream& os) const {
          const size_t elements = m_map.size();
            os.write(reinterpret_cast<const char*>(&elements), sizeof(decltype(elements)));
            for(auto& el : m_map) {
              os.write(reinterpret_cast<char*>(el.first), sizeof(decltype(el.first)));
              os.write(reinterpret_cast<char*>(el.first), sizeof(decltype(el.first)));
            }
        }

        void clear() {
          m_map = map_type();
        }
        void erase(const size_t position) { 
            DCHECK_LT(position, m_map.size());
           DCHECK(false);
        }

        size_t insert(const size_t bucket, const key_type& key, value_type&& value) {
            if(m_map.needs_to_grow_capacity(m_map.size()+2)) {
              return static_cast<size_t>(-1ULL);
            }

            m_bucketfull[bucket] = true;
            auto&& entry = m_map.insert(key, std::move(value));
            DCHECK_EQ(m_map.table_size(), m_positions.size());

            const size_t position = entry.id();
            DDCHECK_LT(position, m_positions.size());
            m_positions[position] = true;

            DDCHECK_EQ(m_map.lookup_id(position).found(), true);
            DDCHECK_EQ(m_map.lookup_id(position).id(), position);
            DDCHECK_EQ(*entry.ptr().val_ptr(), value);

#ifndef NDEBUG
            const size_t initial_address = (m_map.table_size() + position - m_map.placement().displacement_table().get(position)) % m_map.table_size();
            const size_t quotient = entry.ptr().get_quotient();
            DDCHECK_EQ(m_map.compose_key(initial_address, quotient), key);
#endif//NDEBUG


            DCHECK_EQ(this->key(position), key);
            DCHECK_EQ(operator[](position), value);
            return position;
        }
        void resize_buckets(size_t bucketcount) {
          initialize(bucketcount);
          DCHECK_GE(m_map.table_size(), bucketcount); // sets the max. number of elements to the number of buckets in the hash table
          m_bucketfull.resize(bucketcount);
        }
        
        bool need_consult(size_t bucket) const {
          DCHECK_LT(bucket, m_bucketfull.size());
          return m_bucketfull[bucket];
        }
        size_t find(const key_type& key) { // returns position of key
          DCHECK_EQ(m_map.table_size(), m_positions.size());
            if(m_map.find(key) == nullptr) return static_cast<size_t>(-1ULL);
          ON_DEBUG(const size_t formersize = m_map.size(); );
          ON_DEBUG(const size_t formercap = m_map.table_size(); );
            const size_t position = m_map.access_entry(key).id();
            DDCHECK_EQ(formersize, m_map.size());
            DDCHECK_EQ(formercap, m_map.table_size());
            DCHECK(m_positions[position]);
            DCHECK_EQ(key, this->key(position));
            return position;
        }
        value_type& operator[](const size_t position) { // returns value
          DCHECK_EQ(m_map.table_size(), m_positions.size());
           DDCHECK(m_positions[position]);
           DDCHECK_LT(position, m_map.table_size());
           DDCHECK_EQ(m_map.lookup_id(position).found(), true);
           DDCHECK_EQ(m_map.lookup_id(position).id(), position);

           return *m_map.lookup_id(position).ptr().val_ptr();
        }
        const value_type& operator[](const size_t position) const {
          DCHECK_EQ(m_map.table_size(), m_positions.size());
           DDCHECK(m_positions[position]);
           DDCHECK_LT(position, m_map.table_size());
           DDCHECK_EQ(m_map.lookup_id(position).found(), true);
           DDCHECK_EQ(m_map.lookup_id(position).id(), position);

           return *m_map.lookup_id(position).ptr().val_ptr();
        }
        key_type key(const size_t position) {
          DCHECK_EQ(m_map.table_size(), m_positions.size());
           DDCHECK(m_positions[position]);
           DDCHECK_LT(position, m_map.table_size());

           const size_t initial_address = (m_map.table_size() + position - m_map.placement().displacement_table().get(position)) % m_map.table_size();
            const size_t quotient = m_map.lookup_id(position).ptr().get_quotient();
            return m_map.compose_key(initial_address, quotient);
        }
        size_t size_in_bytes() const {
            return 0; //TODO m_bucketfull.bit_size()/8 + sizeof(std::pair<key_type,value_type>) * m_map.bucket_count();
        }

    };
}//ns




int main(int argc, char **argv) {
   std::random_device dev;

   std::mt19937_64 rng(dev());
   std::uniform_int_distribution<std::mt19937::result_type> dist(0); // distribution in range [1, 6]

   tdc::StatPhase root("linear scaling");

#if defined HASH_SPLITMIX
   separate_chaining_map<plain_bucket<uint64_t>, plain_bucket<uint64_t>, hash_mapping_adapter<uint64_t, SplitMix>, incremental_resize, OVERFLOW_TABLE> table;
#elif defined HASH_XORSHIFT
   separate_chaining_map<plain_bucket<uint64_t>, plain_bucket<uint64_t>, xorshift_hash<uint64_t>, incremental_resize, OVERFLOW_TABLE> table;
#else
   separate_chaining_map<plain_bucket<uint64_t>, plain_bucket<uint64_t>, multiplicative_hash<uint64_t>, incremental_resize, OVERFLOW_TABLE> table;
#endif

   root.log("hash", typeid(decltype(table)::hash_mapping_type).name());

   if(argc < 1) {
      std::cerr << "Usage: " << argv[0] << " maxels " << std::endl;
      return 1;
   }
   const size_t maxels = strtoul(argv[1], NULL, 10);
   root.log_stat("elements", maxels);

   size_t progress = 0;
   for(size_t i = 0; i < maxels; ++i) {
      table[dist(rng)] = i;
      if((i*100)/maxels != progress) {
         progress = (i*100)/maxels;
         std::cerr << progress << "%\r";
         std::cerr.flush();
      }
   }
   {
            tdc::StatPhase statphase(std::string("finalize"));
            table.print_stats(statphase);
   }

   std::cout << root.to_json().dump(4) << "\n";
}
