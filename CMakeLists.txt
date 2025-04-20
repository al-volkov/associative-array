cmake_minimum_required(VERSION 3.25)

project(AssociativeArray
        VERSION 1.0
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the build type" FORCE)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

option(AA_ENABLE_WARNINGS "Enable extra warning flags" ON)
option(AA_ENABLE_ASAN "Enable Address Sanitizer" OFF)
option(AA_ENABLE_UBSAN "Enable Undefined-Behavior Sanitizer" OFF)
option(AA_ENABLE_TSAN "Enable Thread Sanitizer" OFF)
option(AA_ENABLE_LSAN "Enable Leak Sanitizer" OFF)
option(AA_ENABLE_MSAN "Enable Memory Sanitizer" OFF)

include(CheckCXXCompilerFlag)
include(GNUInstallDirs)

function(aa_enable_warnings tgt)
  if(AA_ENABLE_WARNINGS)
    if(MSVC)
      target_compile_options(${tgt} PRIVATE /W4 /WX)
    else()
      target_compile_options(${tgt} PRIVATE
        -Wall -Wextra -Wpedantic -Werror -Wno-return-type-c-linkage)
    endif()
  endif()
endfunction()

function(aa_enable_sanitizers tgt)
  if(NOT (AA_ENABLE_ASAN OR AA_ENABLE_UBSAN OR AA_ENABLE_TSAN OR AA_ENABLE_LSAN OR AA_ENABLE_MSAN))
    return()
  endif()

  set(_compile_flags "")
  set(_link_flags    "")

  if(AA_ENABLE_ASAN)
    check_cxx_compiler_flag("-fsanitize=address" HAS_ASAN)
    if(HAS_ASAN)
      list(APPEND _compile_flags "-fsanitize=address" "-fno-omit-frame-pointer")
      list(APPEND _link_flags    "-fsanitize=address")
    else()
      message(WARNING "AddressSanitizer requested but not supported.")
    endif()
  endif()

  if(AA_ENABLE_UBSAN)
    check_cxx_compiler_flag("-fsanitize=undefined" HAS_UBSAN)
    if(HAS_UBSAN)
      list(APPEND _compile_flags "-fsanitize=undefined")
      list(APPEND _link_flags    "-fsanitize=undefined")
    else()
      message(WARNING "UBSan requested but not supported.")
    endif()
  endif()

  if(AA_ENABLE_TSAN)
    check_cxx_compiler_flag("-fsanitize=thread" HAS_TSAN)
    if(HAS_TSAN)
      list(APPEND _compile_flags "-fsanitize=thread")
      list(APPEND _link_flags    "-fsanitize=thread")
    else()
      message(WARNING "ThreadSanitizer requested but not supported.")
    endif()
  endif()

  if(AA_ENABLE_LSAN)
    check_cxx_compiler_flag("-fsanitize=leak" HAS_LSAN)
    if(HAS_LSAN)
      list(APPEND _compile_flags "-fsanitize=leak")
      list(APPEND _link_flags    "-fsanitize=leak")
    else()
      message(WARNING "LeakSanitizer requested but not supported.")
    endif()
  endif()

  if(AA_ENABLE_MSAN)
    check_cxx_compiler_flag("-fsanitize=memory" HAS_MSAN)
    if(HAS_MSAN)
      list(APPEND _compile_flags "-fsanitize=memory")
      list(APPEND _link_flags    "-fsanitize=memory")
    else()
      message(WARNING "MemorySanitizer requested but not supported.")
    endif()
  endif()

  if(_compile_flags)
    target_compile_options(${tgt} PRIVATE ${_compile_flags})
    target_link_options(${tgt}    PRIVATE ${_link_flags})
  endif()
endfunction()

function(aa_add_plugin src)
  get_filename_component(_name ${src} NAME_WE)
  add_library(${_name} SHARED ${src})
  target_include_directories(${_name} PRIVATE ${PROJECT_SOURCE_DIR}/include)
  aa_enable_warnings(${_name})
  aa_enable_sanitizers(${_name})
  set_target_properties(${_name} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/external_funcs_lib"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/external_funcs_lib")
endfunction()

include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.15.0
  GIT_SHALLOW    ON)
