#
# Essentially gets -Weverything flag for either GCC/Clang.
#
# C/CXX Only.
#
# API:
# warn_everything(<outvar_cxx_flags> <outvar_c_flags>)
#
# Warnings are disabled by uncomming values in the GCC/Clang
# cmake file respectively (or default).
#
# All flags are versioned so enabling/disabling a flag should be
# portable across any version of either GCC/Clang.
#
# If no flags are found (very old GCC/Clang version or other
# compiler), fallsback on default list.
#   see: warn-everything-default.cmake
#
# For Clang, since -Weverything is supported, the code contains
# All the supported '-Wno-*' flags by version and sets
# -Weverything (as well as a few other flags). To disable a
# warning just uncomment the respective '-Wno-*' flag.
#
# For GCC, all supported warnings are incrementally added by
# version. For warnings that have multiple settings, the most strict
# version is default set i.e:
#   -Wstrict-aliasing=n default is 1
#   -Wimplicit-fallthrough=n default is 5
# To disable warnings uncomment the warning name in the '*_OFF'
# variables.
#
# Some warnings are only meaningful (or possible) we a specific user
# supplied value. For example '-Wframe-larger-than='. In this case, to
# provide a value (and enable that warning), the value is
# 'W_<uppercase-warning-name>_VAL'. So for '-Wframe-larger-than=', to
# enable it with a value set 'W_FRAME_LARGER_THAN_VAL'.
#
# Finally, some warnings are only meaningful in conjunction with other
# options. For example '-Wstrict-flex-arrays' is only meaningful if
# '-fstrict-array-level' is set. In this case, to specify that the
# paired flag is being passed, set the variable
# 'F_<uppercase-flag-name>'. So to enable '-Wstrict-flex-array' by
# specify that '-fstrict-array-level' is set, set the variable
# 'F_STRICT_ARRAY_LEVEL'.
#
# In both of the above cases, you can also set the warning manually by
# set the variable 'WARN_<uppercase-flag-name>'. So
# 'set(WARN_FRAME_LARGER_THAN -Wframe-larger-than=<your value>' and
# 'set(WARN_STRICT_FLEX_ARRAYS -Wstrict-flex-arrays)'.
#
# The warnings with depending values are:
#   -Wlarger-than=
#       GCC
#   -Wstack-usage=
#       GCC
#   -Wframe-larger-than=
#       GCC/Clang
#   -Walloc-size-larger-than=
#       GCC
#   -Walloca-size-larger-than=
#       GCC
#   -Wvla-larger-than=
#       GCC
#   -Wstrict-flex-arrays
#       GCC
#       Enabled by 'F_STRICT_ARRAY_LEVEL'
#   -Wabi=
#       GCC
#       -Wabi= is a special case, you can also specify
#       'F_ABI_COMPAT_VERSION' or 'F_ABI_VERSION'
#       to set it.
#   -Wabi
#       GCC/Clang
#       -Wabi is a special case, it will default set
#       for GCC < 8.2.0. After 8.2.0 GCC warns if its
#       set and we don't have either '-Wabi', '-fabi-compat-version'
#       or '-fabi-version'. So after 8.2.0 one of those must be set
#       (see above) for it be enabled.
#

