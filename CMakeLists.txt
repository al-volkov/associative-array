cmake_minimum_required(VERSION 3.20)
project(AssociativeArray LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(BOOST_INCLUDE_LIBRARIES container_hash)
set(BOOST_ENABLE_CMAKE ON)
set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/deps")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
        "Choose the type of build (Debug/Release/RelWithDebInfo/MinSizeRel)" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

option(ENABLE_ASAN "Enable Address Sanitizer" ON)
option(ENABLE_UBSAN "Enable Undefined Behavior Sanitizer" ON)

set(SANITIZER_COMPILE_FLAGS "")
set(SANITIZER_LINK_FLAGS "")

# if(ENABLE_ASAN)
# list(APPEND SANITIZER_COMPILE_FLAGS "-fsanitize=address" "-fno-omit-frame-pointer")
# list(APPEND SANITIZER_LINK_FLAGS "-fsanitize=address")
# endif()

# if(ENABLE_UBSAN)
# list(APPEND SANITIZER_COMPILE_FLAGS "-fsanitize=undefined")
# list(APPEND SANITIZER_LINK_FLAGS "-fsanitize=undefined")
# endif()
add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Werror

    $<$<BOOL:${ENABLE_ASAN}>:${SANITIZER_COMPILE_FLAGS}>
    $<$<BOOL:${ENABLE_UBSAN}>:${SANITIZER_COMPILE_FLAGS}>
)

# core
set(LIB_SOURCES
    ${CMAKE_SOURCE_DIR}/src/associativeArray.cpp
    ${CMAKE_SOURCE_DIR}/src/helpFunc.cpp
    ${CMAKE_SOURCE_DIR}/src/misc.cpp
    ${CMAKE_SOURCE_DIR}/src/parser.cpp
    ${CMAKE_SOURCE_DIR}/src/pointers.cpp
    ${CMAKE_SOURCE_DIR}/src/WSEML.cpp
)

# boost
set(BOOST_REQUIRED_SUBMODULES
    "tools/cmake;"
    "libs/config;libs/describe;"
    "libs/mp11;"
    "libs/container_hash;"
)

# os specific
if(WIN32)
    list(APPEND LIB_SOURCES ${CMAKE_SOURCE_DIR}/src/arch/win32/funcCall.cpp)
elseif(UNIX)
    list(APPEND LIB_SOURCES ${CMAKE_SOURCE_DIR}/src/arch/unix/funcCall.cpp)
else()
    message(FATAL_ERROR "Unsupported OS!")
endif()

add_library(aarray STATIC ${LIB_SOURCES})
target_include_directories(aarray PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Download dependencies
include(FetchContent)

FetchContent_Declare(
    Boost
    GIT_REPOSITORY https://github.com/boostorg/boost.git
    GIT_TAG boost-1.87.0
    GIT_PROGRESS ON
    GIT_SHALLOW ON
    GIT_SUBMODULES ${BOOST_REQUIRED_SUBMODULES}
)

FetchContent_Declare(
    range-v3
    GIT_REPOSITORY https://github.com/ericniebler/range-v3.git
    GIT_COMMIT a81477931a8aa2ad025c6bda0609f38e09e4d7ec
    GIT_SHALLOW ON
)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.0
    GIT_SHALLOW ON
)

FetchContent_MakeAvailable(googletest Boost range-v3)

# link deps to core
target_link_libraries(aarray PRIVATE Boost::container_hash range-v3::range-v3)

# external functions

# source files for external functions
set(EXTERNAL_FUNCS_SRC_DIR ${CMAKE_SOURCE_DIR}/external_funcs)

# destination for compiled external functions
set(EXTERNAL_FUNCS_LIB_DIR ${CMAKE_BINARY_DIR}/external_funcs_lib)
file(MAKE_DIRECTORY ${EXTERNAL_FUNCS_LIB_DIR})

file(GLOB EXTERNAL_FUNCS_SOURCES "${EXTERNAL_FUNCS_SRC_DIR}/*.cpp")

