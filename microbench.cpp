#ifdef STATS_ENABLED
#undef STATS_ENABLED
#endif

#include "defs.hpp"
#include <celero/Celero.h>

#include <separate/dcheck.hpp>

#ifdef USE_CHMAP_TABLE
#include <separate/compact_chaining_map.hpp>
#endif//USE_CHMAP_TABLE

#include <separate/group_chaining.hpp>
#include <separate/separate_chaining_table.hpp>
#include <map>


#ifdef USE_BONSAI_TABLES
#include <tudocomp/util/compact_hash/map/typedefs.hpp>
#endif

#ifdef USE_BUCKET_TABLES
#include <separate/bucket_table.hpp>
#endif

#include "cht_overflow.hpp"

#ifdef USE_STANDARD_TABLES
#include <rigtorp/HashMap.h>
#include <tsl/sparse_map.h>
#include <patchmap.hpp>
#include <sparsehash/sparse_hash_map>
#include <sparsepp/spp.h>
#endif//USE_STANDARD_TABLES

using namespace separate_chaining;

CELERO_MAIN



class Fixture {
   public:
   using key_type = uint32_t;
   using value_type = default_value_type;
   static constexpr uint8_t VALUE_WIDTH = default_value_width;
   static constexpr uint8_t KEY_WIDTH = 32; // most_significant_bit(NUM_ELEMENTS)
   static_assert(sizeof(key_type)*8 >= KEY_WIDTH, "Num range must fit into key_type");

#ifdef USE_STANDARD_TABLES 
   using rigtorp_type = rigtorp::HashMap<key_type,value_type, SplitMix>;
   using google_type = google::sparse_hash_map<key_type,value_type, SplitMix>;
   using spp_type = spp::sparse_hash_map<key_type,value_type, SplitMix>;
   using tsl_type = tsl::sparse_map<key_type,value_type, SplitMix>;
   using patchmap_type = whash::patchmap<key_type,value_type, SplitMix>; // to make it fair
   // if you want to allow the patchmap to take advantage of bijectivity, however
   // using patchmap_type = whash::patchmap<key_type,value_type>;
#endif//USE_STANDARD_TABLES

   using unordered_type = std::unordered_map<key_type                      , value_type   , SplitMix>;
   using map_type       = std::map<key_type                                , value_type>;

#ifdef USE_PLAIN_TABLES
   using plain_arb_type     = separate_chaining_map<plain_bucket<key_type> , plain_bucket<value_type>  , hash_mapping_adapter<key_type , SplitMix>, arbitrary_resize>;
   using plain_type     = separate_chaining_map<plain_bucket<key_type> , plain_bucket<value_type>  , hash_mapping_adapter<key_type , SplitMix>>;
#endif//USE_PLAIN_TABLES
#ifdef __AVX2__
   using avx2_type      = separate_chaining_map<avx2_bucket<key_type>  , plain_bucket<value_type>  , hash_mapping_adapter<key_type , SplitMix>, incremental_resize>;
   using avx2_arb_type      = separate_chaining_map<avx2_bucket<key_type>  , plain_bucket<value_type>  , hash_mapping_adapter<key_type , SplitMix>, arbitrary_resize>;
#endif//__AVX2__
   using group_type     = group::group_chaining_table<multiplicative_hash<>>;
#ifdef USE_OVERFLOW_TABLES 
   using groupO_type     = group::group_chaining_table<multiplicative_hash<>, cht_overflow<key_type,value_type>>;
   using compactO_type   = separate_chaining_map<varwidth_bucket<>        , plain_bucket<value_type>  , multiplicative_hash<>, incremental_resize, cht_overflow>;
#endif//USE_OVERFLOW_TABLES 
   using compact_type   = separate_chaining_map<varwidth_bucket<>        , plain_bucket<value_type>  , multiplicative_hash<>, incremental_resize>;
   using compact_arb_type   = separate_chaining_map<varwidth_bucket<>        , plain_bucket<value_type>  , multiplicative_hash<>, arbitrary_resize>;
#ifdef USE_CHMAP_TABLE
   using compact_chain_type   = compact_chaining_map<multiplicative_hash<> >;
#endif//USE_CHMAP_TABLE
#ifdef USE_BONSAI_TABLES
   using eliasS_type     = tdc::compact_hash::map::sparse_elias_hashmap_t<value_type>;
   using clearyS_type    = tdc::compact_hash::map::sparse_cv_hashmap_t<value_type>;
   using layeredS_type   =tdc::compact_hash::map::sparse_layered_hashmap_t<value_type>;
   using eliasP_type     = tdc::compact_hash::map::plain_elias_hashmap_t<value_type>;
   using clearyP_type    = tdc::compact_hash::map::plain_cv_hashmap_t<value_type>;
   using layeredP_type   =tdc::compact_hash::map::plain_layered_hashmap_t<value_type>;
#endif//USE_BONSAI_TABLES

#ifdef USE_BUCKET_TABLES
   using bucket_type   = bucket_table<varwidth_bucket<>        , plain_bucket<value_type>, incremental_resize>;
   using bucket_arb_type   = bucket_table<varwidth_bucket<>        , plain_bucket<value_type>, arbitrary_resize_bucket>;
#ifdef __AVX2__
using bucket_avx2_type   = bucket_table<varwidth_bucket<>        , plain_bucket<value_type>, incremental_resize>;
#endif //__AVX2__
#endif//USE_BUCKET_TABLES

