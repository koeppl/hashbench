#ifdef STATS_ENABLED
#undef STATS_ENABLED
#endif

#include <celero/Celero.h>

#include <rigtorp/HashMap.h>
#include <tsl/sparse_map.h>
#include <sparsehash/sparse_hash_map>

#include <separate/separate_chaining_table.hpp>
#include <separate/compact_chaining_map.hpp>
#include <separate/dcheck.hpp>
#include <map>

#ifdef USE_BONSAI_TABLES
#include <tudocomp/util/compact_hash/map/typedefs.hpp>
#endif

#ifdef USE_BUCKET_TABLES
#include <separate/bucket_table.hpp>
#endif

using namespace separate_chaining;

CELERO_MAIN

template<class T>
T random_int(const T& maxvalue) {
   return static_cast<T>(std::rand() * (1.0 / (RAND_MAX + 1.0 )) * maxvalue);
}


class Fixture {
   public:
   using key_type = uint32_t;
   using value_type = uint64_t;
   static constexpr size_t NUM_RANGE = 32; // most_significant_bit(NUM_ELEMENTS)
   static_assert(sizeof(key_type)*8 >= NUM_RANGE, "Num range must fit into key_type");

   using rigtorp_type = rigtorp::HashMap<key_type,value_type>;
   using google_type = google::sparse_hash_map<key_type,value_type>;
   using tsl_type = tsl::sparse_map<key_type,value_type>;

   using map_type       = std::map<key_type                                , value_type>;
   using unordered_type = std::unordered_map<key_type                      , value_type   , SplitMix>;
   using plain_arb_type     = separate_chaining_map<plain_bucket<key_type> , plain_bucket<value_type>  , hash_mapping_adapter<key_type , SplitMix>, arbitrary_resize>;
   using plain_type     = separate_chaining_map<plain_bucket<key_type> , plain_bucket<value_type>  , hash_mapping_adapter<key_type , SplitMix>>;
   using avx2_type      = separate_chaining_map<avx2_bucket<key_type>  , plain_bucket<value_type>  , hash_mapping_adapter<key_type , SplitMix>, incremental_resize>;
   using avx2_arb_type      = separate_chaining_map<avx2_bucket<key_type>  , plain_bucket<value_type>  , hash_mapping_adapter<key_type , SplitMix>, arbitrary_resize>;
   using compact_type   = separate_chaining_map<varwidth_bucket<>        , plain_bucket<value_type>  , xorshift_hash<>, incremental_resize>;
   using compact_arb_type   = separate_chaining_map<varwidth_bucket<>        , plain_bucket<value_type>  , xorshift_hash<>, arbitrary_resize>;
   using compact_chain_type   = compact_chaining_map<xorshift_hash<> >;
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
   using bucket_avx2_type   = bucket_table<varwidth_bucket<>        , plain_bucket<value_type>, incremental_resize>;
#endif//USE_BUCKET_TABLES

   static constexpr size_t m_missed_els_size = 256;
   std::vector<key_type> m_missed_els;
   map_type* m_map = nullptr;
   unordered_type* m_ordered = nullptr;
   plain_type* m_plain = nullptr;
   plain_arb_type* m_plain_arb = nullptr;
   avx2_type* m_avx = nullptr;
   avx2_arb_type* m_avx_arb = nullptr;
   compact_type* m_compact = nullptr;
   compact_arb_type* m_compact_arb = nullptr;
   compact_chain_type *m_compact_chain = nullptr;

   rigtorp_type* m_rigtorp = nullptr;
   google_type* m_google = nullptr;
   tsl_type* m_tsl = nullptr;

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
   bucket_avx2_type* m_bucket_avx2 = nullptr;
#endif//USE_BUCKET_TABLES

   size_t m_current_instance = 0;

