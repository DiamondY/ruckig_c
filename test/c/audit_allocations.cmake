set(source_dir "${CMAKE_CURRENT_LIST_DIR}/../../src/ruckig_c")
file(GLOB sources "${source_dir}/*.c")

set(failures "")
foreach(source IN LISTS sources)
  get_filename_component(name "${source}" NAME)
  if(name STREQUAL "alloc.c")
    continue()
  endif()

  file(READ "${source}" content)
  if(content MATCHES "(^|[^A-Za-z0-9_])(malloc|calloc|realloc|free)[ \t\r\n]*\\(")
    string(APPEND failures "${source}\n")
  endif()
endforeach()

if(failures)
  message(FATAL_ERROR "Raw heap allocation calls outside src/ruckig_c/alloc.c:\n${failures}")
endif()

