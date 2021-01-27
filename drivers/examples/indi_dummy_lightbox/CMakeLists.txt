# define the project name
project(indi-dummy-lightbox C CXX)
cmake_minimum_required(VERSION 2.8)

include(GNUInstallDirs)

# add our cmake_modules folder
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")

# find our required packages
find_package(INDI 1.8 REQUIRED)
find_package(Nova REQUIRED)
find_package(ZLIB REQUIRED)
find_package(GSL REQUIRED)

# these will be used to set the version number in config.h and our driver's xml file
set(CDRIVER_VERSION_MAJOR 1)
set(CDRIVER_VERSION_MINOR 2)

# do the replacement in the config.h
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

# do the replacement in the driver's xml file
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/indi_dummy_lightbox.xml.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/indi_dummy_lightbox.xml
)

# set our include directories to look for header files
include_directories( ${CMAKE_CURRENT_BINARY_DIR})
include_directories( ${CMAKE_CURRENT_SOURCE_DIR})
include_directories( ${INDI_INCLUDE_DIR})
include_directories( ${NOVA_INCLUDE_DIR})
include_directories( ${EV_INCLUDE_DIR})

include(CMakeCommon)

# tell cmake to build our executable
add_executable(
    indi_dummy_lightbox
    indi_dummy_lightbox.cpp
)

# and link it to these libraries
target_link_libraries(
    indi_dummy_lightbox
    ${INDI_LIBRARIES}
    ${NOVA_LIBRARIES}
    ${GSL_LIBRARIES}
)

# tell cmake where to install our executable
install(TARGETS indi_dummy_lightbox RUNTIME DESTINATION bin)

# and where to put the driver's xml file.
install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/indi_dummy_lightbox.xml
    DESTINATION ${INDI_DATA_DIR}
)