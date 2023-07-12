# add_subdirectory(third-party) maybe gtest by source later

set(POSSIBLE_TEST_DISABLED_WARNINGS
    # -Wno-abi-tag
    -Wno-global-constructors
    -Wno-undef
    -Wno-weak-vtables
    -Wno-deprecated
    # -Wno-suggest-attribute=noreturn
)

find_file(TLO_PERF_TEST_INPUTS_SCRIPT make-test-inputs.py
          PATHS ${CMAKE_SOURCE_DIR}/tests/src/perf
)

find_file(TLO_FILE_TEST_INPUTS_SCRIPT gen-compression-tests.py
          PATHS ${CMAKE_SOURCE_DIR}/tests/src/util
)

if(NOT TLO_PERF_TEST_INPUTS_SCRIPT OR NOT TLO_FILE_TEST_INPUTS_SCRIPT)
  message(FATAL_ERROR "Unable to generate test inputs")
endif()

add_custom_command(
  OUTPUT ${TLO_PERF_TEST_INPUTS_SCRIPT}.done
  COMMAND python3 ${TLO_PERF_TEST_INPUTS_SCRIPT}
          ${CMAKE_SOURCE_DIR}/tests/src/perf
)

add_custom_command(
  OUTPUT ${TLO_FILE_TEST_INPUTS_SCRIPT}.done
  COMMAND python3 ${TLO_FILE_TEST_INPUTS_SCRIPT}
)

add_custom_target(
  tlo_test_generated_files_ DEPENDS ${TLO_PERF_TEST_INPUTS_SCRIPT}.done
                                    ${TLO_FILE_TEST_INPUTS_SCRIPT}.done
)

filter_flags("CXX" POSSIBLE_TEST_DISABLED_WARNINGS CXX_TEST_DISABLED_WARNINGS)
# stringify()

function(test_get_build_target TEST_SRC BUILD_TARGET)
  cmake_path(
    GET
    TEST_SRC
    FILENAME
    TMP_BUILD_TARGET
  )
  cmake_path(REMOVE_EXTENSION TMP_BUILD_TARGET)
  set(${BUILD_TARGET}
      ${TMP_BUILD_TARGET}
      PARENT_SCOPE
  )
endfunction()

function(
  add_test_impl
  SUITE_TARGET
  TEST_TARGET
  BUILD_TARGET
)

  foreach(OPTS_AND_POSTFIX IN LISTS SANITIZE_OPTS_AND_POSTFIXES)
    set(USE_GTEST_STATIC_LIB ${GTEST_STATIC_LIB})
    set(USE_GTEST_STATIC_MAIN_LIB ${GTEST_STATIC_MAIN_LIB})
    if(OPTS_AND_POSTFIX)
      string(
        REPLACE "|"
                ";"
                OPTS_AND_POSTFIX
                ${OPTS_AND_POSTFIX}
      )
      list(
        GET
        OPTS_AND_POSTFIX
        0
        OPTS
      )
      list(
        GET
        OPTS_AND_POSTFIX
        1
        POSTFIX
      )

      if("${POSTFIX}" STREQUAL "-msan")
        if(NOT FOUND_GTEST_MSAN)
          continue()
        endif()
        continue()
      endif()
    else()
      set(OPTS "")
      set(POSTFIX "")
    endif()

    set(BUILD_TARGET_P "${BUILD_TARGET}${POSTFIX}")
    set(HFSORT_LIB_P "${HFSORT_LIB}${POSTFIX}")
    set(TEST_TARGET_P "${TEST_TARGET}${POSTFIX}")
    set(SUITE_TARGET_P "${SUITE_TARGET}${POSTFIX}")

    add_executable(${BUILD_TARGET_P} EXCLUDE_FROM_ALL ${ARGN})
    foreach(CXX_DISABLED_WARNING IN LISTS CXX_TEST_DISABLED_WARNINGS)
      target_compile_options(${BUILD_TARGET_P} PRIVATE ${CXX_DISABLED_WARNING})
    endforeach()
    if(OPTS)
      string(
        REPLACE "-"
                "_"
                DEFINE_ID
                ${POSTFIX}
      )
      string(TOUPPER ${DEFINE_ID} DEFINE_ID)
      set(DEFINE_ID "-DTLO${DEFINE_ID}")
      target_compile_options(
        ${BUILD_TARGET_P} PRIVATE "-fno-omit-frame-pointer" "${DEFINE_ID}"
                                  "-DTLO_SANITIZED"
      )
      string(
        REPLACE " "
                ";"
                OPTS
                ${OPTS}
      )
      foreach(OPT IN LISTS OPTS)
        target_compile_options(${BUILD_TARGET_P} PRIVATE ${OPT})
        target_link_options(${BUILD_TARGET_P} PRIVATE ${OPT})
      endforeach()
      if("${POSTFIX}" STREQUAL "-msan")
        # set(USE_GTEST_STATIC_LIB ${GTEST_MSAN_LIB})
        # set(USE_GTEST_STATIC_MAIN_LIB ${GTEST_MSAN_MAIN_LIB})
        # target_link_options(${BUILD_TARGET_P} PRIVATE "-stdlib=libc++")
        # target_link_options(${BUILD_TARGET_P} PRIVATE "-lstdc++")
        # target_link_options(${BUILD_TARGET_P} PRIVATE "-lc++abi") Need to also
        # include / link the proper stdlib++ directory.
        # target_link_directories(${BUILD_TARGET_P} PRIVATE
        # ${LLVM_LIBCXX_MSAN_DIR})

      endif()
    endif()
    if(FOUND_ZSTD)
      target_compile_options(${BUILD_TARGET_P} PRIVATE "-DTLO_ZSTD")
    endif()

    get_external_libs("${POSTFIX}" EXTERNAL_LIBS)
    target_compile_options(${BUILD_TARGET_P} PRIVATE "-DTLO_DEBUG_ENABLED_GLBL")
    target_link_libraries(
      ${BUILD_TARGET_P}
      ${HFSORT_LIB_P}
      ${EXTERNAL_LIBS}
      ${USE_GTEST_STATIC_LIB}
      ${USE_GTEST_STATIC_MAIN_LIB}
    )
    add_custom_target(
      ${TEST_TARGET_P}
      DEPENDS ${BUILD_TARGET_P} tlo_test_generated_files_
      COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${BUILD_TARGET_P}
    )
    add_dependencies(${SUITE_TARGET_P} ${TEST_TARGET_P})
  endforeach()
  add_static_analysis_test_file(${CMAKE_CURRENT_SOURCE_DIR}/${ARGN})
