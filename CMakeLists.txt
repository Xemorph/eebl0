cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(libeebl0 VERSION 1.0.0 LANGUAGES C)

##############################################
# Find system dependencies
set(MIN_RS232_VERSION 1.0)
find_package(RS232 ${MIN_RS232_VERSION} REQUIRED)

##############################################
# Create target and set properties
add_library(eebl0 src/eebl0.c)

# Add an alias so that library can be used inside the build tree, e.g. when testing
add_library(EEBL0::eebl0 ALIAS eebl0)

#Set target properties
target_include_directories(eebl0
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_features(eebl0 PRIVATE c_std_99)
target_compile_options(eebl0 PRIVATE
  $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
  -Wall -Wextra -Wpedantic>)

# Our required dependencies
target_link_libraries(eebl0 RS232::RS232)

##############################################
# Installation instructions

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/EEBL0)

install(TARGETS eebl0
    EXPORT eebl0-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# This is required so that the exported target has the name EEBL0 and not eebl0
set_target_properties(eebl0 PROPERTIES EXPORT_NAME EEBL0)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Export the targets to a script
install(EXPORT eebl0-targets
    FILE
        EEBL0Targets.cmake
    NAMESPACE
        EEBL0::
    DESTINATION
        ${INSTALL_CONFIGDIR}
)

# Create a ConfigVersion.cmake file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/EEBL0ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/EEBL0Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/EEBL0Config.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

# Install the config, configversion and custom find modules
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/EEBL0Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/EEBL0ConfigVersion.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
)

##############################################
## Exporting from the build tree
#configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/FindRS232.cmake
#    ${CMAKE_CURRENT_BINARY_DIR}/FindRS232.cmake
#    COPYONLY)

export(EXPORT eebl0-targets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/EEBL0Targets.cmake
    NAMESPACE EEBL0::)

#Register package in the User Package Registry
export(PACKAGE EEBL0)

##############################################
## Add test
# Currently none
