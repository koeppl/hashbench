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
else(STATS_DISABLED)
    message("[INFO] Statistic tracking is enabled")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DSTATS_ENABLED=1")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DSTATS_ENABLED=1")
endif(STATS_DISABLED)

set(CXX_STANDARD gnu++17)
#! set 
# -DMALLOC_DISABLED to disable memory measurement 
# -DPRINT_STATS to enable printing detailed stats
# -DSTATS_DISABLED to disable tudocomp stats
#!
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -march=native -fdiagnostics-color=auto -std=${CXX_STANDARD} -DPRINT_STATS") # -DMALLOC_DISABLED=1")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -ggdb -DDEBUG") #-D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mtune=native -march=native -DNDEBUG")

##########
# glog
##########

# seems that it also finds not-installed 'glog's
# FIND_LIBRARY(GLOG_LIBRARY glog)
# FIND_PATH(GLOG_INCLUDE_DIR "glog/logging.h")
if(NOT Glog_FOUND)
    add_subdirectory(external/glog)
    include_directories(external/glog/)
endif(NOT Glog_FOUND)


include_directories(external)
include_directories(external/bit_span/include)
include_directories(external/compact_sparse_hash/include)
include_directories(external/tudostats/include)
include_directories(external/rigtorphash/include)
include_directories(external/googlesparse/src)
include_directories(external/tslsparse/include)
include_directories(external/separate_chaining/include)
include_directories(external/smhasher/src)
include_directories(external/sparsepp)
include_directories(external/broadwordsearch/include)
include_directories(external/patchmap)
# include_directories(external/DySECT/include)
# include_directories(external/DySECT)

add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/external/googlesparse/src/config.h
    COMMAND cd ${PROJECT_SOURCE_DIR}/external/googlesparse && ./configure && make)
add_custom_target(googlesparse_config DEPENDS ${PROJECT_SOURCE_DIR}/external/googlesparse/src/config.h)



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

add_dependencies(microbench googlesparse_config)
add_dependencies(microbench_tiny googlesparse_config)
endif()


##########
# tudocomp stats
##########

add_executable  (randomcopy randomcopy.cpp)
add_executable  (reservecopy reservecopy.cpp)
add_executable  (fingerprint fingerprint.cpp external/smhasher/src/MurmurHash3.cpp)

add_dependencies(randomcopy googlesparse_config)
add_dependencies(reservecopy googlesparse_config)
add_dependencies(fingerprint googlesparse_config)



if(NOT STATS_DISABLED)
    message("[INFO] Statistic tracking is enabled")
target_link_libraries(randomcopy tudocomp_stat glog)
target_link_libraries(reservecopy tudocomp_stat glog)
target_link_libraries(fingerprint tudocomp_stat glog)

add_executable  (linear_scaling linear_scaling.cpp)
target_link_libraries(linear_scaling tudocomp_stat glog)
else()
target_link_libraries(randomcopy glog)
target_link_libraries(reservecopy  glog)
target_link_libraries(fingerprint  glog)
endif(NOT STATS_DISABLED)


##########
# TESTS
##########


find_package(GTest)
if(GTEST_FOUND)
include_directories(${GTEST_INCLUDE_DIR})
add_executable  (test_overflow test_overflow.cpp)

target_link_libraries(test_overflow gtest pthread  tudocomp_stat ${GLOG_LIBRARY})

enable_testing()
add_test(NAME overflow COMMAND test_overflow)
endif(GTEST_FOUND)



MESSAGE( STATUS "With Gtest?: " ${GTEST_INCLUDE_DIR} )



MESSAGE( STATUS "With Celero?: " ${CELERO_INCLUDE_DIR} ${CELERO_LIB_DIR} )
MESSAGE( STATUS "Built Type: " ${CMAKE_BUILD_TYPE} )
MESSAGE( STATUS "CXX Flags: " ${CMAKE_CXX_FLAGS} )

