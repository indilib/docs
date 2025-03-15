---
title: Project Setup
nav_order: 8
parent: Basics
---

## Project Setup

First things first, there is nothing special you need to develop for INDI. You can use any
IDE, or none at all. You only need basic standard tools for C++ development.

### Dependencies

This will install the minimum dependencies for driver development. You may need
additional dev libraries depending on your own driver's requirements.

```sh
sudo apt install build-essential devscripts debhelper fakeroot cdbs software-properties-common cmake
sudo add-apt-repository ppa:mutlaqja/ppa
sudo apt install libindi-dev libnova-dev libz-dev libgsl-dev
```

### Project Structure

A recommended project structure:

```
cmake_modules/
    CMakeCommon.cmake
    FindINDI.cmake
    FindNova.cmake
    UnityBuild.cmake
config.h.cmake
indi_mycustomdriver.cpp
indi_mycustomdriver.h
indi_mycustomdriver.xml.cmake
CMakeLists.txt
```

The files in `cmake_modules` can be copied from the [libindi repo](https://github.com/indilib/indi/tree/master/cmake_modules).

### CMakeLists.txt

Additional information about CMake can be found with [their tutorial](https://cmake.org/cmake/help/latest/guide/tutorial/index.html).

Here's an example file for our new driver.

```cmake
# define the project name
project(indi-mycustomdriver C CXX)
cmake_minimum_required(VERSION 2.8)

include(GNUInstallDirs)

# add our cmake_modules folder
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")

# find our required packages
find_package(INDI REQUIRED)
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
    ${CMAKE_CURRENT_SOURCE_DIR}/indi_mycustomdriver.xml.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/indi_mycustomdriver.xml
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
    indi_mycustomdriver
    indi_mycustomdriver.cpp
)

# and link it to these libraries
target_link_libraries(
    indi_mycustomdriver
    ${INDI_LIBRARIES}
    ${NOVA_LIBRARIES}
    ${GSL_LIBRARIES}
)

# tell cmake where to install our executable
install(TARGETS indi_mycustomdriver RUNTIME DESTINATION bin)

# and where to put the driver's xml file.
install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/indi_mycustomdriver.xml
    DESTINATION ${INDI_DATA_DIR}
)
```

### indi_mycustomdriver.xml.cmake

This file is important to tell INDI about our driver. It is also important that it
be named to match your executable name.

`group` should be one of the following:

- Telescopes
- Focusers
- CCDs
- Spectrographs
- Filter Wheels
- Auxiliary
- Domes
- Weather
- Agent

`device -> label` is a friendly name for your device.

`driver -> name` is a friendly name for your driver.

The text content of the `driver` tag MUST match the executable for your driver.

```xml
<?xml version="1.0" encoding="UTF-8"?>
<driversList>
   <devGroup group="Telescopes">
      <device label="My Custom Driver" manufacturer="My Awesome Business">
         <driver name="My Custom Driver">indi_mycustomdriver</driver>
         <version>@CDRIVER_VERSION_MAJOR@.@CDRIVER_VERSION_MINOR@</version>
      </device>
   </devGroup>
</driversList>
```

### config.h.cmake

This file will get transformed by cmake into `config.h` for inclusion in your driver.

```cpp
#ifndef CONFIG_H
#define CONFIG_H

/* Define INDI Data Dir */
#cmakedefine INDI_DATA_DIR "@INDI_DATA_DIR@"
/* Define Driver version */
#define CDRIVER_VERSION_MAJOR @CDRIVER_VERSION_MAJOR@
#define CDRIVER_VERSION_MINOR @CDRIVER_VERSION_MINOR@

#endif // CONFIG_H
```
