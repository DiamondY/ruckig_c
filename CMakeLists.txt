cmake_minimum_required(VERSION 3.16)

project(ruckig_c_rewrite VERSION 0.2.3 LANGUAGES C)

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
option(RUCKIG_C_ENABLE_CALCULATION_DURATION "Measure ruckig_update calculation duration in microseconds" OFF)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

set(RUCKIG_C_INSTALL_CMAKEDIR
  "${CMAKE_INSTALL_LIBDIR}/cmake/ruckig_c"
  CACHE STRING "Install directory for ruckig_c CMake package files"
)

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
  src/ruckig_c/trajectory.c
  src/ruckig_c/utils.c
  src/ruckig_c/velocity_second_step1.c
  src/ruckig_c/velocity_second_step2.c
  src/ruckig_c/velocity_third_step1.c
  src/ruckig_c/velocity_third_step2.c
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

if(BUILD_RUCKIG_C)
  add_library(ruckig_c ${RUCKIG_C_SOURCES})

  target_compile_features(ruckig_c PUBLIC c_std_99)
  set_target_properties(ruckig_c PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
  )

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
    if(NOT WIN32)
      target_link_libraries(ruckig_c PRIVATE m)
    endif()
  endif()
  ruckig_c_apply_sanitizers(ruckig_c)

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
    if(NOT WIN32)
      target_link_libraries(ruckig_c_tests PRIVATE m)
    endif()
  endif()
  ruckig_c_apply_sanitizers(ruckig_c_tests)
  add_test(NAME ruckig_c_tests COMMAND ruckig_c_tests)
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

  if(WIN32 AND CMAKE_C_COMPILER_ID MATCHES "Clang")
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
endif()

if(BUILD_RUCKIG_C AND BUILD_SHARED_LIBS)
  set(RUCKIG_C_ABI_PLATFORM "linux")
  if(WIN32)
    set(RUCKIG_C_ABI_PLATFORM "windows")
  elseif(APPLE)
    set(RUCKIG_C_ABI_PLATFORM "macos")
  endif()
  set(RUCKIG_C_ABI_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/artifacts/abi/0.2.4")
  set(RUCKIG_C_ABI_EXPORTS_FILE "${RUCKIG_C_ABI_OUTPUT_DIR}/${RUCKIG_C_ABI_PLATFORM}-exports.txt")
  set(RUCKIG_C_ABI_DIFF_FILE "${RUCKIG_C_ABI_OUTPUT_DIR}/${RUCKIG_C_ABI_PLATFORM}-export-diff.txt")
  set(RUCKIG_C_ABI_BASELINE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/docs/abi/v0.2.3/${RUCKIG_C_ABI_PLATFORM}-symbols.txt")
  add_custom_command(
    OUTPUT ${RUCKIG_C_ABI_EXPORTS_FILE}
    COMMAND ${CMAKE_COMMAND}
      "-DRUCKIG_C_SHARED_LIBRARY=$<TARGET_FILE:ruckig_c>"
      "-DRUCKIG_C_OUTPUT_FILE=${RUCKIG_C_ABI_EXPORTS_FILE}"
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