   void setUp(size_t i) {
      DCHECK_GT(i, 0);
      if(i == m_current_instance) return;
      tearDown();
      m_current_instance = i;

      m_map = new map_type();
      m_ordered = new unordered_type();
      m_plain = new plain_type();
      m_plain_arb = new plain_arb_type();
      m_avx = new avx2_type();
      m_avx_arb = new avx2_arb_type();
      m_compact = new compact_type(NUM_RANGE);
      m_compact_arb = new compact_arb_type(NUM_RANGE);
      m_compact_chain = new compact_chain_type(NUM_RANGE, sizeof(value_type)*8);
      m_rigtorp = new rigtorp_type(0, static_cast<key_type>(-1ULL));
      m_google = new google_type();
      m_tsl = new tsl_type();

#ifdef USE_BONSAI_TABLES
      m_eliasS = new eliasS_type(0,NUM_RANGE);
      m_layeredS = new layeredS_type(0,NUM_RANGE);
      m_clearyS = new clearyS_type(0,NUM_RANGE);
      m_eliasP = new eliasP_type(0,NUM_RANGE);
      m_layeredP = new layeredP_type(0,NUM_RANGE);
      m_clearyP = new clearyP_type(0,NUM_RANGE);
#endif//USE_BONSAI_TABLES

#ifdef USE_BUCKET_TABLES
      m_bucket = new bucket_type(NUM_RANGE);
      m_bucket_arb = new bucket_arb_type(NUM_RANGE);
      m_bucket_avx2 = new bucket_avx2_type(NUM_RANGE);
#endif//USE_BUCKET_TABLES

      for(size_t val = 0; val < m_current_instance;) {
	 const key_type key = random_int(1ULL<<NUM_RANGE);
	 if( m_map->find(key) != m_map->end()) continue;
	 (*m_map)[key] = val++;
      }
      m_missed_els.clear();
      m_missed_els.reserve(m_missed_els_size);
      for(size_t val = 0; val < m_missed_els_size;) {
	 const key_type key = random_int(1ULL<<NUM_RANGE);
	 if( m_map->find(key) != m_map->end()) continue;
	 m_missed_els.push_back(key); 
	 ++val;
      }
      for(auto el : *m_map) {
	 (*m_ordered)[el.first] = el.second;
	 (*m_plain)[el.first] = el.second;
	 (*m_plain_arb)[el.first] = el.second;
	 (*m_avx)[el.first] = el.second;
	 (*m_avx_arb)[el.first] = el.second;
	 (*m_compact)[el.first] = el.second;
	 (*m_compact_arb)[el.first] = el.second;
	 (*m_compact_chain)[el.first] = el.second;
	 (*m_google)[el.first] = el.second;
	 (*m_rigtorp)[el.first] = el.second;
	 (*m_tsl)[el.first] = el.second;

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
      (*m_bucket_avx2)[el.first] = el.second;
#endif//USE_BUCKET_TABLES

	 DCHECK_EQ((*m_ordered)[el.first], el.second);
	 DCHECK_EQ((*m_plain)[el.first], el.second);
	 DCHECK_EQ((*m_plain_arb)[el.first], el.second);
	 DCHECK_EQ((*m_avx)[el.first], el.second);
	 DCHECK_EQ((*m_avx_arb)[el.first], el.second);
	 DCHECK_EQ((*m_compact)[el.first], el.second);
	 DCHECK_EQ((*m_compact_arb)[el.first], el.second);
	 DCHECK_EQ((*m_compact_chain)[el.first], el.second);
	 DCHECK_EQ((*m_google)[el.first], el.second);
	 DCHECK_EQ((*m_rigtorp)[el.first], el.second);
	 DCHECK_EQ((*m_tsl)[el.first], el.second);

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
      DCHECK_EQ((*m_bucket_avx2)[el.first] , el.second);
#endif//USE_BUCKET_TABLES

      }
      DCHECK_EQ(m_ordered->size(), m_map->size());
      DCHECK_EQ(m_plain_arb->size(), m_ordered->size());
      DCHECK_EQ(m_avx->size(), m_ordered->size());
      DCHECK_EQ(m_avx_arb->size(), m_ordered->size());
      DCHECK_EQ(m_compact->size(), m_ordered->size());
      DCHECK_EQ(m_compact_arb->size(), m_ordered->size());
      DCHECK_EQ(m_compact_chain->size(), m_ordered->size());
      DCHECK_EQ(m_google->size(), m_ordered->size());
      DCHECK_EQ(m_rigtorp->size(), m_ordered->size());
      DCHECK_EQ(m_tsl->size(), m_ordered->size());

#ifdef USE_BONSAI_TABLES
      DCHECK_EQ(m_clearyP->size(), m_ordered->size());
      DCHECK_EQ(m_eliasP->size(), m_ordered->size());
      DCHECK_EQ(m_layeredP->size(), m_ordered->size());
      DCHECK_EQ(m_clearyS->size(), m_ordered->size());
      DCHECK_EQ(m_eliasS->size(), m_ordered->size());
      DCHECK_EQ(m_layeredS->size(), m_ordered->size());
#endif//USE_BONSAI_TABLES
#ifdef USE_BUCKET_TABLES
      DCHECK_EQ((m_bucket)     ->size() , m_ordered->size());
      DCHECK_EQ((m_bucket_arb) ->size() , m_ordered->size());
      DCHECK_EQ((m_bucket_avx2)->size() , m_ordered->size());
#endif//USE_BUCKET_TABLES
   }
   void tearDown() {
      if(m_map != nullptr) {
	 delete m_map;
	 delete m_ordered;
	 delete m_plain;
	 delete m_plain_arb;
	 delete m_avx;
	 delete m_avx_arb;
	 delete m_compact;
	 delete m_compact_arb;
	 delete m_compact_chain;
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
	 delete m_bucket_avx2;
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

   const size_t m_instance_length;


   std::vector<celero::TestFixture::ExperimentValue> m_problemspace;

   size_t m_current_instance = 0;


   public:

   // const TableInstance& instance() const {
   //    return *m_instances[m_current_instance];
   // }
#ifdef USE_BUCKET_TABLES
   TableFixture()
      : m_instance_length(255/3+2)
      , m_problemspace(m_instance_length,0)
   {
      for(size_t i = 0; i < m_instance_length; ++i) {
	 m_problemspace[i] = 2+3*i; 
      }
   }
#else
   TableFixture()
      : m_instance_length(19)
      , m_problemspace(m_instance_length,0)
   {
      size_t value = 1024;
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


BASELINE_F(query, std, TableFixture, 0, 100)
{
   const auto& map = *(static_fixture.m_ordered);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(map.find(el.first));
   }
}

BENCHMARK_F(query, plainI, TableFixture, 0, 100)
{
   const auto& plain = *(static_fixture.m_plain);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(plain.find(el.first));
   }
}

BENCHMARK_F(query, plainD, TableFixture, 0, 100)
{
   const auto& plain_arb = *(static_fixture.m_plain_arb);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(plain_arb.find(el.first));
   }
}
BENCHMARK_F(query, avxI, TableFixture, 0, 100)
{
   const auto& avx = *(static_fixture.m_avx);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(avx.find(el.first));
   }
}
BENCHMARK_F(query, avxD, TableFixture, 0, 100)
{
   const auto& avx = *(static_fixture.m_avx_arb);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(avx.find(el.first));
   }
}

