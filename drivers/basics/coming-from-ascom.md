---
title: Coming from ASCOM
nav_order: 7
parent: Basics
---

## Coming from ASCOM development

Jumping into INDI development when coming from ASCOM development can be a steep
learning curve, especially if you don't have much C++ experience.

I've found that going from Visual Studio to QTCreator can be difficult, so this
tutorial will use VSCode instead of QTCreator.

But one thing to remember is that you can use ANY IDE you are comfortable with,
or none at all, just using command line tools.

I'm going to start with a few assumptions to kick us off.

- OS: Ubuntu Focal
- IDE: VSCode

You can do development on other operating systems, but it is definitely easiest
to get started in Linux, specifically in Ubuntu. Focal is the latest LTS (long
term support) version of Ubuntu and is well supported.

I'm NOT going to go through the process for installing Ubuntu. There are many
guides on doing just that on the internet already.

Once you have Ubuntu installed, we need to add some general utilities to develop
C++ projects.

From the terminal, install these:

```bash
sudo apt install \
    build-essential \
    software-properties-common \
    cmake
```

Jasem Mutlaq keeps a PPA project repo up-to-date with the latest builds of INDI
and all the drivers, so let's add that to our Ubuntu installation:

This adds the stable releases:

```bash
sudo add-apt-repository ppa:mutlaqja/ppa
```

If you want to use nightly builds instead to get the latest and greatest (and
least stable), use this instead:

```bash
sudo add-apt-repository ppa:mutlaqja/indinightly
```

And then install the dev indi libraries we need to do INDI driver development.
This will install the shared libraries and header files we need to build our own
drivers.

```bash
sudo apt install libindi-dev libnova-dev libz-dev libgsl-dev
```

## Creating a New Driver

INDI development is split across three main repositories:

