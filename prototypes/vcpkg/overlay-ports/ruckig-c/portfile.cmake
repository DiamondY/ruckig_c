get_filename_component(SOURCE_PATH "${CURRENT_PORT_DIR}/../../../.." ABSOLUTE)

vcpkg_cmake_configure(
  SOURCE_PATH "${SOURCE_PATH}"
  OPTIONS
    -DBUILD_RUCKIG_C=ON
    -DBUILD_RUCKIG_C_TESTS=OFF
    -DBUILD_RUCKIG_C_EXAMPLES=OFF
    -DBUILD_RUCKIG_C_ORACLE_TESTS=OFF
    -DBUILD_RUCKIG_C_PERFORMANCE_TESTS=OFF
    -DRUCKIG_C_ENABLE_ASAN=OFF
    -DRUCKIG_C_ENABLE_UBSAN=OFF
    -DRUCKIG_C_ENABLE_CALCULATION_DURATION=OFF
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME ruckig_c CONFIG_PATH lib/cmake/ruckig_c)
if(NOT VCPKG_TARGET_IS_WINDOWS)
  vcpkg_fixup_pkgconfig()
endif()

file(REMOVE_RECURSE
  "${CURRENT_PACKAGES_DIR}/debug/include"
  "${CURRENT_PACKAGES_DIR}/debug/share"
)

if(EXISTS "${CURRENT_PACKAGES_DIR}/debug/lib/pkgconfig/ruckig_c.pc")
  file(REMOVE "${CURRENT_PACKAGES_DIR}/debug/lib/pkgconfig/ruckig_c.pc")
endif()
if(VCPKG_TARGET_IS_WINDOWS)
  file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/lib/pkgconfig")
endif()

file(INSTALL "${SOURCE_PATH}/README.md"
  DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
  RENAME copyright
)
