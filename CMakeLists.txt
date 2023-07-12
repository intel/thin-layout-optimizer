cmake_minimum_required(VERSION 3.1.3)
project(HFSort C CXX)

SET(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  message(FATAL_ERROR "Intel compiler not supported!")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  message(FATAL_ERROR "MSVC compiler not supported!")
endif()

# Export for clang tooling
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

option(ZSTD_PATH
  "If set use this path for zstd, otherwise use what the system offers" "")

option(ZSTD_MSAN_PATH
  "If set use this path for zstd COMPILER WITH MSAN. Otherwise zstd related features are disabled for msan build" "")

option(GTEST_PATH
  "If set use this path for gtest, otherwise use what the system offers" "")

#option(GTEST_MSAN_PATH
#  "If set use this path for gtest COMPILER WITH MSAN. Otherwise gtest related features are disabled for msan build" "")


include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)

include(cmake/compile-options.cmake)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTLO_PROJECT_DIR='\"${CMAKE_SOURCE_DIR}\"'")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DTLO_PROJECT_DIR='\"${CMAKE_SOURCE_DIR}\"'")

#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
#set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

include (cmake/external-libs.cmake)

if(ZSTD_PATH)
  unset(ZSTD_INCLUDE_DIR CACHE)
  unset(ZSTD_STATIC_LIB CACHE)
  unset(ZSTD_SHARED_LIB CACHE)
  find_package_vars(zstd "${ZSTD_PATH}" ZSTD_INCLUDE_DIR ZSTD_STATIC_LIB ZSTD_SHARED_LIB)
  #  unset(ZSTD_PATH CACHE)
else()
  find_path(ZSTD_INCLUDE_DIR
    NAMES zstd.h
  )
  find_library(ZSTD_STATIC_LIB
    NAMES libzstd.a
  )
  find_library(ZSTD_SHARED_LIB
    NAMES libzstd.so
  )
endif()

if(ZSTD_INCLUDE_DIR AND ZSTD_STATIC_LIB AND ZSTD_SHARED_LIB)
  #  message("INCLUDE: ${ZSTD_INCLUDE_DIR}")
  #  message("LSTATIC: ${ZSTD_STATIC_LIB}")
  #  message("LSHARED: ${ZSTD_SHARED_LIB}")
  set(FOUND_ZSTD ON)
else()
  message(WARNING "ZSTD not found. Reading compressed profiles won't be enabled")
endif()

if(FOUND_ZSTD)
  if(ZSTD_MSAN_PATH)
    unset(ZSTD_MSAN_INCLUDE_DIR CACHE)
    unset(ZSTD_MSAN_STATIC_LIB CACHE)
    unset(ZSTD_MSAN_SHARED_LIB CACHE)

    find_package_vars(zstd "${ZSTD_MSAN_PATH}" ZSTD_MSAN_INCLUDE_DIR ZSTD_MSAN_STATIC_LIB ZSTD_MSAN_SHARED_LIB)

    #  unset(ZSTD_MSAN_PATH CACHE)
  endif()

  if(ZSTD_MSAN_INCLUDE_DIR AND ZSTD_MSAN_STATIC_LIB AND ZSTD_MSAN_SHARED_LIB)
    #  message("MINCLUDE: ${ZSTD_MSAN_INCLUDE_DIR}")
    #  message("MLSTATIC: ${ZSTD_MSAN_STATIC_LIB}")
    #  message("MLSHARED: ${ZSTD_MSAN_SHARED_LIB}")
    set(FOUND_ZSTD_MSAN ON)
  endif()
endif()
if(GTEST_PATH)
  unset(GTEST_INCLUDE_DIR CACHE)
  unset(GTEST_STATIC_LIB CACHE)
  unset(GTEST_STATIC_MAIN_LIB CACHE)
  find_package_vars_gtest(gtest "${GTEST_PATH}" GTEST_INCLUDE_DIR GTEST_STATIC_LIB GTEST_STATIC_MAIN_LIB "a")
  #  unset(GTEST_PATH CACHE)
else()
  find_path(GTEST_INCLUDE_DIR
    NAMES gtest.h
  )
  if(NOT GTEST_INCLUDE_DIR)
    find_path(GTEST_INCLUDE_DIR
      NAMES gtest/gtest.h
    )
  endif()
  find_library(GTEST_STATIC_LIB
    NAMES libgtest.a
  )
  find_library(GTEST_STATIC_MAIN_LIB
    NAMES libgtest_main.a
  )