BENCHMARK_F(query, chtI, TableFixture, 0, 100)
{
   const auto& compact = *(static_fixture.m_compact);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(compact.find(el.first));
   }
}
BENCHMARK_F(query, chtD, TableFixture, 0, 100)
{
   const auto& compact = *(static_fixture.m_compact_arb);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(compact.find(el.first));
   }
}
BENCHMARK_F(query, chmap, TableFixture, 0, 100)
{
   const auto& compact = *(static_fixture.m_compact_chain);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(compact.find(el.first));
   }
}
BENCHMARK_F(query, rigtorp, TableFixture, 0, 100)
{
   const auto& mappe = *(static_fixture.m_rigtorp);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(mappe.find(el.first));
   }
}
BENCHMARK_F(query, google, TableFixture, 0, 100)
{
   const auto& mappe = *(static_fixture.m_google);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(mappe.find(el.first));
   }
}
BENCHMARK_F(query, tsl, TableFixture, 0, 100)
{
   const auto& mappe = *(static_fixture.m_tsl);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(mappe.find(el.first));
   }
}

#ifdef USE_BONSAI_TABLES
BENCHMARK_F(query, clearyS, TableFixture, 0, 100)
{
   auto& cleary = *(static_fixture.m_clearyS);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(cleary[el.first]);
   }
}
BENCHMARK_F(query, clearyP, TableFixture, 0, 100)
{
   auto& cleary = *(static_fixture.m_clearyP);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(cleary[el.first]);
   }
}
BENCHMARK_F(query, eliasS, TableFixture, 0, 100)
{
   auto& elias = *(static_fixture.m_eliasS);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(elias[el.first]);
   }
}

