#
# Warn everything for CLANG between 4.0.0 - trunk.17.0.0
# Create: 2023-05-17
#
# Default is all warnings are on and none are off.
# To unset a warning, search for the commented out
# '-Wno-<name>' in one of the '*_ACTIVATED' variables
# and uncomment it.
#
# All warnings should be enablable/disablable without
# touch any variables other than the '*_ACTIVATED' ones
#

function(warn_everything_clang OUTVAR_CXX OUTVAR_C)
  # Enable warnings that require user specified value.
  if (NOT DEFINED WARN_FRAME_LARGER_THAN)
    if (DEFINED W_FRAME_LARGER_THAN_VAL)
      set(WARN_FRAME_LARGER_THAN -Wframe-larger-than=${W_FRAME_LARGER_THAN_VAL})
    endif()
  endif()

  # Set active flags.
  set(CLANG_4_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED
    -Weverything
    # -Wno-CFString-literal
    # -Wno-CL4
    # -Wno-IndependentClass-attribute
    # -Wno-NSObject-attribute
    # -Wno-abi
    # -Wno-absolute-value
    # -Wno-abstract-final-class
    # -Wno-abstract-vbase-init
    # -Wno-address
    # -Wno-address-of-array-temporary
    # -Wno-address-of-packed-member
    # -Wno-address-of-temporary
    # -Wno-aggregate-return
    # -Wno-all
    # -Wno-alloca-with-align-alignof
    # -Wno-ambiguous-delete
    # -Wno-ambiguous-ellipsis
    # -Wno-ambiguous-macro
    # -Wno-ambiguous-member-template
    # -Wno-analyzer-incompatible-plugin
    # -Wno-anonymous-pack-parens
    # -Wno-arc
    # -Wno-arc-bridge-casts-disallowed-in-nonarc
    # -Wno-arc-maybe-repeated-use-of-weak
    # -Wno-arc-non-pod-memaccess
    # -Wno-arc-performSelector-leaks
    # -Wno-arc-repeated-use-of-weak
    # -Wno-arc-retain-cycles
    # -Wno-arc-unsafe-retained-assign
    # -Wno-array-bounds
    # -Wno-array-bounds-pointer-arithmetic
    # -Wno-asm
    # -Wno-asm-ignored-qualifier
    # -Wno-asm-operand-widths
    # -Wno-assign-enum
    # -Wno-assume
    # -Wno-at-protocol
    # -Wno-atomic-memory-ordering
    # -Wno-atomic-properties
    # -Wno-atomic-property-with-user-defined-accessor
    # -Wno-attribute-packed-for-bitfield
    # -Wno-attributes
    # -Wno-auto-disable-vptr-sanitizer
    # -Wno-auto-import
    # -Wno-auto-storage-class
    # -Wno-auto-var-id
    # -Wno-availability
    # -Wno-backend-plugin
    # -Wno-backslash-newline-escape
    # -Wno-bad-function-cast
    # -Wno-bind-to-temporary-copy
    # -Wno-bitfield-constant-conversion
    # -Wno-bitfield-width
    # -Wno-bitwise-op-parentheses
    # -Wno-block-capture-autoreleasing
    # -Wno-bool-conversion
    # -Wno-bool-conversions
    # -Wno-braced-scalar-init
    # -Wno-bridge-cast
    # -Wno-builtin-macro-redefined
    # -Wno-builtin-memcpy-chk-size
    # -Wno-builtin-requires-header
    # -Wno-c++-compat
    # -Wno-c++0x-compat
    # -Wno-c++0x-extensions
    # -Wno-c++0x-narrowing
    # -Wno-c++11-compat
    # -Wno-c++11-compat-deprecated-writable-strings
    # -Wno-c++11-compat-pedantic
    # -Wno-c++11-compat-reserved-user-defined-literal
    # -Wno-c++11-extensions
    # -Wno-c++11-extra-semi
    # -Wno-c++11-inline-namespace
    # -Wno-c++11-long-long
    # -Wno-c++11-narrowing
    # -Wno-c++14-binary-literal
    # -Wno-c++14-compat
    # -Wno-c++14-compat-pedantic
    # -Wno-c++14-extensions
    # -Wno-c++1y-extensions
    # -Wno-c++1z-compat
    # -Wno-c++1z-extensions
    # -Wno-c++98-c++11-c++14-compat
    # -Wno-c++98-c++11-c++14-compat-pedantic
    # -Wno-c++98-c++11-compat
    # -Wno-c++98-c++11-compat-pedantic
    -Wno-c++98-compat
    # -Wno-c++98-compat-bind-to-temporary-copy
    # -Wno-c++98-compat-local-type-template-args
    -Wno-c++98-compat-pedantic
    # -Wno-c++98-compat-unnamed-type-template-args
    # -Wno-c11-extensions
    # -Wno-c99-compat
    # -Wno-c99-extensions
    # -Wno-cast-align
    # -Wno-cast-calling-convention
    # -Wno-cast-of-sel-type
    # -Wno-cast-qual
    # -Wno-char-align
    # -Wno-char-subscripts
    # -Wno-clang-cl-pch
    # -Wno-class-varargs
    # -Wno-comma
    # -Wno-comment
    # -Wno-comments
    # -Wno-compare-distinct-pointer-types
    # -Wno-complex-component-init
    # -Wno-conditional-type-mismatch
    # -Wno-conditional-uninitialized
    # -Wno-config-macros
    # -Wno-constant-conversion
    # -Wno-constant-logical-operand
    # -Wno-constexpr-not-const
    # -Wno-consumed
    # -Wno-conversion
    # -Wno-conversion-null
    # -Wno-covered-switch-default
    # -Wno-cstring-format-directive
    # -Wno-ctor-dtor-privacy
    # -Wno-cuda-compat
    # -Wno-custom-atomic-properties
    # -Wno-dangling-else
    # -Wno-dangling-field
    # -Wno-dangling-initializer-list
    # -Wno-date-time
    # -Wno-dealloc-in-category
    # -Wno-debug-compression-unavailable
    # -Wno-declaration-after-statement
    # -Wno-delegating-ctor-cycles
    # -Wno-delete-incomplete
    # -Wno-delete-non-virtual-dtor
    # -Wno-deprecated
    # -Wno-deprecated-attributes
    # -Wno-deprecated-declarations
    # -Wno-deprecated-dynamic-exception-spec
    # -Wno-deprecated-implementations
    # -Wno-deprecated-increment-bool
    # -Wno-deprecated-objc-isa-usage
    # -Wno-deprecated-objc-pointer-introspection
    # -Wno-deprecated-objc-pointer-introspection-performSelector
    # -Wno-deprecated-register
    # -Wno-deprecated-writable-strings
    # -Wno-direct-ivar-access
    -Wno-disabled-macro-expansion
    # -Wno-disabled-optimization
    # -Wno-discard-qual
    # -Wno-distributed-object-modifiers
    # -Wno-div-by-zero
    # -Wno-division-by-zero
    # -Wno-dll-attribute-on-redeclaration
    # -Wno-dllexport-explicit-instantiation-decl
    # -Wno-dllimport-static-field-def
    # -Wno-documentation
    # -Wno-documentation-deprecated-sync
    # -Wno-documentation-html
    # -Wno-documentation-pedantic
    -Wno-documentation-unknown-command
    # -Wno-dollar-in-identifier-extension
    # -Wno-double-promotion
    # -Wno-duplicate-decl-specifier
    # -Wno-duplicate-enum
    # -Wno-duplicate-method-arg
    # -Wno-duplicate-method-match
    # -Wno-duplicate-protocol
    # -Wno-dynamic-class-memaccess
    # -Wno-dynamic-exception-spec
    # -Wno-embedded-directive
    # -Wno-empty-body
    # -Wno-empty-decomposition
    # -Wno-empty-translation-unit
    # -Wno-encode-type
    # -Wno-endif-labels
    # -Wno-enum-compare
    # -Wno-enum-conversion
    # -Wno-enum-too-large
    # -Wno-exceptions
    # -Wno-exit-time-destructors
    # -Wno-expansion-to-defined
    # -Wno-explicit-initialize-call
    # -Wno-explicit-ownership-type
    # -Wno-extended-offsetof
    # -Wno-extern-c-compat
    # -Wno-extern-initializer
    # -Wno-extra
    # -Wno-extra-qualification
    # -Wno-extra-semi
    # -Wno-extra-tokens
    # -Wno-fallback
    # -Wno-flag-enum
    # -Wno-flexible-array-extensions
    # -Wno-float-conversion
    # -Wno-float-equal
    # -Wno-float-overflow-conversion
    # -Wno-float-zero-conversion
    # -Wno-for-loop-analysis
    # -Wno-format
    # -Wno-format-extra-args
    # -Wno-format-invalid-specifier
    # -Wno-format-non-iso
    # -Wno-format-nonliteral
    # -Wno-format-pedantic
    # -Wno-format-security
    # -Wno-format-y2k
    # -Wno-format-zero-length
    # -Wno-format=2
    # -Wno-four-char-constants
    # -Wno-function-def-in-objc-container
    # -Wno-future-compat
    # -Wno-gcc-compat
    # -Wno-global-constructors
    # -Wno-gnu
    # -Wno-gnu-alignof-expression
    # -Wno-gnu-anonymous-struct
    # -Wno-gnu-array-member-paren-init
    # -Wno-gnu-auto-type
    # -Wno-gnu-binary-literal
    # -Wno-gnu-case-range
    # -Wno-gnu-complex-integer
    # -Wno-gnu-compound-literal-initializer
    # -Wno-gnu-conditional-omitted-operand
    # -Wno-gnu-designator
    # -Wno-gnu-empty-initializer
    # -Wno-gnu-empty-struct
    # -Wno-gnu-flexible-array-initializer
    # -Wno-gnu-flexible-array-union-member
    # -Wno-gnu-folding-constant
    # -Wno-gnu-imaginary-constant
    # -Wno-gnu-include-next
    # -Wno-gnu-label-as-value
    # -Wno-gnu-redeclared-enum
    # -Wno-gnu-statement-expression
    # -Wno-gnu-static-float-init
    # -Wno-gnu-string-literal-operator-template
    # -Wno-gnu-union-cast
    # -Wno-gnu-variable-sized-type-not-at-end
    # -Wno-gnu-zero-line-directive
    -Wno-gnu-zero-variadic-macro-arguments
    # -Wno-header-guard
    # -Wno-header-hygiene
    # -Wno-idiomatic-parentheses
    # -Wno-ignored-attributes
    # -Wno-ignored-optimization-argument
    # -Wno-ignored-pragma-intrinsic
    # -Wno-ignored-pragmas
    # -Wno-ignored-qualifiers
    # -Wno-implicit
    # -Wno-implicit-atomic-properties
    # -Wno-implicit-conversion-floating-point-to-bool
    # -Wno-implicit-exception-spec-mismatch
    # -Wno-implicit-fallthrough
    # -Wno-implicit-fallthrough-per-function
    # -Wno-implicit-function-declaration
    # -Wno-implicit-int
    # -Wno-implicit-retain-self
    # -Wno-implicitly-unsigned-literal
    # -Wno-import
    # -Wno-import-preprocessor-directive-pedantic
    # -Wno-inaccessible-base
    # -Wno-include-next-absolute-path
    # -Wno-include-next-outside-header
    # -Wno-incompatible-exception-spec
    # -Wno-incompatible-function-pointer-types
    # -Wno-incompatible-library-redeclaration
    # -Wno-incompatible-ms-struct
    # -Wno-incompatible-pointer-types
    # -Wno-incompatible-pointer-types-discards-qualifiers
    # -Wno-incompatible-property-type
    # -Wno-incompatible-sysroot
    # -Wno-incomplete-implementation
    # -Wno-incomplete-module
    # -Wno-incomplete-umbrella
    # -Wno-inconsistent-dllimport
    # -Wno-inconsistent-missing-override
    # -Wno-increment-bool
    # -Wno-infinite-recursion
    # -Wno-init-self
    # -Wno-initializer-overrides
    # -Wno-inline
    # -Wno-inline-asm
    # -Wno-inline-new-delete
    # -Wno-instantiation-after-specialization
    # -Wno-int-conversion
    # -Wno-int-conversions
    # -Wno-int-to-pointer-cast
    # -Wno-int-to-void-pointer-cast
    # -Wno-integer-overflow
    # -Wno-invalid-command-line-argument
    # -Wno-invalid-constexpr
    # -Wno-invalid-iboutlet
    # -Wno-invalid-initializer-from-system-header
    # -Wno-invalid-noreturn
    # -Wno-invalid-offsetof
    # -Wno-invalid-or-nonexistent-directory
    # -Wno-invalid-partial-specialization
    # -Wno-invalid-pch
    # -Wno-invalid-pp-token
    # -Wno-invalid-source-encoding
    # -Wno-invalid-token-paste
    # -Wno-jump-seh-finally
    # -Wno-keyword-compat
    # -Wno-keyword-macro
    # -Wno-knr-promoted-parameter
    # -Wno-language-extension-token
    # -Wno-large-by-value-copy
    # -Wno-liblto
    # -Wno-literal-conversion
    # -Wno-literal-range
    # -Wno-local-type-template-args
    # -Wno-logical-not-parentheses
    # -Wno-logical-op-parentheses
    # -Wno-long-long
    # -Wno-loop-analysis
    # -Wno-macro-redefined
    # -Wno-main
    # -Wno-main-return-type
    # -Wno-malformed-warning-check
    # -Wno-many-braces-around-scalar-init
    # -Wno-max-unsigned-zero
    # -Wno-memsize-comparison
    # -Wno-method-signatures
    # -Wno-microsoft
    # -Wno-microsoft-anon-tag
    # -Wno-microsoft-cast
    # -Wno-microsoft-charize
    # -Wno-microsoft-comment-paste
    # -Wno-microsoft-const-init
    # -Wno-microsoft-cpp-macro
    # -Wno-microsoft-default-arg-redefinition
    # -Wno-microsoft-end-of-file
    # -Wno-microsoft-enum-forward-reference
    # -Wno-microsoft-enum-value
    # -Wno-microsoft-exception-spec
    # -Wno-microsoft-exists
    # -Wno-microsoft-explicit-constructor-call
    # -Wno-microsoft-extra-qualification
    # -Wno-microsoft-fixed-enum
    # -Wno-microsoft-flexible-array
    # -Wno-microsoft-goto
    # -Wno-microsoft-include
    # -Wno-microsoft-mutable-reference
    # -Wno-microsoft-pure-definition
    # -Wno-microsoft-redeclare-static
    # -Wno-microsoft-sealed
    # -Wno-microsoft-template
    # -Wno-microsoft-union-member-reference
    # -Wno-microsoft-unqualified-friend
    # -Wno-microsoft-using-decl
    # -Wno-microsoft-void-pseudo-dtor
    # -Wno-mismatched-new-delete
    # -Wno-mismatched-parameter-types
    # -Wno-mismatched-return-types
    # -Wno-mismatched-tags
    # -Wno-missing-braces
    # -Wno-missing-declarations
    # -Wno-missing-exception-spec
    # -Wno-missing-field-initializers
    # -Wno-missing-format-attribute
    # -Wno-missing-include-dirs
    # -Wno-missing-method-return-type
    # -Wno-missing-noreturn
    # -Wno-missing-prototype-for-cc
    # -Wno-missing-prototypes
    # -Wno-missing-selector-name
    # -Wno-missing-sysroot
    # -Wno-missing-variable-declarations
    # -Wno-module-conflict
    # -Wno-module-file-config-mismatch
    # -Wno-module-file-extension
    # -Wno-module-import-in-extern-c
    # -Wno-modules-ambiguous-internal-linkage
    # -Wno-modules-import-nested-redundant
    # -Wno-most
    # -Wno-move
    # -Wno-msvc-include
    # -Wno-multichar
    # -Wno-multiple-move-vbase
    # -Wno-narrowing
    # -Wno-nested-anon-types
    # -Wno-nested-externs
    # -Wno-new-returns-null
    # -Wno-newline-eof
    # -Wno-non-gcc
    # -Wno-non-literal-null-conversion
    # -Wno-non-modular-include-in-framework-module
    # -Wno-non-modular-include-in-module
    # -Wno-non-pod-varargs
    # -Wno-non-virtual-dtor
    # -Wno-nonnull
    # -Wno-nonportable-cfstrings
    # -Wno-nonportable-include-path
    # -Wno-nonportable-system-include-path
    # -Wno-nonportable-vector-initialization
    # -Wno-null-arithmetic
    # -Wno-null-character
    # -Wno-null-conversion
    # -Wno-null-dereference
    # -Wno-nullability
    # -Wno-nullability-completeness
    # -Wno-nullability-completeness-on-arrays
    # -Wno-nullability-declspec
    # -Wno-nullability-extension
    # -Wno-nullability-inferred-on-nested-type
    # -Wno-nullable-to-nonnull-conversion
    # -Wno-objc-autosynthesis-property-ivar-name-match
    # -Wno-objc-circular-container
    # -Wno-objc-cocoa-api
    # -Wno-objc-designated-initializers
    # -Wno-objc-forward-class-redefinition
    # -Wno-objc-interface-ivars
    # -Wno-objc-literal-compare
    # -Wno-objc-literal-conversion
    # -Wno-objc-macro-redefinition
    # -Wno-objc-method-access
    # -Wno-objc-missing-property-synthesis
    # -Wno-objc-missing-super-calls
    # -Wno-objc-multiple-method-names
    # -Wno-objc-noncopy-retain-block-property
    # -Wno-objc-nonunified-exceptions
    # -Wno-objc-property-implementation
    # -Wno-objc-property-implicit-mismatch
    # -Wno-objc-property-matches-cocoa-ownership-rule
    # -Wno-objc-property-no-attribute
    # -Wno-objc-property-synthesis
    # -Wno-objc-protocol-method-implementation
    # -Wno-objc-protocol-property-synthesis
    # -Wno-objc-protocol-qualifiers
    # -Wno-objc-readonly-with-setter-property
    # -Wno-objc-redundant-api-use
    # -Wno-objc-redundant-literal-use
    # -Wno-objc-root-class
    # -Wno-objc-string-compare
    # -Wno-objc-string-concatenation
    # -Wno-odr
    # -Wno-old-style-cast
    # -Wno-old-style-definition
    # -Wno-opencl-unsupported-rgba
    # -Wno-openmp-clauses
    # -Wno-openmp-loop-form
    # -Wno-openmp-target
    # -Wno-option-ignored
    # -Wno-out-of-line-declaration
    # -Wno-over-aligned
    # -Wno-overflow
    # -Wno-overlength-strings
    # -Wno-overloaded-shift-op-parentheses
    # -Wno-overloaded-virtual
    # -Wno-override-module
    # -Wno-overriding-method-mismatch
    # -Wno-overriding-t-option
    # -Wno-packed
    -Wno-padded
    # -Wno-parentheses
    # -Wno-parentheses-equality
    # -Wno-partial-availability
    # -Wno-pass-failed
    # -Wno-pch-date-time
    # -Wno-pedantic
    # -Wno-pedantic-core-features
    # -Wno-pessimizing-move
    # -Wno-pointer-arith
    # -Wno-pointer-bool-conversion
    # -Wno-pointer-sign
    # -Wno-pointer-to-int-cast
    # -Wno-pointer-type-mismatch
    # -Wno-potentially-evaluated-expression
    # -Wno-pragma-once-outside-header
    # -Wno-pragma-system-header-outside-header
    # -Wno-pragmas
    # -Wno-predefined-identifier-outside-function
    # -Wno-private-extern
    # -Wno-private-header
    # -Wno-private-module
    # -Wno-profile-instr-out-of-date
    # -Wno-profile-instr-unprofiled
    # -Wno-property-access-dot-syntax
    # -Wno-property-attribute-mismatch
    # -Wno-protocol
    # -Wno-protocol-property-synthesis-ambiguity
    # -Wno-qualified-void-return-type
    # -Wno-range-loop-analysis
    # -Wno-readonly-iboutlet-property
    # -Wno-receiver-expr
    # -Wno-receiver-forward-class
    # -Wno-redeclared-class-member
    # -Wno-redundant-decls
    # -Wno-redundant-move
    # -Wno-register
    # -Wno-reinterpret-base-class
    # -Wno-reorder
    # -Wno-requires-super-attribute
    # -Wno-reserved-id-macro
    # -Wno-reserved-user-defined-literal
    # -Wno-retained-language-linkage
    # -Wno-return-stack-address
    # -Wno-return-type
    # -Wno-return-type-c-linkage
    # -Wno-rtti-for-exceptions
    # -Wno-section
    # -Wno-selector
    # -Wno-selector-type-mismatch
    # -Wno-self-assign
    # -Wno-self-assign-field
    # -Wno-self-move
    # -Wno-semicolon-before-method-body
    # -Wno-sentinel
    # -Wno-sequence-point
    # -Wno-serialized-diagnostics
    # -Wno-shadow
    # -Wno-shadow-all
    # -Wno-shadow-field-in-constructor
    # -Wno-shadow-field-in-constructor-modified
    # -Wno-shadow-ivar
    # -Wno-shadow-uncaptured-local
    # -Wno-shift-count-negative
    # -Wno-shift-count-overflow
    # -Wno-shift-negative-value
    # -Wno-shift-op-parentheses
    # -Wno-shift-overflow
    # -Wno-shift-sign-overflow
    # -Wno-shorten-64-to-32
    # -Wno-sign-compare
    # -Wno-sign-conversion
    # -Wno-sign-promo
    # -Wno-signed-enum-bitfield
    # -Wno-sizeof-array-argument
    # -Wno-sizeof-array-decay
    # -Wno-sizeof-pointer-memaccess
    # -Wno-sometimes-uninitialized
    # -Wno-source-uses-openmp
    # -Wno-spir-compat
    # -Wno-stack-protector
    # -Wno-static-float-init
    # -Wno-static-in-inline
    # -Wno-static-inline-explicit-instantiation
    # -Wno-static-local-in-inline
    # -Wno-static-self-init
    # -Wno-strict-aliasing
    # -Wno-strict-aliasing=1
    # -Wno-strict-aliasing=2
    # -Wno-strict-overflow
    # -Wno-strict-overflow=1
    # -Wno-strict-overflow=2
    # -Wno-strict-overflow=3
    # -Wno-strict-overflow=4
    # -Wno-strict-overflow=5
    # -Wno-strict-prototypes
    # -Wno-strict-selector-match
    # -Wno-string-compare
    # -Wno-string-conversion
    # -Wno-string-plus-char
    # -Wno-string-plus-int
    # -Wno-strlcpy-strlcat-size
    # -Wno-strncat-size
    # -Wno-super-class-method-mismatch
    # -Wno-switch
    # -Wno-switch-bool
    # -Wno-switch-default
    # -Wno-switch-enum
    # -Wno-sync-fetch-and-nand-semantics-changed
    # -Wno-synth
    # -Wno-tautological-compare
    # -Wno-tautological-constant-out-of-range-compare
    # -Wno-tautological-overlap-compare
    # -Wno-tautological-pointer-compare
    # -Wno-tautological-undefined-compare
    # -Wno-tentative-definition-incomplete-type
    # -Wno-thread-safety
    # -Wno-thread-safety-analysis
    # -Wno-thread-safety-attributes
    # -Wno-thread-safety-beta
    # -Wno-thread-safety-negative
    # -Wno-thread-safety-precise
    # -Wno-thread-safety-reference
    # -Wno-thread-safety-verbose
    # -Wno-trigraphs
    # -Wno-type-limits
    # -Wno-type-safety
    # -Wno-typedef-redefinition
    # -Wno-typename-missing
    # -Wno-unable-to-open-stats-file
    # -Wno-unavailable-declarations
    # -Wno-undeclared-selector
    # -Wno-undef
    # -Wno-undefined-bool-conversion
    # -Wno-undefined-func-template
    # -Wno-undefined-inline
    # -Wno-undefined-internal
    # -Wno-undefined-reinterpret-cast
    # -Wno-undefined-var-template
    # -Wno-unevaluated-expression
    # -Wno-unguarded-availability
    # -Wno-unicode
    # -Wno-unicode-whitespace
    # -Wno-uninitialized
    # -Wno-unknown-argument
    # -Wno-unknown-attributes
    # -Wno-unknown-escape-sequence
    # -Wno-unknown-pragmas
    # -Wno-unknown-sanitizers
    # -Wno-unknown-warning-option
    # -Wno-unnamed-type-template-args
    # -Wno-unneeded-internal-declaration
    # -Wno-unneeded-member-function
    # -Wno-unreachable-code
    # -Wno-unreachable-code-aggressive
    # -Wno-unreachable-code-break
    # -Wno-unreachable-code-loop-increment
    # -Wno-unreachable-code-return
    # -Wno-unsequenced
    # -Wno-unsupported-cb
    # -Wno-unsupported-dll-base-class-template
    # -Wno-unsupported-friend
    # -Wno-unsupported-nan
    # -Wno-unsupported-visibility
    # -Wno-unusable-partial-specialization
    # -Wno-unused
    # -Wno-unused-argument
    -Wno-unused-command-line-argument
    # -Wno-unused-comparison
    # -Wno-unused-const-variable
    # -Wno-unused-exception-parameter
    # -Wno-unused-function
    # -Wno-unused-getter-return-value
    # -Wno-unused-label
    # -Wno-unused-local-typedef
    # -Wno-unused-local-typedefs
    # -Wno-unused-macros
    # -Wno-unused-member-function
    # -Wno-unused-parameter
    # -Wno-unused-private-field
    # -Wno-unused-property-ivar
    # -Wno-unused-result
    # -Wno-unused-value
    # -Wno-unused-variable
    # -Wno-unused-volatile-lvalue
    # -Wno-used-but-marked-unused
    # -Wno-user-defined-literals
    # -Wno-user-defined-warnings
    # -Wno-varargs
    # -Wno-variadic-macros
    # -Wno-vec-elem-size
    # -Wno-vector-conversion
    # -Wno-vector-conversions
    # -Wno-vexing-parse
    # -Wno-visibility
    # -Wno-vla
    # -Wno-vla-extension
    # -Wno-void-ptr-dereference
    # -Wno-volatile-register-var
    # -Wno-weak-template-vtables
    # -Wno-weak-vtables
    # -Wno-writable-strings
    # -Wno-write-strings
    # -Wno-zero-length-array
  )

  set(CLANG_4_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_4_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED)
  set(CLANG_4_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_5_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED
    # -Wno-aligned-allocation-unavailable
    # -Wno-bitfield-enum-conversion
    # -Wno-c++17-compat
    # -Wno-c++17-compat-mangling
    # -Wno-c++17-extensions
    # -Wno-c++1z-compat-mangling
    # -Wno-coroutine
    # -Wno-coroutine-missing-unhandled-exception
    # -Wno-cpp
    # -Wno-inconsistent-missing-destructor-override
    # -Wno-injected-class-name
    # -Wno-invalid-ios-deployment-target
    # -Wno-msvc-not-found
    # -Wno-noexcept-type
    # -Wno-objc-unsafe-perform-selector
    # -Wno-pragma-clang-attribute
    # -Wno-profile-instr-missing
    # -Wno-shadow-field
    # -Wno-slash-u-filename
    # -Wno-unguarded-availability-new
    # -Wno-unsupported-availability-guard
    # -Wno-unused-lambda-capture
    -Wno-unused-template
    # -Wno-zero-as-null-pointer-constant
  )

  set(CLANG_5_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_5_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED)
  set(CLANG_5_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_5_0_2_C_AND_CXX_WARNING_FLAGS_ACTIVATED)
  set(CLANG_5_0_2_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_6_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED
    # -Wno-binary-literal
    # -Wno-binding-in-condition
    # -Wno-c++17-compat-pedantic
    # -Wno-c++2a-compat
    # -Wno-c++2a-compat-pedantic
    # -Wno-c++2a-extensions
    # -Wno-c++98-c++11-c++14-c++17-compat
    # -Wno-c++98-c++11-c++14-c++17-compat-pedantic
    # -Wno-c++98-c++11-compat-binary-literal
    # -Wno-enum-compare-switch
    # -Wno-experimental-isel
    # -Wno-microsoft-inaccessible-base
    # -Wno-missing-noescape
    # -Wno-nsconsumed-mismatch
    # -Wno-nsreturns-mismatch
    # -Wno-null-pointer-arithmetic
    # -Wno-objc-flexible-array
    # -Wno-objc-messaging-id
    # -Wno-out-of-scope-function
    # -Wno-pragma-pack
    # -Wno-pragma-pack-suspicious-include
    # -Wno-redundant-parens
    # -Wno-tautological-constant-compare
    # -Wno-tautological-constant-in-range-compare
    # -Wno-tautological-type-limit-compare
    # -Wno-tautological-unsigned-enum-zero-compare
    # -Wno-tautological-unsigned-zero-compare
    # -Wno-undefined-internal-type
    # -Wno-unicode-homoglyph
    # -Wno-unsupported-abs
    # -Wno-unsupported-gpopt
  )

  set(CLANG_6_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED
    -Wno-extended-offsetof # Deprecated at clang-6.0.0
  )

  set(CLANG_6_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED)
  set(CLANG_6_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_7_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED
    # -Wno-argument-outside-range
    # -Wno-atimport-in-framework-header
    # -Wno-atomic-alignment
    # -Wno-c++98-compat-extra-semi
    # -Wno-cast-qual-unrelated
    # -Wno-dangling
    # -Wno-deprecated-this-capture
    # -Wno-framework-include-private-from-public
    # -Wno-function-multiversion
    # -Wno-ignored-pragma-optimize
    # -Wno-incomplete-framework-module-declaration
    # -Wno-memset-transposed-args
    # -Wno-nontrivial-memaccess
    # -Wno-ordered-compare-function-pointers
    # -Wno-quoted-include-in-framework-header
    # -Wno-return-std-move
    # -Wno-return-std-move-in-c++11
    # -Wno-self-assign-overloaded
    # -Wno-stdlibcxx-not-found
    # -Wno-suspicious-bzero
    # -Wno-suspicious-memaccess
    # -Wno-unsupported-target-opt
  )

  set(CLANG_7_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED
    -Wno-address-of-array-temporary # Deprecated at clang-7.0.0
    -Wno-aligned-allocation-unavailable # Deprecated at clang-7.0.0
    -Wno-rtti-for-exceptions # Deprecated at clang-7.0.0
  )

  set(CLANG_7_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED)
  set(CLANG_7_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_8_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED
    # -Wno-atomic-implicit-seq-cst
    # -Wno-call-to-pure-virtual-from-ctor-dtor
    # -Wno-ctu
    # -Wno-darwin-sdk-settings
    # -Wno-defaulted-function-deleted
    # -Wno-delete-abstract-non-virtual-dtor
    # -Wno-delete-non-abstract-non-virtual-dtor
    # -Wno-empty-init-stmt
    -Wno-extra-semi-stmt
    # -Wno-fixed-enum-extension
    # -Wno-implicit-float-conversion
    # -Wno-implicit-int-conversion
    # -Wno-noderef
    # -Wno-objc-property-assign-on-object-type
    # -Wno-override-init
    # -Wno-pointer-integer-compare
    # -Wno-sizeof-pointer-div
    # -Wno-unicode-zero-width
  )

  set(CLANG_8_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_8_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED)
  set(CLANG_8_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_9_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED
    # -Wno-avr-rtlib-linking-quirks
    # -Wno-concepts-ts-compat
    # -Wno-ctad-maybe-unsupported
    # -Wno-export-unnamed
    # -Wno-export-using-directive
    # -Wno-fortify-source
    # -Wno-implicit-fixed-point-conversion
    # -Wno-incomplete-setjmp-declaration
    # -Wno-microsoft-drectve-section
    # -Wno-objc-bool-constant-conversion
    # -Wno-objc-boxing
    # -Wno-tautological-objc-bool-compare
    # -Wno-underaligned-exception-object
  )

  set(CLANG_9_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_9_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED)
  set(CLANG_9_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_10_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED
    # -Wno-alloca
    # -Wno-ambiguous-reversed-operator
    # -Wno-anon-enum-enum-conversion
    # -Wno-bitwise-conditional-parentheses
    # -Wno-bool-operation
    # -Wno-builtin-assume-aligned-alignment
    # -Wno-c++20-compat
    # -Wno-c++20-compat-pedantic
    # -Wno-c++20-designator
    # -Wno-c++20-extensions
    # -Wno-c99-designator
    # -Wno-constant-evaluated
    # -Wno-dangling-gsl
    # -Wno-deprecated-anon-enum-enum-conversion
    # -Wno-deprecated-array-compare
    # -Wno-deprecated-comma-subscript
    # -Wno-deprecated-copy
    # -Wno-deprecated-copy-dtor
    # -Wno-deprecated-enum-compare
    # -Wno-deprecated-enum-compare-conditional
    # -Wno-deprecated-enum-enum-conversion
    # -Wno-deprecated-enum-float-conversion
    # -Wno-deprecated-volatile
    # -Wno-enum-compare-conditional
    # -Wno-enum-enum-conversion
    # -Wno-enum-float-conversion
    # -Wno-final-dtor-non-final-class
    # -Wno-format-type-confusion
    # -Wno-gnu-inline-cpp-without-extern
    # -Wno-hip-only
    # -Wno-implicit-int-float-conversion
    # -Wno-int-in-bool-context
    # -Wno-invalid-no-builtin-names
    # -Wno-microsoft-template-shadow
    # -Wno-misexpect
    # -Wno-misleading-indentation
    # -Wno-missing-constinit
    # -Wno-objc-signed-char-bool
    # -Wno-objc-signed-char-bool-implicit-float-conversion
    # -Wno-objc-signed-char-bool-implicit-int-conversion
    # -Wno-openmp
    # -Wno-openmp-mapping
    # -Wno-pointer-compare
    # -Wno-poison-system-directories
    # -Wno-range-loop-bind-reference
    # -Wno-range-loop-construct
    # -Wno-reorder-ctor
    # -Wno-reorder-init-list
    # -Wno-requires-expression
    # -Wno-signed-unsigned-wchar
    # -Wno-sizeof-array-div
    # -Wno-stack-exhausted
    # -Wno-tautological-bitwise-compare
    # -Wno-unknown-cuda-version
    # -Wno-xor-used-as-pow
  )

  set(CLANG_10_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_10_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED)
  set(CLANG_10_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_11_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED
    # -Wno-c2x-extensions
    # -Wno-class-conversion
    # -Wno-cmse-union-leak
    # -Wno-dtor-name
    # -Wno-dtor-typedef
    # -Wno-elaborated-enum-base
    # -Wno-elaborated-enum-class
    # -Wno-excess-initializers
    # -Wno-fixed-point-overflow
    # -Wno-frame-address
    # -Wno-global-isel
    # -Wno-implicit-const-int-float-conversion
    # -Wno-inline-namespace-reopened-noninline
    # -Wno-max-tokens
    # -Wno-non-c-typedef-for-linkage
    # -Wno-non-power-of-two-alignment
    # -Wno-objc-dictionary-duplicate-keys
    # -Wno-pointer-to-enum-cast
    # -Wno-potentially-direct-selector
    # -Wno-psabi
    # -Wno-rewrite-not-bool
    # -Wno-slh-asm-goto
    # -Wno-strict-potentially-direct-selector
    # -Wno-suggest-destructor-override
    # -Wno-suggest-override
    # -Wno-undef-prefix
    # -Wno-uninitialized-const-reference
    # -Wno-unsupported-floating-point-opt
    # -Wno-void-pointer-to-enum-cast
    # -Wno-void-pointer-to-int-cast
    # -Wno-wasm-exception-spec
  )

  set(CLANG_11_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED
    -Wno-asm-ignored-qualifier # Deprecated at clang-11.0.0
    -Wno-experimental-isel # Deprecated at clang-11.0.0
  )

  set(CLANG_11_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED)
  set(CLANG_11_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_12_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED
    # -Wno-called-once-parameter
    # -Wno-completion-handler
    # -Wno-compound-token-split
    # -Wno-compound-token-split-by-macro
    # -Wno-compound-token-split-by-space
    # -Wno-format-insufficient-args
    # -Wno-free-nonheap-object
    # -Wno-fuse-ld-path
    # -Wno-misspelled-assumption
    # -Wno-rtti
    # -Wno-string-concatenation
    # -Wno-swift-name-attribute
    # -Wno-tautological-value-range-compare
    # -Wno-tcb-enforcement
    # -Wno-unknown-assumption
  )

  set(CLANG_12_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED
    -Wno-misexpect # Deprecated at clang-12.0.0
  )

  set(CLANG_12_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED)
  set(CLANG_12_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_13_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED
    ${WARN_FRAME_LARGER_THAN}
    # -Wno-aix-compat
    # -Wno-align-mismatch
    # -Wno-c++2b-extensions
    # -Wno-cast-function-type
    # -Wno-cxx-attribute-extension
    # -Wno-deprecated-altivec-src-compat
    # -Wno-deprecated-copy-with-dtor
    # -Wno-deprecated-copy-with-user-provided-copy
    # -Wno-deprecated-copy-with-user-provided-dtor
    # -Wno-ignored-availability-without-sdk-settings
    # -Wno-interrupt-service-routine
    # -Wno-microsoft-abstract
    # -Wno-microsoft-static-assert
    # -Wno-null-pointer-subtraction
    # -Wno-openmp-51-extensions
    # -Wno-pre-c++14-compat
    # -Wno-pre-c++14-compat-pedantic
    # -Wno-pre-c++17-compat
    # -Wno-pre-c++17-compat-pedantic
    # -Wno-pre-c++20-compat
    # -Wno-pre-c++20-compat-pedantic
    # -Wno-pre-c++2b-compat
    # -Wno-pre-c++2b-compat-pedantic
    # -Wno-pre-c2x-compat
    # -Wno-pre-c2x-compat-pedantic
    # -Wno-pre-openmp-51-compat
    # -Wno-reserved-identifier
    # -Wno-reserved-macro-identifier
    # -Wno-source-mgr
    # -Wno-tautological-unsigned-char-zero-compare
    # -Wno-unused-but-set-parameter
    # -Wno-unused-but-set-variable
  )

  set(CLANG_13_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED
    -Wno-fallback # Deprecated at clang-13.0.0
    -Wno-return-std-move-in-c++11 # Deprecated at clang-13.0.0
  )

  set(CLANG_13_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED)
  set(CLANG_13_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_14_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED
    # -Wno-argument-undefined-behaviour
    # -Wno-attribute-warning
    # -Wno-bit-int-extension
    # -Wno-bitwise-instead-of-logical
    # -Wno-branch-protection
    # -Wno-c++14-attribute-extensions
    # -Wno-c++17-attribute-extensions
    # -Wno-c++20-attribute-extensions
    # -Wno-delimited-escape-sequence-extension
    # -Wno-deprecated-coroutine
    # -Wno-deprecated-experimental-coroutine
    # -Wno-deprecated-pragma
    # -Wno-deprecated-type
    # -Wno-final-macro
    # -Wno-future-attribute-extensions
    # -Wno-gpu-maybe-wrong-side
    # -Wno-ignored-reference-qualifiers
    # -Wno-linker-warnings
    # -Wno-pedantic-macros
    # -Wno-redundant-consteval-if
    # -Wno-restrict-expansion
    # -Wno-spirv-compat
    # -Wno-target-clones-mixed-specifiers
    # -Wno-unaligned-access
    # -Wno-unreachable-code-fallthrough
    # -Wno-unsupported-abi
  )

  set(CLANG_14_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED
    -Wno-requires-expression # Deprecated at clang-14.0.0
  )

  set(CLANG_14_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED)
  set(CLANG_14_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_14_0_2_C_AND_CXX_WARNING_FLAGS_ACTIVATED)
  set(CLANG_14_0_2_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_14_0_3_C_AND_CXX_WARNING_FLAGS_ACTIVATED)
  set(CLANG_14_0_3_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_14_0_4_C_AND_CXX_WARNING_FLAGS_ACTIVATED)
  set(CLANG_14_0_4_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_14_0_5_C_AND_CXX_WARNING_FLAGS_ACTIVATED)
  set(CLANG_14_0_5_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_14_0_6_C_AND_CXX_WARNING_FLAGS_ACTIVATED)
  set(CLANG_14_0_6_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_15_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED
    # -Wno-always-inline-coroutine
    # -Wno-array-parameter
    # -Wno-atomic-access
    # -Wno-deprecate-lax-vec-conv-all
    # -Wno-deprecated-builtins
    # -Wno-deprecated-non-prototype
    # -Wno-deprecated-static-analyzer-flag
    # -Wno-gnu-line-marker
    # -Wno-gnu-null-pointer-arithmetic
    # -Wno-gnu-pointer-arith
    # -Wno-gnu-statement-expression-from-macro-expansion
    # -Wno-hlsl-extensions
    # -Wno-invalid-utf8
    # -Wno-sarif-format-unstable
    # -Wno-unaligned-qualifier-implicit-cast
    # -Wno-unknown-directives
    # -Wno-unqualified-std-cast-call
    # -Wno-unreachable-code-generic-assoc
  )

  set(CLANG_15_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_15_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED)
  set(CLANG_15_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_15_0_2_C_AND_CXX_WARNING_FLAGS_ACTIVATED)
  set(CLANG_15_0_2_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_15_0_3_C_AND_CXX_WARNING_FLAGS_ACTIVATED)
  set(CLANG_15_0_3_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_15_0_4_C_AND_CXX_WARNING_FLAGS_ACTIVATED)
  set(CLANG_15_0_4_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_15_0_5_C_AND_CXX_WARNING_FLAGS_ACTIVATED)
  set(CLANG_15_0_5_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_15_0_6_C_AND_CXX_WARNING_FLAGS_ACTIVATED)
  set(CLANG_15_0_6_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_15_0_7_C_AND_CXX_WARNING_FLAGS_ACTIVATED)
  set(CLANG_15_0_7_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_16_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED
    # -Wno-c2x-compat
    # -Wno-cast-function-type-strict
    # -Wno-coro-non-aligned-allocation-funciton
    # -Wno-deprecated-module-ts
    # -Wno-enum-constexpr-conversion
    # -Wno-gnu-offsetof-extensions
    # -Wno-incompatible-function-pointer-types-strict
    # -Wno-mathematical-notation-identifier-extension
    # -Wno-packed-non-pod
    # -Wno-read-only-types
    # -Wno-return-local-addr
    # -Wno-single-bit-bitfield-constant-conversion
    -Wno-unsafe-buffer-usage
  )

  set(CLANG_16_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED
    -Wno-concepts-ts-compat # Deprecated at clang-16.0.0
  )

  set(CLANG_16_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED)
  set(CLANG_16_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_16_0_2_C_AND_CXX_WARNING_FLAGS_ACTIVATED)
  set(CLANG_16_0_2_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_16_0_3_C_AND_CXX_WARNING_FLAGS_ACTIVATED)
  set(CLANG_16_0_3_C_AND_CXX_WARNING_FLAGS_DEPRECATED)
  set(CLANG_17_0_0_TRUNK_C_AND_CXX_WARNING_FLAGS_ACTIVATED
    # -Wno-c++23-default-comp-relaxed-constexpr
    # -Wno-c++23-extensions
    # -Wno-c++26-extensions
    # -Wno-c++2c-extensions
    # -Wno-coro-non-aligned-allocation-function
    # -Wno-dxil-validation
    # -Wno-eager-load-cxx-named-modules
    # -Wno-hip-omp-target-directives
    # -Wno-microsoft-init-from-predefined
    # -Wno-pre-c++23-compat
    # -Wno-pre-c++23-compat-pedantic
    # -Wno-pre-c++26-compat
    # -Wno-pre-c++26-compat-pedantic
    # -Wno-pre-c++2c-compat
    # -Wno-pre-c++2c-compat-pedantic
    # -Wno-reserved-module-identifier
    # -Wno-sync-alignment
  )

  set(CLANG_17_0_0_TRUNK_C_AND_CXX_WARNING_FLAGS_DEPRECATED
    -Wno-coro-non-aligned-allocation-funciton # Deprecated at clang-17.0.0-trunk
    -Wno-deprecated-experimental-coroutine # Deprecated at clang-17.0.0-trunk
    -Wno-deprecated-module-ts # Deprecated at clang-17.0.0-trunk
    -Wno-gnu-empty-initializer # Deprecated at clang-17.0.0-trunk
    -Wno-pre-c++2b-compat # Deprecated at clang-17.0.0-trunk
    -Wno-pre-c++2b-compat-pedantic # Deprecated at clang-17.0.0-trunk
  )

  # Adding flags depending on version.
  if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CLANG_CXX_WARNING_FLAGS_ACTIVATED)
    set(CLANG_CXX_WARNING_FLAGS_DEPRECATED)
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 4.0.0)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_4_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_4_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 4.0.1)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_4_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_4_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 5.0.0)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_5_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_5_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 5.0.1)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_5_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_5_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 5.0.2)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_5_0_2_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_5_0_2_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 6.0.0)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_6_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_6_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 6.0.1)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_6_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_6_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 7.0.0)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_7_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_7_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 7.0.1)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_7_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_7_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8.0.0)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_8_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_8_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8.0.1)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_8_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_8_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 9.0.0)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_9_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_9_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 9.0.1)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_9_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_9_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 10.0.0)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_10_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_10_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 10.0.1)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_10_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_10_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 11.0.0)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_11_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_11_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 11.0.1)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_11_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_11_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 12.0.0)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_12_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_12_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 12.0.1)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_12_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_12_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 13.0.0)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_13_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_13_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 13.0.1)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_13_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_13_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 14.0.0)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_14_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_14_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 14.0.1)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_14_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_14_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 14.0.2)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_14_0_2_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_14_0_2_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 14.0.3)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_14_0_3_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_14_0_3_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 14.0.4)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_14_0_4_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_14_0_4_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 14.0.5)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_14_0_5_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_14_0_5_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 14.0.6)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_14_0_6_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_14_0_6_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 15.0.0)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_15_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_15_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 15.0.1)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_15_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_15_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 15.0.2)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_15_0_2_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_15_0_2_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 15.0.3)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_15_0_3_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_15_0_3_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 15.0.4)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_15_0_4_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_15_0_4_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 15.0.5)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_15_0_5_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_15_0_5_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 15.0.6)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_15_0_6_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_15_0_6_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 15.0.7)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_15_0_7_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_15_0_7_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 16.0.0)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_16_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_16_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 16.0.1)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_16_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_16_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 16.0.2)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_16_0_2_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_16_0_2_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 16.0.3)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_16_0_3_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_16_0_3_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 17.0.0)
      list(APPEND CLANG_CXX_WARNING_FLAGS_ACTIVATED ${CLANG_17_0_0_TRUNK_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_CXX_WARNING_FLAGS_DEPRECATED ${CLANG_17_0_0_TRUNK_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    set(TMP_OUTVAR_CXX ${${OUTVAR_CXX}})
    warn_everything_filter_warnings(
      CLANG_CXX_WARNING_FLAGS_ACTIVATED
      CLANG_CXX_WARNING_FLAGS_DEPRECATED
      TMP_OUTVAR_CXX
    )
    set(${OUTVAR_CXX} ${TMP_OUTVAR_CXX} PARENT_SCOPE)
  endif()

  if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(CLANG_C_WARNING_FLAGS_ACTIVATED)
    set(CLANG_C_WARNING_FLAGS_DEPRECATED)
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 4.0.0)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_4_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_4_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 4.0.1)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_4_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_4_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 5.0.0)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_5_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_5_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 5.0.1)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_5_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_5_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 5.0.2)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_5_0_2_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_5_0_2_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 6.0.0)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_6_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_6_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 6.0.1)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_6_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_6_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 7.0.0)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_7_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_7_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 7.0.1)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_7_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_7_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 8.0.0)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_8_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_8_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 8.0.1)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_8_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_8_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 9.0.0)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_9_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_9_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 9.0.1)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_9_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_9_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 10.0.0)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_10_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_10_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 10.0.1)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_10_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_10_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 11.0.0)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_11_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_11_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 11.0.1)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_11_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_11_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 12.0.0)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_12_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_12_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 12.0.1)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_12_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_12_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 13.0.0)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_13_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_13_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 13.0.1)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_13_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_13_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 14.0.0)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_14_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_14_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 14.0.1)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_14_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_14_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 14.0.2)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_14_0_2_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_14_0_2_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 14.0.3)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_14_0_3_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_14_0_3_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 14.0.4)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_14_0_4_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_14_0_4_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 14.0.5)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_14_0_5_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_14_0_5_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 14.0.6)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_14_0_6_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_14_0_6_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 15.0.0)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_15_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_15_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 15.0.1)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_15_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_15_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 15.0.2)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_15_0_2_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_15_0_2_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 15.0.3)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_15_0_3_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_15_0_3_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 15.0.4)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_15_0_4_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_15_0_4_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 15.0.5)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_15_0_5_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_15_0_5_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 15.0.6)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_15_0_6_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_15_0_6_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 15.0.7)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_15_0_7_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_15_0_7_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 16.0.0)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_16_0_0_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_16_0_0_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 16.0.1)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_16_0_1_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_16_0_1_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 16.0.2)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_16_0_2_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_16_0_2_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 16.0.3)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_16_0_3_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_16_0_3_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 17.0.0)
      list(APPEND CLANG_C_WARNING_FLAGS_ACTIVATED ${CLANG_17_0_0_TRUNK_C_AND_CXX_WARNING_FLAGS_ACTIVATED})
      list(APPEND CLANG_C_WARNING_FLAGS_DEPRECATED ${CLANG_17_0_0_TRUNK_C_AND_CXX_WARNING_FLAGS_DEPRECATED})
    endif()
    set(TMP_OUTVAR_C ${${OUTVAR_C}})
    warn_everything_filter_warnings(
      CLANG_C_WARNING_FLAGS_ACTIVATED
      CLANG_C_WARNING_FLAGS_DEPRECATED
      TMP_OUTVAR_C
    )
    set(${OUTVAR_C} ${TMP_OUTVAR_C} PARENT_SCOPE)
  endif()
endfunction()
