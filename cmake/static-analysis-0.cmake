function(add_static_analysis_file CLANG_TIDY STATIC_ANALYSIS_TARGET STATIC_ANALYSIS_SOURCE_FILES SRC_FNAME)
  string(REGEX REPLACE "[_\./]" "-" CLEANED_FNAME ${SRC_FNAME})
  set(PER_FILE_TARGET "hidden-lint-${CLEANED_FNAME}")
  add_custom_target(
    ${PER_FILE_TARGET}
    COMMAND ${CLANG_TIDY}
    ${SRC_FNAME}
    -p=${CMAKE_CURRENT_BINARY_DIR}
  )
  add_dependencies(${STATIC_ANALYSIS_TARGET} ${PER_FILE_TARGET})

  set_property(TARGET
    ${STATIC_ANALYSIS_SOURCE_FILES}
    APPEND
    PROPERTY
    SOURCES
    ${SRC_FNAME}
  )
endfunction()


if(NOT CLANG_TIDY_EXE)
  find_program(CLANG_TIDY_FOUND_EXE NAMES clang-tidy-16 clang-tidy-15 clang-tidy-14 clang-tidy)
  set(CLANG_TIDY_EXE ${CLANG_TIDY_FOUND_EXE})
endif()

set(STATIC_ANALYSIS_PARALLEL_TARGET "${STATIC_ANALYSIS_TARGET}-parallel")
set(STATIC_ANALYSIS_IMPLS_TARGET "${STATIC_ANALYSIS_TARGET}-impls")
set(STATIC_ANALYSIS_TESTS_TARGET "${STATIC_ANALYSIS_TARGET}-tests")
set(STATIC_ANALYSIS_IMPLS_PARALLEL_TARGET "${STATIC_ANALYSIS_IMPLS_TARGET}-parallel")
set(STATIC_ANALYSIS_IMPLS_SOURCE_LIST "${STATIC_ANALYSIS_IMPLS_TARGET}-source-list")
set(STATIC_ANALYSIS_TESTS_PARALLEL_TARGET "${STATIC_ANALYSIS_TESTS_TARGET}-parallel")
set(STATIC_ANALYSIS_TESTS_SOURCE_LIST "${STATIC_ANALYSIS_TESTS_TARGET}-source-list")

add_custom_target(${STATIC_ANALYSIS_IMPLS_PARALLEL_TARGET})
add_custom_target(${STATIC_ANALYSIS_TESTS_PARALLEL_TARGET})

add_custom_target(${STATIC_ANALYSIS_IMPLS_SOURCE_LIST})
add_custom_target(${STATIC_ANALYSIS_TESTS_SOURCE_LIST})

function(add_static_analysis_impl_file SRC_FNAME)
  add_static_analysis_file(
    ${CLANG_TIDY_EXE}
    ${STATIC_ANALYSIS_IMPLS_PARALLEL_TARGET}
    ${STATIC_ANALYSIS_IMPLS_SOURCE_LIST}
    ${SRC_FNAME}
  )
endfunction()

function(add_static_analysis_test_file SRC_FNAME)
  add_static_analysis_file(
    ${CLANG_TIDY_EXE}
    ${STATIC_ANALYSIS_TESTS_PARALLEL_TARGET}
    ${STATIC_ANALYSIS_TESTS_SOURCE_LIST}
    ${SRC_FNAME}
  )
endfunction()