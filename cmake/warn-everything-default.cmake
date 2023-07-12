#
# Default -Weverything. Mostly incomplete, just sets a few basic
# flags.  Only used if we couldn't find actual warning list for
# compiler being used.
#
# Modify 'DEFAULT_GENERIC_WARNINGS' to update what will be set.
#

function(warn_everything_get_default_warnings LANG DST_LIST)
  include(CheckCCompilerFlag)
  include(CheckCXXCompilerFlag)

  # Only used if we failed to detect a flags for a know compiler/version.
  # Add/Delete from this list as seen fit.
  set(DEFAULT_GENERIC_WARNINGS
    -Wall
    -Wextra
    -Wnull-dereference
    -Wpedantic
    -Wshadow
    -Wuninitialized
    -Wunreachable-code
    -Wunused
    -Wunused-const-variable
    -Wunused-parameter
    -Wunused-result
  )

  message(STATUS "No ${LANG} warnings found for ${CMAKE_${LANG}_COMPILER}. Using default list.")
  set(TMP_LIST)
  foreach(WARNING IN LISTS DEFAULT_GENERIC_WARNINGS)
    if (${LANG} STREQUAL "CXX")
      set(WARNING_SUPPORTED_RESULT "${CMAKE_CXX_COMPILER_ID}_CXX_HAS_WARNING_${WARNING}")
      check_cxx_compiler_flag(${WARNING} ${WARNING_SUPPORTED_RESULT})
    elseif (${LANG} STREQUAL "C")
      set(WARNING_SUPPORTED_RESULT "${CMAKE_C_COMPILER_ID}_C_HAS_WARNING_${WARNING}")
      check_c_compiler_flag(${WARNING} ${WARNING_SUPPORTED_RESULT})
    else()
      message(FATAL_ERROR "Unknown language: ${LANG}")
    endif()
    if(${WARNING_SUPPORTED_RESULT})
      list(APPEND TMP_LIST ${WARNING})
    endif()
  endforeach()
  list(LENGTH TMP_LIST TMP_LENGTH)
  set(${DST_LIST} "${TMP_LIST}" PARENT_SCOPE)
endfunction()


function(warn_everything_fallback_if_needed OUTVAR_CXX OUTVAR_C)
  macro(warn_everything_check_and_get_default LANG)
    list(LENGTH ${OUTVAR_${LANG}} LIST_LENGTH)
    if(${LIST_LENGTH} LESS_EQUAL 0)
      set(TMP_OUTVAR)
      warn_everything_get_default_warnings(${LANG} TMP_OUTVAR)
      set(${OUTVAR_${LANG}} ${TMP_OUTVAR} PARENT_SCOPE)
    endif()
  endmacro()

  warn_everything_check_and_get_default("CXX")
  warn_everything_check_and_get_default("C")
endfunction()
