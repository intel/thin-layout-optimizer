foreach (POSTFIX IN LISTS SANITIZE_POSTFIXES)
  target_sources(${HFSORT_EXE}${POSTFIX} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/main.cc)
endforeach()
add_static_analysis_impl_file(${CMAKE_CURRENT_SOURCE_DIR}/main.cc)
