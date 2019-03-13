#include <celero/Celero.h>

#include <rigtorp/HashMap.h>
#include <tsl/sparse_map.h>
#include <sparsehash/sparse_hash_map>

#include <separate/separate_chaining_table.hpp>
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
   using compact_type   = separate_chaining_map<varwidth_bucket        , plain_bucket<value_type>  , xorshift_hash<>, incremental_resize>;
   using compact_arb_type   = separate_chaining_map<varwidth_bucket        , plain_bucket<value_type>  , xorshift_hash<>, arbitrary_resize>;
#ifdef USE_BONSAI_TABLES
   using elias_type     = tdc::compact_hash::map::sparse_elias_hashmap_t<value_type>;
   using cleary_type    = tdc::compact_hash::map::sparse_cv_hashmap_t<value_type>;
   using layered_type   =tdc::compact_hash::map::sparse_layered_hashmap_t<value_type>;
#endif//USE_BONSAI_TABLES

#ifdef USE_BUCKET_TABLES
   using bucket_type   = bucket_table<varwidth_bucket        , plain_bucket<value_type>, incremental_resize>;
   using bucket_arb_type   = bucket_table<varwidth_bucket        , plain_bucket<value_type>, arbitrary_resize_bucket>;
   using bucket_avx2_type   = bucket_table<varwidth_bucket        , plain_bucket<value_type>, incremental_resize>;
#endif//USE_BUCKET_TABLES

   map_type* m_map = nullptr;
   unordered_type* m_ordered = nullptr;
   plain_type* m_plain = nullptr;
   plain_arb_type* m_plain_arb = nullptr;
   avx2_type* m_avx = nullptr;
   avx2_arb_type* m_avx_arb = nullptr;
   compact_type* m_compact = nullptr;
   compact_arb_type* m_compact_arb = nullptr;

   rigtorp_type* m_rigtorp = nullptr;
   google_type* m_google = nullptr;
   tsl_type* m_tsl = nullptr;

#ifdef USE_BONSAI_TABLES
   elias_type* m_elias = nullptr;
   cleary_type* m_cleary = nullptr;
   layered_type* m_layered = nullptr;
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
      m_rigtorp = new rigtorp_type(0, static_cast<key_type>(-1ULL));
      m_google = new google_type();
      m_tsl = new tsl_type();

#ifdef USE_BONSAI_TABLES
      m_elias = new elias_type(0,NUM_RANGE);
      m_layered = new layered_type(0,NUM_RANGE);
      m_cleary = new cleary_type(0,NUM_RANGE);
#endif//USE_BONSAI_TABLES

#ifdef USE_BUCKET_TABLES
      m_bucket = new bucket_type(NUM_RANGE);
      m_bucket_arb = new bucket_arb_type(NUM_RANGE);
      m_bucket_avx2 = new bucket_avx2_type(NUM_RANGE);
#endif//USE_BUCKET_TABLES

      for(size_t val = 0; val < m_current_instance; ++val) {
	 (*m_map)[random_int(1ULL<<NUM_RANGE)] = val;
      }
      for(auto el : *m_map) {
	 (*m_ordered)[el.first] = el.second;
	 (*m_plain)[el.first] = el.second;
	 (*m_plain_arb)[el.first] = el.second;
	 (*m_avx)[el.first] = el.second;
	 (*m_avx_arb)[el.first] = el.second;
	 (*m_compact)[el.first] = el.second;
	 (*m_compact_arb)[el.first] = el.second;
	 (*m_google)[el.first] = el.second;
	 (*m_rigtorp)[el.first] = el.second;
	 (*m_tsl)[el.first] = el.second;

#ifdef USE_BONSAI_TABLES
	 (*m_cleary)[el.first] = el.second;
	 (*m_layered)[el.first] = el.second;
	 (*m_elias)[el.first] = el.second;
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
	 DCHECK_EQ((*m_google)[el.first], el.second);
	 DCHECK_EQ((*m_rigtorp)[el.first], el.second);
	 DCHECK_EQ((*m_tsl)[el.first], el.second);

#ifdef USE_BONSAI_TABLES
	 DCHECK_EQ((*m_cleary)[el.first], el.second);
	 DCHECK_EQ((*m_elias)[el.first], el.second);
	 DCHECK_EQ((*m_layered)[el.first], el.second);
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
      DCHECK_EQ(m_google->size(), m_ordered->size());
      DCHECK_EQ(m_rigtorp->size(), m_ordered->size());
      DCHECK_EQ(m_tsl->size(), m_ordered->size());

#ifdef USE_BONSAI_TABLES
      DCHECK_EQ(m_cleary->size(), m_ordered->size());
      DCHECK_EQ(m_elias->size(), m_ordered->size());
      DCHECK_EQ(m_layered->size(), m_ordered->size());
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
	 delete m_compact;
#ifdef USE_BONSAI_TABLES
	 delete m_elias;
	 delete m_layered;
	 delete m_cleary;
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
      : m_instance_length(18)
      , m_problemspace(m_instance_length,0)
   {
      for(size_t i = 0; i < m_instance_length; ++i) {
	 m_problemspace[i] = {static_cast<int64_t>(1ULL<<(i+10))};
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
BENCHMARK_F(query, cleary, TableFixture, 0, 100)
{
   auto& cleary = *(static_fixture.m_cleary);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(cleary[el.first]);
   }
}

BENCHMARK_F(query, elias, TableFixture, 0, 100)
{
   auto& elias = *(static_fixture.m_elias);
   for(auto el : *static_fixture.m_map) {
      celero::DoNotOptimizeAway(elias[el.first]);
   }
}

BENCHMARK_F(query, layered, TableFixture, 0, 100)
{
   auto& layered = *(static_fixture.m_layered);
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
BENCH_INSERT(compactI, compact_type(Fixture::NUM_RANGE))
BENCH_INSERT(compactD, compact_arb_type(Fixture::NUM_RANGE))
#ifdef USE_BONSAI_TABLES
BENCH_INSERT(cleary, cleary_type())
BENCH_INSERT(elias, elias_type())
BENCH_INSERT(layered, layered_type())
#endif//USE_BONSAI_TABLES
#ifdef USE_BUCKET_TABLES
BENCH_INSERT(bucket, bucket_type())
BENCH_INSERT(bucket_arb, bucket_arb_type())
BENCH_INSERT(bucket_avx2, bucket_avx2_type())
#endif//USE_BUCKET_TABLES
BENCH_INSERT(google, google_type())
BENCH_INSERT(rigtorp, rigtorp_type(0, static_cast<Fixture::key_type>(-1ULL)))
BENCH_INSERT(tsl, tsl_type())
