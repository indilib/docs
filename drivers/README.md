---
title: Driver Development
nav_order: 5
has_children: true
permalink: /drivers/
---

# INDI Driver Development

This guide provides comprehensive information about developing INDI drivers. It covers the basics of driver development, advanced topics, best practices, and reference information.

## Introduction to INDI Drivers

An INDI driver is a software component that communicates with astronomical hardware devices and exposes their functionality through the INDI protocol. Drivers are responsible for:

- Establishing and managing connections to hardware devices
- Defining and handling device properties
- Processing commands from clients
- Reporting device status and data to clients

INDI drivers are designed to be modular, reusable, and hardware-independent. They allow astronomical software to control devices without needing to know the specific details of each device.

## Driver Architecture

INDI drivers are built on top of the `libindi` C++ library, which provides a framework for implementing INDI-compliant drivers. The core device class is `INDI::DefaultDevice`, which encapsulates the functionality of the most basic INDI device driver.

The driver must implement each `ISxxx()` function but never calls them. The `IS()` functions are called by the reference implementation `main()` as messages arrive from Clients. Within each `IS()` function the driver performs the desired tasks and then may report back to the Client by calling the `IDxxx()` functions.

For specific device types, INDI provides specialized classes that inherit from `INDI::DefaultDevice`:

- `INDI::Telescope` for telescope mounts
- `INDI::CCD` for cameras
- `INDI::Focuser` for focusers
- `INDI::FilterWheel` for filter wheels
- And many more...

These specialized classes provide standard interfaces and functionality for each device type, making it easier to develop drivers for specific types of astronomical equipment.

## Driver Construction

An INDI driver is what directly communicates with your device(s) hardware. An INDI driver may control one or more hardware devices. It is responsible for controlling the device, and for representing the device properties to clients using INDI's protocol structures. _The driver does not contain a `main()`_, as it is expected to operate in an event-driven fashion.

The INDI library provides routines for common tasks such as serial communication, string formatting & conversion, and XML parsing. `libnova`, an external optional library, provides facility for astronomical calculations, while `cfitsio` provides handling for FITS files.

The reference API provides `IExxx()` functions to allow the driver to add its own callback functions if desired. The driver can arrange for functions to be called when reading a file descriptor that will not block; when a time interval has expired; or when there is no other client traffic in progress. Several utility functions to search and find INDI vector structs are provides for convenience in the API.

The sample `indiserver` is a stand-alone process that may be used to run one or more INDI-compliant drivers. It takes the name of each driver process to run from its command line arguments. Once a binary driver is compiled, `indiserver` can load the driver and handle all data steering services between the driver and any number of clients.

## Organization of INDI Drivers