endif()

if(GTEST_INCLUDE_DIR AND GTEST_STATIC_LIB AND GTEST_STATIC_MAIN_LIB)
  #  message("INCLUDE: ${GTEST_INCLUDE_DIR}")
  #  message("LSTATIC: ${GTEST_STATIC_LIB}")
  #  message("LSTATIC_MAIN: ${GTEST_STATIC_MAIN_LIB}")
  set(FOUND_GTEST ON)
  #  message(FATAL_ERROR "Missing GTEST")
else()
  message(WARNING "GTEST not found. Tests won't be enabled")
endif()

if(GTEST_MSAN_PATH)
  unset(GTEST_MSAN_INCLUDE_DIR CACHE)
  unset(GTEST_MSAN_SHARED_LIB CACHE)
  unset(GTEST_MSAN_SHARED_MAIN_LIB CACHE)
  find_package_vars_gtest(gtest "${GTEST_MSAN_PATH}" GTEST_MSAN_INCLUDE_DIR GTEST_MSAN_SHARED_LIB GTEST_MSAN_MAIN_LIB "a")

  #  unset(GTEST_MSAN_PATH CACHE)
endif()

if(GTEST_MSAN_INCLUDE_DIR AND GTEST_MSAN_LIB AND GTEST_MSAN_MAIN_LIB)
  #  message("MINCLUDE: ${GTEST_MSAN_INCLUDE_DIR}")
  #  message("MLSTATIC: ${GTEST_MSAN_LIB}")
  #  message("MLSTATIC_MAIN: ${GTEST_MSAN_MAIN_LIB}")
  set(FOUND_GTEST_MSAN ON)
endif()



set(EXTERNAL_STATIC_LIBS ${ZSTD_STATIC_LIB})
set(EXTERNAL_SHARED_LIBS ${ZSTD_SHARED_LIB})
set(EXTERNAL_MSAN_STATIC_LIBS ${ZSTD_MSAN_STATIC_LIB})
set(EXTERNAL_MSAN_SHARED_LIBS ${ZSTD_MSAN_SHARED_LIB})
if(NOT EXTERNAL_MSAN_STATIC_LIBS)
  set(EXTERNAL_MSAN_STATIC_LIBS ${ZSTD_STATIC_LIB})
  set(EXTERNAL_MSAN_SHARED_LIBS ${ZSTD_SHARED_LIB})
endif()
function(get_external_libs POSTFIX EXTERNAL_LIBS_OUT)
  get_target_property(UTIL_LIB_TYPE ${HFSORT_LIB}${POSTFIX} TYPE)
  if (UTIL_LIB_TYPE STREQUAL STATIC_LIBRARY)
    if (POSTFIX STREQUAL "-msan")
      set(EXTERNAL_LIBS ${EXTERNAL_MSAN_STATIC_LIBS})
    else()
      set(EXTERNAL_LIBS ${EXTERNAL_STATIC_LIBS})
    endif()
  else()
    if (POSTFIX STREQUAL "-msan")
      set(EXTERNAL_LIBS ${EXTERNAL_MSAN_SHARED_LIBS})
    else()
      set(EXTERNAL_LIBS ${EXTERNAL_SHARED_LIBS})
    endif()
  endif()
  set(${EXTERNAL_LIBS_OUT} ${EXTERNAL_LIBS} PARENT_SCOPE)
endfunction()

set(HFSORT_LIB thin-layout-optimizer-util)
set(HFSORT_EXE thin-layout-optimizer)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${ZSTD_INCLUDE_DIR})

set(STATIC_ANALYSIS_TARGET static-analysis)

include(cmake/static-analysis-0.cmake)


set(SANITIZE_OPTS_AND_POSTFIXES ";-fsanitize=leak|-lsan;-fsanitize=address|-asan;-fsanitize=undefined|-usan") #
set(SANITIZE_POSTFIXES ";-lsan;-asan;-usan") # no -msan, don't have instrumented libs
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # ;-fsanitize=memory|-msan
  set(SANITIZE_OPTS_AND_POSTFIXES "${SANITIZE_OPTS_AND_POSTFIXES};-fsanitize=memory|-msan")
  set(SANITIZE_POSTFIXES "${SANITIZE_POSTFIXES};-msan")
endif()

set(SANITIZED_BUILD "san")

add_subdirectory(src)
if(FOUND_GTEST)
  add_subdirectory(tests)
endif()

include(cmake/static-analysis-1.cmake)