   static constexpr size_t m_missed_els_size = 1024;
   std::vector<key_type> m_missed_els;
   map_type* m_map = nullptr;
#ifdef USE_PLAIN_TABLES
   plain_type* m_plain = nullptr;
   plain_arb_type* m_plain_arb = nullptr;
#endif//USE_PLAIN_TABLES
#ifdef __AVX2__
avx2_type* m_avx = nullptr;
avx2_arb_type* m_avx_arb = nullptr;
#endif //__AVX2__
#ifdef USE_OVERFLOW_TABLES 
   groupO_type* m_groupO = nullptr;
   compactO_type* m_compactO = nullptr;
#endif//USE_OVERFLOW_TABLES 
   group_type* m_group = nullptr;
   compact_type* m_compact = nullptr;
   compact_arb_type* m_compact_arb = nullptr;
#ifdef USE_CHMAP_TABLE
   compact_chain_type *m_compact_chain = nullptr;
#endif//USE_CHMAP_TABLE
   unordered_type* m_ordered = nullptr;

#ifdef USE_STANDARD_TABLES 
   rigtorp_type* m_rigtorp = nullptr;
   google_type* m_google = nullptr;
   spp_type* m_spp = nullptr;
   tsl_type* m_tsl = nullptr;
   patchmap_type* m_patchmap = nullptr;
#endif//USE_STANDARD_TABLES

#ifdef USE_BONSAI_TABLES
   eliasP_type* m_eliasP = nullptr;
   clearyP_type* m_clearyP = nullptr;
   layeredP_type* m_layeredP = nullptr;
   eliasS_type* m_eliasS = nullptr;
   clearyS_type* m_clearyS = nullptr;
   layeredS_type* m_layeredS = nullptr;
#endif//USE_BONSAI_TABLES

#ifdef USE_BUCKET_TABLES
   bucket_type* m_bucket = nullptr;
   bucket_arb_type* m_bucket_arb = nullptr;
#ifdef __AVX2__
bucket_avx2_type* m_bucket_avx2 = nullptr;
#endif //__AVX2__
#endif//USE_BUCKET_TABLES

   size_t m_current_instance = 0;

