---
title: Development Environment
nav_order: 5
parent: Basics
---

# Development Environment

This section covers setting up your development environment for INDI driver development, including project structure, IDE configuration, debugging, and coding standards.

## Project Setup

First things first, there is nothing special you need to develop for INDI. You can use any
IDE, or none at all. You only need basic standard tools for C++ development. However, we recommend using Visual Studio Code (VS Code) for the best development experience.

## VS Code Setup

Visual Studio Code provides an excellent environment for INDI driver development. When you first open VS Code in your INDI driver project directory, it may prompt you to install recommended extensions.

### Recommended Extensions

The following extensions are recommended for INDI development:

```json
"recommendations": [
  "chiehyu.vscode-astyle",
  "nick-rudenko.back-n-forth",
  "alefragnani.bookmarks",
  "ms-vscode.cpptools",
  "ms-vscode.cpptools-extension-pack",
  "ms-vscode.cpptools-themes",
  "ms-vscode.cmake-tools",
  "webfreak.debug",
  "ms-vscode.vscode-serial-monitor",
]
```

#### launch.json

This file configures the debugging settings. Note that INDI drivers must be debugged through the INDI server, as they run as forked processes:

```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "Debug INDI Driver",
      "type": "cppdbg",
      "request": "launch",
      "program": "${workspaceFolder}/build/indiserver/indiserver",
      "args": ["-v", "${workspaceFolder}/build/indi_mycustomdriver"],
      "postDebugTask": "Kill indiserver",
      "stopAtEntry": false,
      "cwd": "${fileDirname}",
      "environment": [],
      "externalConsole": false,
      "MIMode": "gdb",
      "setupCommands": [
        {
          "description": "Enable pretty-printing for gdb",
          "text": "-enable-pretty-printing",
          "ignoreFailures": true
        },
        {
          "description": "Set Disassembly Flavor to Intel",
          "text": "-gdb-set disassembly-flavor intel",
          "ignoreFailures": true
        },
        {
          "text": "-gdb-set follow-fork-mode child"
        }
      ]
    }
  ]
}
```

#### tasks.json

This file defines build tasks:

```json
{
  "version": "2.0.0",
  "tasks": [
    {
      "type": "cmake",
      "label": "CMake: build",
      "command": "build",
      "targets": ["all"],
      "group": {
        "kind": "build",
        "isDefault": true
      },
      "problemMatcher": [],
      "detail": "CMake template build task"
    },
    {
      "label": "Kill indiserver",
      "type": "shell",
      "command": "pkill",
      "args": ["indiserver"]
    }
  ]
}
```

#### settings.json

This file configures the CMake build settings:

```json
{
  "cmake.configureArgs": [],
  "cmake.buildDirectory": "${workspaceFolder}/build",
  "cmake.configureSettings": {
    "CMAKE_INSTALL_PREFIX": "/usr",
    "CMAKE_CURRENT_BINARY_DIR": "${workspaceFolder}/build"
  }
}
```

## Debugging INDI Drivers

Debugging INDI drivers requires special consideration because the INDI server forks each driver into a separate process.

### Understanding the INDI Server Process Model

When you run `indiserver`, it:

1. Starts as a single process
2. Forks a new process for each driver
3. Each driver runs independently

This means that when debugging, you need to follow the child process.

### Debugging with VS Code

The launch configuration above is set up to follow the child process with the `-gdb-set follow-fork-mode child` command, which allows you to debug the driver process after it's forked from the INDI server.

### Debugging with GDB Script

For command-line debugging, you can use the following script (save it as `igdb.sh`):

```bash
#!/bin/bash

filename=~/crash_$(date +%Y%m%d_%H%M%S).txt
echo "Starting INDI driver debugging for" $*
gdb -batch -ex "handle SIG33 nostop noprint" -ex "set follow-fork-mode child" -ex "run" -ex "bt" --args indiserver -r 0 -v $* > $filename 2>&1
echo "Debugging finished. Please send" $filename "file to support."
```

Run it with:

```bash
./igdb.sh indi_mycustomdriver
```

This script will:

1. Start the INDI server with your driver
2. Configure GDB to follow the child process
3. Capture a backtrace if the driver crashes
4. Save the output to a timestamped file

## INDI Coding Standards

Following consistent coding standards makes your driver easier to maintain and integrate with the INDI ecosystem.

### Naming Conventions

- **Driver Executable Names**: Use `indi_` prefix followed by a descriptive name and the device family suffix, for example:
  - `indi_mymount_telescope` - For mounts
  - `indi_mycamera_ccd` - For cameras
  - `indi_myfocuser_focuser` - For focusers
  - `indi_myfilterwheel_wheel` - For filter wheels
  - `indi_myrotator_rotator` - For rotators
  - `indi_myaux_aux` - For auxiliary devices
  - `indi_mydome_dome` - For domes
- **Property Names**: Use uppercase with underscores, e.g., `TELESCOPE_PARK`
- **Property Naming Convention**:
  - Switch: SP suffix (e.g., `TELESCOPE_PARK_SP`)
  - Number: NP suffix (e.g., `TELESCOPE_COORDINATES_NP`)
  - Text: TP suffix (e.g., `DEVICE_PORT_TP`)
  - Light: LP suffix (e.g., `TELESCOPE_STATUS_LP`)
  - BLOB: BP suffix (e.g., `CCD_IMAGE_BP`)

### Code Formatting

INDI uses the Artistic Style (astyle) formatter. You can create a `.astylerc` file in your project with the recommended settings.

### Documentation

Document your code with clear comments, especially for:

- Class and method descriptions
- Property definitions
- Complex algorithms
- Protocol implementations

### XML Driver Files

Each driver should have its own XML file that describes the driver to the INDI server. Follow the format shown in the example above.

### Device Types

When creating a driver, make sure to place it in the appropriate device group in the XML file:

- Telescopes
- Focusers
- CCDs
- Spectrographs
- Filter Wheels
- Auxiliary
- Domes
- Weather
- Agent

### Testing

Before submitting your driver, thoroughly test it with:

1. **INDI Control Panel**: Test basic functionality
2. **KStars/Ekos**: Test integration with astronomy software
3. **Different Clients**: Test with various INDI clients to ensure compatibility
4. **Edge Cases**: Test error handling, disconnections, etc.

Remember that INDI drivers should be robust and handle errors gracefully, as they're often used in remote or automated setups where manual intervention is difficult.

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
