cmake_minimum_required(VERSION 3.23)

project(StrataGraph)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAK_C_EXTENSIONS OFF)
set(THREADS_PREFER_PTHREAD_FLAG ON)

add_library(stratagraph)

target_sources(stratagraph
    PRIVATE
        src/event_log.c
        src/snapshot_reader.c

    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            include
        FILES
            include/event_log.h
            include/snapshot_reader.h
)

find_package(Threads REQUIRED)

add_executable(benchmark_event_log)

target_sources(benchmark_event_log
    PRIVATE
        benchmarks/event_log_bench.c
)

target_link_libraries(benchmark_event_log
    PRIVATE
        stratagraph
        Threads::Threads
)