cmake_minimum_required(VERSION 3.0)

project(musl-sem-ext C CXX)

include(GNUInstallDirs)

# Tests
enable_testing()
find_package(GTest REQUIRED)

# musl requires gcc
if(NOT CMAKE_COMPILER_IS_GNUCC)
    message(FATAL_ERROR "musl requires compiler to be gcc")
endif()

# Get target
execute_process(
    COMMAND "${CMAKE_C_COMPILER}" -dumpmachine
    RESULT_VARIABLE result
    OUTPUT_VARIABLE gcc_target
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Failed to get compile target of ${CMAKE_C_COMPILER}")
endif()

# Translate to architecture
if("${gcc_target}" MATCHES "^arm")
    set(arch arm)
elseif("${gcc_target}" MATCHES "^aarch64")
    set(arch aarch64)
elseif("${gcc_target}" MATCHES "^i.86-nt32")
    set(arch nt32)
elseif("${gcc_target}" MATCHES "^i.86")
    set(arch i386)
elseif("${gcc_target}" MATCHES "^(x86_64-x32|x32|x86_64.*x32)")
    set(arch x32)
elseif("${gcc_target}" MATCHES "^x86_64-nt64")
    set(arch nt64)
elseif("${gcc_target}" MATCHES "^x86_64")
    set(arch x86_64)
elseif("${gcc_target}" MATCHES "^(mips64|mipsisa64)")
    set(arch mips64)
elseif("${gcc_target}" MATCHES "^mips")
    set(arch mips)
elseif("${gcc_target}" MATCHES "^microblaze")
    set(arch microblaze)
elseif("${gcc_target}" MATCHES "^or1k")
    set(arch or1k)
elseif("${gcc_target}" MATCHES "^powerpc64")
    set(arch powerpc64)
elseif("${gcc_target}" MATCHES "^powerpc")
    set(arch powerpc)
elseif("${gcc_target}" MATCHES "^(sh[1-9bel-]|sh$|superh)")
    set(arch sh)
elseif("${gcc_target}" MATCHES "^s390x")
    set(arch s390x)
elseif("${gcc_target}" STREQUAL "unknown")
    message(FATAL_ERROR "Failed to detect target architecture")
else()
    message(FATAL_ERROR "Unsupported target: ${gcc_target}")
endif()

add_library(
    musl-sem-ext SHARED
    # Sources
    src/__lock.c
    src/__timedwait.c
    src/sem_destroy.c
    src/sem_getvalue.c
    src/sem_init.c
    src/sem_open.c
    src/sem_post.c
    src/sem_timedwait.c
    src/sem_trywait.c
    src/sem_unlink.c
    src/sem_wait.c
    src/shim.c
)

target_include_directories(
    musl-sem-ext
    PUBLIC include
    PRIVATE include/arch/${arch}
)

target_compile_options(
    musl-sem-ext
    PRIVATE
    # Enable warnings
    -Wall -Wextra -Werror -pedantic
    # gcc doesn't consider "syscall(...) == value" as using the result
    -Wno-error=unused-value
    # For __timedwait_cp()
    -Wno-error=sign-compare
    # Hide symbols by default
    -fvisibility=hidden
    # Make ELF output smaller
    -ffunction-sections -fdata-sections
)

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections")

target_compile_definitions(
    musl-sem-ext
    PRIVATE
    -D_GNU_SOURCE
    -DMUSL_SEM_BUILD
)

target_link_libraries(
    musl-sem-ext
    pthread
    rt
)

set_target_properties(
    musl-sem-ext
    PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED 1
    POSITION_INDEPENDENT_CODE 1
)

install(
    TARGETS musl-sem-ext
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Libraries
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Libraries
    #ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Libraries
)

add_executable(
    musl-sem-ext-test
    test/main.cpp
    test/test_env.cpp
    test/test_sem.cpp
)

target_include_directories(
    musl-sem-ext-test
    PRIVATE
    ${GTEST_INCLUDE_DIRS}
)

target_link_libraries(
    musl-sem-ext-test
    musl-sem-ext
    ${GTEST_BOTH_LIBRARIES}
)

set_target_properties(
    musl-sem-ext-test
    PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED 1
    POSITION_INDEPENDENT_CODE 1
)

add_test(
    NAME musl-sem-ext-test
    COMMAND musl-sem-ext-test
)
