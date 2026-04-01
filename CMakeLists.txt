cmake_minimum_required(VERSION 3.23)

project(StrataGraph)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAK_C_EXTENSIONS OFF)
set(THREADS_PREFER_PTHREAD_FLAG ON)

add_library(stratagraph)

target_sources(stratagraph
    PRIVATE
        src/l1.c
        src/snapshot_reader.c

    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            include
        FILES
            include/l1.h
            include/snapshot_reader.h
)

target_link_libraries(stratagraph
    PRIVATE
        Threads::Threads
)

find_package(Threads REQUIRED)

add_executable(benchmark_l1)

target_sources(benchmark_l1
    PRIVATE
        benchmarks/l1_bench.c
)

target_link_libraries(benchmark_l1
    PRIVATE
        stratagraph
        Threads::Threads
)