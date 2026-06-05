cmake_minimum_required(VERSION 3.16)

project(vcpkg_ruckig_c_consumer LANGUAGES C)

find_package(ruckig_c CONFIG REQUIRED)

get_target_property(ruckig_c_compile_definitions
  ruckig_c::ruckig_c
  INTERFACE_COMPILE_DEFINITIONS
)
if(NOT ruckig_c_compile_definitions)
  set(ruckig_c_compile_definitions "")
endif()

if(VCPKG_TARGET_TRIPLET MATCHES "static")
  if(NOT "RUCKIG_C_STATIC_DEFINE" IN_LIST ruckig_c_compile_definitions)
    message(FATAL_ERROR "Static vcpkg consumer target must propagate RUCKIG_C_STATIC_DEFINE")
  endif()
else()
  if("RUCKIG_C_STATIC_DEFINE" IN_LIST ruckig_c_compile_definitions)
    message(FATAL_ERROR "DLL vcpkg consumer target must not propagate RUCKIG_C_STATIC_DEFINE")
  endif()
endif()

add_executable(vcpkg-ruckig-c-consumer main.c)
target_link_libraries(vcpkg-ruckig-c-consumer PRIVATE ruckig_c::ruckig_c)

if(MSVC AND VCPKG_TARGET_TRIPLET MATCHES "static")
  set_property(TARGET vcpkg-ruckig-c-consumer PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
  )
endif()
