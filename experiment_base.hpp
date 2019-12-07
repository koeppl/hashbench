#pragma once


#include <unordered_map>
#include <rigtorp/HashMap.h>
#include <tsl/sparse_map.h>
#include <separate/separate_chaining_table.hpp>
#include <separate/compact_chaining_map.hpp>
#include <separate/group_chaining.hpp>
#include <sparsehash/sparse_hash_map>

#include <tudocomp_stat/json.hpp>
using namespace nlohmann;
#include <tudocomp_stat/StatPhase.hpp>

#include <tudocomp/util/compact_hash/map/typedefs.hpp>

#include <sparsepp/spp.h>
#include "demangled_type.hpp"
#include "cht_overflow.hpp"

template<class experiment_t>
void run_experiments(experiment_t& ex) {
    using namespace separate_chaining;
    using experiment_type = experiment_t;

    using mock_key_type = typename experiment_type::mock_key_type; //! if we know that the quotients use much less bits then the key_type, we can store the quotients in a plain array of type `mock_key_type` to save space while being faster than using a bit-compact vector for the bitwidth of the quotients
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
    regist([&] {
            group::group_chaining_table<xorshift_hash<>> filter(ex.KEY_BITSIZE, ex.VALUE_BITSIZE);
        ex.execute("grp", filter);
    });
    regist([&] {
            group::group_chaining_table<xorshift_hash<>, cht_overflow<key_type,value_type>> filter(ex.KEY_BITSIZE, ex.VALUE_BITSIZE);
        ex.execute("grp6", filter);
    });

    regist([&] {
        separate_chaining_map<varwidth_bucket<>, value_bucket_type, xorshift_hash<>, incremental_resize, cht_overflow> filter(ex.KEY_BITSIZE, ex.VALUE_BITSIZE);
        ex.execute("chtI6", filter);
    });
    regist([&] {
        compact_chaining_map<xorshift_hash<>, uint8_t> filter(ex.KEY_BITSIZE, ex.VALUE_BITSIZE);
        ex.execute("chmap", filter);
    });
    // regist([&] {
    //     compact_chaining_map<xorshift_hash<>, uint64_t> filter(ex.KEY_BITSIZE, sizeof(value_type)*8);
    //     ex.execute("chmap64", filter);
    // });
    //
    regist([&] {
        separate_chaining_map<varwidth_bucket<>, value_bucket_type, xorshift_hash<>> filter(ex.KEY_BITSIZE, ex.VALUE_BITSIZE);
        ex.execute("chtI", filter);
    });
    regist([&] {
        separate_chaining_map<varwidth_bucket<>, value_bucket_type, xorshift_hash<>, arbitrary_resize> filter(ex.KEY_BITSIZE, ex.VALUE_BITSIZE);
        ex.execute("chtD", filter);
    });

    if constexpr(!std::is_same<mock_key_type, key_type>::value) {
        regist([&] {
            separate_chaining_map<plain_bucket<mock_key_type>, value_bucket_type  , xorshift_hash<key_type, mock_key_type> > filter(ex.KEY_BITSIZE, ex.VALUE_BITSIZE);
            ex.execute("plainMI", filter);
        });
        regist([&] {
            separate_chaining_map<plain_bucket<mock_key_type>, value_bucket_type  , xorshift_hash<key_type, mock_key_type>, arbitrary_resize> filter(ex.KEY_BITSIZE, ex.VALUE_BITSIZE);
            ex.execute("plainMD", filter);
        });

#if defined(STATS_DISABLED) || defined(MALLOC_DISABLED)
        regist([&] {
            separate_chaining_map<avx2_bucket<mock_key_type>, value_bucket_type  , xorshift_hash<key_type, mock_key_type>, incremental_resize> filter(ex.KEY_BITSIZE, ex.VALUE_BITSIZE);
            ex.execute("avxMI", filter);
        });
        regist([&] {
            separate_chaining_map<avx2_bucket<mock_key_type>, value_bucket_type  , xorshift_hash<key_type, mock_key_type>, arbitrary_resize> filter(ex.KEY_BITSIZE, ex.VALUE_BITSIZE);
            ex.execute("avxMD", filter);
        });
#endif
    }//if constexpr

    regist([&] {
        separate_chaining_map<plain_bucket<key_type>, value_bucket_type  , hash_mapping_adapter<key_type , std::hash<key_type> >> filter;
        ex.execute("plainI", filter);
    });
    regist([&] {
        separate_chaining_map<plain_bucket<key_type>, value_bucket_type  , hash_mapping_adapter<key_type , std::hash<key_type> >, arbitrary_resize> filter;
        ex.execute("plainD", filter);
    });

#if defined(STATS_DISABLED) || defined(MALLOC_DISABLED)
    regist([&] {
        separate_chaining_map<avx2_bucket<key_type>, value_bucket_type  , hash_mapping_adapter<key_type , std::hash<key_type> >, incremental_resize> filter;
        ex.execute("avxI", filter);
    });
    regist([&] {
        separate_chaining_map<avx2_bucket<key_type>, value_bucket_type  , hash_mapping_adapter<key_type , std::hash<key_type> >, arbitrary_resize> filter;
        ex.execute("avxD", filter);
    });
#endif

#ifdef USE_BONSAI_TABLES
    regist([&] {
        using filter_t = tdc::compact_hash::map::sparse_elias_hashmap_t<value_type>;
        typename filter_t::config_args config;
        // config.size_manager_config.load_factor = ;
        // config.storage_config = ;
        // config.displacement_config.table_config.bucket_size_config.bucket_size = ;

        filter_t filter(0,ex.KEY_BITSIZE,ex.VALUE_BITSIZE, config);
        ex.execute("eliasS", filter);
    });
    regist([&] {
        using filter_t = tdc::compact_hash::map::sparse_cv_hashmap_t<value_type>;
        typename filter_t::config_args config;
        // config.size_manager_config.load_factor = ;

        filter_t filter(0,ex.KEY_BITSIZE,ex.VALUE_BITSIZE,config);
        ex.execute("clearyS", filter);
    });
    regist([&] {
        using filter_t = tdc::compact_hash::map::sparse_layered_hashmap_t<value_type>;
        typename filter_t::config_args config;
        // config.size_manager_config.load_factor = ;
        // config.displacement_config.table_config.bit_width_config.width = ;

        filter_t filter(0,ex.KEY_BITSIZE,ex.VALUE_BITSIZE,config);
        ex.execute("layeredS", filter);
    });
    regist([&] {
        using filter_t = tdc::compact_hash::map::plain_elias_hashmap_t<value_type>;
        typename filter_t::config_args config;
        // config.size_manager_config.load_factor = ;
        // config.storage_config.empty_value = ;
        // config.displacement_config.table_config.bucket_size_config.bucket_size = ;

    filter_t filter(0,ex.KEY_BITSIZE,ex.VALUE_BITSIZE,config);
        ex.execute("eliasP", filter);
    });
    regist([&] {
        using filter_t = tdc::compact_hash::map::plain_cv_hashmap_t<value_type>;
        typename filter_t::config_args config;
        // config.size_manager_config.load_factor = ;
        // config.storage_config.empty_value = ;

        filter_t filter(0,ex.KEY_BITSIZE,ex.VALUE_BITSIZE,config);
        ex.execute("clearyP", filter);
    });
    regist([&] {
        using filter_t = tdc::compact_hash::map::plain_layered_hashmap_t<value_type>;
        typename filter_t::config_args config;
        // config.size_manager_config.load_factor = ;
        // config.storage_config.empty_value = ;
        // config.displacement_config.table_config.bit_width_config.width = ;

        filter_t filter(0,ex.KEY_BITSIZE,ex.VALUE_BITSIZE,config);
        ex.execute("layeredP", filter);
    });
#endif//USE_BONSAI_TABLES

    regist([&] {
        tsl::sparse_map<key_type,value_type> filter;
        ex.execute("tsl", filter);
    });

    regist([&] {
        google::sparse_hash_map<key_type,value_type> filter;
        ex.execute("google", filter);
    });
    regist([&] {
        rigtorp::HashMap<key_type,value_type> filter(0, static_cast<key_type>(-1ULL));
        ex.execute("rigtorp", filter);
    });
    regist([&] {
        std::unordered_map<key_type,value_type> filter;
        ex.execute("std", filter);
    });
    regist([&] {
        spp::sparse_hash_map<key_type,value_type> filter;
        ex.execute("spp", filter);
    });

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
