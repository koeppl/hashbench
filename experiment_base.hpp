#pragma once
#include "defs.hpp"



// #include "cuckoo_dysect.h" //DySect
// #include "utils/hash/murmur2_hash.h"

#include <tudocomp_stat/json.hpp>
using namespace nlohmann;
#include <tudocomp_stat/StatPhase.hpp>

#include <tudocomp/util/compact_hash/map/typedefs.hpp>

#include "demangled_type.hpp"


#include <unordered_map>

#ifdef USE_CHMAP_TABLE
#include <separate/compact_chaining_map.hpp>
#endif//USE_CHMAP_TABLE

#include <separate/separate_chaining_table.hpp>
#include <separate/group_chaining.hpp>
#include "cht_overflow.hpp"

#ifdef USE_STANDARD_TABLES
#include <rigtorp/HashMap.h>
#include <tsl/sparse_map.h>
#include <sparsehash/sparse_hash_map>
#include <sparsepp/spp.h>
#endif//USE_STANDARD_TABLES


template<class experiment_t>
void run_experiments(experiment_t& ex) {
    using namespace separate_chaining;
    using experiment_type = experiment_t;

    using key_type = typename experiment_type::key_type;
    using value_type = typename experiment_type::value_type;

    using value_bucket_type = varwidth_bucket<>; //plain_bucket<value_type>

#ifdef STATS_DISABLED
    std::cout << "stats disabled. " << std::endl;
#endif

    tdc::StatPhase root(ex.caption());
    ex.init(root);

    ex.baseline();

    std::vector<std::function<void()>> bench_cases;
    auto regist = [&] (auto run_function) {
        bench_cases.push_back(run_function);
    };

    // regist([&] {
	// 		dysect::cuckoo_dysect<key_type, value_type, dysect::hash::murmur2_hash> filter(10000, 1.1);
    //     ex.execute("dysect", filter);
    // });

    regist([&] {
            group::group_chaining_table<multiplicative_hash<>> filter(ex.KEY_BITSIZE, ex.VALUE_BITSIZE);
        ex.execute("grp", filter);
    });
    regist([&] {
        separate_chaining_map<varwidth_bucket<>, value_bucket_type, multiplicative_hash<>> filter(ex.KEY_BITSIZE, ex.VALUE_BITSIZE);
        ex.execute("chtI", filter);
    });
    regist([&] {
        separate_chaining_map<varwidth_bucket<>, value_bucket_type, multiplicative_hash<>, arbitrary_resize> filter(ex.KEY_BITSIZE, ex.VALUE_BITSIZE);
        ex.execute("chtD", filter);
    });


#ifdef USE_CHMAP_TABLE
    regist([&] {
        compact_chaining_map<multiplicative_hash<>, uint8_t> filter(ex.KEY_BITSIZE, ex.VALUE_BITSIZE);
        ex.execute("chmap", filter);
    });
#endif//USE_CHMAP_TABLE

#ifdef USE_OVERFLOW_TABLES
    regist([&] {
            group::group_chaining_table<multiplicative_hash<>, cht_overflow<key_type,value_type>> filter(ex.KEY_BITSIZE, ex.VALUE_BITSIZE);
        ex.execute("grpO", filter);
    });

    regist([&] {
        separate_chaining_map<varwidth_bucket<>, value_bucket_type, multiplicative_hash<>, incremental_resize, cht_overflow> filter(ex.KEY_BITSIZE, ex.VALUE_BITSIZE);
        ex.execute("chtIO", filter);
    });
#endif//USE_OVERFLOW_TABLES

    // regist([&] {
    //     compact_chaining_map<multiplicative_hash<>, uint64_t> filter(ex.KEY_BITSIZE, sizeof(value_type)*8);
    //     ex.execute("chmap64", filter);
    // });
    

#ifdef USE_STANDARD_TABLES
    using mock_key_type = typename experiment_type::mock_key_type; //! if we know that the quotients use much less bits then the key_type, we can store the quotients in a plain array of type `mock_key_type` to save space while being faster than using a bit-compact vector for the bitwidth of the quotients
    if constexpr(!std::is_same<mock_key_type, key_type>::value) {
        regist([&] {
            separate_chaining_map<plain_bucket<mock_key_type>, value_bucket_type  , multiplicative_hash<key_type, mock_key_type> > filter(ex.KEY_BITSIZE, ex.VALUE_BITSIZE);
            ex.execute("plainMI", filter);
        });
        regist([&] {
            separate_chaining_map<plain_bucket<mock_key_type>, value_bucket_type  , multiplicative_hash<key_type, mock_key_type>, arbitrary_resize> filter(ex.KEY_BITSIZE, ex.VALUE_BITSIZE);
            ex.execute("plainMD", filter);
        });

#if defined(__AVX2__) && (defined(STATS_DISABLED) || defined(MALLOC_DISABLED))
        regist([&] {
            separate_chaining_map<avx2_bucket<mock_key_type>, value_bucket_type  , multiplicative_hash<key_type, mock_key_type>, incremental_resize> filter(ex.KEY_BITSIZE, ex.VALUE_BITSIZE);
            ex.execute("avxMI", filter);
        });
        regist([&] {
            separate_chaining_map<avx2_bucket<mock_key_type>, value_bucket_type  , multiplicative_hash<key_type, mock_key_type>, arbitrary_resize> filter(ex.KEY_BITSIZE, ex.VALUE_BITSIZE);
            ex.execute("avxMD", filter);
        });
#endif
    }//if constexpr

    regist([&] {
        separate_chaining_map<plain_bucket<key_type>, value_bucket_type  , hash_mapping_adapter<key_type , SplitMix >> filter;
        ex.execute("plainI", filter);
    });
    regist([&] {
        separate_chaining_map<plain_bucket<key_type>, value_bucket_type  , hash_mapping_adapter<key_type , SplitMix >, arbitrary_resize> filter;
        ex.execute("plainD", filter);
    });
#endif// USE_STANDARD_TABLES

#if defined(__AVX2__) && (defined(STATS_DISABLED) || defined(MALLOC_DISABLED))
    regist([&] {
        separate_chaining_map<avx2_bucket<key_type>, value_bucket_type  , hash_mapping_adapter<key_type , SplitMix >, incremental_resize> filter;
        ex.execute("avxI", filter);
    });
    regist([&] {
        separate_chaining_map<avx2_bucket<key_type>, value_bucket_type  , hash_mapping_adapter<key_type , SplitMix >, arbitrary_resize> filter;
        ex.execute("avxD", filter);
    });
#endif

#ifdef USE_BONSAI_TABLES
    regist([&] {
        using filter_t = tdc::compact_hash::map::sparse_elias_hashmap_t<tdc::dynamic_t>;
        typename filter_t::config_args config;
        // config.size_manager_config.load_factor = ;
        // config.storage_config = ;
        // config.displacement_config.table_config.bucket_size_config.bucket_size = ;

        filter_t filter(0,ex.KEY_BITSIZE,ex.VALUE_BITSIZE, config);
// #ifdef MAX_LOAD_FACTOR // elias is already too slow!
// 		filter.max_load_factor(MAX_LOAD_FACTOR);
// #endif
        ex.execute("eliasS", filter);
    });
    regist([&] {
        using filter_t = tdc::compact_hash::map::sparse_cv_hashmap_t<tdc::dynamic_t>;
        typename filter_t::config_args config;
        // config.size_manager_config.load_factor = ;

        filter_t filter(0,ex.KEY_BITSIZE,ex.VALUE_BITSIZE,config);
#ifdef MAX_LOAD_FACTOR 
		filter.max_load_factor(MAX_LOAD_FACTOR);
#endif
        ex.execute("clearyS", filter);
    });
    regist([&] {
        using filter_t = tdc::compact_hash::map::sparse_layered_hashmap_t<tdc::dynamic_t>;
        typename filter_t::config_args config;
        // config.size_manager_config.load_factor = ;
        // config.displacement_config.table_config.bit_width_config.width = ;

        filter_t filter(0,ex.KEY_BITSIZE,ex.VALUE_BITSIZE,config);
#ifdef MAX_LOAD_FACTOR
		filter.max_load_factor(MAX_LOAD_FACTOR);
#endif
        ex.execute("layeredS", filter);
    });
    regist([&] {
        using filter_t = tdc::compact_hash::map::plain_elias_hashmap_t<tdc::dynamic_t>;
        typename filter_t::config_args config;
        // config.size_manager_config.load_factor = ;
        // config.storage_config.empty_value = ;
        // config.displacement_config.table_config.bucket_size_config.bucket_size = ;

		filter_t filter(0,ex.KEY_BITSIZE,ex.VALUE_BITSIZE,config);
// #ifdef MAX_LOAD_FACTOR // elias is already too slow!
// 		filter.max_load_factor(MAX_LOAD_FACTOR);
// #endif
        ex.execute("eliasP", filter);
    });
    regist([&] {
        using filter_t = tdc::compact_hash::map::plain_cv_hashmap_t<tdc::dynamic_t>;
        typename filter_t::config_args config;
        // config.size_manager_config.load_factor = ;
        // config.storage_config.empty_value = ;

        filter_t filter(0,ex.KEY_BITSIZE,ex.VALUE_BITSIZE,config);
#ifdef MAX_LOAD_FACTOR
		filter.max_load_factor(MAX_LOAD_FACTOR);
#endif
        ex.execute("clearyP", filter);
    });
    regist([&] {
        using filter_t = tdc::compact_hash::map::plain_layered_hashmap_t<tdc::dynamic_t>;
        typename filter_t::config_args config;
        // config.size_manager_config.load_factor = ;
        // config.storage_config.empty_value = ;
        // config.displacement_config.table_config.bit_width_config.width = ;

        filter_t filter(0,ex.KEY_BITSIZE,ex.VALUE_BITSIZE,config);
        ex.execute("layeredP", filter);
    });
#endif//USE_BONSAI_TABLES


#ifdef USE_STANDARD_TABLES
    regist([&] {
        tsl::sparse_map<key_type,value_type,SplitMix> filter;
#ifdef MAX_LOAD_FACTOR
		filter.max_load_factor(MAX_LOAD_FACTOR);
#endif
        ex.execute("tsl", filter);
    });

    regist([&] {
        google::sparse_hash_map<key_type,value_type,SplitMix> filter;
		filter.set_deleted_key(static_cast<key_type>(-1ULL));
#ifdef MAX_LOAD_FACTOR
		filter.max_load_factor(MAX_LOAD_FACTOR);
#endif
        ex.execute("google", filter);
    });
    regist([&] {
        rigtorp::HashMap<key_type,value_type,SplitMix> filter(0, static_cast<key_type>(-1ULL));
        ex.execute("rigtorp", filter);
    });
    regist([&] {
        std::unordered_map<key_type,value_type,SplitMix> filter;
        ex.execute("std", filter);
    });
    regist([&] {
        spp::sparse_hash_map<key_type,value_type,SplitMix> filter;
#ifdef MAX_LOAD_FACTOR
		filter.max_load_factor(MAX_LOAD_FACTOR);
#endif
        ex.execute("spp", filter);
    });
#endif //USE_STANDARD_TABLES

    // TODO: make algorithmens selectable per CLI flag
    for (auto& bench_case : bench_cases) {
        bench_case();
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
