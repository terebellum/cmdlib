cmake_minimum_required(VERSION 3.14)
project(cmdlib
    VERSION 0.1.0
    DESCRIPTION "Template for C++ library built with CMake"
    LANGUAGES CXX)

#----------------------------------------------------------------------------------------------------------------------
# general settings and options
#----------------------------------------------------------------------------------------------------------------------

include(cmake/utils.cmake)
include(GNUInstallDirs)

string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" is_top_level)

# CMDLIB_SHARED_LIBS option (undefined by default) can be used to force shared/static build
option(CMDLIB_BUILD_TESTS "Build cmdlib tests" OFF)
option(CMDLIB_BUILD_EXAMPLES "Build cmdlib examples" OFF)
option(CMDLIB_BUILD_DOCS "Build cmdlib documentation" OFF)
option(CMDLIB_INSTALL "Generate target for installing cmdlib" ${is_top_level})
set_if_undefined(CMDLIB_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/cmdlib" CACHE STRING
    "Install path for cmdlib package-related CMake files")

if(DEFINED CMDLIB_SHARED_LIBS)
    set(BUILD_SHARED_LIBS ${CMDLIB_SHARED_LIBS})
endif()

if(NOT DEFINED CMAKE_BUILD_TYPE AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set_if_undefined(CMAKE_CXX_VISIBILITY_PRESET hidden)
set_if_undefined(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

add_library(cmdlib) # initialized below
add_library(cmdlib::cmdlib ALIAS cmdlib)

#----------------------------------------------------------------------------------------------------------------------
# cmdlib dependencies
#----------------------------------------------------------------------------------------------------------------------

# Search for your dependencies here

#----------------------------------------------------------------------------------------------------------------------
# cmdlib sources
#----------------------------------------------------------------------------------------------------------------------

include(GenerateExportHeader)
set(export_file_name "export_shared.hpp")

if(NOT BUILD_SHARED_LIBS)
    set(export_file_name "export_static.hpp")
endif()

generate_export_header(cmdlib EXPORT_FILE_NAME include/cmdlib/${export_file_name})



IF (WIN32)
ELSE()
set(platform_sources
    include/cmdlib/unix/command-impl.hpp
    src/unix/command-impl.cpp)
ENDIF()


set(sources
    ${platform_sources}
    include/cmdlib/export.hpp
    include/cmdlib/command.hpp
    include/cmdlib/command-impl.hpp
    src/handle-stream.cpp
    src/command.cpp)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})

#----------------------------------------------------------------------------------------------------------------------
# cmdlib target
#----------------------------------------------------------------------------------------------------------------------

include(CMakePackageConfigHelpers)

target_sources(cmdlib PRIVATE ${sources})
target_compile_definitions(cmdlib PUBLIC "$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:CMDLIB_STATIC_DEFINE>")

target_include_directories(cmdlib
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>")

set_target_properties(cmdlib PROPERTIES
    SOVERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION})

if(CMDLIB_INSTALL AND NOT CMAKE_SKIP_INSTALL_RULES)
    configure_package_config_file(cmake/cmdlib-config.cmake.in cmdlib-config.cmake
        INSTALL_DESTINATION "${CMDLIB_INSTALL_CMAKEDIR}")

    write_basic_package_version_file(cmdlib-config-version.cmake
        COMPATIBILITY SameMajorVersion)

    install(TARGETS cmdlib EXPORT cmdlib_export
        RUNTIME COMPONENT cmdlib
        LIBRARY COMPONENT cmdlib NAMELINK_COMPONENT cmdlib-dev
        ARCHIVE COMPONENT cmdlib-dev
        INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
    install(DIRECTORY include/
        TYPE INCLUDE
        COMPONENT cmdlib-dev)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/cmdlib/${export_file_name}"
        COMPONENT cmdlib-dev
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/cmdlib")

    set(targets_file "cmdlib-shared-targets.cmake")

    if(NOT BUILD_SHARED_LIBS)
        set(targets_file "cmdlib-static-targets.cmake")
    endif()

    install(EXPORT cmdlib_export
        COMPONENT cmdlib-dev
        FILE "${targets_file}"
        DESTINATION "${CMDLIB_INSTALL_CMAKEDIR}"
        NAMESPACE cmdlib::)

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/cmdlib-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/cmdlib-config-version.cmake"
        COMPONENT cmdlib-dev
        DESTINATION "${CMDLIB_INSTALL_CMAKEDIR}")

    if(MSVC)
        set(pdb_file "")
        set(pdb_file_destination "")

        if(BUILD_SHARED_LIBS)
            set(pdb_file "$<TARGET_PDB_FILE:cmdlib>")
            set(pdb_file_destination "${CMAKE_INSTALL_BINDIR}")
        else()
            # TARGET_PDB_FILE does not work for pdb file generated for static library build, determining it manually
            set(pdb_file "$<TARGET_FILE_DIR:cmdlib>/$<TARGET_FILE_PREFIX:cmdlib>$<TARGET_FILE_BASE_NAME:cmdlib>.pdb")
            set(pdb_file_destination "${CMAKE_INSTALL_LIBDIR}")
        endif()

        install(FILES "${pdb_file}"
            COMPONENT cmdlib-dev
            CONFIGURATIONS Debug RelWithDebInfo
            DESTINATION "${pdb_file_destination}"
            OPTIONAL)
    endif()
endif()

#----------------------------------------------------------------------------------------------------------------------
# other targets
#----------------------------------------------------------------------------------------------------------------------

if(CMDLIB_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(CMDLIB_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(CMDLIB_BUILD_DOCS)
    find_package(Doxygen REQUIRED)
    doxygen_add_docs(docs include)
endif()
