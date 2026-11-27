set(WARN_CFLAGS
    -Wall
    -Wundef
    -Wstrict-prototypes
    -Wno-trigraphs
    -Werror-implicit-function-declaration
    -Wdeclaration-after-statement
    -Wvla
    -Wno-format-security
    -Wno-sign-compare
)

if(APPLE)
    # Add macOS-specific warning suppressions for external headers
    list(APPEND WARN_CFLAGS -Wno-unused-parameter)
endif()

set(OPTIMIZE_CFLAGS
    -O2
    -fno-strict-aliasing
    -fno-common
    -fno-delete-null-pointer-checks
)

set(SECURITY_CFLAGS
    -fstack-protector-strong
)

set(EXTRA_WARN_CFLAGS
    # General extra warnings
    -Wextra -Werror -Wformat=2 -Wshadow

    # Prototypes & declarations
    -Wmissing-prototypes -Wold-style-definition -Wmissing-declarations -Wredundant-decls

    # Type & cast issues
    -Wcast-align -Wpointer-arith -Wwrite-strings

    # Switch & logic correctness
    -Wlogical-op -Waggregate-return -Winit-self

    # Flow / duplication warnings
    -Wduplicated-cond -Wduplicated-branches -Wnull-dereference -Wjump-misses-init

    # Format & string issues
    -Wformat-overflow=2 -Wformat-truncation=2 -Wstringop-overflow=4

    # Allocation & stack safety
    -Walloc-zero -Walloca -Wtrampolines
)

# Extra security / hardening flags
set(EXTRA_SECURITY_CFLAGS
    -D_FORTIFY_SOURCE=2
    -fstack-clash-protection
    -fcf-protection=full
    -fPIE
    -pie
    -Wl,-z,relro
    -Wl,-z,now
    -Wl,-z,noexecstack
    -fno-plt
)

# Build-type dependent flags
set(DEBUG_CFLAGS
    -Og -g
)

set(RELEASE_CFLAGS
    -O2
    -s
)

if(APPLE)
    set(FEATURE_FLAGS
        -D_DARWIN_C_SOURCE
    )
else()
    set(FEATURE_FLAGS
        -D_XOPEN_SOURCE=500
        -D_DEFAULT_SOURCE
    )
endif()

### Combine all flags
if(APPLE)
    # On macOS, skip GCC-specific flags that don't work with Clang
    set(TDNF_CFLAGS
        ${WARN_CFLAGS}
        ${OPTIMIZE_CFLAGS}
        ${SECURITY_CFLAGS}
        ${FEATURE_FLAGS}
    )
else()
    set(TDNF_CFLAGS
        ${WARN_CFLAGS}
        ${OPTIMIZE_CFLAGS}
        ${SECURITY_CFLAGS}
        ${EXTRA_WARN_CFLAGS}
        ${EXTRA_SECURITY_CFLAGS}
        ${FEATURE_FLAGS}
    )
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND TDNF_CFLAGS ${DEBUG_CFLAGS})
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    list(APPEND TDNF_CFLAGS ${RELEASE_CFLAGS})
endif()

foreach(flag IN LISTS TDNF_CFLAGS)
    add_c_compiler_flag(${flag})
endforeach()
