cmake_minimum_required(VERSION 3.16)

project(ruckig_c_consumer LANGUAGES C)

if(DEFINED RUCKIG_C_CONSUMER_FIND_VERSION AND NOT RUCKIG_C_CONSUMER_FIND_VERSION STREQUAL "")
  find_package(ruckig_c ${RUCKIG_C_CONSUMER_FIND_VERSION} CONFIG REQUIRED)
else()
  find_package(ruckig_c CONFIG REQUIRED)
endif()

set(RUCKIG_C_EXPECT_STATIC_DEFINE "" CACHE STRING "Whether the installed target should propagate RUCKIG_C_STATIC_DEFINE")
set(RUCKIG_C_EXPECT_NO_INSTRUMENTATION_USAGE_REQUIREMENTS ON CACHE BOOL "Reject sanitizer/coverage usage requirements on the installed target")
set(RUCKIG_C_EXPECT_NO_DURATION_USAGE_REQUIREMENT ON CACHE BOOL "Reject duration compile definition on the installed target")

if(RUCKIG_C_EXPECT_NO_INSTRUMENTATION_USAGE_REQUIREMENTS)
  foreach(prop IN ITEMS INTERFACE_COMPILE_DEFINITIONS INTERFACE_COMPILE_OPTIONS INTERFACE_LINK_OPTIONS INTERFACE_LINK_LIBRARIES)
    get_target_property(value ruckig_c::ruckig_c ${prop})
    if(value)
      string(REPLACE ";" " " value_text "${value}")
      if(value_text MATCHES "fsanitize|sanitize=|fprofile-instr-generate|fcoverage-mapping")
        message(FATAL_ERROR "Unexpected instrumentation usage requirement in ${prop}: ${value_text}")
      endif()
    endif()
  endforeach()
endif()

if(RUCKIG_C_EXPECT_NO_DURATION_USAGE_REQUIREMENT)
  get_target_property(value ruckig_c::ruckig_c INTERFACE_COMPILE_DEFINITIONS)
  if(value)
    string(REPLACE ";" " " value_text "${value}")
    if(value_text MATCHES "(^|[ ;])RUCKIG_C_ENABLE_CALCULATION_DURATION($|[ ;])")
      message(FATAL_ERROR "Unexpected duration usage requirement in INTERFACE_COMPILE_DEFINITIONS: ${value_text}")
    endif()
  endif()
endif()

add_executable(ruckig_c_consumer main.c)
target_link_libraries(ruckig_c_consumer PRIVATE ruckig_c::ruckig_c)
if(RUCKIG_C_EXPECT_STATIC_DEFINE STREQUAL "ON")
  target_compile_definitions(ruckig_c_consumer PRIVATE RUCKIG_C_EXPECT_STATIC_DEFINE)
elseif(RUCKIG_C_EXPECT_STATIC_DEFINE STREQUAL "OFF")
  target_compile_definitions(ruckig_c_consumer PRIVATE RUCKIG_C_EXPECT_NO_STATIC_DEFINE)
endif()
set_target_properties(ruckig_c_consumer PROPERTIES
  C_STANDARD 99
  C_STANDARD_REQUIRED ON
  C_EXTENSIONS OFF
)
