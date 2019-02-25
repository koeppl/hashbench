#define STATS_ENABLED 1
#define USE_BONSAI_HASH 1
#include "experiment_base.hpp"

template<class T>
T random_int(const T& maxvalue) {
   return static_cast<T>(std::rand() * (1.0 / (RAND_MAX + 1.0 )) * maxvalue);
}



class CopyExperiment {
   public:
      using key_type = uint32_t;
      using value_type = uint32_t;

      constexpr static size_t NUM_ELEMENTS = 2000000;
      static constexpr uint8_t KEY_BITSIZE = 32;
      static_assert(sizeof(key_type)*8 >= KEY_BITSIZE, "Num range must fit into key_type");

   private:
      using map_type = std::unordered_map<key_type, value_type>;
      
      map_type m_map;

      const char*const m_caption;

   public:
      const char* caption() const { return m_caption; }

      CopyExperiment(const char*const caption) : m_caption(caption) {
	    for(size_t i = 0; i < NUM_ELEMENTS; ++i) {
	       m_map[random_int(1ULL<<KEY_BITSIZE)] = i;
	    }
      }


      template<class T>
	 void execute(const char*const label, T& filter) {
	    tdc::StatPhase v(label);
	    for(auto el : m_map) {
	       filter[el.first] = el.second;
	    }
	    if constexpr (has_print_stats_fn<T>::value) {
	       tdc::StatPhase finalize("finalize");
	       filter.print_stats(finalize);
	    }
	 }

      void baseline() {
	 tdc::StatPhase v("plain array");
	 key_type* keys = new key_type[m_map.size()];
	 value_type* values = new value_type[m_map.size()];
	 size_t i = 0;
	 for(auto el : m_map) {
	    keys[i] = el.first;
	    values[i] = el.second;
	 }
	 delete [] keys;
	 delete [] values;
      }
};

int main() {
	CopyExperiment ex("Random Copy");
	run_experiments(ex);
	return 0;
}
