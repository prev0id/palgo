cmake_minimum_required(VERSION 3.31)

include(FetchContent)

FetchContent_Declare(
  parlaylib
  GIT_REPOSITORY https://github.com/cmuparlay/parlaylib.git
  GIT_TAG master
)
FetchContent_MakeAvailable(parlaylib)

add_executable(main main.cpp)

target_link_libraries(main PRIVATE parlay)

target_compile_features(main PRIVATE cxx_std_17)
