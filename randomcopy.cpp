#include "experiment_base.hpp"

template<class T>
T random_int(const T& maxvalue) {
   return static_cast<T>(std::rand() * (1.0 / (RAND_MAX + 1.0 )) * maxvalue);
}



class CopyExperiment {
   public:
      using key_type = uint32_t;
      using value_type = uint32_t;

      const size_t NUM_ELEMENTS = 2000000;
      static constexpr uint8_t KEY_BITSIZE = 32;
      static_assert(sizeof(key_type)*8 >= KEY_BITSIZE, "Num range must fit into key_type");

   private:
      using map_type = std::unordered_map<key_type, value_type>;
      
      map_type m_map;

      const char*const m_caption;

   public:
      void init(tdc::StatPhase& phase) const {
	 phase.log("size", NUM_ELEMENTS);
	 phase.log("key bit size", KEY_BITSIZE);
      }
      const char* caption() const { return m_caption; }

      CopyExperiment(const char*const caption, size_t num_elements) : NUM_ELEMENTS(num_elements), m_caption(caption) {
	    for(size_t i = 0; i < NUM_ELEMENTS; ++i) {
	       m_map[random_int(1ULL<<KEY_BITSIZE)] = i;
	    }
      }


      template<class T>
	 void execute(const char*const label, T& filter) {
	    tdc::StatPhase v(label);
	    {
	       tdc::StatPhase v2("insert");
	       for(auto el : m_map) {
		  filter[el.first] = el.second;
	       }
	    }
	    {
	       tdc::StatPhase v2("query");
	       for(auto el : m_map) {
		  if(filter[el.first] != el.second) {
		     throw std::runtime_error("Element not found!");
		  }
	       }
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
	 {
	    tdc::StatPhase v2("insert");
	    size_t i = 0;
	    for(auto el : m_map) {
	       keys[i] = el.first;
	       values[i] = el.second;
	       ++i;
	    }
	 }
	 {
	    tdc::StatPhase v2("query");
	    size_t i = 0;
	    for(auto el : m_map) {
	       if(keys[i] != el.first || values[i] != el.second) {
		  std::runtime_error("key/value mismatch!");
	       }
	       ++i;
	    }
	 }
	 delete [] keys;
	 delete [] values;
      }
};

int main(int argc, char** argv) {
   if(argc != 2) {
      std::cerr << "Usage: " << argv[0] << " problem-size" << std::endl;
      return 1;
   }
   const size_t num_elements = strtoul(argv[1], NULL, 10);


   CopyExperiment ex("Random Copy", num_elements);
   run_experiments(ex);
   return 0;
}