BENCHMARK_F(query, eliasP, TableFixture, 0, 100)
{
   auto& elias = *(static_fixture.m_eliasP);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(elias[el.first]);
   }
}
BENCHMARK_F(query, layeredS, TableFixture, 0, 100)
{
   auto& layered = *(static_fixture.m_layeredS);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(layered[el.first]);
   }
}

BENCHMARK_F(query, layeredP, TableFixture, 0, 100)
{
   auto& layered = *(static_fixture.m_layeredP);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(layered[el.first]);
   }
}
#endif//USE_BONSAI_TABLES
#ifdef USE_BUCKET_TABLES
BENCHMARK_F(query, bucket, TableFixture, 0, 100)
{
   const auto& bucket = *(static_fixture.m_bucket);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(bucket.find(el.first));
   }
}
BENCHMARK_F(query, bucket_arb, TableFixture, 0, 100)
{
   const auto& bucket_arb = *(static_fixture.m_bucket_arb);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(bucket_arb.find(el.first));
   }
}
BENCHMARK_F(query, bucket_avx2, TableFixture, 0, 100)
{
   const auto& bucket_avx2 = *(static_fixture.m_bucket_avx2);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(bucket_avx2.find(el.first));
   }
}
#endif//USE_BUCKET_TABLES

#define BENCH_INSERT(name,cons) \
	 BENCHMARK_F(insert, name, TableFixture, 0, 100) { \
	    auto map = Fixture::cons; \
	    for(auto el : *static_fixture.m_map) { map[el.first] = el.second; }}



BASELINE_F(insert, std, TableFixture, 0, 100)
{
   auto map = Fixture::unordered_type();
   for(auto el : *static_fixture.m_map) {
      map[el.first] = el.second;
   }
}
BENCH_INSERT(plainI, plain_type())
BENCH_INSERT(plainD, plain_arb_type())
BENCH_INSERT(avxI, avx2_type())
BENCH_INSERT(avxD, avx2_arb_type())
BENCH_INSERT(chtI, compact_type(Fixture::NUM_RANGE))
BENCH_INSERT(chtD, compact_arb_type(Fixture::NUM_RANGE))
BENCH_INSERT(chmap, compact_chain_type(Fixture::NUM_RANGE, sizeof(Fixture::value_type)*8))
#ifdef USE_BONSAI_TABLES
BENCH_INSERT(clearyP, clearyP_type())
BENCH_INSERT(eliasP, eliasP_type())
BENCH_INSERT(layeredP, layeredP_type())
BENCH_INSERT(clearyS, clearyS_type())
BENCH_INSERT(eliasS, eliasS_type())
BENCH_INSERT(layeredS, layeredS_type())
#endif//USE_BONSAI_TABLES
#ifdef USE_BUCKET_TABLES
BENCH_INSERT(bucket, bucket_type())
BENCH_INSERT(bucket_arb, bucket_arb_type())
BENCH_INSERT(bucket_avx2, bucket_avx2_type())
#endif//USE_BUCKET_TABLES
BENCH_INSERT(google, google_type())
BENCH_INSERT(rigtorp, rigtorp_type(0, static_cast<Fixture::key_type>(-1ULL)))
BENCH_INSERT(tsl, tsl_type())


BASELINE_F(miss, std, TableFixture, 0, 100)
{
   auto& map = *(static_fixture.m_ordered);
   for(auto el : static_fixture.m_missed_els) { celero::DoNotOptimizeAway(map[el]); }
}


#define BENCH_MISS(name,instance) \
      BENCHMARK_F(miss, name, TableFixture, 0, 100) { \
	 auto& map = *(static_fixture.instance);\
	 for(auto el : static_fixture.m_missed_els) { celero::DoNotOptimizeAway(map[el]); }}


BENCH_MISS(plainI, m_plain)
BENCH_MISS(plainD, m_plain_arb)
BENCH_MISS(avxI, m_avx)
BENCH_MISS(avxD, m_avx_arb)
BENCH_MISS(chtI, m_compact)
BENCH_MISS(chtD, m_compact_arb)
BENCH_MISS(chmap, m_compact_chain)
#ifdef USE_BONSAI_TABLES
BENCH_MISS(clearyP, m_clearyP)
BENCH_MISS(eliasP, m_eliasP)
BENCH_MISS(layeredP, m_layeredP)
BENCH_MISS(clearyS, m_clearyS)
BENCH_MISS(eliasS, m_eliasS)
BENCH_MISS(layeredS, m_layeredS)
#endif//USE_BONSAI_TABLES
BENCH_MISS(google, m_google)
BENCH_MISS(rigtorp, m_rigtorp)
BENCH_MISS(tsl, m_tsl)



