---
title: Driver Development
nav_order: 3
has_children: true
permalink: /drivers/
---

# INDI Driver Development

This guide is written with the assumption that you are using the `libindi` C++
library to write your driver.

## Driver Documentation

For comprehensive guides on INDI driver development, please see the following pages:

### Basic Driver Development

- [Driver Interface](basics/driver-interface.md): Overview of the different device interfaces in INDI
- [Creating Custom INDI Drivers](basics/custom-driver.md): Guide to creating custom INDI drivers
- [Properties](basics/properties.md): Information about INDI properties and how to use them
- [Serial Connection](basics/serialconnection.md): Guide to implementing serial connections in INDI drivers
- [Binary Transfers](basics/binary-transfers.md): Documentation on handling binary data transfers in INDI
- [Project Setup](basics/project-setup.md): Instructions for setting up an INDI driver project
- [Helpful Functions](basics/helpful-functions.md): Useful functions for INDI driver development

### Advanced Driver Development

- [Inter-Driver Communication](advanced/inter-driver-communication.md): Guide to communication between INDI drivers
- [Logging](advanced/logging.md): Documentation on implementing logging in INDI drivers
- [Loops](advanced/loops.md): Information about handling loops and timing in INDI drivers

### Device Interfaces

- [INDI Connection Plugins](../guides/connection-plugins.md): Guide to using connection plugins in INDI drivers
- [Implementing the CCD Interface](../interfaces/ccd-interface.md): Guide to implementing the CCD interface
- [Implementing the Telescope Interface](../interfaces/telescope-interface.md): Guide to implementing the Telescope interface
- [Implementing the Focuser Interface](../interfaces/focuser.md): Guide to implementing the Focuser interface
- [Implementing the Guider Interface](../interfaces/guider.md): Guide to implementing the Guider interface

### General Resources

- [INDI Driver Development Best Practices](../guides/best-practices.md): Best practices for developing INDI drivers
- [Troubleshooting INDI Drivers](../guides/troubleshooting.md): Solutions to common issues in INDI driver development

## Organization of INDI Drivers

The [INDI Main Repo](https://github.com/indilib/indi) includes several default drivers in addition to the main library. These drivers don't require extra libraries to work typically just serial or TCP communication with the hardware. The [INDI 3rd Party Repo](https://github.com/indilib/indi-3rdparty) contains many more drivers that can be developed independently of the main drivers. These may require extra libraries (for instance working with USB cameras) or just have release cycles outside of the slower main library. Having them separate also reduces the size of the main library and it's codebase.

When is a 3rd party driver required?

- Driver requires external dependencies (other than the indi-core package)
- Driver requires a separate release cycle
- Driver has a different license than indi-core

If you want to develop a new driver, it is recommended to create it separate from both, and once you are ready, integrate it into the 3rd party repo.

## Driver Construction

Example drivers can be found at [in the repo for this documentation](https://github.com/indilib/docs/tree/master/drivers/examples/).

An INDI driver is what directly communicates with your device(s) hardware. An INDI driver may control one or more hardware devices. It is responsible for controlling the device, and for representing the device properties to clients using INDI's protocol structures. _The driver does not contain a `main()`_, as it is expected to operate in an event-driven fashion. The core device class is `INDI::DefaultDevice`, and it encapsulates the functionality of the most basic INDI device driver.

The driver must implement each `ISxxx()` function but never calls them. The `IS()` functions are called by the reference implementation `main()` as messages arrive from Clients. Within each `IS()` function the driver performs the desired tasks and then may report back to the Client by calling the `IDxxx()` functions. The INDI library provides routines for common tasks such as serial communication, string formatting & conversion, and XML parsing. `libnova`, an external optional library, provides facility for astronomical calculations, while `cfitsio` provides handling for FITS files.

The reference API provides `IExxx()` functions to allow the driver to add its own callback functions if desired. The driver can arrange for functions to be called when reading a file descriptor that will not block; when a time interval has expired; or when there is no other client traffic in progress. Several utility functions to search and find INDI vector structs are provides for convenience in the API. The sample `indiserver` is a stand-alone process that may be used to run one or more INDI-compliant drivers. It takes the name of each driver process to run from its command line arguments. Once a binary driver is compiled, `indiserver` can load the driver and handle all data steering services between the driver and any number of clients.

The `INDI::Telescope`, `INDI::CCD`, `INDI::Focuser`, and `INDI::FilterWheel` classes provides the standard interface for those classes of devices. You need to subclass those classes in order to develop a driver for your device. If your device does not belong to those classes (i.e. weather station, or rain detector), then you may subclass `INDI::DefaultDevice` directly.

The device properties may be either hard-coded in the driver's code, or stored externally in a skeleton file. Which method you opt to utitlize depends on your preferences & devices. Drivers utilizing skeleton files have their properties stored in an XML file (usually under `/usr/share/indi` in Linux). The skeleton file naming convention is `driver_name_sk.xml`. For `indi_spectracyber` it is `indi_spectracyber_sk.xml`. The contents of the skeleton file is a list of `defXXX` XML statements enclosed by `<INDIDriver>` opening and closing tags. Support for automatic auxiliary controls a Driver may add debug, simulation, and configuration controls to the driver by calling `addAuxControls()`. For each driver, you must provide the device's default name and driver version information.

Tutorial Four is a simple driver to illustrate the skeleton method. Please make sure to install `tutorial_four_sk.xml` to `/usr/share/indi` or define `INDISKEL` envrionment variable to the path of this file before running the tutorial.

INDI Library uses CMake build system, please refer to the CMakeLists.txt file shipped with `libindi` and 3rd party drivers.

## Standard Properties

INDI drivers use standard properties to establish interoperability with clients. For a comprehensive list of all standard properties and their detailed descriptions, please refer to the [INDI Standard Properties](../guides/standard-properties.md) documentation.
