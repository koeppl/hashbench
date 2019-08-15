#include "linear_scaling.h"
#include <iostream>
#include <map>
#include <algorithm>
#include <separate/separate_chaining_table.hpp>
#include <separate/compact_chaining_map.hpp>
#include <separate/keysplit_adapter.hpp>
#include <separate/bijective_hash.hpp>
#include <separate/bucket_table.hpp>
#include "cht_overflow.hpp"
#include <random>


#ifndef STATS_ENABLED
#error need stats enabled!
#endif//STATS_ENABLED


#include <tudocomp_stat/json.hpp>
using namespace nlohmann;
#include <tudocomp_stat/StatPhase.hpp>

using namespace separate_chaining;



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