   void setUp(size_t i) {
      DCHECK_GT(i, 0);
      if(i == m_current_instance) return;
      tearDown();
      m_current_instance = i;

      m_map = new map_type();

#ifdef USE_PLAIN_TABLES
      m_plain = new plain_type(KEY_WIDTH, VALUE_WIDTH);
      m_plain_arb = new plain_arb_type(KEY_WIDTH, VALUE_WIDTH);
#endif//USE_PLAIN_TABLES

#ifdef __AVX2__
      m_avx = new avx2_type(KEY_WIDTH, VALUE_WIDTH);
      m_avx_arb = new avx2_arb_type(KEY_WIDTH, VALUE_WIDTH);
#endif //__AVX2__
#ifdef USE_OVERFLOW_TABLES 
      m_groupO = new groupO_type(KEY_WIDTH, VALUE_WIDTH);
      m_compactO = new compactO_type(KEY_WIDTH, VALUE_WIDTH);
#endif//USE_OVERFLOW_TABLES 
      m_group = new group_type(KEY_WIDTH, VALUE_WIDTH);
      m_compact = new compact_type(KEY_WIDTH, VALUE_WIDTH);
      m_compact_arb = new compact_arb_type(KEY_WIDTH, VALUE_WIDTH);
#ifdef USE_CHMAP_TABLE
      m_compact_chain = new compact_chain_type(KEY_WIDTH, VALUE_WIDTH);
#endif//USE_CHMAP_TABLE
      m_ordered = new unordered_type();
#ifdef USE_STANDARD_TABLES
      m_rigtorp = new rigtorp_type(0, static_cast<key_type>(-1ULL));
      m_google = new google_type();
      m_spp = new spp_type();
      m_tsl = new tsl_type();
      m_patchmap = new patchmap_type();
#ifdef MAX_LOAD_FACTOR 
	  m_google->max_load_factor(MAX_LOAD_FACTOR);
	  m_spp->max_load_factor(MAX_LOAD_FACTOR);
	  m_tsl->max_load_factor(MAX_LOAD_FACTOR);
//  patchmap does not support setting max load factor at runtime
#endif//MAX_LOAD_FACTOR
#endif//USE_STANDARD_TABLES

#ifdef USE_BONSAI_TABLES
      m_eliasS = new eliasS_type(0,KEY_WIDTH, VALUE_WIDTH);
      m_layeredS = new layeredS_type(0,KEY_WIDTH, VALUE_WIDTH);
      m_clearyS = new clearyS_type(0,KEY_WIDTH, VALUE_WIDTH);
      m_eliasP = new eliasP_type(0,KEY_WIDTH, VALUE_WIDTH);
      m_layeredP = new layeredP_type(0,KEY_WIDTH, VALUE_WIDTH);
      m_clearyP = new clearyP_type(0,KEY_WIDTH, VALUE_WIDTH);
#ifdef MAX_LOAD_FACTOR 
	  m_layeredS->max_load_factor(MAX_LOAD_FACTOR);
	  m_clearyS->max_load_factor(MAX_LOAD_FACTOR);
	  m_layeredP->max_load_factor(MAX_LOAD_FACTOR);
	  m_clearyP->max_load_factor(MAX_LOAD_FACTOR);
#endif//MAX_LOAD_FACTOR
#endif//USE_BONSAI_TABLES

#ifdef USE_BUCKET_TABLES
      m_bucket = new bucket_type(KEY_WIDTH);
      m_bucket_arb = new bucket_arb_type(KEY_WIDTH);
#ifdef __AVX2__
m_bucket_avx2 = new bucket_avx2_type(KEY_WIDTH);
#endif //__AVX2__
#endif//USE_BUCKET_TABLES

	  random_int_reset();
      for(size_t val = 0; val < m_current_instance;) {
	 const key_type key = random_int(1ULL<<KEY_WIDTH);
	 if( m_map->find(key) != m_map->end()) continue;
	 (*m_map)[key] = static_cast<value_type>(val++ % std::numeric_limits<value_type>::max());
      }
      m_missed_els.clear();
      m_missed_els.reserve(m_missed_els_size);
      for(size_t val = 0; val < m_missed_els_size;) {
	 const key_type key = random_int(1ULL<<KEY_WIDTH);
	 if( m_map->find(key) != m_map->end()) continue;
	 m_missed_els.push_back(key); 
	 ++val;
      }
	  CHECK_EQ(m_missed_els.size(), m_missed_els_size);
      for(auto el : *m_map) {
#ifdef __AVX2__
(*m_avx)[el.first] = el.second;
(*m_avx_arb)[el.first] = el.second;
#endif //__AVX2__
#ifdef USE_OVERFLOW_TABLES 
	 (*m_groupO)[el.first] = el.second;
	 (*m_compactO)[el.first] = el.second;
#endif//USE_OVERFLOW_TABLES 

	 (*m_group)[el.first] = el.second;
	 (*m_compact)[el.first] = el.second;
	 (*m_compact_arb)[el.first] = el.second;
#ifdef USE_CHMAP_TABLE
	 (*m_compact_chain)[el.first] = el.second;
#endif//USE_CHMAP_TABLE

	 (*m_ordered)[el.first] = el.second;
#ifdef USE_PLAIN_TABLES
	 (*m_plain)[el.first] = el.second;
	 (*m_plain_arb)[el.first] = el.second;
#endif//USE_PLAIN_TABLES
#ifdef USE_STANDARD_TABLES 
	 (*m_google)[el.first] = el.second;
	 (*m_spp)[el.first] = el.second;
	 (*m_rigtorp)[el.first] = el.second;
	 (*m_tsl)[el.first] = el.second;
	 (*m_patchmap)[el.first] = el.second;
#endif//USE_STANDARD_TABLES

#ifdef USE_BONSAI_TABLES
	 (*m_clearyP)[el.first] = el.second;
	 (*m_layeredP)[el.first] = el.second;
	 (*m_eliasP)[el.first] = el.second;
	 (*m_clearyS)[el.first] = el.second;
	 (*m_layeredS)[el.first] = el.second;
	 (*m_eliasS)[el.first] = el.second;
#endif//USE_BONSAI_TABLES
#ifdef USE_BUCKET_TABLES
      (*m_bucket)[el.first]      = el.second;
      (*m_bucket_arb)[el.first]  = el.second;
#ifdef __AVX2__
(*m_bucket_avx2)[el.first] = el.second;
#endif //__AVX2__
#endif//USE_BUCKET_TABLES

#ifdef __AVX2__
DCHECK_EQ((*m_avx)[el.first], el.second);
DCHECK_EQ((*m_avx_arb)[el.first], el.second);
#endif //__AVX2__
#ifdef USE_OVERFLOW_TABLES 
	 DCHECK_EQ((*m_groupO)[el.first], el.second);
	 DCHECK_EQ((*m_compactO)[el.first], el.second);
#endif//USE_OVERFLOW_TABLES 
	 DCHECK_EQ((*m_group)[el.first], el.second);
	 DCHECK_EQ((*m_compact)[el.first], el.second);
	 DCHECK_EQ((*m_compact_arb)[el.first], el.second);

#ifdef USE_CHMAP_TABLE
	 DCHECK_EQ((*m_compact_chain)[el.first], el.second);
#endif//USE_CHMAP_TABLE


	 DCHECK_EQ((*m_ordered)[el.first], el.second);
#ifdef USE_PLAIN_TABLES
	 DCHECK_EQ((*m_plain)[el.first], el.second);
	 DCHECK_EQ((*m_plain_arb)[el.first], el.second);
#endif//USE_PLAIN_TABLES
#ifdef USE_STANDARD_TABLES
	 DCHECK_EQ((*m_google)[el.first], el.second);
	 DCHECK_EQ((*m_spp)[el.first], el.second);
	 DCHECK_EQ((*m_rigtorp)[el.first], el.second);
	 DCHECK_EQ((*m_tsl)[el.first], el.second);
	 DCHECK_EQ((*m_patchmap)[el.first], el.second);
#endif//USE_STANDARD_TABLES

#ifdef USE_BONSAI_TABLES
	 DCHECK_EQ((*m_clearyP)[el.first], el.second);
	 DCHECK_EQ((*m_eliasP)[el.first], el.second);
	 DCHECK_EQ((*m_layeredP)[el.first], el.second);
	 DCHECK_EQ((*m_clearyS)[el.first], el.second);
	 DCHECK_EQ((*m_eliasS)[el.first], el.second);
	 DCHECK_EQ((*m_layeredS)[el.first], el.second);
#endif//USE_BONSAI_TABLES
#ifdef USE_BUCKET_TABLES
      DCHECK_EQ((*m_bucket)[el.first]      , el.second);
      DCHECK_EQ((*m_bucket_arb)[el.first]  , el.second);
#ifdef __AVX2__
DCHECK_EQ((*m_bucket_avx2)[el.first] , el.second);
#endif //__AVX2__
#endif//USE_BUCKET_TABLES

      }
#ifdef __AVX2__
DCHECK_EQ(m_avx->size(), m_map->size());
DCHECK_EQ(m_avx_arb->size(), m_map->size());
#endif //__AVX2__
#ifdef USE_OVERFLOW_TABLES 
      DCHECK_EQ(m_groupO->size(), m_map->size());
      DCHECK_EQ(m_compactO->size(), m_map->size());
#endif//USE_OVERFLOW_TABLES 
      DCHECK_EQ(m_group->size(), m_map->size());
      DCHECK_EQ(m_compact->size(), m_map->size());
      DCHECK_EQ(m_compact_arb->size(), m_map->size());

#ifdef USE_CHMAP_TABLE
      DCHECK_EQ(m_compact_chain->size(), m_map->size());
#endif//USE_CHMAP_TABLE


      DCHECK_EQ(m_ordered->size(), m_map->size());
#ifdef USE_PLAIN_TABLES
      DCHECK_EQ(m_plain->size(), m_map->size());
      DCHECK_EQ(m_plain_arb->size(), m_map->size());
#endif//USE_PLAIN_TABLES
#ifdef USE_STANDARD_TABLES
      DCHECK_EQ(m_google->size(), m_map->size());
      DCHECK_EQ(m_spp->size(), m_map->size());
      DCHECK_EQ(m_rigtorp->size(), m_map->size());
      DCHECK_EQ(m_tsl->size(), m_map->size());
      DCHECK_EQ(m_patchmap->size(), m_map->size());
#endif//USE_STANDARD_TABLES

#ifdef USE_BONSAI_TABLES
      DCHECK_EQ(m_clearyP->size(), m_map->size());
      DCHECK_EQ(m_eliasP->size(), m_map->size());
      DCHECK_EQ(m_layeredP->size(), m_map->size());
      DCHECK_EQ(m_clearyS->size(), m_map->size());
      DCHECK_EQ(m_eliasS->size(), m_map->size());
      DCHECK_EQ(m_layeredS->size(), m_map->size());
#endif//USE_BONSAI_TABLES
#ifdef USE_BUCKET_TABLES
      DCHECK_EQ((m_bucket)     ->size() , m_map->size());
      DCHECK_EQ((m_bucket_arb) ->size() , m_map->size());
#ifdef __AVX2__
DCHECK_EQ((m_bucket_avx2)->size() , m_map->size());
#endif //__AVX2__
#endif//USE_BUCKET_TABLES
   }
   void tearDown() {
      if(m_map != nullptr) {
	 delete m_map;
	 delete m_ordered;
#ifdef USE_PLAIN_TABLES
	 delete m_plain;
	 delete m_plain_arb;
#endif//USE_PLAIN_TABLES
#ifdef USE_STANDARD_TABLES
	 delete m_rigtorp;
	 delete m_google;
	 delete m_spp;
	 delete m_tsl;
	 delete m_patchmap;
#endif//USE_STANDARD_TABLES
#ifdef __AVX2__
delete m_avx;
delete m_avx_arb;
#endif //__AVX2__
#ifdef USE_OVERFLOW_TABLES 
	 delete m_groupO;
	 delete m_compactO;
#endif//USE_OVERFLOW_TABLES 
	 delete m_group;
	 delete m_compact;
	 delete m_compact_arb;
#ifdef USE_CHMAP_TABLE
	 delete m_compact_chain;
#endif//USE_CHMAP_TABLE
#ifdef USE_BONSAI_TABLES
	 delete m_eliasS;
	 delete m_layeredS;
	 delete m_clearyS;
	 delete m_eliasP;
	 delete m_layeredP;
	 delete m_clearyP;
#endif//USE_BONSAI_TABLES
#ifdef USE_BUCKET_TABLES
	 delete m_bucket;
	 delete m_bucket_arb; 
#ifdef __AVX2__
delete m_bucket_avx2;
#endif //__AVX2__
#endif//USE_BUCKET_TABLES
      }
      m_map = nullptr;
   }
   ~Fixture() {
      tearDown();
   }
};
Fixture static_fixture;



