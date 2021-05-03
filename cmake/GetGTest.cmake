find_package(Threads REQUIRED)

include(ExternalProject)
ExternalProject_Add(
    GTestProject
    URL https://github.com/google/googletest/archive/refs/tags/release-1.10.0.zip
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/gtest_source
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/gtest_binary
    CMAKE_ARGS "-Dgtest_force_shared_crt=ON"
    INSTALL_COMMAND  ""
    )
add_library(GTest INTERFACE)
target_link_directories(
    GTest INTERFACE
    ${CMAKE_CURRENT_BINARY_DIR}/gtest_binary
    ${CMAKE_CURRENT_BINARY_DIR}/gtest_binary/googlemock/gtest/${CMAKE_BUILD_TYPE}
    ${CMAKE_CURRENT_BINARY_DIR}/gtest_binary/googlemock/gtest
    )

target_include_directories(GTest SYSTEM INTERFACE
    ${CMAKE_CURRENT_BINARY_DIR}/gtest_source/googletest/include
    )

target_link_libraries(
    GTest INTERFACE
    gtest$<$<AND:$<OR:$<PLATFORM_ID:Windows>,$<AND:$<PLATFORM_ID:Darwin>,$<CXX_COMPILER_ID:AppleClang>>>,$<CONFIG:Debug>>:d>
    ${THREADS_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    )

add_dependencies(GTest GTestProject)
