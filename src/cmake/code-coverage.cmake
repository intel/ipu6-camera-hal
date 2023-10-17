# Options
option(
  CODE_COVERAGE
  "Builds targets with code coverage instrumentation. (Requires GCC or Clang)"
  OFF)

# Enable code coverage
if(CODE_COVERAGE)
  if("${CMAKE_C_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang"
     OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang")
    add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
    add_link_options(-fprofile-instr-generate -fcoverage-mapping)
  elseif(CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-fprofile-arcs -ftest-coverage)
    link_libraries(gcov)
  endif()
endif()