class TableFixture : public celero::TestFixture {

   protected:

#ifndef INSTANCE_LENGTH 
#ifdef USE_BUCKET_TABLES
   static constexpr size_t m_instance_length = 255/3+2;
#else
   static constexpr size_t m_instance_length = 19;
#endif//USE_BUCKET_TABLES
#else 
   static constexpr size_t m_instance_length = INSTANCE_LENGTH;
#endif//INSTANCE_LENGTH


   std::vector<celero::TestFixture::ExperimentValue> m_problemspace;

   size_t m_current_instance = 0;


   public:

   // const TableInstance& instance() const {
   //    return *m_instances[m_current_instance];
   // }
#ifdef USE_BUCKET_TABLES
   TableFixture()
      : m_problemspace(m_instance_length,0)
   {
      for(size_t i = 0; i < m_instance_length; ++i) {
	 m_problemspace[i] = 2+3*i; 
      }
   }
#else
   TableFixture()
      : m_problemspace(m_instance_length,0)
   {
      size_t value = 32768;
      for(size_t i = 0; i < m_instance_length; ++i) {
	 m_problemspace[i] = value;
	 value = (value*3)/2;
      }
   }
#endif

   ~TableFixture() {
   }


   virtual std::vector<celero::TestFixture::ExperimentValue> getExperimentValues() const override
   {
      return m_problemspace;
   }

