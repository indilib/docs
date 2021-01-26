# INDI Driver Development

This guide is written with the assumption that you are using the `libindi` C++
library to write your driver.

## Driver Construction

An INDI driver is what directly communicates with your device(s) hardware. An INDI driver may control one or more hardware devices. It is responsible for controlling the device, and for representing the device properties to clients using INDI's protocol structures. *The driver does not contain a `main()`*, as it is expected to operate in an event-driven fashion. The core device class is `INDI::DefaultDevice`, and it encapsulates the functionality of the most basic INDI device driver.

The driver must implement each `ISxxx()` function but never calls them. The `IS()` functions are called by the reference implementation `main()` as messages arrive from Clients. Within each `IS()` function the driver performs the desired tasks and then may report back to the Client by calling the `IDxxx()` functions. The INDI library provides routines for common tasks such as serial communication, string formatting & conversion, and XML parsing. `libnova`, an external optional library, provides facility for astronomical calculations, while `cfitsio` provides handling for FITS files.

The reference API provides `IExxx()` functions to allow the driver to add its own callback functions if desired. The driver can arrange for functions to be called when reading a file descriptor that will not block; when a time interval has expired; or when there is no other client traffic in progress. Several utility functions to search and find INDI vector structs are provides for convenience in the API. The sample `indiserver` is a stand-alone process that may be used to run one or more INDI-compliant drivers. It takes the name of each driver process to run from its command line arguments. Once a binary driver is compiled, `indiserver` can load the driver and handle all data steering services between the driver and any number of clients.

The `INDI::Telescope`, `INDI::CCD`, `INDI::Focuser`, and `INDI::FilterWheel` classes provides the standard interface for those classes of devices. You need to subclass those classes in order to develop a driver for your device. If your device does not belong to those classes (i.e. weather station, or rain detector), then you may subclass `INDI::DefaultDevice` directly.

The device properties may be either hard-coded in the driver's code, or stored externally in a skeleton file. Which method you opt to utitlize depends on your preferences & devices. Drivers utilizing skeleton files have their properties stored in an XML file (usually under `/usr/share/indi` in Linux). The skeleton file naming convention is `driver_name_sk.xml`. For `indi_spectracyber` it is `indi_spectracyber_sk.xml`. The contents of the skeleton file is a list of `defXXX` XML statements enclosed by `<INDIDriver>` opening and closing tags. Support for automatic auxiliary controls a Driver may add debug, simulation, and configuration controls to the driver by calling `addAuxControls()`. For each driver, you must provide the device's default name and driver version information.

Tutorial Four is a simple driver to illustrate the skeleton method. Please make sure to install `tutorial_four_sk.xml` to `/usr/share/indi` or define `INDISKEL` envrionment variable to the path of this file before running the tutorial.

INDI Library uses CMake build system, please refer to the CMakeLists.txt file shipped with `libindi` and 3rd party drivers.

## Debugging and Logging

INDI provides a [Debugging and Logging API](http://www.indilib.org/api/structINDI_1_1Logger.html) where the driver can log different types of messages to the client and/or log file along with time in seconds since the start of the driver.

Furthermore, the driver can log messages locally to stderr using printf-style `IDLog(const char *msg, ...)`. For example:

```cpp
IDLog("Hello Driver!");
int number = 10;
IDLog("My Number is %d\n", number);
```

Note that `IDLog` does not append a new line, you have to insert it explicitly. If you don't add a new line, your message will not show in `stderr` until another message is logged that does include a new line.

When using the Debugging & Logging API, the driver can log messages using `DEBUG` and `DEBUGF` macros. By default, the logger defines 4 levels of debugging/logging levels:

* Errors: Use macro `DEBUG(INDI::Logger::DBG_ERROR, "My Error Message")`
* Warnings: Use macro `DEBUG(INDI::Logger::DBG_WARNING, "My Warning Message")`
* Session: Session messages are the regular status messages from the driver. Use macro `DEBUG(INDI::Logger::DBG_SESSION, "My Message")`
* Driver Debug: Driver debug messages are detailed driver debug output used for diagnostic purposes. Use macro `DEBUG(INDI::Logger::DBG_DEBUG, "My Driver Debug Message")`

Alternatively, The `DEBUGF` macro is used if you have printf-style message. e.g. `DEBUGF(INDI::Logger::DBG_SESSION, "Hello %s!", "There")`

To enable Debug and Logging support in your driver. Simple call `addDebugControl()` from within the `initProperties()` function. The end user is presented with a debug control as illustrated below:

![indi_debug](../images/indi_debug.png)

If enabled, then the Errors, Warnings, and Messages driver output are logged to the client by default. Debug Levels refer to the debug levels logged to the client. On the other hand, Logging Levels property indicates which levels should be logged to a file, if the user enables file logging. The driver simply calls `DEBUG` and `DEBUGF` macros. It is up to the end user to decide whether these messages are logged to the client, or a file, or both, or neither.

In addition to the default four built-in logging levels, you can add additional debug/logging levels up to a maximum of eight. To add a new debug level, call `addDebugLevel(..)` and pass the desired names of the debug and logging level required. The function returns a bitmask you can use for any calls to `DEBUG` and `DEBUGF` macros. For example, to add a new debug level for `SCOPE`, where `SCOPE` contains detailed telescope data:

```cpp
int DBG_SCOPE = addDebugLevel("SCOPE", "SCOPE");
double ra = 12, de = 90;
DEBUGF(DBG_SCOPE, "Mount RA: %g DE: %g", ra, de);
```

The log file is stored in `/tmp` and its name is the driver name plus a time-stamp of the creation date (e.g. `/tmp/indi_simulator_ccd_2016-04-04T06:17:21.log`).
