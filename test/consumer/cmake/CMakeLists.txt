cmake_minimum_required(VERSION 3.16)

project(ruckig_c_consumer LANGUAGES C)

if(DEFINED RUCKIG_C_CONSUMER_FIND_VERSION AND NOT RUCKIG_C_CONSUMER_FIND_VERSION STREQUAL "")
  find_package(ruckig_c ${RUCKIG_C_CONSUMER_FIND_VERSION} CONFIG REQUIRED)
else()
  find_package(ruckig_c CONFIG REQUIRED)
endif()

add_executable(ruckig_c_consumer main.c)
target_link_libraries(ruckig_c_consumer PRIVATE ruckig_c::ruckig_c)
set_target_properties(ruckig_c_consumer PROPERTIES
  C_STANDARD 99
  C_STANDARD_REQUIRED ON
  C_EXTENSIONS OFF
)