   virtual void setUp(const celero::TestFixture::ExperimentValue& experimentValue) override {
      static_fixture.setUp(experimentValue.Value);
   }
   virtual void tearDown() override {
   }


};


BASELINE_F(query, std, TableFixture, CELERO_SAMPLING_COUNT, CELERO_OPERATION_COUNT)
{
   const auto& map = *(static_fixture.m_ordered);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(map.find(el.first));
   }
}

#ifdef USE_PLAIN_TABLES
BENCHMARK_F(query, plainI, TableFixture, CELERO_SAMPLING_COUNT, CELERO_OPERATION_COUNT)
{
   const auto& plain = *(static_fixture.m_plain);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(plain.find(el.first));
   }
}

BENCHMARK_F(query, plainD, TableFixture, CELERO_SAMPLING_COUNT, CELERO_OPERATION_COUNT)
{
   const auto& plain_arb = *(static_fixture.m_plain_arb);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(plain_arb.find(el.first));
   }
}
#endif//USE_PLAIN_TABLES

#ifdef __AVX2__
BENCHMARK_F(query, avxI, TableFixture, CELERO_SAMPLING_COUNT, CELERO_OPERATION_COUNT)
{
   const auto& avx = *(static_fixture.m_avx);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(avx.find(el.first));
   }
}
BENCHMARK_F(query, avxD, TableFixture, CELERO_SAMPLING_COUNT, CELERO_OPERATION_COUNT)
{
   const auto& avx = *(static_fixture.m_avx_arb);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(avx.find(el.first));
   }
}
#endif //__AVX2__

#ifdef USE_OVERFLOW_TABLES 
BENCHMARK_F(query, grpO, TableFixture, CELERO_SAMPLING_COUNT, CELERO_OPERATION_COUNT)
{
   const auto& groupO = *(static_fixture.m_groupO);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(groupO.find(el.first));
   }
}
BENCHMARK_F(query, chtIO, TableFixture, CELERO_SAMPLING_COUNT, CELERO_OPERATION_COUNT)
{
   const auto& compactO = *(static_fixture.m_compactO);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(compactO.find(el.first));
   }
}
#endif//USE_OVERFLOW_TABLES 

BENCHMARK_F(query, grp, TableFixture, CELERO_SAMPLING_COUNT, CELERO_OPERATION_COUNT)
{
   const auto& group = *(static_fixture.m_group);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(group.find(el.first));
   }
}


BENCHMARK_F(query, chtI, TableFixture, CELERO_SAMPLING_COUNT, CELERO_OPERATION_COUNT)
{
   const auto& compact = *(static_fixture.m_compact);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(compact.find(el.first));
   }
}
BENCHMARK_F(query, chtD, TableFixture, CELERO_SAMPLING_COUNT, CELERO_OPERATION_COUNT)
{
   const auto& compact = *(static_fixture.m_compact_arb);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(compact.find(el.first));
   }
}
#ifdef USE_CHMAP_TABLE
BENCHMARK_F(query, chmap, TableFixture, CELERO_SAMPLING_COUNT, CELERO_OPERATION_COUNT)
{
   const auto& compact = *(static_fixture.m_compact_chain);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(compact.find(el.first));
   }
}
#endif//USE_CHMAP_TABLE

