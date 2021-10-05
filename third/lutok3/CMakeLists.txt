cmake_minimum_required (VERSION 3.8.0)
project (lutok3)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++latest")
    endif()
endif()

add_subdirectory(src)
add_subdirectory(test)
