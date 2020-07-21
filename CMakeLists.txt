cmake_minimum_required(VERSION 3.13)
project(hwcinstr)
enable_language(CXX)

find_package(PkgConfig REQUIRED)

find_package(LLVM CONFIG REQUIRED)
message(STATUS "LLVM: ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM include dir: " ${LLVM_INCLUDE_DIR})
message(STATUS "LLVM lib dir: ${LLVM_LIBRARY_DIR}")

find_package(Clang CONFIG REQUIRED)
message(STATUS "clang include dir: ${CLANG_INCLUDE_DIRS}")
message(STATUS "clang lib dir: ${Clang_DIR}")

pkg_check_modules(PAPI papi REQUIRED)
message(STATUS "PAPI include dir: ${PAPI_INCLUDEDIR}")
message(STATUS "PAPI lib dir: ${PAPI_LIBDIR}")

pkg_check_modules(OPENSSL openssl REQUIRED)
message(STATUS "openssl include dir: ${OPENSSL_INCLUDEDIR}")
message(STATUS "openssl lib dir: ${OPENSSL_LIBDIR}")

pkg_check_modules(LIBYAML yaml-0.1 REQUIRED)
message(STATUS "libyaml include dir: ${LIBYAML_INCLUDEDIR}")
message(STATUS "libyaml lib dir; ${LIBYAML_LIBDIR}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_compile_options(-std=c++17 -Wall -Wextra -fPIC)

set(CMAKE_BUILD_RPATH ${LLVM_LIBRARY_DIR} ${PAPI_LIBDIR})
set(CMAKE_INSTALL_RPATH ${LLVM_LIBRARY_DIR} ${PAPI_LIBDIR})

set(CMAKE_PROJECT_BINDIR "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_PROJECT_LIBDIR "${PROJECT_BINARY_DIR}/lib")

# Determine the Clang and GCC executables to use
set(CLANG ${LLVM_TOOLS_BINARY_DIR}/clang)
set(CLANGXX ${LLVM_TOOLS_BINARY_DIR}/clang++)

add_subdirectory(cfe)
add_subdirectory(rt)
add_subdirectory(drivers)