#ifdef USE_STANDARD_TABLES
BENCHMARK_F(query, rigtorp, TableFixture, CELERO_SAMPLING_COUNT, CELERO_OPERATION_COUNT)
{
   const auto& mappe = *(static_fixture.m_rigtorp);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(mappe.find(el.first));
   }
}
BENCHMARK_F(query, google, TableFixture, CELERO_SAMPLING_COUNT, CELERO_OPERATION_COUNT)
{
   const auto& mappe = *(static_fixture.m_google);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(mappe.find(el.first));
   }
}
BENCHMARK_F(query, spp, TableFixture, CELERO_SAMPLING_COUNT, CELERO_OPERATION_COUNT)
{
   const auto& mappe = *(static_fixture.m_spp);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(mappe.find(el.first));
   }
}
BENCHMARK_F(query, tsl, TableFixture, CELERO_SAMPLING_COUNT, CELERO_OPERATION_COUNT)
{
   const auto& mappe = *(static_fixture.m_tsl);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(mappe.find(el.first));
   }
}
BENCHMARK_F(query, patchmap, TableFixture, CELERO_SAMPLING_COUNT, CELERO_OPERATION_COUNT)
{
   const auto& mappe = *(static_fixture.m_patchmap);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(mappe.find(el.first));
   }
}
#endif//USE_STANDARD_TABLES

#ifdef USE_BONSAI_TABLES
BENCHMARK_F(query, clearyS, TableFixture, CELERO_SAMPLING_COUNT, CELERO_OPERATION_COUNT)
{
   auto& cleary = *(static_fixture.m_clearyS);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(cleary[el.first]);
   }
}
BENCHMARK_F(query, clearyP, TableFixture, CELERO_SAMPLING_COUNT, CELERO_OPERATION_COUNT)
{
   auto& cleary = *(static_fixture.m_clearyP);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(cleary[el.first]);
   }
}
BENCHMARK_F(query, eliasS, TableFixture, CELERO_SAMPLING_COUNT, CELERO_OPERATION_COUNT)
{
   auto& elias = *(static_fixture.m_eliasS);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(elias[el.first]);
   }
}

BENCHMARK_F(query, eliasP, TableFixture, CELERO_SAMPLING_COUNT, CELERO_OPERATION_COUNT)
{
   auto& elias = *(static_fixture.m_eliasP);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(elias[el.first]);
   }
}
BENCHMARK_F(query, layeredS, TableFixture, CELERO_SAMPLING_COUNT, CELERO_OPERATION_COUNT)
{
   auto& layered = *(static_fixture.m_layeredS);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(layered[el.first]);
   }
}

BENCHMARK_F(query, layeredP, TableFixture, CELERO_SAMPLING_COUNT, CELERO_OPERATION_COUNT)
{
   auto& layered = *(static_fixture.m_layeredP);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(layered[el.first]);
   }
}
#endif//USE_BONSAI_TABLES
#ifdef USE_BUCKET_TABLES
BENCHMARK_F(query, bucket, TableFixture, CELERO_SAMPLING_COUNT, CELERO_OPERATION_COUNT)
{
   const auto& bucket = *(static_fixture.m_bucket);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(bucket.find(el.first));
   }
}
BENCHMARK_F(query, bucket_arb, TableFixture, CELERO_SAMPLING_COUNT, CELERO_OPERATION_COUNT)
{
   const auto& bucket_arb = *(static_fixture.m_bucket_arb);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(bucket_arb.find(el.first));
   }
}
#ifdef __AVX2__
BENCHMARK_F(query, bucket_avx2, TableFixture, CELERO_SAMPLING_COUNT, CELERO_OPERATION_COUNT)
{
   const auto& bucket_avx2 = *(static_fixture.m_bucket_avx2);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(bucket_avx2.find(el.first));
   }
}
#endif //__AVX2__
#endif//USE_BUCKET_TABLES

#define BENCH_INSERT(name,cons) \
	 BENCHMARK_F(insert, name, TableFixture, CELERO_SAMPLING_COUNT, CELERO_OPERATION_COUNT) { \
	    auto map = Fixture::cons; \
	    for(auto el : *static_fixture.m_map) { map[el.first] = el.second; }}



