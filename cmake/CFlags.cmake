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

# Extra security / hardening flags for executables
set(EXTRA_SECURITY_CFLAGS_EXE
    -fstack-clash-protection
    -fPIE
    -pie
    -Wl,-z,relro
    -Wl,-z,now
    -Wl,-z,noexecstack
    -fno-plt
)

# Extra security / hardening flags for shared libraries
set(EXTRA_SECURITY_CFLAGS_SO
    -fstack-clash-protection
    -fPIC
    -Wl,-z,relro
    -Wl,-z,now
    -Wl,-z,noexecstack
    -fno-plt
)

# Extra security / hardening flags for static libraries (no linker flags, no LTO)
set(EXTRA_SECURITY_CFLAGS_STATIC
    -fstack-clash-protection
    -fPIC
    -fno-plt
    -fno-lto
)

# Add architecture-specific flags
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64")
    list(APPEND EXTRA_SECURITY_CFLAGS_EXE -fcf-protection=full)
    list(APPEND EXTRA_SECURITY_CFLAGS_SO -fcf-protection=full)
    list(APPEND EXTRA_SECURITY_CFLAGS_STATIC -fcf-protection=full)
endif()

# Build-type dependent flags
set(DEBUG_CFLAGS
    -Og -g
)

set(RELEASE_CFLAGS
    -O2
    -s
)

set(FEATURE_FLAGS
    -D_XOPEN_SOURCE=500
    -D_DEFAULT_SOURCE
)

# Add _FORTIFY_SOURCE if not already defined by the build system
# Check if _FORTIFY_SOURCE is already in the compiler flags
string(FIND "${CMAKE_C_FLAGS}" "_FORTIFY_SOURCE" FORTIFY_POS)
if(FORTIFY_POS EQUAL -1)
    list(APPEND FEATURE_FLAGS -D_FORTIFY_SOURCE=2)
endif()

### Combine all flags for executables
set(TDNF_CFLAGS_EXE
    ${WARN_CFLAGS}
    ${OPTIMIZE_CFLAGS}
    ${SECURITY_CFLAGS}
    ${EXTRA_WARN_CFLAGS}
    ${EXTRA_SECURITY_CFLAGS_EXE}
    ${FEATURE_FLAGS}
)

### Combine all flags for shared libraries
set(TDNF_CFLAGS_SO
    ${WARN_CFLAGS}
    ${OPTIMIZE_CFLAGS}
    ${SECURITY_CFLAGS}
    ${EXTRA_WARN_CFLAGS}
    ${EXTRA_SECURITY_CFLAGS_SO}
    ${FEATURE_FLAGS}
)

### Combine all flags for static libraries
set(TDNF_CFLAGS_STATIC
    ${WARN_CFLAGS}
    ${OPTIMIZE_CFLAGS}
    ${SECURITY_CFLAGS}
    ${EXTRA_WARN_CFLAGS}
    ${EXTRA_SECURITY_CFLAGS_STATIC}
    ${FEATURE_FLAGS}
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND TDNF_CFLAGS_EXE ${DEBUG_CFLAGS})
    list(APPEND TDNF_CFLAGS_SO ${DEBUG_CFLAGS})
    list(APPEND TDNF_CFLAGS_STATIC ${DEBUG_CFLAGS})
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    list(APPEND TDNF_CFLAGS_EXE ${RELEASE_CFLAGS})
    list(APPEND TDNF_CFLAGS_SO ${RELEASE_CFLAGS})
    list(APPEND TDNF_CFLAGS_STATIC ${RELEASE_CFLAGS})
endif()

# Apply flags to executables by default
foreach(flag IN LISTS TDNF_CFLAGS_EXE)
    add_c_compiler_flag(${flag})
endforeach()

# Function to apply appropriate flags based on target type
function(apply_tdnf_flags target_name target_type)
    if(target_type STREQUAL "SHARED")
        target_compile_options(${target_name} PRIVATE ${TDNF_CFLAGS_SO})
    elseif(target_type STREQUAL "STATIC")
        target_compile_options(${target_name} PRIVATE ${TDNF_CFLAGS_STATIC})
    else()
        target_compile_options(${target_name} PRIVATE ${TDNF_CFLAGS_EXE})
    endif()
endfunction()
