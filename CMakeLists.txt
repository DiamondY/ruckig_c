cmake_minimum_required(VERSION 3.16)

project(ruckig_c_rewrite VERSION 0.8.0 LANGUAGES C)

include(CTest)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(BUILD_RUCKIG_C "Build the pure C Ruckig library" ON)
option(BUILD_RUCKIG_C_TESTS "Build pure C Ruckig tests" ON)
option(BUILD_RUCKIG_C_EXAMPLES "Build pure C Ruckig examples" ON)
option(BUILD_RUCKIG_C_ORACLE_TESTS "Build oracle differential tests against original C++ Ruckig" OFF)
option(BUILD_RUCKIG_C_PERFORMANCE_TESTS "Build C/C++ oracle performance benchmark" OFF)
option(RUCKIG_C_ENABLE_ASAN "Enable AddressSanitizer for supported compilers" OFF)
option(RUCKIG_C_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer for supported compilers" OFF)
option(RUCKIG_C_ENABLE_COVERAGE "Enable local LLVM source coverage instrumentation" OFF)
option(RUCKIG_C_ENABLE_CALCULATION_DURATION "Measure ruckig_update calculation duration in microseconds" OFF)
option(RUCKIG_C_STRICT_PUBLIC_ABI "Fail shared-build ABI comparison on public exported-symbol drift" OFF)
option(RUCKIG_C_ENABLE_MSVC_CL_CONSUMER_SMOKE "Enable optional standalone MSVC cl consumer smoke tests on Windows" OFF)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

set(RUCKIG_C_INSTALL_CMAKEDIR
  "${CMAKE_INSTALL_LIBDIR}/cmake/ruckig_c"
  CACHE STRING "Install directory for ruckig_c CMake package files"
)
set(RUCKIG_C_ABI_PUBLIC_SYMBOLS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/docs/abi/public-symbols.txt")
set(RUCKIG_C_ABI_PUBLIC_EXCEPTIONS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/docs/abi/public-symbol-exceptions.txt")

set(RUCKIG_C_PC_PRIVATE_LIBS "")
if(NOT WIN32)
  list(APPEND RUCKIG_C_PC_PRIVATE_LIBS "-lm")
endif()
if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
  set(RUCKIG_C_PC_SANITIZERS "")
  if(RUCKIG_C_ENABLE_ASAN)
    list(APPEND RUCKIG_C_PC_SANITIZERS address)
  endif()
  if(RUCKIG_C_ENABLE_UBSAN)
    list(APPEND RUCKIG_C_PC_SANITIZERS undefined)
  endif()
  if(RUCKIG_C_PC_SANITIZERS)
    list(JOIN RUCKIG_C_PC_SANITIZERS "," RUCKIG_C_PC_SANITIZER_FLAGS)
    list(APPEND RUCKIG_C_PC_PRIVATE_LIBS "-fsanitize=${RUCKIG_C_PC_SANITIZER_FLAGS}")
  endif()
endif()
list(JOIN RUCKIG_C_PC_PRIVATE_LIBS " " RUCKIG_C_PC_PRIVATE_LIBS)

set(RUCKIG_C_SOURCES
  src/ruckig_c/alloc.c
  src/ruckig_c/block.c
  src/ruckig_c/brake.c
  src/ruckig_c/input.c
  src/ruckig_c/output.c
  src/ruckig_c/position_first_step1.c
  src/ruckig_c/position_first_step2.c
  src/ruckig_c/position_second_step1.c
  src/ruckig_c/position_second_step2.c
  src/ruckig_c/position_third_step1.c
  src/ruckig_c/position_third_step2.c
  src/ruckig_c/profile.c
  src/ruckig_c/roots.c
  src/ruckig_c/ruckig.c
  src/ruckig_c/tracking.c
  src/ruckig_c/trajectory.c
  src/ruckig_c/utils.c
  src/ruckig_c/velocity_second_step1.c
  src/ruckig_c/velocity_second_step2.c
  src/ruckig_c/velocity_third_step1.c
  src/ruckig_c/velocity_third_step2.c
  src/ruckig_c/waypoint.c
)

function(ruckig_c_apply_sanitizers target)
  if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    set(sanitizers "")
    if(RUCKIG_C_ENABLE_ASAN)
      list(APPEND sanitizers address)
    endif()
    if(RUCKIG_C_ENABLE_UBSAN)
      list(APPEND sanitizers undefined)
    endif()
    if(sanitizers)
      list(JOIN sanitizers "," sanitizer_flags)
      target_compile_options(${target} PRIVATE -fsanitize=${sanitizer_flags} -fno-omit-frame-pointer)
      target_link_options(${target} PUBLIC -fsanitize=${sanitizer_flags})
    endif()
  elseif(MSVC)
    if(RUCKIG_C_ENABLE_ASAN)
      target_compile_options(${target} PRIVATE /fsanitize=address)
      target_link_options(${target} PUBLIC /fsanitize=address)
    endif()
  endif()
endfunction()

function(ruckig_c_apply_coverage target)
  if(NOT RUCKIG_C_ENABLE_COVERAGE)
    return()
  endif()
  if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    target_compile_options(${target} PRIVATE -fprofile-instr-generate -fcoverage-mapping)
    target_link_options(${target} PUBLIC -fprofile-instr-generate -fcoverage-mapping)
  else()
    message(FATAL_ERROR "RUCKIG_C_ENABLE_COVERAGE currently requires a Clang C compiler")
  endif()
endfunction()

if(BUILD_RUCKIG_C)
  add_library(ruckig_c ${RUCKIG_C_SOURCES})

  target_compile_features(ruckig_c PUBLIC c_std_99)
  set_target_properties(ruckig_c PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
  )
  if(BUILD_SHARED_LIBS AND NOT WIN32 AND CMAKE_C_COMPILER_ID MATCHES "Clang|GNU|AppleClang")
    set_target_properties(ruckig_c PROPERTIES
      C_VISIBILITY_PRESET hidden
    )
    target_compile_options(ruckig_c PRIVATE -fvisibility=hidden)
    if(NOT APPLE AND EXISTS "${RUCKIG_C_ABI_PUBLIC_SYMBOLS_FILE}")
      file(STRINGS "${RUCKIG_C_ABI_PUBLIC_SYMBOLS_FILE}" ruckig_c_public_symbol_lines)
      set(ruckig_c_version_script "{\n  global:\n")
      foreach(symbol IN LISTS ruckig_c_public_symbol_lines)
        string(STRIP "${symbol}" symbol)
        if(symbol MATCHES "^ruckig_[A-Za-z0-9_]+$")
          string(APPEND ruckig_c_version_script "    ${symbol};\n")
        endif()
      endforeach()
      string(APPEND ruckig_c_version_script "  local:\n    *;\n};\n")
      set(RUCKIG_C_PUBLIC_VERSION_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/ruckig_c_public_symbols.map")
      file(WRITE "${RUCKIG_C_PUBLIC_VERSION_SCRIPT}" "${ruckig_c_version_script}")
      target_link_options(ruckig_c PRIVATE "LINKER:--version-script,${RUCKIG_C_PUBLIC_VERSION_SCRIPT}")
    endif()
  endif()

  target_include_directories(ruckig_c PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )
  target_include_directories(ruckig_c PRIVATE src)
  if(BUILD_SHARED_LIBS)
    target_compile_definitions(ruckig_c PRIVATE RUCKIG_C_BUILDING_LIBRARY)
  else()
    target_compile_definitions(ruckig_c PUBLIC RUCKIG_C_STATIC_DEFINE)
  endif()
  if(RUCKIG_C_ENABLE_CALCULATION_DURATION)
    target_compile_definitions(ruckig_c PUBLIC RUCKIG_C_ENABLE_CALCULATION_DURATION)
  endif()

  if(MSVC)
    target_compile_options(ruckig_c PRIVATE /W4)
    target_compile_definitions(ruckig_c PRIVATE _USE_MATH_DEFINES)
  else()
    target_compile_options(ruckig_c PRIVATE -Wall -Wextra -Wpedantic)
    if(NOT WIN32 OR MINGW)
      target_link_libraries(ruckig_c PRIVATE m)
    endif()
  endif()
  ruckig_c_apply_sanitizers(ruckig_c)
  ruckig_c_apply_coverage(ruckig_c)

  add_library(ruckig_c::ruckig_c ALIAS ruckig_c)
endif()

if(BUILD_RUCKIG_C_TESTS)
  if(NOT BUILD_RUCKIG_C)
    message(FATAL_ERROR "BUILD_RUCKIG_C_TESTS requires BUILD_RUCKIG_C")
  endif()
  enable_language(CXX)

  add_executable(ruckig_c_tests
    test/c/test_api.c
    test/c/test_brake.c
    test/c/test_profile.c
    test/c/test_roots.c
    test/c/test_solver_branch_coverage.c
    test/c/test_utils.c
    ${RUCKIG_C_SOURCES}
  )
  target_compile_definitions(ruckig_c_tests PRIVATE RUCKIG_C_STATIC_DEFINE)
  if(RUCKIG_C_ENABLE_CALCULATION_DURATION)
    target_compile_definitions(ruckig_c_tests PRIVATE RUCKIG_C_ENABLE_CALCULATION_DURATION)
  endif()
  target_include_directories(ruckig_c_tests PRIVATE include src)
  if(MSVC)
    target_compile_options(ruckig_c_tests PRIVATE /W4)
  else()
    target_compile_options(ruckig_c_tests PRIVATE -Wall -Wextra -Wpedantic)
    if(NOT WIN32 OR MINGW)
      target_link_libraries(ruckig_c_tests PRIVATE m)
    endif()
  endif()
  ruckig_c_apply_sanitizers(ruckig_c_tests)
  ruckig_c_apply_coverage(ruckig_c_tests)
  add_test(NAME ruckig_c_tests COMMAND ruckig_c_tests)
  add_test(NAME ruckig_c_waypoint_optimizer COMMAND ruckig_c_tests --waypoint)
  add_test(NAME ruckig_c_per_section_constraints COMMAND ruckig_c_tests --per-section)
  add_test(NAME ruckig_c_waypoint_quality COMMAND ruckig_c_tests --waypoint-quality)
  add_test(NAME ruckig_c_solver_branch_coverage COMMAND ruckig_c_tests --solver-branch-coverage)
  add_test(NAME ruckig_c_tracking_api COMMAND ruckig_c_tests --tracking-api)
  add_test(NAME ruckig_c_tracking_validation COMMAND ruckig_c_tests --tracking-validation)
  add_test(NAME ruckig_c_tracking_online COMMAND ruckig_c_tests --tracking-online)
  add_test(NAME ruckig_c_tracking_fixed_corpus COMMAND ruckig_c_tests --tracking-fixed-corpus)
  add_test(NAME ruckig_c_tracking_offline COMMAND ruckig_c_tests --tracking-offline)
  add_test(NAME ruckig_c_tracking_optimized COMMAND ruckig_c_tests --tracking-optimized)
  add_test(NAME ruckig_c_tracking_quality COMMAND ruckig_c_tests --tracking-quality)
  add_test(NAME ruckig_c_tracking_random_seed_1 COMMAND ruckig_c_tests --tracking-random 100000 --seed 1)
  add_test(NAME ruckig_c_tracking_random_seed_2 COMMAND ruckig_c_tests --tracking-random 100000 --seed 2)
  add_test(NAME ruckig_c_tracking_random_seed_41 COMMAND ruckig_c_tests --tracking-random 100000 --seed 41)
  add_test(NAME ruckig_c_tracking_random_audit COMMAND ruckig_c_tests --tracking-random-audit 10000 --seed 1)
  add_test(NAME ruckig_c_tracking_no_allocation COMMAND ruckig_c_tests --tracking-no-allocation)
  add_test(NAME ruckig_c_allocation_audit
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/test/c/audit_allocations.cmake
  )

  add_executable(ruckig_c_header_c test/c/header_compile.c)
  target_link_libraries(ruckig_c_header_c PRIVATE ruckig_c)
  add_test(NAME ruckig_c_header_c COMMAND ruckig_c_header_c)

  add_executable(ruckig_c_header_cpp test/c/header_compile.cpp)
  target_link_libraries(ruckig_c_header_cpp PRIVATE ruckig_c)
  target_compile_features(ruckig_c_header_cpp PRIVATE cxx_std_11)
  ruckig_c_apply_sanitizers(ruckig_c_header_c)
  ruckig_c_apply_sanitizers(ruckig_c_header_cpp)
  add_test(NAME ruckig_c_header_cpp COMMAND ruckig_c_header_cpp)

  configure_file(
    test/cmake/test_cmake_consumer.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/test_cmake_consumer.cmake
    @ONLY
  )
  add_test(NAME ruckig_c_cmake_consumer
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/test_cmake_consumer.cmake
  )

  if(UNIX)
    find_program(PKG_CONFIG_EXECUTABLE pkg-config)
    if(PKG_CONFIG_EXECUTABLE)
      configure_file(
        test/cmake/test_pkg_config_consumer.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/test_pkg_config_consumer.cmake
        @ONLY
      )
      add_test(NAME ruckig_c_pkg_config_consumer
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/test_pkg_config_consumer.cmake
      )
      configure_file(
        test/cmake/test_shared_install_consumer.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/test_shared_install_consumer.cmake
        @ONLY
      )
      add_test(NAME ruckig_c_shared_install_consumer
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/test_shared_install_consumer.cmake
      )
    endif()
  endif()

  if(WIN32 AND CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    if(BUILD_SHARED_LIBS)
      configure_file(
        test/cmake/test_windows_dll_consumer.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/test_windows_dll_consumer.cmake
        @ONLY
      )
      add_test(NAME ruckig_c_windows_dll_consumer
        COMMAND ${CMAKE_COMMAND}
          "-DRUCKIG_C_IMPORT_LIBRARY=$<TARGET_LINKER_FILE:ruckig_c>"
          "-DRUCKIG_C_DLL_DIR=$<TARGET_FILE_DIR:ruckig_c>"
          -P ${CMAKE_CURRENT_BINARY_DIR}/test_windows_dll_consumer.cmake
      )
    else()
      configure_file(
        test/cmake/test_windows_static_consumer.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/test_windows_static_consumer.cmake
        @ONLY
      )
      add_test(NAME ruckig_c_windows_manual_static_consumer
        COMMAND ${CMAKE_COMMAND}
          "-DRUCKIG_C_STATIC_LIBRARY=$<TARGET_LINKER_FILE:ruckig_c>"
          -P ${CMAKE_CURRENT_BINARY_DIR}/test_windows_static_consumer.cmake
      )
    endif()
  endif()

  if(WIN32 AND RUCKIG_C_ENABLE_MSVC_CL_CONSUMER_SMOKE)
    if(BUILD_SHARED_LIBS)
      configure_file(
        test/cmake/test_msvc_cl_dll_consumer.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/test_msvc_cl_dll_consumer.cmake
        @ONLY
      )
      add_test(NAME ruckig_c_msvc_cl_dll_consumer
        COMMAND ${CMAKE_COMMAND}
          "-DRUCKIG_C_IMPORT_LIBRARY=$<TARGET_LINKER_FILE:ruckig_c>"
          "-DRUCKIG_C_DLL_DIR=$<TARGET_FILE_DIR:ruckig_c>"
          -P ${CMAKE_CURRENT_BINARY_DIR}/test_msvc_cl_dll_consumer.cmake
      )
    else()
      configure_file(
        test/cmake/test_msvc_cl_static_consumer.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/test_msvc_cl_static_consumer.cmake
        @ONLY
      )
      add_test(NAME ruckig_c_msvc_cl_static_consumer
        COMMAND ${CMAKE_COMMAND}
          "-DRUCKIG_C_STATIC_LIBRARY=$<TARGET_LINKER_FILE:ruckig_c>"
          -P ${CMAKE_CURRENT_BINARY_DIR}/test_msvc_cl_static_consumer.cmake
      )
    endif()
  endif()
endif()

if(BUILD_RUCKIG_C_EXAMPLES)
  if(NOT BUILD_RUCKIG_C)
    message(FATAL_ERROR "BUILD_RUCKIG_C_EXAMPLES requires BUILD_RUCKIG_C")
  endif()

  add_executable(example-ruckig-c-00-minimal-offline examples/c/00_minimal_offline.c)
  target_link_libraries(example-ruckig-c-00-minimal-offline PRIVATE ruckig_c)
  add_test(NAME example_ruckig_c_00_minimal_offline COMMAND example-ruckig-c-00-minimal-offline)

  add_executable(example-ruckig-c-01-position examples/c/01_position.c)
  target_link_libraries(example-ruckig-c-01-position PRIVATE ruckig_c)
  add_test(NAME example_ruckig_c_01_position COMMAND example-ruckig-c-01-position)

  add_executable(example-ruckig-c-02-position-offline examples/c/02_position_offline.c)
  target_link_libraries(example-ruckig-c-02-position-offline PRIVATE ruckig_c)
  add_test(NAME example_ruckig_c_02_position_offline COMMAND example-ruckig-c-02-position-offline)

  add_executable(example-ruckig-c-03-minimal-online examples/c/03_minimal_online.c)
  target_link_libraries(example-ruckig-c-03-minimal-online PRIVATE ruckig_c)
  add_test(NAME example_ruckig_c_03_minimal_online COMMAND example-ruckig-c-03-minimal-online)

  add_executable(example-ruckig-c-04-per-dof-override examples/c/04_per_dof_override.c)
  target_link_libraries(example-ruckig-c-04-per-dof-override PRIVATE ruckig_c)
  add_test(NAME example_ruckig_c_04_per_dof_override COMMAND example-ruckig-c-04-per-dof-override)

  add_executable(example-ruckig-c-05-velocity examples/c/05_velocity.c)
  target_link_libraries(example-ruckig-c-05-velocity PRIVATE ruckig_c)
  add_test(NAME example_ruckig_c_05_velocity COMMAND example-ruckig-c-05-velocity)

  add_executable(example-ruckig-c-06-stop examples/c/06_stop.c)
  target_link_libraries(example-ruckig-c-06-stop PRIVATE ruckig_c)
  add_test(NAME example_ruckig_c_06_stop COMMAND example-ruckig-c-06-stop)

  add_executable(example-ruckig-c-07-minimum-duration examples/c/07_minimum_duration.c)
  target_link_libraries(example-ruckig-c-07-minimum-duration PRIVATE ruckig_c)
  add_test(NAME example_ruckig_c_07_minimum_duration COMMAND example-ruckig-c-07-minimum-duration)

  add_executable(example-ruckig-c-08-per-dof-online examples/c/08_per_dof_online.c)
  target_link_libraries(example-ruckig-c-08-per-dof-online PRIVATE ruckig_c)
  add_test(NAME example_ruckig_c_08_per_dof_online COMMAND example-ruckig-c-08-per-dof-online)

  add_executable(example-ruckig-c-09-waypoints-offline examples/c/09_waypoints_offline.c)
  target_link_libraries(example-ruckig-c-09-waypoints-offline PRIVATE ruckig_c)
  add_test(NAME example_ruckig_c_09_waypoints_offline COMMAND example-ruckig-c-09-waypoints-offline)

  add_executable(example-ruckig-c-10-per-section-minimum-duration examples/c/10_per_section_minimum_duration.c)
  target_link_libraries(example-ruckig-c-10-per-section-minimum-duration PRIVATE ruckig_c)
  add_test(NAME example_ruckig_c_10_per_section_minimum_duration COMMAND example-ruckig-c-10-per-section-minimum-duration)

  add_executable(example-ruckig-c-11-waypoints-online examples/c/11_waypoints_online.c)
  target_link_libraries(example-ruckig-c-11-waypoints-online PRIVATE ruckig_c)
  add_test(NAME example_ruckig_c_11_waypoints_online COMMAND example-ruckig-c-11-waypoints-online)

  add_executable(example-ruckig-c-12-per-section-limits examples/c/12_per_section_limits.c)
  target_link_libraries(example-ruckig-c-12-per-section-limits PRIVATE ruckig_c)
  add_test(NAME example_ruckig_c_12_per_section_limits COMMAND example-ruckig-c-12-per-section-limits)

  add_executable(example-ruckig-c-13-filter-intermediate-positions examples/c/13_filter_intermediate_positions.c)
  target_link_libraries(example-ruckig-c-13-filter-intermediate-positions PRIVATE ruckig_c)
  add_test(NAME example_ruckig_c_13_filter_intermediate_positions COMMAND example-ruckig-c-13-filter-intermediate-positions)

  add_executable(example-ruckig-c-14-dynamic-dofs-waypoints examples/c/14_dynamic_dofs_waypoints.c)
  target_link_libraries(example-ruckig-c-14-dynamic-dofs-waypoints PRIVATE ruckig_c)
  add_test(NAME example_ruckig_c_14_dynamic_dofs_waypoints COMMAND example-ruckig-c-14-dynamic-dofs-waypoints)

  add_executable(example-ruckig-c-15-tracking-online-fast-ramp examples/c/15_tracking_online_fast_ramp.c)
  target_link_libraries(example-ruckig-c-15-tracking-online-fast-ramp PRIVATE ruckig_c)
  add_test(NAME example_ruckig_c_15_tracking_online_fast_ramp COMMAND example-ruckig-c-15-tracking-online-fast-ramp)

  add_executable(example-ruckig-c-16-tracking-online-constant-acceleration examples/c/16_tracking_online_constant_acceleration.c)
  target_link_libraries(example-ruckig-c-16-tracking-online-constant-acceleration PRIVATE ruckig_c)
  add_test(NAME example_ruckig_c_16_tracking_online_constant_acceleration COMMAND example-ruckig-c-16-tracking-online-constant-acceleration)

  add_executable(example-ruckig-c-17-tracking-offline-sequence examples/c/17_tracking_offline_sequence.c)
  target_link_libraries(example-ruckig-c-17-tracking-offline-sequence PRIVATE ruckig_c)
  if(NOT WIN32)
    target_link_libraries(example-ruckig-c-17-tracking-offline-sequence PRIVATE m)
  endif()
  add_test(NAME example_ruckig_c_17_tracking_offline_sequence COMMAND example-ruckig-c-17-tracking-offline-sequence)

  add_executable(example-ruckig-c-18-tracking-online-optimized-lookahead examples/c/18_tracking_online_optimized_lookahead.c)
  target_link_libraries(example-ruckig-c-18-tracking-online-optimized-lookahead PRIVATE ruckig_c)
  add_test(NAME example_ruckig_c_18_tracking_online_optimized_lookahead COMMAND example-ruckig-c-18-tracking-online-optimized-lookahead)

  add_executable(example-ruckig-c-19-tracking-online-optimized-sinus examples/c/19_tracking_online_optimized_sinus.c)
  target_link_libraries(example-ruckig-c-19-tracking-online-optimized-sinus PRIVATE ruckig_c)
  if(NOT WIN32)
    target_link_libraries(example-ruckig-c-19-tracking-online-optimized-sinus PRIVATE m)
  endif()
  add_test(NAME example_ruckig_c_19_tracking_online_optimized_sinus COMMAND example-ruckig-c-19-tracking-online-optimized-sinus)

  add_executable(example-ruckig-c-20-tracking-offline-optimized-sequence examples/c/20_tracking_offline_optimized_sequence.c)
  target_link_libraries(example-ruckig-c-20-tracking-offline-optimized-sequence PRIVATE ruckig_c)
  if(NOT WIN32)
    target_link_libraries(example-ruckig-c-20-tracking-offline-optimized-sequence PRIVATE m)
  endif()
  add_test(NAME example_ruckig_c_20_tracking_offline_optimized_sequence COMMAND example-ruckig-c-20-tracking-offline-optimized-sequence)

  add_custom_target(ruckig_c_examples
    DEPENDS
      example-ruckig-c-00-minimal-offline
      example-ruckig-c-01-position
      example-ruckig-c-02-position-offline
      example-ruckig-c-03-minimal-online
      example-ruckig-c-04-per-dof-override
      example-ruckig-c-05-velocity
      example-ruckig-c-06-stop
      example-ruckig-c-07-minimum-duration
      example-ruckig-c-08-per-dof-online
      example-ruckig-c-09-waypoints-offline
      example-ruckig-c-10-per-section-minimum-duration
      example-ruckig-c-11-waypoints-online
      example-ruckig-c-12-per-section-limits
      example-ruckig-c-13-filter-intermediate-positions
      example-ruckig-c-14-dynamic-dofs-waypoints
      example-ruckig-c-15-tracking-online-fast-ramp
      example-ruckig-c-16-tracking-online-constant-acceleration
      example-ruckig-c-17-tracking-offline-sequence
      example-ruckig-c-18-tracking-online-optimized-lookahead
      example-ruckig-c-19-tracking-online-optimized-sinus
      example-ruckig-c-20-tracking-offline-optimized-sequence
  )
endif()

if(BUILD_RUCKIG_C_ORACLE_TESTS)
  enable_language(CXX)
  add_executable(ruckig_c_oracle_tests
    test/cpp/oracle_compare.cpp
    original/ruckig-main/src/ruckig/brake.cpp
    original/ruckig-main/src/ruckig/position_first_step1.cpp
    original/ruckig-main/src/ruckig/position_first_step2.cpp
    original/ruckig-main/src/ruckig/position_second_step1.cpp
    original/ruckig-main/src/ruckig/position_second_step2.cpp
    original/ruckig-main/src/ruckig/position_third_step1.cpp
    original/ruckig-main/src/ruckig/position_third_step2.cpp
    original/ruckig-main/src/ruckig/velocity_second_step1.cpp
    original/ruckig-main/src/ruckig/velocity_second_step2.cpp
    original/ruckig-main/src/ruckig/velocity_third_step1.cpp
    original/ruckig-main/src/ruckig/velocity_third_step2.cpp
  )
  target_link_libraries(ruckig_c_oracle_tests PRIVATE ruckig_c)
  target_include_directories(ruckig_c_oracle_tests PRIVATE
    original/ruckig-main/include
  )
  target_compile_features(ruckig_c_oracle_tests PRIVATE cxx_std_20)
  if(MSVC)
    target_compile_options(ruckig_c_oracle_tests PRIVATE /W4)
    target_compile_definitions(ruckig_c_oracle_tests PRIVATE _USE_MATH_DEFINES)
  else()
    target_compile_options(ruckig_c_oracle_tests PRIVATE -Wall -Wextra)
    target_compile_definitions(ruckig_c_oracle_tests PRIVATE _USE_MATH_DEFINES)
  endif()
  ruckig_c_apply_sanitizers(ruckig_c_oracle_tests)
  add_test(NAME ruckig_c_oracle_tests COMMAND ruckig_c_oracle_tests)
  add_test(NAME ruckig_c_waypoint_section_oracle COMMAND ruckig_c_oracle_tests --waypoint-section-oracle)
  add_test(NAME ruckig_c_oracle_random_smoke COMMAND ruckig_c_oracle_tests --random 100 --seed 1)
  add_test(NAME ruckig_c_oracle_random_per_dof_smoke COMMAND ruckig_c_oracle_tests --random-per-dof 100 --seed 1)
  add_test(NAME ruckig_c_oracle_random_development COMMAND ruckig_c_oracle_tests --random 100000 --seed 1)
  add_test(NAME ruckig_c_oracle_random_release COMMAND ruckig_c_oracle_tests --random 1000000 --seed 1)
endif()

if(BUILD_RUCKIG_C_PERFORMANCE_TESTS)
  if(NOT BUILD_RUCKIG_C)
    message(FATAL_ERROR "BUILD_RUCKIG_C_PERFORMANCE_TESTS requires BUILD_RUCKIG_C")
  endif()
  enable_language(CXX)
  add_executable(ruckig_c_performance_benchmark
    test/cpp/performance_benchmark.cpp
    original/ruckig-main/src/ruckig/brake.cpp
    original/ruckig-main/src/ruckig/position_first_step1.cpp
    original/ruckig-main/src/ruckig/position_first_step2.cpp
    original/ruckig-main/src/ruckig/position_second_step1.cpp
    original/ruckig-main/src/ruckig/position_second_step2.cpp
    original/ruckig-main/src/ruckig/position_third_step1.cpp
    original/ruckig-main/src/ruckig/position_third_step2.cpp
    original/ruckig-main/src/ruckig/velocity_second_step1.cpp
    original/ruckig-main/src/ruckig/velocity_second_step2.cpp
    original/ruckig-main/src/ruckig/velocity_third_step1.cpp
    original/ruckig-main/src/ruckig/velocity_third_step2.cpp
  )
  target_link_libraries(ruckig_c_performance_benchmark PRIVATE ruckig_c)
  target_include_directories(ruckig_c_performance_benchmark PRIVATE
    original/ruckig-main/include
  )
  target_compile_features(ruckig_c_performance_benchmark PRIVATE cxx_std_20)
  if(MSVC)
    target_compile_options(ruckig_c_performance_benchmark PRIVATE /W4)
    target_compile_definitions(ruckig_c_performance_benchmark PRIVATE _USE_MATH_DEFINES)
  else()
    target_compile_options(ruckig_c_performance_benchmark PRIVATE -Wall -Wextra)
    target_compile_definitions(ruckig_c_performance_benchmark PRIVATE _USE_MATH_DEFINES)
  endif()
  ruckig_c_apply_sanitizers(ruckig_c_performance_benchmark)
  add_test(NAME ruckig_c_performance_benchmark COMMAND ruckig_c_performance_benchmark --samples 10000 --seed 1)
  add_test(NAME ruckig_c_waypoint_performance_benchmark COMMAND ruckig_c_performance_benchmark --samples 1000 --seed 1 --waypoints)
endif()

set(RUCKIG_C_ABI_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/artifacts/abi/0.8.0")
set(RUCKIG_C_ABI_PUBLIC_GENERATED_FILE "${RUCKIG_C_ABI_OUTPUT_DIR}/public-symbols-from-header.txt")
set(RUCKIG_C_ABI_PUBLIC_VERIFY_FILE "${RUCKIG_C_ABI_OUTPUT_DIR}/public-symbols-verify.txt")
add_custom_command(
  OUTPUT ${RUCKIG_C_ABI_PUBLIC_VERIFY_FILE}
  BYPRODUCTS ${RUCKIG_C_ABI_PUBLIC_GENERATED_FILE}
  COMMAND ${CMAKE_COMMAND}
    "-DRUCKIG_C_PUBLIC_HEADER=${CMAKE_CURRENT_SOURCE_DIR}/include/ruckig_c/ruckig.h"
    "-DRUCKIG_C_OUTPUT_FILE=${RUCKIG_C_ABI_PUBLIC_GENERATED_FILE}"
    "-DRUCKIG_C_EXPECTED_PUBLIC_SYMBOLS=${RUCKIG_C_ABI_PUBLIC_SYMBOLS_FILE}"
    "-DRUCKIG_C_VERIFY_OUTPUT=${RUCKIG_C_ABI_PUBLIC_VERIFY_FILE}"
    -DRUCKIG_C_FAIL_ON_MISMATCH=ON
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_public_symbols.cmake
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ruckig_c/ruckig.h
    ${RUCKIG_C_ABI_PUBLIC_SYMBOLS_FILE}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_public_symbols.cmake
  VERBATIM
)
add_custom_target(ruckig_c_verify_public_symbols
  DEPENDS ${RUCKIG_C_ABI_PUBLIC_VERIFY_FILE}
)

if(BUILD_RUCKIG_C AND BUILD_SHARED_LIBS)
  set(RUCKIG_C_ABI_PLATFORM "linux")
  set(RUCKIG_C_ABI_EXPORT_FORMAT "elf")
  if(WIN32)
    set(RUCKIG_C_ABI_PLATFORM "windows")
    set(RUCKIG_C_ABI_EXPORT_FORMAT "coff")
  elseif(APPLE)
    set(RUCKIG_C_ABI_PLATFORM "macos")
    set(RUCKIG_C_ABI_EXPORT_FORMAT "macho")
  endif()
  set(RUCKIG_C_ABI_EXPORTS_FILE "${RUCKIG_C_ABI_OUTPUT_DIR}/${RUCKIG_C_ABI_PLATFORM}-exports.txt")
  set(RUCKIG_C_ABI_DIFF_FILE "${RUCKIG_C_ABI_OUTPUT_DIR}/${RUCKIG_C_ABI_PLATFORM}-export-diff.txt")
  set(RUCKIG_C_ABI_PUBLIC_DIFF_FILE "${RUCKIG_C_ABI_OUTPUT_DIR}/${RUCKIG_C_ABI_PLATFORM}-public-export-diff.txt")
  set(RUCKIG_C_ABI_BASELINE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/docs/abi/v0.2.5/${RUCKIG_C_ABI_PLATFORM}-symbols.txt")
  add_custom_command(
    OUTPUT ${RUCKIG_C_ABI_EXPORTS_FILE}
    COMMAND ${CMAKE_COMMAND}
      "-DRUCKIG_C_SHARED_LIBRARY=$<TARGET_FILE:ruckig_c>"
      "-DRUCKIG_C_OUTPUT_FILE=${RUCKIG_C_ABI_EXPORTS_FILE}"
      "-DRUCKIG_C_EXPORT_FORMAT=${RUCKIG_C_ABI_EXPORT_FORMAT}"
      -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_exported_symbols.cmake
    DEPENDS ruckig_c
    VERBATIM
  )
  add_custom_target(ruckig_c_exported_symbols
    DEPENDS ${RUCKIG_C_ABI_EXPORTS_FILE}
  )
  if(EXISTS "${RUCKIG_C_ABI_BASELINE_FILE}")
    add_custom_command(
      OUTPUT ${RUCKIG_C_ABI_DIFF_FILE}
      COMMAND ${CMAKE_COMMAND}
        "-DRUCKIG_C_CURRENT_EXPORTS=${RUCKIG_C_ABI_EXPORTS_FILE}"
        "-DRUCKIG_C_BASELINE_SYMBOLS=${RUCKIG_C_ABI_BASELINE_FILE}"
        "-DRUCKIG_C_DIFF_OUTPUT=${RUCKIG_C_ABI_DIFF_FILE}"
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/compare_exported_symbols.cmake
      DEPENDS ${RUCKIG_C_ABI_EXPORTS_FILE} ${RUCKIG_C_ABI_BASELINE_FILE}
      VERBATIM
    )
    add_custom_target(ruckig_c_compare_exported_symbols
      DEPENDS ${RUCKIG_C_ABI_DIFF_FILE}
    )
    if(EXISTS "${RUCKIG_C_ABI_PUBLIC_SYMBOLS_FILE}")
      add_custom_command(
        OUTPUT ${RUCKIG_C_ABI_PUBLIC_DIFF_FILE}
        COMMAND ${CMAKE_COMMAND}
          "-DRUCKIG_C_CURRENT_EXPORTS=${RUCKIG_C_ABI_EXPORTS_FILE}"
          "-DRUCKIG_C_BASELINE_SYMBOLS=${RUCKIG_C_ABI_BASELINE_FILE}"
          "-DRUCKIG_C_PUBLIC_SYMBOLS=${RUCKIG_C_ABI_PUBLIC_SYMBOLS_FILE}"
          "-DRUCKIG_C_PUBLIC_ABI_EXCEPTIONS=${RUCKIG_C_ABI_PUBLIC_EXCEPTIONS_FILE}"
          "-DRUCKIG_C_PUBLIC_DIFF_OUTPUT=${RUCKIG_C_ABI_PUBLIC_DIFF_FILE}"
          "-DRUCKIG_C_STRICT_PUBLIC_ABI=${RUCKIG_C_STRICT_PUBLIC_ABI}"
          -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/compare_public_exported_symbols.cmake
        DEPENDS ${RUCKIG_C_ABI_EXPORTS_FILE} ${RUCKIG_C_ABI_BASELINE_FILE} ${RUCKIG_C_ABI_PUBLIC_SYMBOLS_FILE} ${RUCKIG_C_ABI_PUBLIC_EXCEPTIONS_FILE}
        VERBATIM
      )
      add_custom_target(ruckig_c_compare_public_exported_symbols
        DEPENDS ${RUCKIG_C_ABI_PUBLIC_DIFF_FILE}
      )
    endif()
  endif()
endif()

if(BUILD_RUCKIG_C)
  configure_package_config_file(
    cmake/ruckig_cConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ruckig_cConfig.cmake
    INSTALL_DESTINATION ${RUCKIG_C_INSTALL_CMAKEDIR}
  )
  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/ruckig_cConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
  )
  configure_file(
    cmake/ruckig_c.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/ruckig_c.pc
    @ONLY
  )

  install(TARGETS ruckig_c
    EXPORT ruckig_c-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
  install(DIRECTORY include/ruckig_c DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
  install(EXPORT ruckig_c-targets
    NAMESPACE ruckig_c::
    DESTINATION ${RUCKIG_C_INSTALL_CMAKEDIR}
  )
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ruckig_cConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/ruckig_cConfigVersion.cmake
    DESTINATION ${RUCKIG_C_INSTALL_CMAKEDIR}
  )
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ruckig_c.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
  )
endif()
