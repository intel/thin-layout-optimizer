#
# Warn everything for GCC between 4.4.7 - trunk.14.0.0
# Create: 2023-05-23
#
# Default is all known warnings are on.
# To disable a warning look for the warning;
# '-W<name> in one of the '*_ACTIVATED' variables and
# comment it out.
#
# All warnings should be enablable/disablable without
# touch any variables other than the '*_ACTIVATED' ones
#

function(warn_everything_gcc OUTVAR_CXX OUTVAR_C)
  # Enable warnings that require user specified value.
  if (NOT DEFINED WARN_LARGER_THAN)
    if (DEFINED W_LARGER_THAN_VAL)
      set(WARN_LARGER_THAN -Wlarger-than=${W_LARGER_THAN_VAL})
    endif()
  endif()

  if (NOT DEFINED WARN_STACK_USAGE)
    if (DEFINED W_STACK_USAGE_VAL)
      set(WARN_STACK_USAGE -Wstack-usage=${W_STACK_USAGE_VAL})
    endif()
  endif()

  if (NOT DEFINED WARN_FRAME_LARGER_THAN)
    if (DEFINED W_FRAME_LARGER_THAN_VAL)
      set(WARN_FRAME_LARGER_THAN -Wframe-larger-than=${W_FRAME_LARGER_THAN_VAL})
    endif()
  endif()

  if (NOT DEFINED WARN_ALLOC_SIZE_LARGER_THAN)
    if (DEFINED W_ALLOC_SIZE_LARGE_THAN_VAL)
      set(WARN_ALLOC_SIZE_LARGER_THAN -Walloc-size-larger-than=${W_ALLOC_SIZE_LARGE_THAN_VAL})
    endif()
  endif()

  if (NOT DEFINED WARN_ALLOCA_LARGER_THAN)
    if (DEFINED W_ALLOCA_LARGER_THAN_VAL)
      set(WARN_ALLOCA_LARGER_THAN -Walloca-larger-than=${W_ALLOCA_LARGER_THAN_VAL})
    endif()
  endif()

  if (NOT DEFINED WARN_VLA_LARGER_THAN)
    if (DEFINED W_WVLA_LARGER_THAN_VAL)
      set(WARN_VLA_LARGER_THAN -Wvla-larger-than=${W_WVLA_LARGER_THAN_VAL})
    endif()
  endif()

  if (NOT DEFINED WARN_STRICT_FLEX_ARRAYS)
    if (DEFINED F_STRICT_ARRAY_LEVEL)
      set(WARN_STRICT_FLEX_ARRAYS -Wstrict-flex-arrays)
    endif()
  endif()

  if (NOT DEFINED WARN_ABI_EQ)
    if (DEFINED W_ABI_VAL)
      set(WARN_ABI_EQ -Wabi=${W_ABI_VAL})
    elseif (DEFINED F_ABI_COMPAT_VERSION)
      set(WARN_ABI_EQ -Wabi=${F_ABI_COMPAT_VERSION})
    elseif (DEFINED F_ABI_VERSION)
      set(WARN_ABI_EQ -Wabi=${F_ABI_VERSION})
    endif()
  endif()

  if (NOT DEFINED WARN_ABI_HIDE_8_2)
    if (NOT DEFINED WARN_ABI_EQ)
      set(WARN_ABI_HIDE_8_2 -Wabi)
    endif()
  endif()

  # Set active flags.
  set(GCC_4_4_7_GENERIC_WARNING_FLAGS_ACTIVATED
    ${WARN_LARGER_THAN}
    -Wabi
    -Waddress
    # -Waggregate-return
    -Wall
    -Warray-bounds
    -Wattributes
    -Wbuiltin-macro-redefined
    -Wcast-align
    -Wcast-qual
    -Wchar-subscripts
    -Wclobbered
    -Wcomment
    -Wcomments
    -Wconversion
    -Wcoverage-mismatch
    -Wdeprecated
    -Wdeprecated-declarations
    -Wdisabled-optimization
    -Wdiv-by-zero
    -Wempty-body
    -Wendif-labels
    -Wextra
    -Wfloat-equal
    -Wformat
    -Wformat-contains-nul
    -Wformat-extra-args
    -Wformat-nonliteral
    -Wformat-security
    -Wformat-y2k
    -Wformat=2
    -Wignored-qualifiers
    -Winit-self
    # -Winline
    -Winvalid-pch
    -Wlogical-op
    # -Wlong-long
    -Wmain
    -Wmissing-braces
    -Wmissing-declarations
    -Wmissing-field-initializers
    -Wmissing-format-attribute
    -Wmissing-include-dirs
    -Wmissing-noreturn
    -Wmudflap
    -Wmultichar
    # -Wnormalized=id
    -Wnormalized=nfc
    # -Wnormalized=nfkc
    -Woverflow
    -Woverlength-strings
    -Wpacked
    -Wpacked-bitfield-compat
    # -Wpadded
    -Wparentheses
    -Wpointer-arith
    -Wpragmas
    -Wredundant-decls
    -Wreturn-type
    -Wsequence-point
    -Wshadow
    -Wsign-compare
    -Wstack-protector
    # -Wstrict-aliasing
    -Wstrict-aliasing=1
    # -Wstrict-aliasing=2
    # -Wstrict-aliasing=3
    -Wstrict-overflow
    # -Wstrict-overflow=1
    # -Wstrict-overflow=2
    # -Wstrict-overflow=3
    # -Wstrict-overflow=4
    # -Wstrict-overflow=5
    -Wswitch
    -Wswitch-default
    -Wswitch-enum
    -Wsync-nand
    # -Wsystem-headers
    -Wtrigraphs
    -Wtype-limits
    -Wundef
    -Wuninitialized
    -Wunknown-pragmas
    -Wunreachable-code
    -Wunsafe-loop-optimizations
    -Wunused
    -Wunused-function
    -Wunused-label
    -Wunused-macros
    -Wunused-value
    -Wunused-variable
    -Wvariadic-macros
    -Wvla
    -Wvolatile-register-var
    -Wwrite-strings
  )

  set(GCC_4_5_3_GENERIC_WARNING_FLAGS_ACTIVATED
    -Wenum-compare
    -Wunused-result
  )

  set(GCC_4_6_4_GENERIC_WARNING_FLAGS_ACTIVATED
    -Wcpp
    -Wdouble-promotion
    -Wint-to-pointer-cast
    -Wsuggest-attribute=const
    -Wsuggest-attribute=noreturn
    -Wsuggest-attribute=pure
    -Wtrampolines
    -Wunused-but-set-parameter
    -Wunused-but-set-variable
  )

  set(GCC_4_7_1_GENERIC_WARNING_FLAGS_ACTIVATED
    ${WARN_STACK_USAGE}
    -Wformat-zero-length
    -Wfree-nonheap-object
    -Winvalid-memory-model
    -Wmaybe-uninitialized
    -Wnarrowing
    -Wnonnull
    -Wunused-local-typedefs
    -Wvector-operation-performance
  )

  set(GCC_4_7_2_GENERIC_WARNING_FLAGS_ACTIVATED)
  set(GCC_4_7_3_GENERIC_WARNING_FLAGS_ACTIVATED)
  set(GCC_4_7_4_GENERIC_WARNING_FLAGS_ACTIVATED)

  set(GCC_4_8_1_GENERIC_WARNING_FLAGS_ACTIVATED
    -Waggressive-loop-optimizations
    # -Wformat=1
    -Wpedantic
    -Wreturn-local-addr
    -Wsizeof-pointer-memaccess
    -Wsuggest-attribute=format
    -Wvarargs
  )

  set(GCC_4_8_2_GENERIC_WARNING_FLAGS_ACTIVATED)
  set(GCC_4_8_3_GENERIC_WARNING_FLAGS_ACTIVATED)
  set(GCC_4_8_4_GENERIC_WARNING_FLAGS_ACTIVATED)
  set(GCC_4_8_5_GENERIC_WARNING_FLAGS_ACTIVATED)

  set(GCC_4_9_0_GENERIC_WARNING_FLAGS_ACTIVATED
    -Wdate-time
    -Wopenmp-simd
  )

  set(GCC_4_9_1_GENERIC_WARNING_FLAGS_ACTIVATED)
  set(GCC_4_9_2_GENERIC_WARNING_FLAGS_ACTIVATED)
  set(GCC_4_9_3_GENERIC_WARNING_FLAGS_ACTIVATED)
  set(GCC_4_9_4_GENERIC_WARNING_FLAGS_ACTIVATED)

  set(GCC_5_1_0_GENERIC_WARNING_FLAGS_ACTIVATED
    ${WARN_ABI_EQ}
    # -Warray-bounds
    # -Warray-bounds=1
    -Warray-bounds=2
    -Wbool-compare
    -Wchkp
    -Wformat-signedness
    -Wlogical-not-parentheses
    -Wmemset-transposed-args
    # -Wnormalized=none
    -Wodr
    -Wshift-count-negative
    -Wshift-count-overflow
    -Wsizeof-array-argument
    -Wsuggest-final-methods
    -Wsuggest-final-types
    -Wswitch-bool
  )

  set(GCC_5_2_0_GENERIC_WARNING_FLAGS_ACTIVATED)
  set(GCC_5_3_0_GENERIC_WARNING_FLAGS_ACTIVATED)
  set(GCC_5_4_0_GENERIC_WARNING_FLAGS_ACTIVATED)
  set(GCC_5_5_0_GENERIC_WARNING_FLAGS_ACTIVATED)

  set(GCC_6_1_0_GENERIC_WARNING_FLAGS_ACTIVATED
    ${WARN_FRAME_LARGER_THAN}
    -Wduplicated-cond
    -Wfloat-conversion
    -Wframe-address
    -Whsa
    -Wignored-attributes
    -Wlto-type-mismatch
    -Wmisleading-indentation
    -Wnonnull-compare
    # -Wnormalized
    -Wnull-dereference
    -Wscalar-storage-order
    -Wshift-negative-value
    # -Wshift-overflow
    # -Wshift-overflow=1
    -Wshift-overflow=2
    -Wsign-conversion
    -Wtautological-compare
    # -Wunused-const-variable
    -Wunused-const-variable=1
    #-Wunused-const-variable=2
  )

  set(GCC_6_2_0_GENERIC_WARNING_FLAGS_ACTIVATED
    -Wpsabi
  )

  set(GCC_6_3_0_GENERIC_WARNING_FLAGS_ACTIVATED)
  set(GCC_6_4_0_GENERIC_WARNING_FLAGS_ACTIVATED)

  set(GCC_7_1_0_GENERIC_WARNING_FLAGS_ACTIVATED
    ${WARN_ALLOCA_LARGER_THAN}
    ${WARN_ALLOC_SIZE_LARGER_THAN}
    ${WARN_VLA_LARGER_THAN}
    -Walloc-zero
    -Walloca
    -Wbool-operation
    -Wbuiltin-declaration-mismatch
    -Wdangling-else
    -Wduplicated-branches
    -Wexpansion-to-defined
    # -Wformat-overflow
    # -Wformat-overflow=1
    -Wformat-overflow=2
    # -Wformat-truncation
    # -Wformat-truncation=1
    -Wformat-truncation=2
    # -Wimplicit-fallthrough
    # -Wimplicit-fallthrough=1
    # -Wimplicit-fallthrough=2
    # -Wimplicit-fallthrough=3
    # -Wimplicit-fallthrough=4
    -Wimplicit-fallthrough=5
    -Wint-in-bool-context
    -Wmemset-elt-size
    -Wpointer-compare
    -Wrestrict
    -Wshadow-compatible-local
    -Wshadow-local
    # -Wshadow=compatible-local
    # -Wshadow=global
    # -Wshadow=local
    # -Wstringop-overflow
    # -Wstringop-overflow=1
    # -Wstringop-overflow=2
    # -Wstringop-overflow=3
    -Wstringop-overflow=4
    -Wswitch-unreachable
  )

  set(GCC_7_2_0_GENERIC_WARNING_FLAGS_ACTIVATED)
  set(GCC_7_3_0_GENERIC_WARNING_FLAGS_ACTIVATED)
  set(GCC_7_4_0_GENERIC_WARNING_FLAGS_ACTIVATED)
  set(GCC_7_5_0_GENERIC_WARNING_FLAGS_ACTIVATED)

  set(GCC_8_1_0_GENERIC_WARNING_FLAGS_ACTIVATED
    -Wattribute-alias
    # -Wcast-align
    -Wcast-align=strict
    -Wcast-function-type
    -Wif-not-aligned
    -Wmissing-attributes
    -Wmultistatement-macros
    -Wpacked-not-aligned
    -Wsizeof-pointer-div
    -Wstringop-truncation
    -Wsuggest-attribute=cold
    -Wsuggest-attribute=malloc
  )

  set(GCC_8_2_0_GENERIC_WARNING_FLAGS_ACTIVATED)
  set(GCC_8_3_0_GENERIC_WARNING_FLAGS_ACTIVATED)
  set(GCC_8_4_0_GENERIC_WARNING_FLAGS_ACTIVATED)
  set(GCC_8_5_0_GENERIC_WARNING_FLAGS_ACTIVATED)

  set(GCC_9_1_0_GENERIC_WARNING_FLAGS_ACTIVATED
    -Waddress-of-packed-member
    # -Wattribute-alias
    # -Wattribute-alias=1
    -Wattribute-alias=2
    -Wattribute-warning
    -Wcannot-profile
    -Wmissing-profile
    -Wprio-ctor-dtor
  )

  set(GCC_9_2_0_GENERIC_WARNING_FLAGS_ACTIVATED)
  set(GCC_9_3_0_GENERIC_WARNING_FLAGS_ACTIVATED)
  set(GCC_9_4_0_GENERIC_WARNING_FLAGS_ACTIVATED)
  set(GCC_9_5_0_GENERIC_WARNING_FLAGS_ACTIVATED)

  set(GCC_10_1_0_GENERIC_WARNING_FLAGS_ACTIVATED
    -Wanalyzer-double-fclose
    -Wanalyzer-double-free
    -Wanalyzer-exposure-through-output-file
    -Wanalyzer-file-leak
    -Wanalyzer-free-of-non-heap
    -Wanalyzer-malloc-leak
    -Wanalyzer-null-argument
    -Wanalyzer-null-dereference
    -Wanalyzer-possible-null-argument
    -Wanalyzer-possible-null-dereference
    -Wanalyzer-stale-setjmp-buffer
    -Wanalyzer-tainted-array-index
    -Wanalyzer-too-complex
    -Wanalyzer-unsafe-call-within-signal-handler
    -Wanalyzer-use-after-free
    -Wanalyzer-use-of-pointer-in-stale-stack-frame
    -Warith-conversion
    -Wformat-diag
    -Wstring-compare
    -Wswitch-outside-range
    -Wzero-length-bounds
  )

  set(GCC_10_2_0_GENERIC_WARNING_FLAGS_ACTIVATED)
  set(GCC_10_3_0_GENERIC_WARNING_FLAGS_ACTIVATED)
  set(GCC_10_4_0_GENERIC_WARNING_FLAGS_ACTIVATED)

  set(GCC_11_1_0_GENERIC_WARNING_FLAGS_ACTIVATED
    -WNSObject-attribute
    -Wanalyzer-mismatching-deallocation
    -Wanalyzer-shift-count-negative
    -Wanalyzer-shift-count-overflow
    -Wanalyzer-write-to-const
    -Wanalyzer-write-to-string-literal
    # -Warray-parameter
    # -Warray-parameter=1
    -Warray-parameter=2
    -Wenum-conversion
    -Wmismatched-dealloc
    -Wsizeof-array-div
    -Wstringop-overread
    -Wtsan
    -Wvla-parameter
  )

  set(GCC_11_2_0_GENERIC_WARNING_FLAGS_ACTIVATED)
  set(GCC_11_3_0_GENERIC_WARNING_FLAGS_ACTIVATED)

  set(GCC_12_1_0_GENERIC_WARNING_FLAGS_ACTIVATED
    -Wanalyzer-tainted-allocation-size
    -Wanalyzer-tainted-divisor
    -Wanalyzer-tainted-offset
    -Wanalyzer-tainted-size
    -Wanalyzer-use-of-uninitialized-value
    -Warray-compare
    # -Wbidi-chars
    # -Wbidi-chars=any
    -Wbidi-chars=any,ucn
    # -Wbidi-chars=none
    # -Wbidi-chars=ucn
    # -Wbidi-chars=unpaired
    # -Wbidi-chars=unpaired,ucn
    -Wcoverage-invalid-line-number
    # -Wdangling-pointer
    # -Wdangling-pointer=1
    -Wdangling-pointer=2
    -Winfinite-recursion
    -Wopenacc-parallelism
    -Wtrivial-auto-var-init
    # -Wuse-after-free
    # -Wuse-after-free=1
    # -Wuse-after-free=2
    -Wuse-after-free=3
  )

  set(GCC_12_2_0_GENERIC_WARNING_FLAGS_ACTIVATED)
  set(GCC_12_3_0_GENERIC_WARNING_FLAGS_ACTIVATED)

  set(GCC_13_1_0_GENERIC_WARNING_FLAGS_ACTIVATED
    -Wanalyzer-allocation-size
    -Wanalyzer-deref-before-check
    -Wanalyzer-exposure-through-uninit-copy
    -Wanalyzer-fd-access-mode-mismatch
    -Wanalyzer-fd-double-close
    -Wanalyzer-fd-leak
    -Wanalyzer-fd-phase-mismatch
    -Wanalyzer-fd-type-mismatch
    -Wanalyzer-fd-use-after-close
    -Wanalyzer-fd-use-without-check
    -Wanalyzer-imprecise-fp-arithmetic
    -Wanalyzer-infinite-recursion
    -Wanalyzer-jump-through-null
    -Wanalyzer-out-of-bounds
    -Wanalyzer-putenv-of-auto-var
    -Wanalyzer-tainted-assertion
    -Wanalyzer-va-arg-type-mismatch
    -Wanalyzer-va-list-exhausted
    -Wanalyzer-va-list-leak
    -Wanalyzer-va-list-use-after-va-end
    -Wcomplain-wrong-lang
    -Winvalid-utf8
    -Wunicode
    -Wxor-used-as-pow
  )

  set(GCC_14_0_0_TRUNK_GENERIC_WARNING_FLAGS_ACTIVATED)

  set(GCC_4_4_7_C_WARNING_FLAGS_ACTIVATED
    -Wbad-function-cast
    -Wc++-compat
    -Wdeclaration-after-statement
    -Wformat-zero-length
    -Wimplicit
    -Wimplicit-function-declaration
    -Wimplicit-int
    -Wint-to-pointer-cast
    -Wmissing-parameter-type
    -Wmissing-prototypes
    -Wnested-externs
    -Wnonnull
    -Wold-style-declaration
    -Wold-style-definition
    -Woverride-init
    -Wpointer-sign
    -Wpointer-to-int-cast
    -Wstrict-prototypes
    -Wtraditional
    -Wtraditional-conversion
  )

  set(GCC_4_5_3_C_WARNING_FLAGS_ACTIVATED
    -Wjump-misses-init
    -Wunsuffixed-float-constants
  )

  set(GCC_4_6_4_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_4_7_1_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_4_7_2_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_4_7_3_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_4_7_4_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_4_8_1_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_4_8_2_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_4_8_3_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_4_8_4_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_4_8_5_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_4_9_0_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_4_9_1_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_4_9_2_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_4_9_3_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_4_9_4_C_WARNING_FLAGS_ACTIVATED)

  set(GCC_5_1_0_C_WARNING_FLAGS_ACTIVATED
    -Wc90-c99-compat
    -Wc99-c11-compat
    -Wdesignated-init
    -Wdiscarded-array-qualifiers
    -Wdiscarded-qualifiers
    -Wincompatible-pointer-types
    -Wint-conversion
  )

  set(GCC_5_2_0_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_5_3_0_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_5_4_0_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_5_5_0_C_WARNING_FLAGS_ACTIVATED)

  set(GCC_6_1_0_C_WARNING_FLAGS_ACTIVATED
    -Woverride-init-side-effects
  )

  set(GCC_6_2_0_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_6_3_0_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_6_4_0_C_WARNING_FLAGS_ACTIVATED)

  set(GCC_7_1_0_C_WARNING_FLAGS_ACTIVATED
    -Wduplicate-decl-specifier
  )

  set(GCC_7_2_0_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_7_3_0_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_7_4_0_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_7_5_0_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_8_1_0_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_8_2_0_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_8_3_0_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_8_4_0_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_8_5_0_C_WARNING_FLAGS_ACTIVATED)

  set(GCC_9_1_0_C_WARNING_FLAGS_ACTIVATED
    -Wabsolute-value
    -Wc11-c2x-compat
  )

  set(GCC_9_2_0_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_9_3_0_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_9_4_0_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_9_5_0_C_WARNING_FLAGS_ACTIVATED)

  set(GCC_10_1_0_C_WARNING_FLAGS_ACTIVATED
    -Wenum-conversion
  )

  set(GCC_10_2_0_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_10_3_0_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_10_4_0_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_11_1_0_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_11_2_0_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_11_3_0_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_12_1_0_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_12_2_0_C_WARNING_FLAGS_ACTIVATED)
  set(GCC_12_3_0_C_WARNING_FLAGS_ACTIVATED)

  set(GCC_13_1_0_C_WARNING_FLAGS_ACTIVATED
    ${WARN_STRICT_FLEX_ARRAYS}
    -Wenum-int-mismatch
  )

  set(GCC_14_0_0_TRUNK_C_WARNING_FLAGS_ACTIVATED)

  set(GCC_4_4_7_CXX_WARNING_FLAGS_ACTIVATED
    -Wc++0x-compat
    -Wctor-dtor-privacy
    # -Weffc++
    -Wenum-compare
    -Winvalid-offsetof
    -Wnon-template-friend
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Woverloaded-virtual
    -Wpmf-conversions
    -Wreorder
    -Wsign-promo
    -Wstrict-null-sentinel
    -Wsynth
  )

  set(GCC_4_5_3_CXX_WARNING_FLAGS_ACTIVATED
    -Wconversion-null
  )

  set(GCC_4_6_4_CXX_WARNING_FLAGS_ACTIVATED
    -Wnoexcept
  )

  set(GCC_4_7_1_CXX_WARNING_FLAGS_ACTIVATED
    -Wc++11-compat
    -Wdelete-non-virtual-dtor
    -Wzero-as-null-pointer-constant
  )

  set(GCC_4_7_2_CXX_WARNING_FLAGS_ACTIVATED)
  set(GCC_4_7_3_CXX_WARNING_FLAGS_ACTIVATED)
  set(GCC_4_7_4_CXX_WARNING_FLAGS_ACTIVATED)

  set(GCC_4_8_1_CXX_WARNING_FLAGS_ACTIVATED
    # -Wabi-tag
    -Winherited-variadic-ctor
    -Wliteral-suffix
    # -Wuseless-cast
    -Wvirtual-move-assign
  )

  set(GCC_4_8_2_CXX_WARNING_FLAGS_ACTIVATED)
  set(GCC_4_8_3_CXX_WARNING_FLAGS_ACTIVATED)
  set(GCC_4_8_4_CXX_WARNING_FLAGS_ACTIVATED)
  set(GCC_4_8_5_CXX_WARNING_FLAGS_ACTIVATED)

  set(GCC_4_9_0_CXX_WARNING_FLAGS_ACTIVATED
    -Wconditionally-supported
    -Wdelete-incomplete
  )

  set(GCC_4_9_1_CXX_WARNING_FLAGS_ACTIVATED)
  set(GCC_4_9_2_CXX_WARNING_FLAGS_ACTIVATED)
  set(GCC_4_9_3_CXX_WARNING_FLAGS_ACTIVATED)
  set(GCC_4_9_4_CXX_WARNING_FLAGS_ACTIVATED)

  set(GCC_5_1_0_CXX_WARNING_FLAGS_ACTIVATED
    -Wc++14-compat
    -Wsized-deallocation
    -Wsuggest-override
  )

  set(GCC_5_2_0_CXX_WARNING_FLAGS_ACTIVATED)
  set(GCC_5_3_0_CXX_WARNING_FLAGS_ACTIVATED)
  set(GCC_5_4_0_CXX_WARNING_FLAGS_ACTIVATED)
  set(GCC_5_5_0_CXX_WARNING_FLAGS_ACTIVATED)

  set(GCC_6_1_0_CXX_WARNING_FLAGS_ACTIVATED
    -Wmultiple-inheritance
    # -Wnamespaces
    # -Wplacement-new
    # -Wplacement-new=1
    -Wplacement-new=2
    -Wsubobject-linkage
    # -Wtemplates
    -Wterminate
    -Wvirtual-inheritance
  )

  set(GCC_6_2_0_CXX_WARNING_FLAGS_ACTIVATED)
  set(GCC_6_3_0_CXX_WARNING_FLAGS_ACTIVATED)
  set(GCC_6_4_0_CXX_WARNING_FLAGS_ACTIVATED)

  set(GCC_7_1_0_CXX_WARNING_FLAGS_ACTIVATED
    -Waligned-new=all
    # -Waligned-new=global
    # -Waligned-new=none
    -Wc++17-compat
    -Wc++1z-compat
    -Wnoexcept-type
    -Wregister
  )

  set(GCC_7_2_0_CXX_WARNING_FLAGS_ACTIVATED)
  set(GCC_7_3_0_CXX_WARNING_FLAGS_ACTIVATED)
  set(GCC_7_4_0_CXX_WARNING_FLAGS_ACTIVATED)
  set(GCC_7_5_0_CXX_WARNING_FLAGS_ACTIVATED)

  set(GCC_8_1_0_CXX_WARNING_FLAGS_ACTIVATED
    # -Wcatch-value
    # -Wcatch-value=1
    # -Wcatch-value=2
    -Wcatch-value=3
    -Wclass-memaccess
    -Wextra-semi
  )

  set(GCC_8_2_0_CXX_WARNING_FLAGS_ACTIVATED)
  set(GCC_8_3_0_CXX_WARNING_FLAGS_ACTIVATED)
  set(GCC_8_4_0_CXX_WARNING_FLAGS_ACTIVATED)
  set(GCC_8_5_0_CXX_WARNING_FLAGS_ACTIVATED)

  set(GCC_9_1_0_CXX_WARNING_FLAGS_ACTIVATED
    -Wclass-conversion
    -Wdeprecated-copy
    -Wdeprecated-copy-dtor
    -Winit-list-lifetime
    -Wpessimizing-move
    -Wredundant-move
  )

  set(GCC_9_2_0_CXX_WARNING_FLAGS_ACTIVATED)
  set(GCC_9_3_0_CXX_WARNING_FLAGS_ACTIVATED)
  set(GCC_9_4_0_CXX_WARNING_FLAGS_ACTIVATED)
  set(GCC_9_5_0_CXX_WARNING_FLAGS_ACTIVATED)

  set(GCC_10_1_0_CXX_WARNING_FLAGS_ACTIVATED
    -Wc++20-compat
    -Wc++2a-compat
    -Wcomma-subscript
    -Winaccessible-base
    -Wmismatched-tags
    -Wredundant-tags
    -Wvolatile
  )

  set(GCC_10_2_0_CXX_WARNING_FLAGS_ACTIVATED)
  set(GCC_10_3_0_CXX_WARNING_FLAGS_ACTIVATED)
  set(GCC_10_4_0_CXX_WARNING_FLAGS_ACTIVATED)

  set(GCC_11_1_0_CXX_WARNING_FLAGS_ACTIVATED
    -Wctad-maybe-unsupported
    -Wdeprecated-enum-enum-conversion
    -Wdeprecated-enum-float-conversion
    -Wmismatched-new-delete
    -Wrange-loop-construct
    -Wvexing-parse
  )

  set(GCC_11_2_0_CXX_WARNING_FLAGS_ACTIVATED
    -Wexceptions
    -Winvalid-imported-macros
  )

  set(GCC_11_3_0_CXX_WARNING_FLAGS_ACTIVATED)

  set(GCC_12_1_0_CXX_WARNING_FLAGS_ACTIVATED
    -Wc++11-extensions
    -Wc++14-extensions
    -Wc++17-extensions
    -Wc++20-extensions
    -Wc++23-extensions
    -Winterference-size
    -Wmissing-requires
    -Wmissing-template-keyword
  )

  set(GCC_12_2_0_CXX_WARNING_FLAGS_ACTIVATED)
  set(GCC_12_3_0_CXX_WARNING_FLAGS_ACTIVATED)

  set(GCC_13_1_0_CXX_WARNING_FLAGS_ACTIVATED
    -Wchanges-meaning
    -Wdangling-reference
    -Winvalid-constexpr
    # -Woverloaded-virtual=1
    # -Woverloaded-virtual=2
    -Wself-move
  )

  set(GCC_14_0_0_TRUNK_CXX_WARNING_FLAGS_ACTIVATED)

  # Clear flags.
  set(GCC_4_4_7_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_5_3_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_6_4_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_7_1_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_7_2_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_7_3_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_7_4_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_8_1_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_8_2_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_8_3_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_8_4_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_8_5_GENERIC_WARNING_FLAGS_DEPRECATED)

  set(GCC_4_9_0_GENERIC_WARNING_FLAGS_DEPRECATED
    -Wmudflap # Deprecated at gcc-4.9.0
  )

  set(GCC_4_9_1_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_9_2_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_9_3_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_9_4_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_5_1_0_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_5_2_0_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_5_3_0_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_5_4_0_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_5_5_0_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_6_1_0_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_6_2_0_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_6_3_0_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_6_4_0_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_7_1_0_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_7_2_0_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_7_3_0_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_7_4_0_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_7_5_0_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_8_1_0_GENERIC_WARNING_FLAGS_DEPRECATED)

  set(GCC_8_2_0_GENERIC_WARNING_FLAGS_DEPRECATED
    ${WARN_ABI_HIDE_8_2} # Set 'F_ABI_VERSION' (for '-fabi-version') to keep
  )

  set(GCC_8_3_0_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_8_4_0_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_8_5_0_GENERIC_WARNING_FLAGS_DEPRECATED)

  set(GCC_9_1_0_GENERIC_WARNING_FLAGS_DEPRECATED
    -Wchkp # Deprecated at gcc-9.1.0
  )

  set(GCC_9_2_0_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_9_3_0_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_9_4_0_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_9_5_0_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_10_1_0_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_10_2_0_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_10_3_0_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_10_4_0_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_11_1_0_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_11_2_0_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_11_3_0_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_12_1_0_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_12_2_0_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_12_3_0_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_13_1_0_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_14_0_0_TRUNK_GENERIC_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_4_7_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_5_3_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_6_4_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_7_1_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_7_2_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_7_3_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_7_4_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_8_1_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_8_2_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_8_3_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_8_4_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_8_5_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_9_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_9_1_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_9_2_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_9_3_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_9_4_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_5_1_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_5_2_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_5_3_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_5_4_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_5_5_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_6_1_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_6_2_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_6_3_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_6_4_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_7_1_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_7_2_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_7_3_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_7_4_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_7_5_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_8_1_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_8_2_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_8_3_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_8_4_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_8_5_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_9_1_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_9_2_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_9_3_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_9_4_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_9_5_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_10_1_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_10_2_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_10_3_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_10_4_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_11_1_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_11_2_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_11_3_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_12_1_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_12_2_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_12_3_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_13_1_0_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_14_0_0_TRUNK_C_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_4_7_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_5_3_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_6_4_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_7_1_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_7_2_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_7_3_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_7_4_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_8_1_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_8_2_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_8_3_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_8_4_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_8_5_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_9_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_9_1_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_9_2_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_9_3_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_4_9_4_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_5_1_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_5_2_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_5_3_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_5_4_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_5_5_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_6_1_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_6_2_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_6_3_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_6_4_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_7_1_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_7_2_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_7_3_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_7_4_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_7_5_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_8_1_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_8_2_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_8_3_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_8_4_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_8_5_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_9_1_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_9_2_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_9_3_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_9_4_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_9_5_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_10_1_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_10_2_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_10_3_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_10_4_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_11_1_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_11_2_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_11_3_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_12_1_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_12_2_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_12_3_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_13_1_0_CXX_WARNING_FLAGS_DEPRECATED)
  set(GCC_14_0_0_TRUNK_CXX_WARNING_FLAGS_DEPRECATED)

  # Adding flags depending on version.
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(GCC_CXX_WARNING_FLAGS_ACTIVATED)
    set(GCC_CXX_WARNING_FLAGS_DEPRECATED)
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 4.4.7)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_4_7_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_4_7_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_4_7_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_4_7_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 4.5.3)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_5_3_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_5_3_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_5_3_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_5_3_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 4.6.4)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_6_4_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_6_4_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_6_4_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_6_4_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 4.7.1)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_7_1_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_7_1_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_7_1_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_7_1_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 4.7.2)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_7_2_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_7_2_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_7_2_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_7_2_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 4.7.3)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_7_3_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_7_3_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_7_3_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_7_3_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 4.7.4)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_7_4_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_7_4_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_7_4_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_7_4_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 4.8.1)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_8_1_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_8_1_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_8_1_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_8_1_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 4.8.2)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_8_2_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_8_2_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_8_2_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_8_2_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 4.8.3)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_8_3_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_8_3_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_8_3_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_8_3_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 4.8.4)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_8_4_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_8_4_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_8_4_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_8_4_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 4.8.5)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_8_5_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_8_5_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_8_5_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_8_5_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 4.9.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_9_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_9_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_9_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_9_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 4.9.1)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_9_1_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_9_1_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_9_1_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_9_1_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 4.9.2)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_9_2_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_9_2_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_9_2_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_9_2_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 4.9.3)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_9_3_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_9_3_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_9_3_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_9_3_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 4.9.4)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_9_4_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_4_9_4_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_9_4_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_4_9_4_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 5.1.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_5_1_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_5_1_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_5_1_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_5_1_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 5.2.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_5_2_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_5_2_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_5_2_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_5_2_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 5.3.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_5_3_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_5_3_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_5_3_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_5_3_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 5.4.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_5_4_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_5_4_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_5_4_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_5_4_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 5.5.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_5_5_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_5_5_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_5_5_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_5_5_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 6.1.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_6_1_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_6_1_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_6_1_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_6_1_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 6.2.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_6_2_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_6_2_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_6_2_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_6_2_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 6.3.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_6_3_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_6_3_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_6_3_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_6_3_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 6.4.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_6_4_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_6_4_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_6_4_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_6_4_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 7.1.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_7_1_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_7_1_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_7_1_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_7_1_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 7.2.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_7_2_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_7_2_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_7_2_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_7_2_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 7.3.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_7_3_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_7_3_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_7_3_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_7_3_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 7.4.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_7_4_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_7_4_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_7_4_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_7_4_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 7.5.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_7_5_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_7_5_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_7_5_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_7_5_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8.1.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_8_1_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_8_1_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_8_1_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_8_1_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8.2.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_8_2_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_8_2_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_8_2_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_8_2_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8.3.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_8_3_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_8_3_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_8_3_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_8_3_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8.4.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_8_4_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_8_4_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_8_4_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_8_4_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8.5.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_8_5_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_8_5_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_8_5_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_8_5_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 9.1.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_9_1_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_9_1_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_9_1_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_9_1_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 9.2.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_9_2_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_9_2_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_9_2_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_9_2_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 9.3.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_9_3_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_9_3_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_9_3_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_9_3_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 9.4.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_9_4_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_9_4_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_9_4_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_9_4_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 9.5.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_9_5_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_9_5_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_9_5_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_9_5_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 10.1.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_10_1_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_10_1_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_10_1_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_10_1_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 10.2.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_10_2_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_10_2_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_10_2_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_10_2_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 10.3.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_10_3_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_10_3_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_10_3_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_10_3_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 10.4.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_10_4_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_10_4_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_10_4_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_10_4_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 11.1.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_11_1_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_11_1_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_11_1_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_11_1_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 11.2.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_11_2_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_11_2_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_11_2_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_11_2_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 11.3.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_11_3_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_11_3_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_11_3_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_11_3_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 12.1.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_12_1_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_12_1_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_12_1_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_12_1_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 12.2.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_12_2_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_12_2_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_12_2_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_12_2_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 12.3.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_12_3_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_12_3_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_12_3_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_12_3_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 13.1.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_13_1_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_13_1_0_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_13_1_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_13_1_0_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 14.0.0)
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_14_0_0_TRUNK_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_ACTIVATED ${GCC_14_0_0_TRUNK_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_14_0_0_TRUNK_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_CXX_WARNING_FLAGS_DEPRECATED ${GCC_14_0_0_TRUNK_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    set(TMP_OUTVAR_CXX ${${OUTVAR_CXX}})
    warn_everything_filter_warnings(
      GCC_CXX_WARNING_FLAGS_ACTIVATED
      GCC_CXX_WARNING_FLAGS_DEPRECATED
      TMP_OUTVAR_CXX
    )
    set(${OUTVAR_CXX} ${TMP_OUTVAR_CXX} PARENT_SCOPE)
  endif()

  if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(GCC_C_WARNING_FLAGS_ACTIVATED)
    set(GCC_C_WARNING_FLAGS_DEPRECATED)
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 4.4.7)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_4_7_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_4_7_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_4_7_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_4_7_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 4.5.3)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_5_3_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_5_3_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_5_3_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_5_3_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 4.6.4)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_6_4_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_6_4_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_6_4_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_6_4_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 4.7.1)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_7_1_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_7_1_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_7_1_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_7_1_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 4.7.2)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_7_2_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_7_2_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_7_2_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_7_2_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 4.7.3)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_7_3_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_7_3_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_7_3_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_7_3_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 4.7.4)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_7_4_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_7_4_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_7_4_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_7_4_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 4.8.1)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_8_1_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_8_1_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_8_1_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_8_1_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 4.8.2)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_8_2_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_8_2_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_8_2_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_8_2_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 4.8.3)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_8_3_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_8_3_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_8_3_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_8_3_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 4.8.4)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_8_4_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_8_4_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_8_4_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_8_4_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 4.8.5)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_8_5_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_8_5_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_8_5_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_8_5_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 4.9.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_9_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_9_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_9_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_9_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 4.9.1)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_9_1_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_9_1_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_9_1_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_9_1_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 4.9.2)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_9_2_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_9_2_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_9_2_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_9_2_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 4.9.3)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_9_3_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_9_3_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_9_3_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_9_3_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 4.9.4)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_9_4_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_4_9_4_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_9_4_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_4_9_4_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 5.1.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_5_1_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_5_1_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_5_1_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_5_1_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 5.2.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_5_2_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_5_2_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_5_2_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_5_2_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 5.3.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_5_3_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_5_3_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_5_3_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_5_3_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 5.4.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_5_4_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_5_4_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_5_4_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_5_4_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 5.5.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_5_5_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_5_5_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_5_5_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_5_5_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 6.1.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_6_1_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_6_1_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_6_1_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_6_1_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 6.2.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_6_2_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_6_2_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_6_2_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_6_2_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 6.3.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_6_3_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_6_3_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_6_3_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_6_3_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 6.4.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_6_4_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_6_4_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_6_4_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_6_4_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 7.1.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_7_1_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_7_1_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_7_1_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_7_1_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 7.2.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_7_2_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_7_2_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_7_2_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_7_2_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 7.3.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_7_3_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_7_3_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_7_3_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_7_3_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 7.4.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_7_4_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_7_4_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_7_4_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_7_4_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 7.5.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_7_5_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_7_5_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_7_5_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_7_5_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 8.1.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_8_1_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_8_1_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_8_1_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_8_1_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 8.2.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_8_2_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_8_2_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_8_2_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_8_2_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 8.3.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_8_3_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_8_3_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_8_3_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_8_3_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 8.4.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_8_4_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_8_4_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_8_4_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_8_4_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 8.5.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_8_5_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_8_5_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_8_5_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_8_5_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 9.1.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_9_1_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_9_1_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_9_1_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_9_1_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 9.2.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_9_2_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_9_2_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_9_2_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_9_2_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 9.3.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_9_3_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_9_3_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_9_3_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_9_3_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 9.4.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_9_4_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_9_4_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_9_4_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_9_4_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 9.5.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_9_5_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_9_5_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_9_5_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_9_5_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 10.1.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_10_1_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_10_1_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_10_1_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_10_1_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 10.2.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_10_2_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_10_2_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_10_2_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_10_2_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 10.3.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_10_3_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_10_3_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_10_3_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_10_3_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 10.4.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_10_4_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_10_4_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_10_4_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_10_4_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 11.1.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_11_1_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_11_1_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_11_1_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_11_1_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 11.2.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_11_2_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_11_2_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_11_2_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_11_2_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 11.3.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_11_3_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_11_3_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_11_3_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_11_3_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 12.1.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_12_1_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_12_1_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_12_1_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_12_1_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 12.2.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_12_2_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_12_2_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_12_2_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_12_2_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 12.3.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_12_3_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_12_3_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_12_3_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_12_3_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 13.1.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_13_1_0_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_13_1_0_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_13_1_0_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_13_1_0_C_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 14.0.0)
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_14_0_0_TRUNK_GENERIC_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_ACTIVATED ${GCC_14_0_0_TRUNK_C_WARNING_FLAGS_ACTIVATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_14_0_0_TRUNK_GENERIC_WARNING_FLAGS_DEPRECATED})
      list(APPEND GCC_C_WARNING_FLAGS_DEPRECATED ${GCC_14_0_0_TRUNK_C_WARNING_FLAGS_DEPRECATED})
    endif()
    set(TMP_OUTVAR_C ${${OUTVAR_C}})
    warn_everything_filter_warnings(
      GCC_C_WARNING_FLAGS_ACTIVATED
      GCC_C_WARNING_FLAGS_DEPRECATED
      TMP_OUTVAR_C
    )
    set(${OUTVAR_C} ${TMP_OUTVAR_C} PARENT_SCOPE)
  endif()
endfunction()
