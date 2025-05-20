---
title: Home
nav_order: 1
has_children: true
permalink: /
---

# INDI Technical Documentation

Welcome to the home of [INDI](https://indilib.org) technical documentation.
Here you will find information on how to build INDI drivers and clients, as well
as information on the INDI protocol itself.

## Documentation Overview

This documentation is organized into several sections to help you get started with INDI development:

- **[Guides](guides/)**: General documentation including getting started, best practices, client development, and troubleshooting.
- **[INDI Server](indiserver/)**: Guide to setting up, configuring, and managing the INDI server.
- **[Protocol](protocol/)**: Detailed explanation of the INDI protocol, message format, and communication flow.
- **Driver Development**:
  - **[Basics](drivers/basics/)**: Fundamental concepts for creating INDI drivers
  - **[Advanced](drivers/advanced/)**: Advanced topics for driver development
  - **[Examples](drivers/examples/)**: Example driver implementations
- **[Device Interfaces](interfaces/)**: Documentation for implementing various device interfaces (CCD, Telescope, Focuser, etc.).
- **[Images](images/)**: Diagrams and screenshots illustrating INDI concepts.

### Key Documentation Pages

- [Getting Started with INDI](guides/getting-started.md)
- [INDI Server Guide](indiserver/indiserver-guide.md)
- [INDI Protocol Guide](guides/protocol-guide.md)
- [Creating Custom INDI Drivers](drivers/basics/custom-driver.md)
- [INDI Driver Interface](drivers/basics/driver-interface.md)
- [INDI Client Development Guide](guides/client-development.md)
- [INDI Connection Plugins](guides/connection-plugins.md)
- [Implementing the CCD Interface](interfaces/ccd-interface.md)
- [Implementing the Telescope Interface](interfaces/telescope-interface.md)
- [Implementing the Focuser Interface](interfaces/focuser.md)
- [Implementing the Guider Interface](interfaces/guider.md)
- [INDI Driver Development Best Practices](guides/best-practices.md)
- [Troubleshooting INDI Drivers](guides/troubleshooting.md)

First, some helpful links:

- INDI Links
  - [INDI Main Repo](https://github.com/indilib/indi)
  - [INDI 3rd Party Repo](https://github.com/indilib/indi-3rdparty)
  - [libindi API Docs](https://www.indilib.org/api/index.html)
  - [INDI Stable Builds](https://launchpad.net/~mutlaqja/+archive/ubuntu/ppa)
  - [INDI Nightly Builds](https://launchpad.net/~mutlaqja/+archive/ubuntu/indinightly)
- Build Tools
  - [CMake](https://cmake.org/cmake/help/latest/)
- Helpful Libraries
  - [libnova](http://libnova.sourceforge.net/)
  - [libgsl](https://www.gnu.org/software/gsl/)
  - [libcfitsio](https://heasarc.gsfc.nasa.gov/fitsio/)
  - [libxisf](https://gitea.nouspiro.space/nou/libXISF)
- **Bundled** Header-only Libraries
  The following C++ header-only libraries are included with libindi and may be used by any driver to handle common tasks around JSON and RESTful web services.
  - [json](https://github.com/nlohmann/json): Write & parse JSON in INDI drivers (e.g. [Primalucelab Devices](https://github.com/indilib/indi/blob/master/drivers/focuser/primalucacommandset.cpp))
  - [httplib](https://github.com/yhirose/cpp-httplib): Create http(s) servers and clients. Make GET/POST/PUT calls for RESTful services (e.g. [Planewave Mount](https://github.com/indilib/indi/blob/master/drivers/telescope/planewave_mount.cpp)).
  - [inifile-cpp](https://github.com/Rookfighter/inifile-cpp): a simple and easy to use single header-only ini file encoder and decoder for C++ (e.g. [Planewave Mount](https://github.com/indilib/indi/blob/master/drivers/telescope/planewave_mount.cpp)).
  - [nanomodbus](https://github.com/debevv/nanoMODBUS): A compact MODBUS RTU/TCP C library for embedded controllers (e.g. [Waveshare Relay](https://github.com/indilib/indi/blob/master/drivers/auxiliary/waveshare_modbus_relay.h)).

## Introduction

The Instrument-Neutral-Distributed-Interface control protocol (INDI) is a key
technology for device automation and control. INDI introduces a control protocol
standard for rapid development of robust, adaptive, and scalable device drivers
under several platforms.

INDI has many advantages over similar technologies, including loose coupling
between hardware devices and software drivers. Clients that use the device
drivers are completely unaware of the device capabilities. In run time, clients
discover the device capabilities through introspection. This enables clients to
build a completely dynamical GUI based on services provided by the device.
Hence, when new or updated device drivers are developed, clients can take full
advantage of them without any changes on the client side; thanks to the
self-describing nature of INDI.

Since developers don't have to worry about updating GUI clients to reflect
changes in their drivers, they can concentrate their time and effort on the
development and testing of drivers. This leads to a significant cut in
development time and cost, and paves the way for painless maintenance and
efficient deployment. Employing XML as the language of the protocol adds other
advantages as the protocol can be parsed and processed using any XML library.

Furthermore, remote control of devices is seamless with INDI's server/client
architecture. Distributed devices can be controlled from one centralized
environment.

The [INDI wire protocol](protocol/INDI.pdf) only describes the rules,
structures, and mechanisms underlying the protocol's architecture. What will be
discussed throughout this manual is a specific POSIX implementation of the INDI
protocol. We shall refer to this implementation hereforth as the INDI library.

The INDI library is released under the GNU Library General Public License (LGPL)
and is currently maintained in the
[libindi GitHub repo](https://github.com/indilib/indi).

## Intended Audience

The INDI library is geared toward experienced programmers planning to develop
backend hardware drivers to run under the INDI architecture. The task of
developing hardware drivers requires programmers with sufficient experience in
at least one high level programming language such as C++.

Naturally, you need to understand the ins and outs of your hardware thoroughly.
This includes communication, control of electronics/motors, physical
limitations, and safety considerations.

While the INDI wire protocol is platform-independent, the official INDI Library
is designed to operate specifically on POSIX platforms. Developers can port the
library and device drivers to different platforms as desired.

Supported operating systems include:

- Linux: Full Support
- MacOS: Full Support, except for few Linux-only drivers.
- Windows: Partial driver support via Cygwin. Client support.
- BSD: Full Support, except for few Linux-only drivers.
- iOS: Client support only.
- Android: Client support only.

INDI Library provides
[Python client bindings](https://github.com/indilib/pyindi-client) to access INDI server and drivers.

## Getting INDI

### Linux/Mac

- [INDI Downloads](https://indilib.org/download.html)
- [Nou Scripts](https://gitea.nouspiro.space/nou/astro-soft-build)

### Raspberry Pi

- [Astroberry Repo](https://www.astroberry.io/repo/)
- [AstroPI3 Script](https://github.com/rlancaste/AstroPi3)
- [Nou Scripts](https://gitea.nouspiro.space/nou/astro-soft-build)

### StellarMate

- [StellarMate Website](https://www.stellarmate.com/)

### Source

Instructions for building from source are available in the GitHub repos.

- [INDI GitHub](https://github.com/indilib/indi)
- [INDI 3rd Party GitHub](https://github.com/indilib/indi-3rdparty)

## Contributing

As an open source project, you can contribute to making INDI better. See the
repositories above to make code contributions. You can also help make the
documentation better by forking [the docs repo](https://github.com/indilib/docs/),
making changes in your fork, and creating a merge request.

Any help you can provide makes INDI a better project!