# create shared libraries for each external function
foreach(FUNC_SRC_FILE ${EXTERNAL_FUNCS_SOURCES})
    get_filename_component(FUNC_LIB_NAME ${FUNC_SRC_FILE} NAME_WE)

    add_library(${FUNC_LIB_NAME} SHARED ${FUNC_SRC_FILE})
    target_include_directories(${FUNC_LIB_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

    set_target_properties(${FUNC_LIB_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${EXTERNAL_FUNCS_LIB_DIR}"
        RUNTIME_OUTPUT_DIRECTORY "${EXTERNAL_FUNCS_LIB_DIR}"
    )

    target_compile_options(${FUNC_LIB_NAME} PRIVATE
        $<$<BOOL:${ENABLE_ASAN}>:${SANITIZER_COMPILE_FLAGS}>
        $<$<BOOL:${ENABLE_UBSAN}>:${SANITIZER_COMPILE_FLAGS}>
    )
    target_link_options(${FUNC_LIB_NAME} PRIVATE
        $<$<BOOL:${ENABLE_ASAN}>:${SANITIZER_LINK_FLAGS}>
        $<$<BOOL:${ENABLE_UBSAN}>:${SANITIZER_LINK_FLAGS}>
    )

    message(STATUS "Configuring external function library: ${FUNC_LIB_NAME}")
endforeach()

if(UNIX)
  target_link_options(aarray PUBLIC "-rdynamic")
endif()


# main executable
add_executable(main_exe ${CMAKE_SOURCE_DIR}/main.cpp)
target_link_libraries(main_exe PRIVATE aarray)
target_include_directories(main_exe PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_options(main_exe PRIVATE
    $<$<BOOL:${ENABLE_ASAN}>:${SANITIZER_LINK_FLAGS}>
    $<$<BOOL:${ENABLE_UBSAN}>:${SANITIZER_LINK_FLAGS}>
)

if(UNIX AND NOT APPLE)
    set_target_properties(main_exe PROPERTIES
        BUILD_RPATH "${EXTERNAL_FUNCS_LIB_DIR}"
        INSTALL_RPATH "\$ORIGIN/../external_funcs_lib"
        BUILD_WITH_INSTALL_RPATH FALSE
        SKIP_BUILD_RPATH FALSE
        INSTALL_RPATH_USE_LINK_PATH TRUE)
elseif(APPLE)
    set_target_properties(main_exe PROPERTIES
        BUILD_RPATH "${EXTERNAL_FUNCS_LIB_DIR}"
        INSTALL_RPATH "@loader_path/../external_funcs_lib")
elseif(WIN32)
    foreach(FUNC_SRC_FILE ${EXTERNAL_FUNCS_SOURCES})
        get_filename_component(FUNC_LIB_NAME ${FUNC_SRC_FILE} NAME_WE)
        add_custom_command(TARGET main_exe POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${FUNC_LIB_NAME}>
            $<TARGET_FILE_DIR:main_exe>
            COMMENT "Copying ${FUNC_LIB_NAME}.dll for main_exe"
        )
    endforeach()
endif()

# testing
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
enable_testing()
file(GLOB TEST_SOURCES "${CMAKE_SOURCE_DIR}/tests/*.cpp")

if(TEST_SOURCES)
    add_executable(tests ${TEST_SOURCES})
    target_link_libraries(tests PRIVATE aarray gtest gtest_main)
    target_include_directories(tests PRIVATE ${CMAKE_SOURCE_DIR}/include)

    target_link_options(tests PRIVATE
        $<$<BOOL:${ENABLE_ASAN}>:${SANITIZER_LINK_FLAGS}>
        $<$<BOOL:${ENABLE_UBSAN}>:${SANITIZER_LINK_FLAGS}>
    )

    # ensure that they are findable
    if(UNIX AND NOT APPLE)
        set_target_properties(tests PROPERTIES
            BUILD_RPATH "${EXTERNAL_FUNCS_LIB_DIR}"
            INSTALL_RPATH "\$ORIGIN/../external_funcs_lib"
            BUILD_WITH_INSTALL_RPATH FALSE
            SKIP_BUILD_RPATH FALSE
            INSTALL_RPATH_USE_LINK_PATH TRUE)
    elseif(APPLE)
        set_target_properties(tests PROPERTIES
            BUILD_RPATH "${EXTERNAL_FUNCS_LIB_DIR}"
            INSTALL_RPATH "@loader_path/../external_funcs_lib")
    elseif(WIN32)
        foreach(FUNC_SRC_FILE ${EXTERNAL_FUNCS_SOURCES})
            get_filename_component(FUNC_LIB_NAME ${FUNC_SRC_FILE} NAME_WE)
            add_custom_command(TARGET tests POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:${FUNC_LIB_NAME}>
                $<TARGET_FILE_DIR:tests>
                COMMENT "Copying ${FUNC_LIB_NAME}.dll for tests"
            )
        endforeach()
    endif()

    include(GoogleTest)
    gtest_discover_tests(tests)
endif()

# helper
function(add_single_file_executable exe_file)
    get_filename_component(exe_name ${exe_file} NAME_WE)
    add_executable(${exe_name} ${exe_file})
    target_link_libraries(${exe_name} PRIVATE aarray)
    target_include_directories(${exe_name} PRIVATE ${CMAKE_SOURCE_DIR}/include)

    target_link_options(${exe_name} PRIVATE
        $<$<BOOL:${ENABLE_ASAN}>:${SANITIZER_LINK_FLAGS}>
        $<$<BOOL:${ENABLE_UBSAN}>:${SANITIZER_LINK_FLAGS}>
    )
endfunction()