cmake_minimum_required(VERSION 3.16..3.25)

list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

###
### Project
###

project(libbinio VERSION 1.5)

if(CMAKE_VERSION VERSION_LESS 3.21)
  # Set "top level" if the current CMake is too old to do so in project().
  string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" libbinio_IS_TOP_LEVEL)
endif()

###
### C++ Language
###

if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
if(NOT DEFINED CMAKE_CXX_STANDARD_REQUIRED)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()
if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD OFF)
endif()

add_compile_definitions($<$<CONFIG:Debug>:DEBUG>)

if(MSVC)
  # Tell MSVC to use UTF-8 when reading the source code and when generating the
  # object file.  Otherwise, both the compiler's input and output can vary
  # depending on the current user code page.  (MSVC always uses UTF-8
  # internally.)
  # https://learn.microsoft.com/en-us/cpp/build/reference/utf-8-set-source-and-executable-character-sets-to-utf-8
  add_compile_options("/utf-8")
endif()

###
### Library type
###

if(DEFINED libbinio_BUILD_SHARED_LIBS)
  set(BUILD_SHARED_LIBS "${libbinio_BUILD_SHARED_LIBS}")
endif ()

if(BUILD_SHARED_LIBS AND NOT DEFINED CMAKE_INTERPROCEDURAL_OPTIMIZATION)
  message(STATUS "Enabling interprocedural optimization")
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)
endif()


if(WIN32)
  if(BUILD_SHARED_LIBS)
    set(CMAKE_DEBUG_POSTFIX "d")
  else()
    set(CMAKE_RELEASE_POSTFIX "_static")
    set(CMAKE_RELWITHDEBINFO_POSTFIX "_static")
    set(CMAKE_DEBUG_POSTFIX "_staticd")
  endif()
endif()

###
### Configuration checks
###

include(CheckTypeSize)
include(CheckCXXSourceCompiles)
include(CheckLibraryExists)

# Use a function for the configuration to keep the any variables in their own
# local scope (e.g., CMAKE_REQUIRED_LIBRARIES).
function(configure_libbinio)
  if(UNIX)
    check_library_exists(m sin "" HAVE_LIB_M)
    if(HAVE_LIB_M)
      list(APPEND CMAKE_REQUIRED_LIBRARIES m)
      set(HAVE_LIB_M 1 PARENT_SCOPE)
    endif()
  endif()

  check_type_size("long long" SIZEOF_LONG_LONG)
  if(HAVE_SIZEOF_LONG_LONG)
    set(TYPE_INT "long long" CACHE STRING "Integer type")
  else()
    set(TYPE_INT "long" CACHE STRING "Integer type")
  endif()

  check_type_size("long double" SIZEOF_LONG_DOUBLE)
  if(HAVE_SIZEOF_LONG_DOUBLE)
    set(TYPE_FLOAT "long double" CACHE STRING "Floating point type")
  else()
    set(TYPE_FLOAT "double" CACHE STRING "Floating point type")
  endif()

  check_cxx_source_compiles(
    "
    #include <math.h>
    int main(int, char**)
    { (void)ldexp(1.0, 5); }
    "
    HAVE_LDEXP)
  if(NOT HAVE_LDEXP)
    set(HAVE_LDEXP 0)
  endif()

  include(TestForANSIStreamHeaders)

  if(${CMAKE_NO_ANSI_STREAM_HEADERS})
    set(HAVE_STREAMS 0)
  else()
    set(HAVE_STREAMS 1)
  endif()

  set(ENABLE_IOSTREAM ${HAVE_STREAMS} CACHE STRING "Enable IOStream")
  set(ENABLE_STRING 1 CACHE STRING "Enable string")
  set(ISO_STDLIB ${HAVE_STREAMS} CACHE STRING "Use ISO stdlib")
  set(WITH_MATH ${HAVE_LDEXP} CACHE STRING "Use math.h")
endfunction()

configure_libbinio()

###
### Core
###

add_subdirectory(doc)
add_subdirectory(src)

option(libbinio_INCLUDE_PACKAGING "Include packaging rules for libbinio" "${libbinio_IS_TOP_LEVEL}")

if(libbinio_INCLUDE_PACKAGING)
  add_subdirectory(packaging)
endif()
