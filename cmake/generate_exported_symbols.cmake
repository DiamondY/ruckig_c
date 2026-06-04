if(NOT DEFINED RUCKIG_C_SHARED_LIBRARY)
  message(FATAL_ERROR "RUCKIG_C_SHARED_LIBRARY is required")
endif()
if(NOT DEFINED RUCKIG_C_OUTPUT_FILE)
  message(FATAL_ERROR "RUCKIG_C_OUTPUT_FILE is required")
endif()

if(NOT EXISTS "${RUCKIG_C_SHARED_LIBRARY}")
  message(FATAL_ERROR "shared library does not exist: ${RUCKIG_C_SHARED_LIBRARY}")
endif()

get_filename_component(output_dir "${RUCKIG_C_OUTPUT_FILE}" DIRECTORY)
file(MAKE_DIRECTORY "${output_dir}")

if(WIN32)
  find_program(LLVM_READOBJ_EXECUTABLE NAMES llvm-readobj llvm-readobj.exe)
  find_program(DUMPBIN_EXECUTABLE NAMES dumpbin dumpbin.exe)
  if(LLVM_READOBJ_EXECUTABLE)
    execute_process(
      COMMAND "${LLVM_READOBJ_EXECUTABLE}" --coff-exports "${RUCKIG_C_SHARED_LIBRARY}"
      OUTPUT_VARIABLE exports_output
      ERROR_VARIABLE exports_error
      RESULT_VARIABLE exports_result
    )
  elseif(DUMPBIN_EXECUTABLE)
    execute_process(
      COMMAND "${DUMPBIN_EXECUTABLE}" /EXPORTS "${RUCKIG_C_SHARED_LIBRARY}"
      OUTPUT_VARIABLE exports_output
      ERROR_VARIABLE exports_error
      RESULT_VARIABLE exports_result
    )
  else()
    message(FATAL_ERROR "Neither llvm-readobj nor dumpbin was found for Windows export inspection")
  endif()
elseif(APPLE)
  find_program(NM_EXECUTABLE NAMES nm)
  if(NOT NM_EXECUTABLE)
    message(FATAL_ERROR "nm was not found for exported symbol inspection")
  endif()
  execute_process(
    COMMAND "${NM_EXECUTABLE}" -gU "${RUCKIG_C_SHARED_LIBRARY}"
    OUTPUT_VARIABLE exports_output
    ERROR_VARIABLE exports_error
    RESULT_VARIABLE exports_result
  )
else()
  find_program(NM_EXECUTABLE NAMES nm)
  if(NOT NM_EXECUTABLE)
    message(FATAL_ERROR "nm was not found for exported symbol inspection")
  endif()
  execute_process(
    COMMAND "${NM_EXECUTABLE}" -D --defined-only "${RUCKIG_C_SHARED_LIBRARY}"
    OUTPUT_VARIABLE exports_output
    ERROR_VARIABLE exports_error
    RESULT_VARIABLE exports_result
  )
endif()

if(NOT exports_result EQUAL 0)
  message(FATAL_ERROR "exported symbol inspection failed: ${exports_error}")
endif()

file(WRITE "${RUCKIG_C_OUTPUT_FILE}" "${exports_output}")
message(STATUS "Wrote exported symbols to ${RUCKIG_C_OUTPUT_FILE}")
