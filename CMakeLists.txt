# require 3.15 for GNUInstallDirs
cmake_minimum_required(VERSION 3.15...3.18)

project(QuaZip VERSION 1.5)

include(cmake/clone-repo.cmake)

set(QUAZIP_LIB_VERSION ${QuaZip_VERSION})
set(QUAZIP_LIB_SOVERSION 1.5.0)

if(EMSCRIPTEN)
    #option(ZLIB_INCLUDE "Path to include dir" "")
    #option(ZLIB_LIBRARY "Path to library dir" "")
    option(BUILD_SHARED_LIBS "" OFF)
    option(QUAZIP_INSTALL "" OFF)
    option(QUAZIP_USE_QT_ZLIB "" OFF)
    option(QUAZIP_ENABLE_TESTS "Build QuaZip tests" OFF)
else()
    option(BUILD_SHARED_LIBS "" ON)
    option(QUAZIP_INSTALL "" ON)
    option(QUAZIP_USE_QT_ZLIB "" OFF)
    option(QUAZIP_ENABLE_TESTS "Build QuaZip tests" OFF)
endif()

OPTION(ZLIB_CONST "Sets ZLIB_CONST preprocessor definition" OFF)

# Make BZIP2 optional
option(QUAZIP_BZIP2 "Enables BZIP2 compression" OFF)
option(QUAZIP_BZIP2_STDIO "Output BZIP2 errors to stdio" ON)

option(QUAZIP_FETCH_LIBS "Enables fetching third-party libraries if not found" ${WIN32})
option(QUAZIP_FORCE_FETCH_LIBS "Enables fetching third-party libraries always" OFF)

if (QUAZIP_USE_QT_ZLIB AND BUILD_SHARED_LIBS)
    message(FATAL_ERROR "Using BUILD_SHARED_LIBS=ON together with QUAZIP_USE_QT_ZLIB=ON is not supported." )
endif()

set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RELEASE)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_DEBUG_POSTFIX d)