The [INDI Main Repo](https://github.com/indilib/indi) includes several default drivers in addition to the main library. These drivers don't require extra libraries to work typically just serial or TCP communication with the hardware. The [INDI 3rd Party Repo](https://github.com/indilib/indi-3rdparty) contains many more drivers that can be developed independently of the main drivers. These may require extra libraries (for instance working with USB cameras) or just have release cycles outside of the slower main library. Having them separate also reduces the size of the main library and it's codebase.

When is a 3rd party driver required?

- Driver requires external dependencies (other than the indi-core package)
- Driver requires a separate release cycle
- Driver has a different license than indi-core

If you want to develop a new driver, it is recommended to create it separate from both, and once you are ready, integrate it into the 3rd party repo.

## Getting Started with Driver Development

To get started with INDI driver development, you should:

1. **Understand the INDI protocol**: Familiarize yourself with the INDI protocol and how it works.
2. **Set up your development environment**: Install the necessary tools and libraries for INDI development.
3. **Study existing drivers**: Look at existing INDI drivers to understand how they are structured and implemented.
4. **Create a simple driver**: Start with a simple driver to get familiar with the INDI driver framework.
5. **Test your driver**: Test your driver with INDI clients to ensure it works correctly.

For practical examples of driver development, refer to the [INDI Tutorials](tutorials/README.md) section. [Tutorial Four](tutorials/tutorial-four.md) specifically illustrates the skeleton method. Please make sure to install `tutorial_four_sk.xml` to `/usr/share/indi` or define `INDISKEL` environment variable to the path of this file before running the tutorial.

## Driver Documentation

For comprehensive guides on INDI driver development, please see the following pages:

### Step-by-Step Tutorials

- [INDI Tutorials](tutorials/README.md): A series of step-by-step tutorials for developing INDI drivers

### Basic Driver Development

- [Simple Driver](basics/simple-driver.md): A minimal example of an INDI driver
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

- [Connection Plugins](connection-plugins.md): Guide to using connection plugins in INDI drivers
- [Implementing the CCD Interface](interfaces/ccd-interface.md): Guide to implementing the CCD interface
- [Implementing the Telescope Interface](interfaces/telescope-interface.md): Guide to implementing the Telescope interface
- [Implementing the Focuser Interface](interfaces/focuser.md): Guide to implementing the Focuser interface
- [Implementing the Guider Interface](interfaces/guider.md): Guide to implementing the Guider interface

### General Resources

- [INDI Driver Development Best Practices](best-practices.md): Best practices for developing INDI drivers
- [INDI Standard Properties](standard-properties.md): Documentation on standard INDI properties
- [Troubleshooting INDI Drivers](../support/troubleshooting.md): Solutions to common issues in INDI driver development
- [Driver Submission Guide](driver-submission-guide.md): How to submit your driver to the INDI library

## Example Drivers

Example drivers can be found at [in the repo for this documentation](https://github.com/indilib/docs/tree/master/drivers/examples/).

These examples demonstrate how to implement various types of INDI drivers, including:

- [Dummy Dome](examples/indi_dummy_dome/): A simple dome driver
- [Dummy Dustcap](examples/indi_dummy_dustcap/): A simple dustcap driver
- [Dummy Filter Wheel](examples/indi_dummy_filterwheel/): A simple filter wheel driver
- [Dummy Focuser](examples/indi_dummy_focuser/): A simple focuser driver
- [Dummy GPS](examples/indi_dummy_gps/): A simple GPS driver
- [Dummy Lightbox](examples/indi_dummy_lightbox/): A simple lightbox driver
- [My Custom Driver](examples/indi_mycustomdriver/): A template for creating custom drivers

These examples provide a good starting point for developing your own INDI drivers.

## Connection Plugins

INDI provides a flexible connection framework that allows drivers to connect to devices using different communication methods. The connection framework is based on plugins, which are modular components that implement specific connection protocols.

The main advantages of using connection plugins are:

- **Modularity**: Each connection plugin is a self-contained module that can be used by any driver.
- **Reusability**: Connection plugins can be reused across multiple drivers, reducing code duplication.
- **Flexibility**: Drivers can support multiple connection methods without having to implement each one from scratch.
- **Standardization**: Connection plugins provide a standardized way to handle device connections, making drivers more consistent and easier to use.

For more information, see the [Connection Plugins](connection-plugins.md) guide.

## Standard Properties

INDI defines a set of standard properties that establish a level of interoperability among INDI drivers and clients. These properties encapsulate the most common characteristics of astronomical instrumentation of interest.

Standard properties ensure that clients can interact with drivers in a consistent and predictable way. For example, all telescope drivers should implement the `EQUATORIAL_EOD_COORD` property to allow clients to control the telescope's position.

For a comprehensive list of all standard properties and their detailed descriptions, please refer to the [INDI Standard Properties](standard-properties.md) documentation.

## Best Practices

When developing INDI drivers, it's important to follow best practices to ensure that your drivers are robust, maintainable, and user-friendly. Some key best practices include:

- **Code Organization**: Organize your driver code with a clear and consistent structure.
- **Error Handling**: Implement robust error checking and graceful degradation.
- **Resource Management**: Manage memory, file descriptors, and threads carefully.
- **Performance Optimization**: Use efficient algorithms and minimize I/O operations.
- **Testing and Debugging**: Write unit tests and use debugging techniques to ensure your driver works correctly.
- **Documentation**: Document your code thoroughly and provide comprehensive user documentation.
- **Compatibility and Portability**: Make your driver compatible with different platforms and maintain backward compatibility.
- **Security**: Validate all input and implement secure communication with devices.
- **User Experience**: Design a user-friendly interface and provide responsive feedback to user actions.
- **Logging**: Use appropriate log levels and include relevant contextual information in log messages.

For more detailed information, see the [INDI Driver Development Best Practices](best-practices.md) guide.

## Driver Submission

If you've developed an INDI driver that you'd like to contribute to the INDI project, you can submit it for inclusion in the INDI library. The submission process involves:

1. **Preparing your driver**: Ensure your driver follows INDI standards and best practices.
2. **Testing your driver**: Test your driver thoroughly to ensure it works correctly.
3. **Documenting your driver**: Provide comprehensive documentation for your driver.
4. **Submitting your driver**: Submit your driver to the INDI project for review.

For detailed instructions, see the [Driver Submission Guide](driver-submission-guide.md).

## Troubleshooting

If you encounter issues while developing or using INDI drivers, the [Troubleshooting INDI Drivers](../support/troubleshooting.md) guide provides solutions to common problems, including:

- **Connection Problems**: Issues with connecting to devices.
- **Communication Issues**: Problems with sending commands to devices or receiving responses.
- **Property Issues**: Issues with property updates and state inconsistencies.
- **Performance Issues**: Slow response times and memory leaks.
- **Driver Initialization Issues**: Problems with property initialization and driver loading.
- **Thread-Related Issues**: Deadlocks and race conditions.
- **Device-Specific Issues**: Firmware compatibility and hardware limitations.

## Resources

For more information on INDI driver development, refer to the following resources:

- [INDI Library API Documentation](https://www.indilib.org/api/index.html)
- [INDI Protocol Specification](https://www.indilib.org/develop/developer-manual/104-indi-protocol.html)
- [INDI Driver Development Guide](https://www.indilib.org/develop/developer-manual/100-driver-development.html)
- [INDI GitHub Repository](https://github.com/indilib/indi)
- [INDI 3rd Party Repository](https://github.com/indilib/indi-3rdparty)
- [INDI Forum](https://www.indilib.org/forum.html)
