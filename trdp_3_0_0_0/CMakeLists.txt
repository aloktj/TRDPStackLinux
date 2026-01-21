cmake_minimum_required(VERSION 3.16)

project(TRDPStack C)

option(TRDP_BUILD_EXAMPLES "Build TRDP example applications" OFF)
option(TRDP_BUILD_TESTS "Build TRDP gtest-based tests" OFF)
option(TRDP_ENABLE_MD "Enable MD support" ON)
option(TRDP_FULL_BUILD "Enable full TRDP build (XML/marshalling/etc.)" OFF)
option(TRDP_ENABLE_SOA "Enable service orientation support" OFF)
option(TRDP_ENABLE_TSN "Enable TSN support" OFF)
option(TRDP_HIGH_PERF_INDEXED "Enable high performance indexed PD" OFF)
option(TRDP_HIGH_PERF_BASE2 "Use base-2 cycle time for high performance PD" OFF)
option(TRDP_PD_UNICAST_SUPPORT "Enable PD unicast support" OFF)
option(TRDP_RT_THREADS "Enable realtime threads" OFF)
option(TRDP_USE_UUID "Enable UUID support when available" ON)

set(TRDP_VOS "posix" CACHE STRING "VOS implementation to use")

if(NOT TRDP_VOS STREQUAL "posix")
    message(FATAL_ERROR "Only TRDP_VOS=posix is supported by this CMake setup.")
endif()

set(TRDP_COMMON_DIR "${CMAKE_CURRENT_LIST_DIR}/src/common")
set(TRDP_VOS_COMMON_DIR "${CMAKE_CURRENT_LIST_DIR}/src/vos/common")
set(TRDP_VOS_DIR "${CMAKE_CURRENT_LIST_DIR}/src/vos/${TRDP_VOS}")

set(TRDP_CORE_SOURCES
    "${TRDP_COMMON_DIR}/trdp_pdcom.c"
    "${TRDP_COMMON_DIR}/trdp_utils.c"
    "${TRDP_COMMON_DIR}/tlp_if.c"
    "${TRDP_COMMON_DIR}/tlc_if.c"
    "${TRDP_COMMON_DIR}/trdp_stats.c"
    "${TRDP_VOS_COMMON_DIR}/vos_utils.c"
    "${TRDP_VOS_COMMON_DIR}/vos_mem.c"
    "${TRDP_VOS_DIR}/vos_sock.c"
    "${TRDP_VOS_DIR}/vos_thread.c"
    "${TRDP_VOS_DIR}/vos_shared_mem.c"
)

if(TRDP_ENABLE_MD)
    list(APPEND TRDP_CORE_SOURCES
        "${TRDP_COMMON_DIR}/trdp_mdcom.c"
        "${TRDP_COMMON_DIR}/tlm_if.c"
    )
endif()

if(TRDP_FULL_BUILD)
    list(APPEND TRDP_CORE_SOURCES
        "${TRDP_COMMON_DIR}/trdp_xml.c"
        "${TRDP_COMMON_DIR}/tau_xml.c"
        "${TRDP_COMMON_DIR}/tau_marshall.c"
        "${TRDP_COMMON_DIR}/tau_dnr.c"
        "${TRDP_COMMON_DIR}/tau_tti.c"
        "${TRDP_COMMON_DIR}/tau_ctrl.c"
    )
endif()

if(TRDP_ENABLE_SOA)
    list(APPEND TRDP_CORE_SOURCES "${TRDP_COMMON_DIR}/tau_so_if.c")
endif()

if(TRDP_HIGH_PERF_INDEXED)
    list(APPEND TRDP_CORE_SOURCES "${TRDP_COMMON_DIR}/trdp_pdindex.c")
endif()

if(TRDP_ENABLE_TSN)
    list(APPEND TRDP_CORE_SOURCES "${TRDP_VOS_DIR}/vos_sockTSN.c")
endif()

add_library(trdp ${TRDP_CORE_SOURCES})
add_library(trdp::trdp ALIAS trdp)

