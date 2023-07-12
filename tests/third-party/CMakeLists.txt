########################### GTEST
# Enable ExternalProject CMake module
INCLUDE(ExternalProject)

# Set default ExternalProject root directory
SET_DIRECTORY_PROPERTIES(PROPERTIES EP_PREFIX ${CMAKE_BINARY_DIR}/test/third_party/gtest)

# Add gtest
# http://stackoverflow.com/questions/9689183/cmake-googletest
ExternalProject_Add(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.tar.gz
  # TIMEOUT 10
  # # Force separate output paths for debug and release builds to allow easy
  # # identification of correct lib in subsequent TARGET_LINK_LIBRARIES commands
  # CMAKE_ARGS -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG:PATH=DebugLibs
  #            -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE:PATH=ReleaseLibs
  #            -Dgtest_force_shared_crt=ON
  # Disable install step
  INSTALL_COMMAND ""
  DOWNLOAD_EXTRACT_TIMESTAMP ON
  # Wrap download, configure and build steps in a script to log output
  LOG_DOWNLOAD ON
  LOG_CONFIGURE ON
  LOG_BUILD ON)

# Specify include dir
ExternalProject_Get_Property(googletest source_dir)
set(GTEST_INCLUDE_DIR ${source_dir}/include)

# Library
ExternalProject_Get_Property(googletest binary_dir)
set(GTEST_STATIC_LIBRARY_PATH ${binary_dir}/${CMAKE_FIND_LIBRARY_PREFIXES}gtest.a)
set(GTEST_STATIC_LIBRARY gtest)
add_library(${GTEST_STATIC_LIBRARY} UNKNOWN IMPORTED)
set_property(TARGET ${GTEST_STATIC_LIBRARY} PROPERTY IMPORTED_LOCATION
  ${GTEST_STATIC_LIBRARY_PATH} )
add_dependencies(${GTEST_STATIC_LIBRARY} googletest EXCLUDE_FROM_ALL)