- [INDI](https://github.com/indilib/indi)
- [INDI 3rd Party](https://github.com/indilib/indi-3rdparty)
- [KStars](https://invent.kde.org/education/kstars/)

The first is the main repo for core indi drivers, base classes, indiserver, etc.
The second is used for drivers that require 3rd party libraries in order to work.
Most cameras will be in this repo, because they usually require linking to separate
libraries outside of INDI. The third is where development of KStars is done.

This tutorial will focus only on INDI driver development.

I recommend creating your own repo, separate from either INDI or INDI 3rd Party,
to do your initial driver development. Once you get your driver working well,
outside of these other repos, then you can request help from the community to get
it integrated into one of the INDI repos.

Follow the [Project Setup Guide](project-setup.md) to get your project repo
set up.

For the purposes of this tutorial, I'm assuming you are using the `indi_mycustomdriver`
example. All commands from this point on assume you are located in the root folder
of that driver (the folder with the README.md, config.h.cmake, etc files).

Some things to keep in mind coming from ASCOM:

### CMakeLists.txt / .sln,.csproj

CMake is kinda like the command line `msbuild` (not really, it's more like just
a part of `msbuild`). CMake will be used to build our project, but it also does
more. It configures our project as well. What does that mean? Well, when doing
C++ development, we need to know where to find any libraries we want to link
against, and their header files. CMake gives us a pretty standardized way of
finding them. It also lets us define some variables that can be replaced in
files (things like version numbers), which will happen when we configure our
project.

The `CMakeLists.txt` file is much like your `.sln` and `.csproj` files in .Net
development. The difference is that you don't have a GUI to configure them,
so you need a little more understanding of what's going on.

The first line defines the name of the project, and the languages we are using.

[CMake project](https://cmake.org/cmake/help/latest/command/project.html)

```cmake
project(indi-mycustomdriver C CXX)
```

And we tell cmake to setup linux standard folders. This will set some variables
we can use in our `CMakeLists.txt` file later.

[CMake include](https://cmake.org/cmake/help/latest/command/include.html)

```cmake
include(GNUInstallDirs)
```

C++ development in linux doesn't have a Global Assembly Cache where all .Net
libraries are registered, so we need to tell cmake how to find the different
libraries we need to link to.

We do this with `Find*.cmake` files. Examples of these are provided to find
libindi and libnova.

To tell cmake about these `Find*.cmake` files, we need to include them.

[CMake CMAKE_MODULE_PATH](https://cmake.org/cmake/help/latest/variable/CMAKE_MODULE_PATH.html)

```cmake
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")
```

Then we tell cmake to find the packages we need:

[CMake find_package](https://cmake.org/cmake/help/latest/command/find_package.html)

```cmake
find_package(INDI REQUIRED)
```

If this is successful when we "configure" our project, it will set the
`INDI_LIBRARIES` and `INDI_INCLUDE_DIR` we will need later on.

Next we'll set some variables in cmake to use in our `config.h` and `xml` files:

[CMake set](https://cmake.org/cmake/help/latest/command/set.html)

```cmake
set(CDRIVER_VERSION_MAJOR 1)
set(CDRIVER_VERSION_MINOR 2)
```

Now we tell cmake to do the replacements in those files:

[CMake configure_file](https://cmake.org/cmake/help/latest/command/configure_file.html)

```cmake
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
```

These commands will create a `config.h` and `indi_mycustomdriver.xml` file in
the `build` folder.

We also call `include_directories` several times to tell cmake where to look for
C++ header files.

Now we can tell cmake how to build our executable:

[CMake add_executable](https://cmake.org/cmake/help/latest/command/add_executable.html)

```cmake
add_executable(
    indi_mycustomdriver
    indi_mycustomdriver.cpp
)
```

You would list all the `.cpp` files you need to compile to create your driver,
in this case, just the one.

And tell cmake to link our executable to the actual shared libraries it found
earlier (this is much like adding a "Reference" in .Net).

[CMake target_link_libraries](https://cmake.org/cmake/help/latest/command/target_link_libraries.html)

```cmake
target_link_libraries(
    indi_mycustomdriver
    ${INDI_LIBRARIES}
    ${NOVA_LIBRARIES}
    ${GSL_LIBRARIES}
)
```

Finally, we need to tell cmake where to install our application.

[CMake install](https://cmake.org/cmake/help/latest/command/install.html)

```cmake
install(TARGETS indi_mycustomdriver RUNTIME DESTINATION bin)
install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/indi_mycustomdriver.xml
    DESTINATION ${INDI_DATA_DIR}
)
```

The first line tells cmake to install to `DESTINATION bin`, which was set by
`include(GNUInstallDirs)`. The next install command tells cmake to install our
xml file to `${INDI_DATA_DIR}` which is set when we call
`find_package(INDI REQUIRED)`.

### Building from the terminal

Before we setup an IDE to help us, let's just build a driver from the command
line to get familiar with what is going on.

First we need to create a separate build folder to operate in.

```bash
mkdir build
cd build
```

Next we need to configure our project. This is only needed when you update
`CMakeLists.txt`.

```bash
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Debug ../
```

Here we are telling cmake to generate all the files we need to actually build
and install our driver.

Now we can build the driver with:

```bash
make
```

And install it with:

```bash
sudo make install
```

To use your driver, you'll need to install the driver every time you build it,
since indiserver looks for drivers on your PATH.

Now we have built and installed the driver, we need to run it. There are two ways
to do this (more really, but two main ways). One is typically used when we are
running our driver normally, using it in the field, the other is when developing
the driver.

The normal way is to add the driver to your profile in KStars Ekos, and let Ekos
start it for you. This is great, but makes things difficult to debug.

The second way is to start it manually, and have KStars Ekos connect to it. This
way let's us easily attach debuggers, which we'll get to later.

To start our driver manually, we just need to run indiserver from the terminal
and tell it which drivers to start.

Let's start a telescope simulator, ccd simulator, and our custom driver:

```bash
indiserver -v indi_simulator_telescope indi_simulator_ccd indi_mycustomdriver
```

Now we can set up a "Remote" profile and tell KStars Ekos to just connect to our
running indiserver. You'll need to set the IP address or hostname of the computer
running inderserver in the profile.

Now when you start the profile, you'll see your running drivers.

### VS Code

Now that we've built, installed, and run our driver using the command line tools,
let's make things easier on ourselves and use an IDE.

First, install VS Code:

Download the Ubuntu deb file from [the VSCode download page](https://code.visualstudio.com/download)
and install with:

```bash
sudo dpkg -i code_*.deb
```

Launch VS Code, and install the following extensions to make our lives easier:

- ms-vscode.cpptools
- ms-vscode.cmake-tools
- twxs.cmake
- dotjoshjohnson.xml

Once these are installed, we need to open the folder holding our source. You can
do this from the terminal by going to the root folder of your source (the folder
with the config.h.cmake file) and running:

```bash
code .
```

If you are prompted allow cmake-tools to configure vscode, allow it. If you are
prompted to choose a build type, choose "Unspecified" to let cmake figure it out.

You should now have a `.vscode` folder in your project with a `settings.json` file.
We'll create a couple more files in here to make development and debugging easier.

First, `tasks.json`:

```json
{
  // See https://go.microsoft.com/fwlink/?LinkId=733558
  // for the documentation about the tasks.json format
  "version": "2.0.0",
  "tasks": [
    {
      "label": "build-cmake",
      "type": "shell",
      "options": {
        "cwd": "${workspaceRoot}/build"
      },
      "command": "cmake --build .",
      "problemMatcher": ["$gcc"]
    },
    {
      "label": "kill",
      "type": "shell",
      "options": {
        "cwd": "${workspaceRoot}/build"
      },
      "command": "killall indiserver",
      "problemMatcher": []
    },
    {
      "label": "Build and Kill",
      "dependsOn": ["kill", "build-cmake"]
    }
  ]
}
```

Next, `launch.json`:

```json
{
  // Use IntelliSense to learn about possible attributes.
  // Hover to view descriptions of existing attributes.
  // For more information, visit: https://go.microsoft.com/fwlink/?linkid=830387
  "version": "0.2.0",
  "configurations": [
    {
      "name": "(gdb) Launch",
      "type": "cppdbg",
      "request": "launch",
      "program": "/usr/bin/indiserver",
      "args": [
        "-v",
        "indi_mycustomdriver",
        "indi_simulator_telescope",
        "indi_simulator_ccd"
      ],
      "stopAtEntry": false,
      "cwd": "${workspaceFolder}",
      "environment": [
        {
          "name": "PATH",
          "value": "${workspaceFolder}/build:${env:PATH}"
        }
      ],
      "externalConsole": false,
      "MIMode": "gdb",
      "setupCommands": [
        {
          "description": "Enable pretty-printing for gdb",
          "text": "-enable-pretty-printing",
          "ignoreFailures": true
        },
        {
          "description": "Follow exec for gdb",
          "text": "-gdb-set follow-fork-mode child",
          "ignoreFailures": true
        }
      ],
      "preLaunchTask": "Build and Kill"
    }
  ]
}
```

The `launch.json` is the most important, and will be used to launch the debugger
for us. The most important part here is the `(gdb) Launch` config. Here we launch
`indiserver` and specify the arguments we pass to it. In this case, we are basically
running the same command we ran earlier on the terminal. If you want to debug a
different driver, you'll need to update the list of `args`. It is important to
set the driver you want to debug FIRST in the list.

Now that we are configured, we should be able to just press F5 and start debugging.

You can set breakpoints now!

### Driver Communication

In INDI, all communication between clients and drivers is done through XML messages.
The benefit is that we can run our drivers separately from the clients, even on
different machines. The downside is that we can't just set the value of a property
and expect a client to see it. We need to tell the client about the new value.

When a client first connects, it sends an XML message to get all properties from
all drivers. This is the appropriately named `getProperties` message. The base
classes for drivers (and the indiserver itself) will handle translating that
XML message into a call to a driver's `ISGetProperties` method. In this method,
we would "define" our properties to the client, by calling `defineProperty` method.
This method does more than just send a message to the
client, they also register the property with the base class. If you really wanted
to handle everything manually, you could just call `IDDefText`, etc to just send
the `defineText` message to the client, but this is not recommended.

If you have properties that you only want defined once you are connected (or disconnected)
you can instead call `defineProperty` in the `updateProperties` method. The base class
will call this method when the connection state changes.

When a client wants to make a change to a property, it sends a `new*` message
(`newText`, `newLignt`, `newSwitch`, etc.) which will in turn cause the `ISNew*`
methods to be invoked. These can come at any time from the client. It's up to the
driver to determine if the `new*` message is for this device, and if we can
handle the property change. We do this by inspecting the `dev` and `name` parameters.
`dev` should match our device name, and `name` should match the property name.

When a driver wants to update a property value and inform the clients about it,
it would first update the value directly on the in memory property, then send
a `set*` message to the client. This is done by calling `IDSetSwitch`, `IDSetText`,
etc.

If a method starts with `IS` it is meant to be called by the [I]NDI [S]erver and
handled by the driver. If it starts with `ID` it is meant to be called by the
[I]NDI [D]river and handled by the server or client.
