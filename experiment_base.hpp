#pragma once


#include <unordered_map>
#include <rigtorp/HashMap.h>
#include <tsl/sparse_map.h>
#include <separate/separate_chaining_table.hpp>
#include <sparsehash/sparse_hash_map>

#include <tudocomp_stat/json.hpp>
using namespace nlohmann;
#include <tudocomp_stat/StatPhase.hpp>

#include <tudocomp/util/compact_hash/map/typedefs.hpp>

template<class experiment_t>
void run_experiments(experiment_t& ex) {
      using namespace separate_chaining;
      using experiment_type = experiment_t;

      using key_type = typename experiment_type::key_type;
      using value_type = typename experiment_type::value_type;

#ifdef STATS_DISABLED
      std::cout << "stats disabled. " << std::endl;
#endif

      tdc::StatPhase root(ex.caption());
      ex.init(root);

      ex.baseline();
#ifdef USE_BONSAI_HASH
      if(ex.KEY_BITSIZE < 64)
      {
         tdc::compact_hash::map::sparse_elias_hashmap_t<value_type> filter(0,ex.KEY_BITSIZE);
         ex.execute("bonsai elias", filter);
      }
      if(ex.KEY_BITSIZE < 64)
      {
         tdc::compact_hash::map::sparse_cv_hashmap_t<value_type> filter(0,ex.KEY_BITSIZE);
         ex.execute("bonsai cleary", filter);
      }
      if(ex.KEY_BITSIZE < 64)
      {
         tdc::compact_hash::map::sparse_layered_hashmap_t<value_type> filter(0,ex.KEY_BITSIZE);
         ex.execute("bonsai layered", filter);
      }
#endif//USE_BONSAIHASH
      {
	 separate_chaining_map<plain_bucket<key_type>, plain_bucket<value_type>  , hash_mapping_adapter<key_type , std::hash<key_type> >> filter;
	 ex.execute("sep plain 1", filter);
      }
      {
	 separate_chaining_map<plain_bucket<key_type>, plain_bucket<value_type>  , hash_mapping_adapter<key_type , std::hash<key_type> >, arbitrary_resize> filter;
	 ex.execute("sep plain 2", filter);
      }

#if defined(STATS_DISABLED) || defined(MALLOC_DISABLED)
      {
	 separate_chaining_map<avx2_bucket<key_type>, plain_bucket<value_type>  , hash_mapping_adapter<key_type , std::hash<key_type> >, arbitrary_resize> filter;
	 ex.execute("sep avx 2", filter);
      }
#endif

      {
	 separate_chaining_map<varwidth_bucket, plain_bucket<value_type>, xorshift_hash<>> filter(ex.KEY_BITSIZE);
	 ex.execute("sep cht 1", filter);
      }
      {
	 separate_chaining_map<varwidth_bucket, plain_bucket<value_type>, xorshift_hash<>, arbitrary_resize> filter(ex.KEY_BITSIZE);
	 ex.execute("sep cht 2", filter);
      }

      {
	 tsl::sparse_map<key_type,value_type> filter;
	 ex.execute("tsl::sparse-map", filter);
      }

      {
	 google::sparse_hash_map<key_type,value_type> filter;
	 ex.execute("google::sparse_hash_map", filter);
      }
      {
	 rigtorp::HashMap<key_type,value_type> filter(0, static_cast<key_type>(-1ULL));
	 ex.execute("rigtorp", filter);
      }
      {
	 std::unordered_map<key_type,value_type> filter;
	 ex.execute("unordered_map", filter);
      }
      std::cout << root.to_json().dump(4) << "\n";
}

#include <type_traits>
#include <utility>

template <typename T, typename = void>
struct has_print_stats_fn
    : std::false_type
{};

template <typename T>
struct has_print_stats_fn<T, std::void_t<decltype(std::declval<T>().print_stats(std::declval<tdc::StatPhase&>()  ))>> : std::true_type
{};
