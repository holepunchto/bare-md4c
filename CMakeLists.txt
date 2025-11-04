cmake_minimum_required(VERSION 4.0)

find_package(cmake-bare REQUIRED PATHS node_modules/cmake-bare)
find_package(cmake-fetch REQUIRED PATHS node_modules/cmake-fetch)

project(bare_md4c C CXX)

fetch_package("github:mity/md4c#481fbfb" SOURCE_DIR MD4C_DIR)
fetch_package("github:holepunchto/libjstl#34a7894")

add_bare_module(bare_md4c)

target_sources(
  ${bare_md4c}
  PRIVATE
    binding.cc
)

target_link_libraries(
  ${bare_md4c}
  PRIVATE
    $<TARGET_OBJECTS:md4c>
    $<TARGET_OBJECTS:md4c-html>
    jstl
)

target_include_directories(
  ${bare_md4c}
  PRIVATE
    ${MD4C_DIR}/src
)