class EraseFixture : public TableFixture {
   public:
      using key_type = Fixture::key_type;
      using value_type = Fixture::value_type;
   static constexpr size_t m_delete_entries_size = 256;
   std::vector<std::pair<key_type, value_type>> m_delete_entries;

   EraseFixture() { }


   virtual std::vector<celero::TestFixture::ExperimentValue> getExperimentValues() const override {
      return TableFixture::getExperimentValues();
   }

   virtual void setUp(const celero::TestFixture::ExperimentValue& experimentValue) override {
      TableFixture::setUp(experimentValue.Value);
      m_delete_entries.clear();
      m_delete_entries.reserve(m_delete_entries_size);
      size_t el = 0;
      for(auto it = static_fixture.m_map->begin(); it != static_fixture.m_map->end(); ++it) {
	 if(el % (static_fixture.m_map->size() / m_delete_entries_size) == 0) {
	    m_delete_entries.push_back(std::make_pair(it->first, it->second));
	 }
      }
      
   }

   template<class T>
   void reinsert_elements(T& table) {
      for(auto p : m_delete_entries) {
	 table[p.first] = p.second;
      }
   }

   virtual void tearDown() override {
      reinsert_elements(*static_fixture.m_ordered);
      reinsert_elements(*static_fixture.m_plain);
      reinsert_elements(*static_fixture.m_plain_arb);
      reinsert_elements(*static_fixture.m_avx);
      reinsert_elements(*static_fixture.m_avx_arb);
      reinsert_elements(*static_fixture.m_compact);
      reinsert_elements(*static_fixture.m_compact_arb);
      reinsert_elements(*static_fixture.m_compact_chain);
// #ifdef USE_BONSAI_TABLES
//       reinsert_elements(*static_fixture.m_clearyP);
//       reinsert_elements(*static_fixture.m_eliasP);
//       reinsert_elements(*static_fixture.m_layeredP);
//       reinsert_elements(*static_fixture.m_clearyS);
//       reinsert_elements(*static_fixture.m_eliasS);
//       reinsert_elements(*static_fixture.m_layeredS);
// #endif//USE_BONSAI_TABLES
      reinsert_elements(*static_fixture.m_google);
      reinsert_elements(*static_fixture.m_rigtorp);
      reinsert_elements(*static_fixture.m_tsl);
   }


};

BASELINE_F(erase, std, EraseFixture, 0, 100)
{
   auto& map = *(static_fixture.m_ordered);
   for(auto el : this->m_delete_entries) { celero::DoNotOptimizeAway(map.erase(el.first)); }
}


#define BENCH_ERASE(name,instance) \
      BENCHMARK_F(erase, name, EraseFixture, 0, 100) { \
	 auto& map = *(static_fixture.instance);\
	 for(auto el : this->m_delete_entries) { celero::DoNotOptimizeAway(map.erase(el.first)); }}


BENCH_ERASE(plainI, m_plain)
BENCH_ERASE(plainD, m_plain_arb)
BENCH_ERASE(avxI, m_avx)
BENCH_ERASE(avxD, m_avx_arb)
BENCH_ERASE(chtI, m_compact)
BENCH_ERASE(chtD, m_compact_arb)
BENCH_ERASE(chmap, m_compact_chain)
// #ifdef USE_BONSAI_TABLES
// BENCH_ERASE(clearyP, m_clearyP)
// BENCH_ERASE(eliasP, m_eliasP)
// BENCH_ERASE(layeredP, m_layeredP)
// BENCH_ERASE(clearyS, m_clearyS)
// BENCH_ERASE(eliasS, m_eliasS)
// BENCH_ERASE(layeredS, m_layeredS)
// #endif//USE_BONSAI_TABLES
BENCH_ERASE(google, m_google)
BENCH_ERASE(rigtorp, m_rigtorp)
BENCH_ERASE(tsl, m_tsl)