cmake_path(GET CMAKE_CURRENT_LIST_FILE PARENT_PATH WARN_EVERYTHING_MODULE_INCLUDE_PATH)
function(warn_everything_list WARNINGS_OUT_CXX WARNINGS_OUT_C)
  function(warn_everything_filter_warnings WARNINGS SKIP_WARNINGS DST)
    set(TMP)
    foreach(WARNING IN LISTS ${WARNINGS})

      list(FIND ${SKIP_WARNINGS} ${WARNING} IS_IN)
      if(${IS_IN} LESS 0)
        message(VERBOSE "-- Use Flag : ${WARNING}")
        list(APPEND TMP "${WARNING}")
      else()
        message(VERBOSE "-- Skip Flag: ${WARNING}")
      endif()
    endforeach()
    set(${DST} "${${DST}}" "${TMP}" PARENT_SCOPE)
  endfunction()

  set(WARN_EVERYTHING_CXX ${${WARNINGS_OUT_CXX}})
  set(WARN_EVERYTHING_C ${${WARNINGS_OUT_C}})

  include(${WARN_EVERYTHING_MODULE_INCLUDE_PATH}/warn-everything-gcc.cmake)
  include(${WARN_EVERYTHING_MODULE_INCLUDE_PATH}/warn-everything-clang.cmake)
  include(${WARN_EVERYTHING_MODULE_INCLUDE_PATH}/warn-everything-default.cmake)

  warn_everything_gcc(WARN_EVERYTHING_BASE_CXX WARN_EVERYTHING_BASE_C)
  warn_everything_clang(WARN_EVERYTHING_BASE_CXX WARN_EVERYTHING_BASE_C)

  # Only does anything if we didn't get warnings for GCC/Clang
  warn_everything_fallback_if_needed(WARN_EVERYTHING_BASE_CXX WARN_EVERYTHING_BASE_C)

  warn_everything_filter_warnings(WARN_EVERYTHING_BASE_CXX FILTER WARN_EVERYTHING_CXX)
  warn_everything_filter_warnings(WARN_EVERYTHING_BASE_C FILTER WARN_EVERYTHING_C)

  list(LENGTH WARN_EVERYTHING_CXX WARN_EVERYTHING_CXX_LENGTH)
  list(LENGTH WARN_EVERYTHING_C WARN_EVERYTHING_C_LENGTH)

  message(STATUS "Found ${WARN_EVERYTHING_CXX_LENGTH} CXX warnings.")
  message(STATUS "Found ${WARN_EVERYTHING_C_LENGTH} C warnings.")

  set(${WARNINGS_OUT_CXX} ${WARN_EVERYTHING_CXX} PARENT_SCOPE)
  set(${WARNINGS_OUT_C} ${WARN_EVERYTHING_C} PARENT_SCOPE)
endfunction()

function(warn_everything WARNINGS_OUT_CXX WARNINGS_OUT_C)
  warn_everything_list(TMP_WARNINGS_OUT_CXX TMP_WARNINGS_OUT_C)

  string(REPLACE ";" " " TMP_WARNINGS_OUT_CXX "${TMP_WARNINGS_OUT_CXX}")
  string(REPLACE ";" " " TMP_WARNINGS_OUT_C "${TMP_WARNINGS_OUT_C}")

  set(${WARNINGS_OUT_CXX} ${TMP_WARNINGS_OUT_CXX} PARENT_SCOPE)
  set(${WARNINGS_OUT_C} ${TMP_WARNINGS_OUT_C} PARENT_SCOPE)
endfunction()

function(warn_everything_filtered REGEX_FILTERS WARNINGS_OUT_CXX WARNINGS_OUT_C)
  warn_everything_list(TMP_WARNINGS_OUT_CXX TMP_WARNINGS_OUT_C)
  foreach(ITEM IN LISTS ${REGEX_FILTERS})
    list(FILTER TMP_WARNINGS_OUT_CXX EXCLUDE REGEX ${ITEM})
    list(FILTER TMP_WARNINGS_OUT_C EXCLUDE REGEX ${ITEM})
  endforeach()

  string(REPLACE ";" " " TMP_WARNINGS_OUT_CXX "${TMP_WARNINGS_OUT_CXX}")
  string(REPLACE ";" " " TMP_WARNINGS_OUT_C "${TMP_WARNINGS_OUT_C}")
  
  set(${WARNINGS_OUT_CXX} ${TMP_WARNINGS_OUT_CXX} PARENT_SCOPE)
  set(${WARNINGS_OUT_C} ${TMP_WARNINGS_OUT_C} PARENT_SCOPE)
endfunction()
