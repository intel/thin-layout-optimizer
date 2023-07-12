include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)

cmake_path(GET CMAKE_CURRENT_LIST_FILE PARENT_PATH COMPILE_OPT_MODULE_INCLUDE_PATH)

include(${COMPILE_OPT_MODULE_INCLUDE_PATH}/warn-everything.cmake)
set(CMAKE_CXX_FLAGS "")
function(filter_flags_impl LANG FLAGS OUTVAR_FLAGS)
  set(TMP_LIST)
  #  set(SAVE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
  #  set(SAVE_C_FLAGS ${CMAKE_C_FLAGS})

  foreach(FLAG IN LISTS ${FLAGS})
    set(FLAG_SUPPORTED_RESULT "")
    string(REPLACE "=" "_" FLAG_FLAG ${FLAG})
    string(REPLACE "-" "_" FLAG_FLAG ${FLAG_FLAG})
    if (${LANG} STREQUAL "CXX")
      set(FLAG_SUPPORTED_RESULT "${CMAKE_CXX_COMPILER_ID}_CXX_HAS_FLAG_${FLAG_FLAG}")
      check_cxx_compiler_flag(${FLAG} ${FLAG_SUPPORTED_RESULT})
    elseif (${LANG} STREQUAL "C")
      set(FLAG_SUPPORTED_RESULT "${CMAKE_C_COMPILER_ID}_C_HAS_FLAG_${FLAG_FLAG}")
      check_c_compiler_flag(${FLAG} ${FLAG_SUPPORTED_RESULT})
    else()
      message(FATAL_ERROR "Unknown language: ${LANG}")
    endif()
    if(${FLAG_SUPPORTED_RESULT} EQUAL 1)
      list(APPEND TMP_LIST ${FLAG})
    endif()
  endforeach()
  set(${OUTVAR_FLAGS} ${TMP_LIST} PARENT_SCOPE)
endfunction()

macro(filter_flags LANG FLAGS OUTVAR_FLAGS)
  set(SAVE_CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
  set(SAVE_CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
  set(CMAKE_CXX_FLAGS "")
  set(CMAKE_C_FLAGS "")
  filter_flags_impl(${LANG} ${FLAGS} ${OUTVAR_FLAGS})
  set(CMAKE_CXX_FLAGS ${SAVE_CMAKE_CXX_FLAGS})
  set(CMAKE_C_FLAGS ${SAVE_CMAKE_C_FLAGS})
endmacro()

function(invert_warning FLAG OUTVAR_FLAG)
  string(REGEX MATCH "^-Wno-.*" IS_WNO ${FLAG})
  string(REGEX MATCH "^-W.*" IS_W ${FLAG})
  if(IS_WNO)
    string(REPLACE "^-Wno-" "-W" FLAG ${FLAG})
  elseif(IS_W)
    string(REGEX REPLACE "^-W" "-Wno-" FLAG ${FLAG})
    string(REGEX MATCH "=.*" IS_EQ ${FLAG})
    if(IS_EQ)
      string(REGEX REPLACE "=.*" "" FLAG ${FLAG})
    endif()
  else()
    message(FATAL_ERROR "Unkown warning format: ${FLAG}")
  endif()
  set(${OUTVAR_FLAG} ${FLAG} PARENT_SCOPE)
endfunction()

function(invert_warning_flags FLAGS)
  set(TMP_LIST)
  foreach(FLAG IN LISTS ${FLAGS})
    invert_warning(${FLAG} FLAG)
    list(APPEND TMP_LIST ${FLAG})
  endforeach()
  set(${FLAGS} ${TMP_LIST} PARENT_SCOPE)
endfunction()

macro(stringify LI)
  string(REPLACE ";" " " ${LI} "${${LI}}")
endmacro()


set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(F_STRICT_FLEX_ARRAYS -fstrict-flex-arrays=2)
set(POSSIBLE_OPT_FLAGS
  -O3
  -D_FORTIFY_SOURCE=2
  -fstack-protector
  -fno-plt
  -fno-exceptions
  -fno-rtti
  -march=native
  ${F_STRICT_FLEX_ARRAYS}
  -fstrict-enums
)

set(WARNINGS_SKIP
  #  -Wsuggest-attribute=noreturn
  #    -Wsuggest-attribute=pure
)
set(WARNINGS_OFF
  -Wno-unused-function
  -Wno-suggest-attribute=pure
  -Wno-float-equal
)

warn_everything_filtered(WARNINGS_SKIP CXX_WARNING_FLAGS C_WARNING_FLAGS)

filter_flags("CXX" WARNINGS_OFF CXX_WARNINGS_OFF)
filter_flags("C" WARNINGS_OFF C_WARNINGS_OFF)
stringify(CXX_WARNINGS_OFF)
stringify(C_WARNINGS_OFF)
set(CXX_WARNING_FLAGS "${CXX_WARNING_FLAGS} ${CXX_WARNINGS_OFF}")
set(C_WARNING_FLAGS "${C_WARNING_FLAGS} ${C_WARNINGS_OFF}")



filter_flags("CXX" POSSIBLE_OPT_FLAGS CXX_OPT_FLAGS)
filter_flags("C" POSSIBLE_OPT_FLAGS C_OPT_FLAGS)
stringify(CXX_OPT_FLAGS)
stringify(C_OPT_FLAGS)



set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_OPT_FLAGS}  ${CXX_WARNING_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_OPT_FLAGS} ${C_WARNING_FLAGS}")

string(REGEX MATCH "(--param=ssp-buffer-size=[0-9]+)" TLO_HAS_SSP_BUFFER_SIZE_CXX "${CMAKE_CXX_FLAGS}")
string(REGEX MATCH "(--param=ssp-buffer-size=[0-9]+)" TLO_HAS_SSP_BUFFER_SIZE_C "${CMAKE_C_FLAGS}")
if(TLO_HAS_SSP_BUFFER_SIZE_CXX)
  string(REPLACE "--param=ssp-buffer-size" "-DTLO_SSP_BUFFER_SIZE" TLO_HAS_SSP_BUFFER_SIZE_CXX ${TLO_HAS_SSP_BUFFER_SIZE_CXX})
else()
  set(TLO_HAS_SSP_BUFFER_SIZE_CXX "")
endif()

if(TLO_HAS_SSP_BUFFER_SIZE_C)
  string(REPLACE "--param=ssp-buffer-size" "-DTLO_SSP_BUFFER_SIZE" TLO_HAS_SSP_BUFFER_SIZE_C ${TLO_HAS_SSP_BUFFER_SIZE_C})
else()
  set(TLO_HAS_SSP_BUFFER_SIZE_C "")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TLO_HAS_SSP_BUFFER_SIZE_CXX}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TLO_HAS_SSP_BUFFER_SIZE_C}")
