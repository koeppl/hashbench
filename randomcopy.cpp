#include "experiment_base.hpp"

template <typename T, typename = void>
struct has_erase_fn
    : std::false_type
{};

template <typename T>
struct has_erase_fn<T, std::void_t<decltype(std::declval<T>().erase(std::declval<const default_value_type&>()  ))>> : std::true_type
{};



constexpr bool exit_on_error = false;

class CopyExperiment {
   public:
      using mock_key_type = uint32_t;
      using key_type = uint32_t;
      using value_type = default_value_type;

      const size_t NUM_ELEMENTS = 2000000;
      static constexpr uint8_t KEY_BITSIZE = sizeof(key_type)*8;
      const uint8_t VALUE_BITSIZE; // = sizeof(value_type)*8;
      static_assert(sizeof(key_type)*8 >= KEY_BITSIZE, "Num range must fit into key_type");

	  static constexpr size_t m_erase_entries_size = 1024;
   private:
      using map_type = std::unordered_map<key_type, value_type>;

      map_type m_map;
	  std::vector<std::pair<key_type, value_type>> m_erase_entries;

      const char*const m_caption;

   public:
      void init(tdc::StatPhase& phase) const {
	 phase.log("size", NUM_ELEMENTS);
	 phase.log("key bit size", KEY_BITSIZE);
	 phase.log("value bit size", VALUE_BITSIZE);
      }
      const char* caption() const { return m_caption; }

	  CopyExperiment(const char*const caption, size_t num_elements, uint8_t value_width) 
		  : NUM_ELEMENTS(num_elements), VALUE_BITSIZE(value_width),  m_caption(caption) {
			  const size_t maxvalue = (-1ULL)>>(64-VALUE_BITSIZE);
			  DCHECK_LT(NUM_ELEMENTS, 1ULL<<KEY_BITSIZE);
			  m_erase_entries.reserve(m_erase_entries_size);

			  random_int_reset();
			  for(size_t i = 0; m_map.size() < NUM_ELEMENTS; ++i) {
				  const key_type key = random_int(1ULL<<KEY_BITSIZE);
				  const value_type value = static_cast<value_type>(i % maxvalue);
				  m_map[key] = value;
				  if(m_map.size() % (NUM_ELEMENTS / m_erase_entries_size) == 0) {
					  m_erase_entries.push_back({key, value});
				  }
			  }
		  }


      template<class T>
	 void execute(const char*const label, T& filter) {
	    tdc::StatPhase v(label);
	    {
	       tdc::StatPhase v2("insert");
	       for(auto el : m_map) {
		  filter[el.first] = el.second + 1;
	       }
	    }
	    if constexpr (has_print_stats_fn<T>::value) {
	       tdc::StatPhase finalize("finalize");
	       filter.print_stats(finalize);
	    }
	    {
	       tdc::StatPhase v2("query");
	       for(auto el : m_map) {
		  if(static_cast<size_t>(filter[el.first]) != static_cast<size_t>(el.second + 1)) {
		     std::cerr << "Element " << el.first << " -> " << (el.second + 1) << " not found in table " << demangled_type(T) << std::endl;
		     if (exit_on_error) std::exit(1);
		  }
	       }
	    }
	    if constexpr (has_erase_fn<T>::value) {
	       tdc::StatPhase v2("erase");
		   for(auto el : m_erase_entries) {
			   filter.erase(el.first);
		   }
	    }
	 }

      void baseline() {
	 tdc::StatPhase v("array");
	 key_type* keys = nullptr;
	 value_type* values = nullptr;
	 {
	    tdc::StatPhase v2("insert");
	    keys = new key_type[m_map.size()];
	    values = new value_type[m_map.size()];
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
   ::google::InitGoogleLogging(argv[0]);

   if(argc != 3) {
      std::cerr << "Usage: " << argv[0] << " problem-size value-bitwidth" << std::endl;
      return 1;
   }
   const size_t num_elements = strtoul(argv[1], NULL, 10);
   const size_t value_width = strtoul(argv[2], NULL, 10);
   CHECK_GT(num_elements, CopyExperiment::m_erase_entries_size);
   CHECK_LE(value_width, default_value_width);
   CHECK_GT(value_width, 0);


   CopyExperiment ex("Random Copy", num_elements, value_width);
   run_experiments(ex);
   return 0;
}