set(QUAZIP_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(QUAZIP_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(QUAZIP_LIB_FILE_NAME quazip${QuaZip_VERSION_MAJOR}-qt6)
set(QUAZIP_LIB_TARGET_NAME QuaZip)
set(QUAZIP_DIR_NAME QuaZip-Qt6-${QUAZIP_LIB_VERSION})
set(QUAZIP_PACKAGE_NAME QuaZip-Qt6)

find_package(Qt6 REQUIRED COMPONENTS Core
    OPTIONAL_COMPONENTS Network Test)
message(STATUS "Found Qt version ${Qt6_VERSION} at ${Qt6_DIR}")
set(QUAZIP_QT_ZLIB_COMPONENT BundledZLIB)
set(QUAZIP_QT_ZLIB_HEADER_COMPONENT ZlibPrivate)
set(QUAZIP_LIB_LIBRARIES Qt6::Core)
set(QUAZIP_TEST_QT_LIBRARIES Qt6::Core Qt6::Network Qt6::Test)
set(QUAZIP_PKGCONFIG_REQUIRES "zlib, Qt6Core")

set(QUAZIP_QT_ZLIB_USED OFF)
if(QUAZIP_USE_QT_ZLIB)
    find_package(Qt6 OPTIONAL_COMPONENTS ${QUAZIP_QT_ZLIB_COMPONENT})
    set(QUAZIP_QT_ZLIB_COMPONENT_FOUND Qt6${QUAZIP_QT_ZLIB_COMPONENT}_FOUND)
    if (DEFINED QUAZIP_QT_ZLIB_HEADER_COMPONENT)
        find_package(Qt6 OPTIONAL_COMPONENTS ${QUAZIP_QT_ZLIB_HEADER_COMPONENT})
        set(QUAZIP_QT_ZLIB_HEADER_COMPONENT_FOUND Qt6${QUAZIP_QT_ZLIB_HEADER_COMPONENT}_FOUND)
    else()
        set(QUAZIP_QT_ZLIB_HEADER_COMPONENT_FOUND ON)
    endif()
    if(QUAZIP_QT_ZLIB_COMPONENT_FOUND AND QUAZIP_QT_ZLIB_HEADER_COMPONENT_FOUND)
        message(STATUS "Qt component ${QUAZIP_QT_ZLIB_COMPONENT} found")
        set(QUAZIP_LIB_LIBRARIES ${QUAZIP_LIB_LIBRARIES} Qt6::${QUAZIP_QT_ZLIB_COMPONENT})
        if(DEFINED QUAZIP_QT_ZLIB_HEADER_COMPONENT)
            message(STATUS "Qt component ${QUAZIP_QT_ZLIB_HEADER_COMPONENT} found")
            set(QUAZIP_LIB_LIBRARIES ${QUAZIP_LIB_LIBRARIES} Qt6::${QUAZIP_QT_ZLIB_HEADER_COMPONENT})
        endif()
        set(QUAZIP_QT_ZLIB_USED ON)
    else()
        message(FATAL_ERROR "QUAZIP_USE_QT_ZLIB was set but bundled zlib was not found. Terminating to prevent accidental linking to system libraries.")
    endif()
endif()

if(QUAZIP_QT_ZLIB_USED AND QUAZIP_QT_ZLIB_COMPONENT STREQUAL BundledZLIB)
    # Qt's new BundledZLIB uses z-prefix in zlib
    add_compile_definitions(Z_PREFIX)
endif()

if(NOT QUAZIP_QT_ZLIB_USED)
    if(EMSCRIPTEN)
        if(NOT DEFINED ZLIB_LIBRARY)
            message(WARNING "ZLIB_LIBRARY is not set")
        endif()

        if(NOT DEFINED ZLIB_INCLUDE)
            message(WARNING "ZLIB_INCLUDE is not set")
        else()
            include_directories(${ZLIB_INCLUDE})
        endif()

        if(NOT DEFINED ZCONF_INCLUDE)
            message(WARNING "ZCONF_INCLUDE is not set")
        else()
            include_directories(${ZCONF_INCLUDE})
        endif()

        set(QUAZIP_LIB_LIBRARIES ${QUAZIP_LIB_LIBRARIES} ${ZLIB_LIBRARY})
    else()
        find_package(ZLIB REQUIRED)
        set(QUAZIP_LIB_LIBRARIES ${QUAZIP_LIB_LIBRARIES} ZLIB::ZLIB)
    endif()
endif()

if (ZLIB_CONST)
    add_compile_definitions(ZLIB_CONST)
endif()

set(QUAZIP_DEP)
set(QUAZIP_INC)
set(QUAZIP_LIB)
set(QUAZIP_LBD)

if(QUAZIP_BZIP2)
    # Check if bzip2 is present
    set(QUAZIP_BZIP2 ON)

    if(NOT QUAZIP_FORCE_FETCH_LIBS)
        find_package(BZip2 QUIET)
    endif()

    if(BZIP2_FOUND AND NOT QUAZIP_FORCE_FETCH_LIBS)
        message(STATUS "Using BZIP2 ${BZIP2_VERSION_STRING}")

        list(APPEND QUAZIP_INC ${BZIP2_INCLUDE_DIRS})
        list(APPEND QUAZIP_LIB ${BZIP2_LIBRARIES})
        list(APPEND QUAZIP_LBD ${BZIP2_LIBRARY_DIRS})

        set(PC_PRIVATE_LIBS "${PC_PRIVATE_LIBS} -lbzip2")
    elseif(QUAZIP_FETCH_LIBS)
        clone_repo(bzip2 https://sourceware.org/git/bzip2.git)

        # BZip2 repository does not support cmake so we have to create
        # the bzip2 library ourselves
        set(BZIP2_SRC
            ${BZIP2_SOURCE_DIR}/blocksort.c
            ${BZIP2_SOURCE_DIR}/bzlib.c
            ${BZIP2_SOURCE_DIR}/compress.c
            ${BZIP2_SOURCE_DIR}/crctable.c
            ${BZIP2_SOURCE_DIR}/decompress.c
            ${BZIP2_SOURCE_DIR}/huffman.c
            ${BZIP2_SOURCE_DIR}/randtable.c)

        set(BZIP2_HDR
            ${BZIP2_SOURCE_DIR}/bzlib.h
            ${BZIP2_SOURCE_DIR}/bzlib_private.h)

        add_library(bzip2 STATIC ${BZIP2_SRC} ${BZIP2_HDR})

        if(NOT QUAZIP_BZIP2_STDIO)
            target_compile_definitions(bzip2 PRIVATE -DBZ_NO_STDIO)
        endif()

        list(APPEND QUAZIP_DEP bzip2)
        list(APPEND QUAZIP_LIB bzip2)
        list(APPEND QUAZIP_INC ${BZIP2_SOURCE_DIR})
    else()
        message(STATUS "BZip2 library not found")

        set(QUAZIP_BZIP2 OFF)
    endif()

    if(QUAZIP_BZIP2)
        find_package(BZip2)
        add_compile_definitions(HAVE_BZIP2)
    endif()
endif()

add_subdirectory(quazip)

if(QUAZIP_ENABLE_TESTS)
    message(STATUS "Building QuaZip tests")
    enable_testing()
    add_subdirectory(qztest)
endif()
