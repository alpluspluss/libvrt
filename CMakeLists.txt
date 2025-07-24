# this project is part of the vrt project; licensed under the MIT license. see LICENSE for more info

cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0167 NEW)

project(libvrt
        VERSION 0.1.1
        DESCRIPTION "Make variants switchable safely again"
        LANGUAGES CXX
)

option(LIBVRT_BUILD_TESTS "Build tests" ON)
option(LIBVRT_BUILD_BENCHMARKS "Build benchmarks" ON)
option(LIBVRT_INSTALL "Enable installation" ON)
option(LIBVRT_BENCHMARK_BOOST "Include Boost variants in benchmarks" ON)

add_library(vrt INTERFACE)
add_library(vrt::vrt ALIAS vrt)

target_compile_features(vrt INTERFACE cxx_std_20)

target_include_directories(vrt INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

if(LIBVRT_BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)

    add_executable(vrt-tests
            tests/access.cpp
            tests/assignment.cpp
            tests/cmp.cpp
            tests/construction.cpp
            tests/fun-mechanism.cpp
            tests/utility.cpp
            tests/visitor.cpp
    )
    target_link_libraries(vrt-tests PRIVATE
            vrt::vrt
            GTest::gtest
            GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(vrt-tests)
endif()

if(LIBVRT_BUILD_BENCHMARKS)
    find_package(benchmark REQUIRED)

    add_executable(vrt-benchmarks benches/variant.cpp)
    target_link_libraries(vrt-benchmarks PRIVATE vrt::vrt benchmark::benchmark)

    target_compile_options(vrt-benchmarks PRIVATE -O3 -DNDEBUG -march=native)

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(vrt-benchmarks PRIVATE -fno-omit-frame-pointer)
    endif()

    if(LIBVRT_BENCHMARK_BOOST)
        find_package(Boost QUIET)
        set(BOOST_VARIANT_FOUND FALSE)

        if(Boost_FOUND)
            target_link_libraries(vrt-benchmarks PRIVATE Boost::boost)
            set(BOOST_VARIANT_FOUND TRUE)
            message(STATUS "Boost found")
            target_compile_definitions(vrt-benchmarks PRIVATE HAS_BOOST_VARIANT HAS_BOOST_VARIANT2)
        else()
            find_path(BOOST_INCLUDE_DIR boost/variant.hpp)
            if(BOOST_INCLUDE_DIR)
                target_include_directories(vrt-benchmarks PRIVATE ${BOOST_INCLUDE_DIR})
                set(BOOST_VARIANT_FOUND TRUE)
                message(STATUS "Boost found (header-only) - assuming both variants available")
                target_compile_definitions(vrt-benchmarks PRIVATE HAS_BOOST_VARIANT HAS_BOOST_VARIANT2)
            else()
                message(WARNING "Boost not found, realistic benchmarks will be limited")
            endif()
        endif()
    endif()
endif()

if(LIBVRT_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    install(FILES include/vrt
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(TARGETS vrt
            EXPORT vrtTargets
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(EXPORT vrtTargets
            FILE vrtTargets.cmake
            NAMESPACE vrt::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vrt
    )

    configure_package_config_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/cmake/vrtConfig.cmake.in
            ${CMAKE_CURRENT_BINARY_DIR}/vrtConfig.cmake
            INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vrt
    )

    write_basic_package_version_file(
            ${CMAKE_CURRENT_BINARY_DIR}/vrtConfigVersion.cmake
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY SameMajorVersion
    )

    install(FILES
            ${CMAKE_CURRENT_BINARY_DIR}/vrtConfig.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/vrtConfigVersion.cmake
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vrt
    )

    configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/cmake/vrtConfig.cmake.in
            ${CMAKE_CURRENT_BINARY_DIR}/vrt.pc
            @ONLY
    )

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/vrt.pc
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    )
endif()

message(STATUS "libvrt configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build tests: ${LIBVRT_BUILD_TESTS}")
message(STATUS "  Build benchmarks: ${LIBVRT_BUILD_BENCHMARKS}")
if(LIBVRT_BUILD_BENCHMARKS AND LIBVRT_BENCHMARK_BOOST)
    message(STATUS "  Boost variants available: ${BOOST_VARIANT_FOUND}")
endif()
message(STATUS "  Install: ${LIBVRT_INSTALL}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