BASELINE_F(insert, std, TableFixture, CELERO_SAMPLING_COUNT, CELERO_OPERATION_COUNT)
{
   auto map = Fixture::unordered_type();
   for(auto el : *static_fixture.m_map) {
      map[el.first] = el.second;
   }
}
#ifdef USE_PLAIN_TABLES
BENCH_INSERT(plainI, plain_type(Fixture::KEY_WIDTH, Fixture::VALUE_WIDTH))
BENCH_INSERT(plainD, plain_arb_type(Fixture::KEY_WIDTH, Fixture::VALUE_WIDTH))
#endif//USE_PLAIN_TABLES
#ifdef __AVX2__
BENCH_INSERT(avxI, avx2_type(Fixture::KEY_WIDTH, Fixture::VALUE_WIDTH))
BENCH_INSERT(avxD, avx2_arb_type(Fixture::KEY_WIDTH, Fixture::VALUE_WIDTH))
#endif //__AVX2__
#ifdef USE_OVERFLOW_TABLES
BENCH_INSERT(grpO, groupO_type(Fixture::KEY_WIDTH, Fixture::VALUE_WIDTH))
BENCH_INSERT(chtIO, compactO_type(Fixture::KEY_WIDTH, Fixture::VALUE_WIDTH))
#endif//USE_OVERFLOW_TABLES
BENCH_INSERT(grp, group_type(Fixture::KEY_WIDTH, Fixture::VALUE_WIDTH))
BENCH_INSERT(chtI, compact_type(Fixture::KEY_WIDTH, Fixture::VALUE_WIDTH))
BENCH_INSERT(chtD, compact_arb_type(Fixture::KEY_WIDTH, Fixture::VALUE_WIDTH))
#ifdef USE_CHMAP_TABLE
BENCH_INSERT(chmap, compact_chain_type(Fixture::KEY_WIDTH, Fixture::VALUE_WIDTH))
#endif//USE_CHMAP_TABLE
#ifdef USE_BONSAI_TABLES
BENCH_INSERT(clearyP, clearyP_type(0,Fixture::KEY_WIDTH, Fixture::VALUE_WIDTH))
BENCH_INSERT(eliasP, eliasP_type(0,Fixture::KEY_WIDTH, Fixture::VALUE_WIDTH))
BENCH_INSERT(layeredP, layeredP_type(0,Fixture::KEY_WIDTH, Fixture::VALUE_WIDTH))
BENCH_INSERT(clearyS, clearyS_type(0,Fixture::KEY_WIDTH, Fixture::VALUE_WIDTH))
BENCH_INSERT(eliasS, eliasS_type(0,Fixture::KEY_WIDTH, Fixture::VALUE_WIDTH))
BENCH_INSERT(layeredS, layeredS_type(0,Fixture::KEY_WIDTH, Fixture::VALUE_WIDTH))
#endif//USE_BONSAI_TABLES
#ifdef USE_BUCKET_TABLES
BENCH_INSERT(bucket, bucket_type())
BENCH_INSERT(bucket_arb, bucket_arb_type())
#ifdef __AVX2__
BENCH_INSERT(bucket_avx2, bucket_avx2_type())
#endif //__AVX2__
#endif//USE_BUCKET_TABLES
#ifdef USE_STANDARD_TABLES
BENCH_INSERT(google, google_type())
BENCH_INSERT(spp, spp_type())
BENCH_INSERT(rigtorp, rigtorp_type(0, static_cast<Fixture::key_type>(-1ULL)))
BENCH_INSERT(tsl, tsl_type())
BENCH_INSERT(patchmap, patchmap_type())
#endif //USE_STANDARD_TABLES


BASELINE_F(miss, std, TableFixture, 100, CELERO_OPERATION_COUNT)
{
   auto& map = *(static_fixture.m_ordered);
   for(auto el : static_fixture.m_missed_els) { celero::DoNotOptimizeAway(map[el]); }
}


#define BENCH_MISS(name,instance) \
      BENCHMARK_F(miss, name, TableFixture, 100 , CELERO_OPERATION_COUNT) { \
	 auto& map = *(static_fixture.instance);\
	 for(auto el : static_fixture.m_missed_els) { celero::DoNotOptimizeAway(map[el]); }}


#ifdef __AVX2__
BENCH_MISS(avxI, m_avx)
BENCH_MISS(avxD, m_avx_arb)
#endif //__AVX2__
#ifdef USE_OVERFLOW_TABLES 
BENCH_MISS(grpO, m_groupO)
BENCH_MISS(chtIO, m_compactO)
#endif//USE_OVERFLOW_TABLES 
BENCH_MISS(grp, m_group)
BENCH_MISS(chtI, m_compact)
BENCH_MISS(chtD, m_compact_arb)
#ifdef USE_CHMAP_TABLE
BENCH_MISS(chmap, m_compact_chain)
#endif//USE_CHMAP_TABLE
#ifdef USE_BONSAI_TABLES
BENCH_MISS(clearyP, m_clearyP)
BENCH_MISS(eliasP, m_eliasP)
BENCH_MISS(layeredP, m_layeredP)
BENCH_MISS(clearyS, m_clearyS)
BENCH_MISS(eliasS, m_eliasS)
BENCH_MISS(layeredS, m_layeredS)
#endif//USE_BONSAI_TABLES

#ifdef USE_PLAIN_TABLES
BENCH_MISS(plainI, m_plain)
BENCH_MISS(plainD, m_plain_arb)
#endif//USE_PLAIN_TABLES

#ifdef USE_STANDARD_TABLES
BENCH_MISS(google, m_google)
BENCH_MISS(spp, m_spp)
BENCH_MISS(rigtorp, m_rigtorp)
BENCH_MISS(tsl, m_tsl)
BENCH_MISS(patchmap, m_patchmap)
#endif//USE_STANDARD_TABLES



class EraseFixture : public TableFixture {
   public:
      using key_type = Fixture::key_type;
      using value_type = Fixture::value_type;
   static constexpr size_t m_delete_entries_size = 1024;
   std::vector<std::pair<key_type, value_type>> m_delete_entries;

   EraseFixture() { }


   virtual std::vector<celero::TestFixture::ExperimentValue> getExperimentValues() const override {
      return TableFixture::getExperimentValues();
   }

