

add_cc_source_cur(
  verbosity.cc
  global-stats.cc
)
if(FOUND_ZSTD)
  add_cc_source_cur(
    compressed-reader.cc
  )
endif()