endfunction()

function(add_tests_too_target SUITE_TARGET TEST_TARGET)
  foreach(TEST_SRC IN LISTS ARGN)
    test_get_build_target(${TEST_SRC} BUILD_TARGET)
    add_test_impl(
      ${SUITE_TARGET}
      ${TEST_TARGET}
      ${BUILD_TARGET}
      ${TEST_SRC}
    )
  endforeach()
endfunction()

function(add_tests_as_files SUITE_TARGET)
  foreach(TEST_SRC IN LISTS ARGN)
    test_get_build_target(${TEST_SRC} BUILD_TARGET)
    string(
      REGEX
      REPLACE "^test[-_]"
              ""
              TEST_TARGET
              ${BUILD_TARGET}
    )
    set(TEST_TARGET "check-${TEST_TARGET}")
    add_test_impl(
      ${SUITE_TARGET}
      ${TEST_TARGET}
      ${BUILD_TARGET}
      ${TEST_SRC}
    )
  endforeach()
endfunction()

macro(add_tests_as_files_cur)
  set_current_test_suite()
  add_tests_as_files(${CURRENT_TEST_SUITE} ${ARGN})
endmacro()

macro(add_tests_too_target_cur TEST_TARGET BUILD_TARGET)
  set_current_test_suite()
  add_tests_too_target(
    ${CURRENT_TEST_SUITE}
    ${TEST_TARGET}
    ${BUILD_TARGET}
    ${ARGN}
  )
endmacro()

macro(add_test_cur TEST_TARGET BUILD_TARGET)
  set_current_test_suite()
  add_test_impl(
    ${CURRENT_TEST_SUITE}
    ${TEST_TARGET}
    ${BUILD_TARGET}
    ${ARGN}
  )
endmacro()

function(test_dir_to_target DIR DIR_TARGET)
  string(
    REGEX
    REPLACE "[_\.]"
            "-"
            DIR
            ${DIR}
  )
  set(${DIR_TARGET}
      "check-${DIR}"
      PARENT_SCOPE
  )
endfunction()

macro(set_current_test_suite)
  if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR}/tests)
    set(CURRENT_TEST_SUITE "check-all")
  else()
    cmake_path(
      GET
      CMAKE_CURRENT_SOURCE_DIR
      FILENAME
      CURRENT_TEST_SUITE
    )
    test_dir_to_target(${CURRENT_TEST_SUITE} CURRENT_TEST_SUITE)
  endif()
endmacro()

macro(add_test_directory DIR)
  add_subdirectory(${DIR})
  set_current_test_suite()
  test_dir_to_target(${DIR} DIR_SUITE)
  foreach(POSTFIX IN LISTS SANITIZE_POSTFIXES)
    add_dependencies(${CURRENT_TEST_SUITE}${POSTFIX} ${DIR_SUITE}${POSTFIX})
  endforeach()
endmacro()

macro(init_test_directory)
  include_directories(${GTEST_INCLUDE_DIR})
  set_current_test_suite()
  foreach(POSTFIX IN LISTS SANITIZE_POSTFIXES)
    add_custom_target(${CURRENT_TEST_SUITE}${POSTFIX})
  endforeach()
endmacro()

set(SANITIZED_TEST_TARGET "check-all-${SANITIZED_BUILD}")
add_custom_target(${SANITIZED_TEST_TARGET})
foreach(POSTFIX IN LISTS SANITIZE_POSTFIXES)
  if(POSTFIX)
    add_dependencies(${SANITIZED_TEST_TARGET} "check-all${POSTFIX}")
  endif()
endforeach()

add_custom_target(check-all-all DEPENDS check-all ${SANITIZED_TEST_TARGET})

init_test_directory()
add_test_directory(src)
