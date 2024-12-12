set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(test)

target_include_directories(test PRIVATE
    ./caddies
    ./caddies/log
    ./caddies/memory
    ./caddies/regex
    ./caddies/file
    ./caddies/threadpool
    ./caddies/json
    ./caddies/perf
    ./caddies/ext
)

aux_source_directory(. sources)
aux_source_directory(./caddies/file sources)
aux_source_directory(./caddies/log sources)
aux_source_directory(./caddies/memory sources)
aux_source_directory(./caddies/regex sources)
aux_source_directory(./caddies/threadpool sources)
aux_source_directory(./caddies/json sources)
aux_source_directory(./caddies/perf sources)
aux_source_directory(./caddies/ext sources)

target_sources(test PUBLIC ${sources})
