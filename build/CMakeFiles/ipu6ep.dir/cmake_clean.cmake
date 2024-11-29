file(REMOVE_RECURSE
  "ipu6ep.pdb"
  "ipu6ep.so"
)

# Per-language clean rules from dependency scanning.
foreach(lang C CXX)
  include(CMakeFiles/ipu6ep.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
