cmake_minimum_required(VERSION 3.0.2)
project (hashbench)


if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
    #set(CMAKE_BUILD_TYPE "Debug")
endif(NOT CMAKE_BUILD_TYPE)

if(STATS_DISABLED)
    message("[INFO] Statistic tracking is disabled")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DSTATS_DISABLED=1")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DSTATS_DISABLED=1")
endif(STATS_DISABLED)

set(CXX_STANDARD gnu++17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -march=native -std=${CXX_STANDARD} ")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -ggdb ") #-D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC")
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
# celero benchmarks
##########

set(NO_CELERO 0)
find_path(CELERO_INCLUDE_DIR celero/Celero.h)
find_library(CELERO_LIB libcelero.so)
if(CELERO_INCLUDE_DIR STREQUAL "CELERO_INCLUDE_DIR-NOTFOUND" OR CELERO_LIB_DIR STREQUAL "CELERO_LIB-NOTFOUND")
	set(NO_CELERO 1)
	add_definitions(-DNO_CELERO=1)
else()
	include_directories(${CELERO_INCLUDE_DIR})
endif()

if(NO_CELERO EQUAL 0)
    include_directories(external/compact_sparse_hash/include)

    add_executable(microbench microbench.cpp)
    target_link_libraries(microbench ${CELERO_LIB} glog)

    add_executable(microbench_tiny microbench.cpp)
    target_compile_definitions(microbench_tiny PUBLIC USE_BUCKET_TABLES=1)
    target_link_libraries(microbench_tiny ${CELERO_LIB} glog)

endif()


##########
# tudocomp stats
##########

add_executable  (randomcopy randomcopy.cpp)
add_executable  (bloomfilter bloomfilter.cpp external/smhasher/src/MurmurHash3.cpp) 


if(NOT STATS_DISABLED)
    message("[INFO] Statistic tracking is enabled")
target_link_libraries(randomcopy tudocomp_stat glog)
target_link_libraries(bloomfilter tudocomp_stat glog)
endif(NOT STATS_DISABLED)


MESSAGE( STATUS "With Celero?: " ${CELERO_INCLUDE_DIR} ${CELERO_LIB_DIR} )
MESSAGE( STATUS "Built Type: " ${CMAKE_BUILD_TYPE} )
MESSAGE( STATUS "CXX Flags: " ${CMAKE_CXX_FLAGS} )

