# this project is part of the vrt project; licensed under the MIT license. see LICENSE for more info

cmake_minimum_required(VERSION 3.20)
project(libvrt
        VERSION 1.0.0
        DESCRIPTION "Type-safe variant with switch case support and small storage optimization"
        LANGUAGES CXX
)

option(LIBVRT_BUILD_TESTS "Build tests" ON)
option(LIBVRT_BUILD_BENCHMARKS "Build benchmarks" ON)
option(LIBVRT_INSTALL "Enable installation" ON)

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

    add_executable(vrt-tests tests/variant.cpp)
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
    target_link_libraries(vrt-benchmarks
            PRIVATE
            vrt::vrt
            benchmark::benchmark
    )

    target_compile_options(vrt-benchmarks PRIVATE -O3 -DNDEBUG)
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
message(STATUS "  Install: ${LIBVRT_INSTALL}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