   virtual void setUp(const celero::TestFixture::ExperimentValue& experimentValue) override {
	   TableFixture::setUp(experimentValue.Value);
	   m_delete_entries.clear();
	   m_delete_entries.reserve(m_delete_entries_size);
	   size_t el = 0; // TODO: this is a bug since it never gets incremented
	   for(auto it = static_fixture.m_map->begin(); it != static_fixture.m_map->end() && m_delete_entries.size() < m_delete_entries_size; ++it) {
		   if(el % (static_fixture.m_map->size() / m_delete_entries_size) == 0) {
			   m_delete_entries.push_back(std::make_pair(it->first, it->second));
		   }
		   ++el;
	   }
	   CHECK_EQ(m_delete_entries.size(), m_delete_entries_size);
   }

   template<class T>
   void reinsert_elements(T& table) {
      for(auto p : m_delete_entries) {
	 table[p.first] = p.second;
      }
   }

   virtual void tearDown() override {
      reinsert_elements(*static_fixture.m_ordered);
#ifdef USE_PLAIN_TABLES
      reinsert_elements(*static_fixture.m_plain);
      reinsert_elements(*static_fixture.m_plain_arb);
#endif//USE_PLAIN_TABLES
#ifdef __AVX2__
reinsert_elements(*static_fixture.m_avx);
reinsert_elements(*static_fixture.m_avx_arb);
#endif //__AVX2__
#ifdef USE_OVERFLOW_TABLES 
      reinsert_elements(*static_fixture.m_groupO);
      reinsert_elements(*static_fixture.m_compactO);
#endif//USE_OVERFLOW_TABLES 
      reinsert_elements(*static_fixture.m_group);
      reinsert_elements(*static_fixture.m_compact);
      reinsert_elements(*static_fixture.m_compact_arb);
#ifdef USE_CHMAP_TABLE
      reinsert_elements(*static_fixture.m_compact_chain);
#endif//USE_CHMAP_TABLE
// #ifdef USE_BONSAI_TABLES
//       reinsert_elements(*static_fixture.m_clearyP);
//       reinsert_elements(*static_fixture.m_eliasP);
//       reinsert_elements(*static_fixture.m_layeredP);
//       reinsert_elements(*static_fixture.m_clearyS);
//       reinsert_elements(*static_fixture.m_eliasS);
//       reinsert_elements(*static_fixture.m_layeredS);
// #endif//USE_BONSAI_TABLES
#ifdef USE_STANDARD_TABLES
      reinsert_elements(*static_fixture.m_spp);
      reinsert_elements(*static_fixture.m_google);
      reinsert_elements(*static_fixture.m_rigtorp);
      reinsert_elements(*static_fixture.m_tsl);
      reinsert_elements(*static_fixture.m_patchmap);
#endif//USE_STANDARD_TABLES
   }


};

BASELINE_F(erase, std, EraseFixture, 100 , CELERO_OPERATION_COUNT)
{
   auto& map = *(static_fixture.m_ordered);
   for(auto el : this->m_delete_entries) { celero::DoNotOptimizeAway(map.erase(el.first)); }
}


#define BENCH_ERASE(name,instance) \
      BENCHMARK_F(erase, name, EraseFixture, 100, CELERO_OPERATION_COUNT) { \
	 auto& map = *(static_fixture.instance);\
	 for(auto el : this->m_delete_entries) { celero::DoNotOptimizeAway(map.erase(el.first)); }}


#ifdef USE_PLAIN_TABLES
BENCH_ERASE(plainI, m_plain)
BENCH_ERASE(plainD, m_plain_arb)
#endif//USE_PLAIN_TABLES
#ifdef __AVX2__
BENCH_ERASE(avxI, m_avx)
BENCH_ERASE(avxD, m_avx_arb)
#endif //__AVX2__
#ifdef USE_OVERFLOW_TABLES 
BENCH_ERASE(grpO, m_groupO)
BENCH_ERASE(chtIO, m_compactO)
#endif//USE_OVERFLOW_TABLES 
BENCH_ERASE(grp, m_group)
BENCH_ERASE(chtI, m_compact)
BENCH_ERASE(chtD, m_compact_arb)
#ifdef USE_CHMAP_TABLE
BENCH_ERASE(chmap, m_compact_chain)
#endif//USE_CHMAP_TABLE
// #ifdef USE_BONSAI_TABLES
// BENCH_ERASE(clearyP, m_clearyP)
// BENCH_ERASE(eliasP, m_eliasP)
// BENCH_ERASE(layeredP, m_layeredP)
// BENCH_ERASE(clearyS, m_clearyS)
// BENCH_ERASE(eliasS, m_eliasS)
// BENCH_ERASE(layeredS, m_layeredS)
// #endif//USE_BONSAI_TABLES

#ifdef USE_STANDARD_TABLES
BENCH_ERASE(google, m_google)
BENCH_ERASE(spp, m_spp)
BENCH_ERASE(rigtorp, m_rigtorp)
BENCH_ERASE(tsl, m_tsl)
BENCH_ERASE(patchmap, m_patchmap)
#endif//USE_STANDARD_TABLES

