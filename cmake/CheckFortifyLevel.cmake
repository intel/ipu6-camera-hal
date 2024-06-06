# CheckFortifySource.cmake
function(check_fortify_source output_variable)
  # Create a simple C++ source file to check _FORTIFY_SOURCE
  set(CHECK_SOURCE_CODE "#include <stdio.h>
#ifndef _FORTIFY_SOURCE
#define _FORTIFY_SOURCE 0
#endif
int main(){printf(\"%d\", _FORTIFY_SOURCE);return 0;}")

  # Set the full path for the source file
  set(SOURCE_FILE_NAME "${CMAKE_BINARY_DIR}/check_fortify_source.c")
  file(WRITE "${SOURCE_FILE_NAME}" "${CHECK_SOURCE_CODE}")

  # Try to compile the source file
  try_compile(FORTIFY_SOURCE_COMPILED
    "${CMAKE_BINARY_DIR}/temp" "${SOURCE_FILE_NAME}"
    COMPILE_DEFINITIONS "-O2"
    COPY_FILE "${CMAKE_BINARY_DIR}/check_fortify_source.out"
    OUTPUT_VARIABLE COMPILE_OUTPUT
  )

  # Check if compilation was successful
  if(FORTIFY_SOURCE_COMPILED)
    # Run the compiled program to get the value of _FORTIFY_SOURCE
    execute_process(COMMAND "${CMAKE_BINARY_DIR}/check_fortify_source.out"
                    RESULT_VARIABLE RUN_RESULT
                    OUTPUT_VARIABLE FORTIFY_SOURCE_VALUE
                    ERROR_QUIET
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(RUN_RESULT EQUAL 0)
      # Set the variable outside the function
      set(${output_variable} "${FORTIFY_SOURCE_VALUE}" PARENT_SCOPE)
    else()
      message(STATUS "Failed to run the compiled test program.")
      set(${output_variable} "" PARENT_SCOPE)
    endif()
  else()
    message(STATUS "Compilation failed; _FORTIFY_SOURCE is not defined.")
    message(STATUS "Compiler output: ${COMPILE_OUTPUT}")
    set(${output_variable} "" PARENT_SCOPE)
  endif()
endfunction()