set_target_properties(trdp PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED YES
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(trdp PRIVATE LINUX)
endif()

target_compile_definitions(trdp PRIVATE POSIX)

if(TRDP_ENABLE_MD)
    target_compile_definitions(trdp PRIVATE MD_SUPPORT=1)
else()
    target_compile_definitions(trdp PRIVATE MD_SUPPORT=0)
endif()

if(TRDP_ENABLE_SOA)
    target_compile_definitions(trdp PRIVATE SOA_SUPPORT)
endif()

if(TRDP_ENABLE_TSN)
    target_compile_definitions(trdp PRIVATE TSN_SUPPORT RT_THREADS)
endif()

if(TRDP_RT_THREADS)
    target_compile_definitions(trdp PRIVATE RT_THREADS)
endif()

if(TRDP_HIGH_PERF_INDEXED)
    target_compile_definitions(trdp PRIVATE HIGH_PERF_INDEXED)
endif()

if(TRDP_HIGH_PERF_BASE2)
    target_compile_definitions(trdp PRIVATE HIGH_PERF_BASE2)
endif()

if(TRDP_PD_UNICAST_SUPPORT)
    target_compile_definitions(trdp PRIVATE PD_UNICAST_SUPPORT)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(trdp PRIVATE _GNU_SOURCE)
endif()

target_include_directories(trdp
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/src/api>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/src/vos/api>"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/src/common"
        "${CMAKE_CURRENT_LIST_DIR}/src/vos/common"
        "${CMAKE_CURRENT_LIST_DIR}/src/vos/${TRDP_VOS}"
)

find_package(Threads REQUIRED)
target_link_libraries(trdp PRIVATE Threads::Threads)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(trdp PRIVATE rt)
endif()

if(TRDP_USE_UUID)
    find_library(TRDP_UUID_LIBRARY uuid)
    if(TRDP_UUID_LIBRARY)
        target_compile_definitions(trdp PRIVATE HAS_UUID)
        target_link_libraries(trdp PRIVATE ${TRDP_UUID_LIBRARY})
    else()
        message(WARNING "libuuid not found; building without HAS_UUID support")
    endif()
endif()

if(TRDP_BUILD_EXAMPLES)
    set(TRDP_EXAMPLE_DIR "${CMAKE_CURRENT_LIST_DIR}/example")

    add_executable(trdp_echoCallback "${TRDP_EXAMPLE_DIR}/echoCallback.c")
    target_link_libraries(trdp_echoCallback PRIVATE trdp)

    add_executable(trdp_receivePolling "${TRDP_EXAMPLE_DIR}/echoPolling.c")
    target_link_libraries(trdp_receivePolling PRIVATE trdp)

    add_executable(trdp_sendHello "${TRDP_EXAMPLE_DIR}/sendHello.c")
    target_link_libraries(trdp_sendHello PRIVATE trdp)

    add_executable(trdp_receiveHello "${TRDP_EXAMPLE_DIR}/receiveHello.c")
    target_link_libraries(trdp_receiveHello PRIVATE trdp)

    add_executable(trdp_sendData "${TRDP_EXAMPLE_DIR}/sendData.c")
    target_link_libraries(trdp_sendData PRIVATE trdp)

    add_executable(trdp_sourceFiltering "${TRDP_EXAMPLE_DIR}/sourceFilter/sourceFiltering.c")
    target_include_directories(trdp_sourceFiltering PRIVATE "${TRDP_EXAMPLE_DIR}/sourceFilter")
    target_link_libraries(trdp_sourceFiltering PRIVATE trdp)
endif()

if(TRDP_BUILD_TESTS)
    include(FetchContent)

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.13.0.zip
    )
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_executable(trdp_api_gtest "test/gtest/trdp_api_gtest.cpp")
    target_link_libraries(trdp_api_gtest PRIVATE trdp GTest::gtest_main)
    add_test(NAME trdp_api_gtest COMMAND trdp_api_gtest)
endif()