FetchContent_MakeAvailable(googletest)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GMP REQUIRED IMPORTED_TARGET gmp)

set(LIB_SOURCES
    src/associativeArray.cpp
    src/helpFunc.cpp
    src/misc.cpp
    src/parser.cpp
    src/pointers.cpp
    src/WSEML.cpp)

if(WIN32)
  list(APPEND LIB_SOURCES src/arch/win32/funcCall.cpp)
elseif(UNIX)
  list(APPEND LIB_SOURCES src/arch/unix/funcCall.cpp)
else()
  message(FATAL_ERROR "Unsupported operating system")
endif()

add_library(AssociativeArray STATIC ${LIB_SOURCES})

target_include_directories(AssociativeArray PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${GMP_INCLUDE_DIRS})

target_link_libraries(AssociativeArray PUBLIC
    PkgConfig::GMP
    gmpxx)

if(UNIX)
  target_link_options(AssociativeArray PUBLIC -rdynamic)
endif()

aa_enable_warnings(AssociativeArray)
aa_enable_sanitizers(AssociativeArray)

file(GLOB _plugin_sources CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/external_funcs/*.cpp)
foreach(_src IN LISTS _plugin_sources)
  aa_add_plugin(${_src})
endforeach()

add_executable(aarray_main main.cpp)
target_link_libraries(aarray_main PRIVATE AssociativeArray)
target_include_directories(aarray_main PRIVATE ${PROJECT_SOURCE_DIR}/include)
aa_enable_warnings(aarray_main)
aa_enable_sanitizers(aarray_main)

if(UNIX AND NOT APPLE)
  set_target_properties(aarray_main PROPERTIES
      BUILD_RPATH "$ORIGIN/../external_funcs_lib"
      INSTALL_RPATH "$ORIGIN/../external_funcs_lib")
elseif(APPLE)
  set_target_properties(aarray_main PROPERTIES
      BUILD_RPATH "@loader_path/../external_funcs_lib"
      INSTALL_RPATH "@loader_path/../external_funcs_lib")
elseif(WIN32)
  add_custom_command(TARGET aarray_main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${PROJECT_BINARY_DIR}/external_funcs_lib"
      $<TARGET_FILE_DIR:aarray_main>)
endif()

enable_testing()
file(GLOB _test_sources CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/tests/*.cpp)

if(_test_sources)
  add_executable(aarray_tests ${_test_sources})
  target_link_libraries(aarray_tests PRIVATE AssociativeArray GTest::gtest GTest::gtest_main)
  aa_enable_warnings(aarray_tests)
  aa_enable_sanitizers(aarray_tests)

  if(UNIX AND NOT APPLE)
    set_target_properties(aarray_tests PROPERTIES
        BUILD_RPATH "$ORIGIN/../external_funcs_lib"
        INSTALL_RPATH "$ORIGIN/../external_funcs_lib")
  elseif(APPLE)
    set_target_properties(aarray_tests PROPERTIES
        BUILD_RPATH "@loader_path/../external_funcs_lib"
        INSTALL_RPATH "@loader_path/../external_funcs_lib")
  elseif(WIN32)
    add_custom_command(TARGET aarray_tests POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_BINARY_DIR}/external_funcs_lib"
        $<TARGET_FILE_DIR:aarray_tests>)
  endif()

  include(GoogleTest)
  gtest_discover_tests(aarray_tests)
endif()

function(aa_add_tool source)
  get_filename_component(_name ${source} NAME_WE)
  add_executable(${_name} ${source})
  target_link_libraries(${_name} PRIVATE AssociativeArray)
  aa_enable_warnings(${_name})
  aa_enable_sanitizers(${_name})
endfunction()

