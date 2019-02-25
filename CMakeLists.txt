cmake_minimum_required(VERSION 3.0.2)
project (hashbench)


if(NOT CMAKE_BUILD_TYPE)
    #set(CMAKE_BUILD_TYPE "Release")
    set(CMAKE_BUILD_TYPE "Debug")
endif(NOT CMAKE_BUILD_TYPE)

set(CXX_STANDARD gnu++17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -march=native -std=${CXX_STANDARD} ")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -ggdb -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mtune=native")

include_directories(include)
include_directories(external/bit_span/include)
include_directories(external/compact_sparse_hash/include)
include_directories(external/tudostats/include)
include_directories(external/rigtorphash/include)
include_directories(external/googlesparse/src)
include_directories(external/tslsparse/include)
include_directories(external/separate_chaining/include)
include_directories(external/smhasher/src)


add_subdirectory(external/bit_span)
add_subdirectory(external/tudostats)

##########
# stats
##########

add_executable  (randomcopy randomcopy.cpp)
add_executable  (bloomfilter bloomfilter.cpp external/smhasher/src/MurmurHash3.cpp) 

target_link_libraries(randomcopy tudocomp_stat glog)
target_link_libraries(bloomfilter tudocomp_stat glog)
 
