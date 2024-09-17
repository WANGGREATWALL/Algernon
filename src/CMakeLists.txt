set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(test)

target_include_directories(test PRIVATE
    ./log
    ./memory
    ./regex
    ./file
    ./threadpool
    ./json
    ./perf
)

aux_source_directory(. sources)
aux_source_directory(./file sources)
aux_source_directory(./log sources)
aux_source_directory(./memory sources)
aux_source_directory(./regex sources)
aux_source_directory(./threadpool sources)
aux_source_directory(./json sources)
aux_source_directory(./perf sources)

target_sources(test PUBLIC ${sources})

file(COPY ${CMAKE_SOURCE_DIR}/src/main.json DESTINATION ${CMAKE_BINARY_DIR}/src)
