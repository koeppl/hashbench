#define STATS_ENABLED 1
#include <iostream>
#include <fstream>
#include <MurmurHash3.h>
#include <ctime>

#include "experiment_base.hpp"
using namespace std;


template<class hashmap>
void bloomfilter_test(istream& keyword_stream, istream& query_stream, hashmap& filter) {
	uint32_t seed = 3;
	//std::unordered_map<uint64_t,uint64_t> filter;
	//std::unordered_set<uint64_t> filter1;

	size_t elements = 0;
	{
		tdc::StatPhase v("construction");
		std::string line;
		clock_t begin = clock();
		uint64_t arr[2];
		while(std::getline(keyword_stream, line)) {
			MurmurHash3_x64_128(line.data(), line.length(), seed, arr);
			if(filter.find(arr[0]) != filter.end()) { // && filter1.find(arr[1]) != filter1.end()) {
				throw std::runtime_error("Duplicates detected!");
			}
			filter[arr[0]] = elements;
			//filter1.insert(arr[1]);
			++elements;
		}
		clock_t end = clock();
		double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
		v.log("entries", filter.size());
		v.log("seconds", elapsed_secs);
		v.log("nanoseconds", (((elapsed_secs)*10e8)/elements));
	}
	{
		tdc::StatPhase v("query");
		std::string line;
		clock_t begin = clock();
		uint64_t arr[2];
		while(std::getline(query_stream, line)) {
			MurmurHash3_x64_128(line.data(), line.length(), seed, arr);
			if(filter.find(arr[0]) == filter.end()) { //|| filter1.find(arr[1]) == filter1.end()) {
				throw std::runtime_error("Filter failed!");
			}
		}
		clock_t end = clock();
		double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
		v.log("seconds", elapsed_secs);
		v.log("nanoseconds", (((elapsed_secs)*10e8)/elements));
	}
}



class BloomFilterExperiment {
   public:
      using key_type = uint64_t;
      using value_type = uint32_t;

      static constexpr uint8_t KEY_BITSIZE = 64;

   private:
      const char*const m_caption;
      const char*const m_keyword_file;
      const char*const m_query_file;

   public:
      const char* caption() const { return m_caption; }

      BloomFilterExperiment(const char*const caption, const char*const keyword_file, const char*const query_file) : m_caption(caption), m_keyword_file(keyword_file), m_query_file(query_file) {
      }

      template<class T>
	 void execute(const char*const label, T& filter) {
	    tdc::StatPhase v(label);
	    ifstream keyword_stream(m_keyword_file);
	    ifstream query_stream(m_query_file);
	    bloomfilter_test(keyword_stream, query_stream, filter);
	    if constexpr (has_print_stats_fn<T>::value) {
	       tdc::StatPhase finalize("finalize");
	       filter.print_stats(finalize);
	    }
	 }

      constexpr void baseline() {}
};




int main(int argc, char **argv) {
	free(malloc(42));


	if(argc < 3) {
		std::cout << argv[0] << " [keyword-file] [query-file] " << std::endl;
		abort();
	}
	{
		ifstream is(argv[1]);
		if(!is) {
			std::cout << "Cannot open file " << argv[1] << std::endl;
			abort();
		}
	}
	{
		ifstream is(argv[2]);
		if(!is) {
			std::cout << "Cannot open file " << argv[2] << std::endl;
			abort();
		}
	}


	// tdc::StatPhase root("construction");

	BloomFilterExperiment ex("Bloomfilter", argv[1], argv[2]);
	run_experiments(ex);

	return 0;
}

