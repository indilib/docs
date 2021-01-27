# INDI Driver Development

This guide is written with the assumption that you are using the `libindi` C++
library to write your driver.

## Organization of INDI Drivers

The [INDI Main Repo](https://github.com/indilib/indi) includes several default drivers in addition to the main library. These drivers don't require extra libraries to work typically just serial or TCP communication with the hardware. The [INDI 3rd Party Repo](https://github.com/indilib/indi-3rdparty) contains many more drivers that can be developed independently of the main drivers. These may require extra libraries (for instance working with USB cameras) or just have release cycles outside of the slower main library. Having them separate also reduces the size of the main library and it's codebase.

When is a 3rd party driver required?

* Driver requires external dependencies (other than the indi-core package)
* Driver requires a separate release cycle
* Driver has a different license than indi-core

If you want to develop a new driver, it is recommended to create it separate from both, and once you are ready, integrate it into the 3rd party repo.

## Driver Construction

An INDI driver is what directly communicates with your device(s) hardware. An INDI driver may control one or more hardware devices. It is responsible for controlling the device, and for representing the device properties to clients using INDI's protocol structures. *The driver does not contain a `main()`*, as it is expected to operate in an event-driven fashion. The core device class is `INDI::DefaultDevice`, and it encapsulates the functionality of the most basic INDI device driver.

The driver must implement each `ISxxx()` function but never calls them. The `IS()` functions are called by the reference implementation `main()` as messages arrive from Clients. Within each `IS()` function the driver performs the desired tasks and then may report back to the Client by calling the `IDxxx()` functions. The INDI library provides routines for common tasks such as serial communication, string formatting & conversion, and XML parsing. `libnova`, an external optional library, provides facility for astronomical calculations, while `cfitsio` provides handling for FITS files.

The reference API provides `IExxx()` functions to allow the driver to add its own callback functions if desired. The driver can arrange for functions to be called when reading a file descriptor that will not block; when a time interval has expired; or when there is no other client traffic in progress. Several utility functions to search and find INDI vector structs are provides for convenience in the API. The sample `indiserver` is a stand-alone process that may be used to run one or more INDI-compliant drivers. It takes the name of each driver process to run from its command line arguments. Once a binary driver is compiled, `indiserver` can load the driver and handle all data steering services between the driver and any number of clients.

The `INDI::Telescope`, `INDI::CCD`, `INDI::Focuser`, and `INDI::FilterWheel` classes provides the standard interface for those classes of devices. You need to subclass those classes in order to develop a driver for your device. If your device does not belong to those classes (i.e. weather station, or rain detector), then you may subclass `INDI::DefaultDevice` directly.

The device properties may be either hard-coded in the driver's code, or stored externally in a skeleton file. Which method you opt to utitlize depends on your preferences & devices. Drivers utilizing skeleton files have their properties stored in an XML file (usually under `/usr/share/indi` in Linux). The skeleton file naming convention is `driver_name_sk.xml`. For `indi_spectracyber` it is `indi_spectracyber_sk.xml`. The contents of the skeleton file is a list of `defXXX` XML statements enclosed by `<INDIDriver>` opening and closing tags. Support for automatic auxiliary controls a Driver may add debug, simulation, and configuration controls to the driver by calling `addAuxControls()`. For each driver, you must provide the device's default name and driver version information.

Tutorial Four is a simple driver to illustrate the skeleton method. Please make sure to install `tutorial_four_sk.xml` to `/usr/share/indi` or define `INDISKEL` envrionment variable to the path of this file before running the tutorial.

INDI Library uses CMake build system, please refer to the CMakeLists.txt file shipped with `libindi` and 3rd party drivers.

## Standard Properties

INDI does not place any special semantics on property names (i.e. properties are just texts, numbers, lights, blobs, or switches that represent no physical function). While GUI clients can construct graphical representation of properties in order to permit the user to operate the device, we run into situations where clients and drivers need to agree on the exact meaning of some fundamental properties.

What if some client need to be aware of the existence of some property in order to perform some function useful to the user? How can that client tie itself to such a property if the property can be arbitrary defined by drivers?

The solution is to define Standard Properties in order to establish a level of interoperability among INDI drivers and clients. We propose a set of shared INDI properties that encapsulate the most common characteristics of astronomical instrumentation of interest.

If the semantics of such properties are properly defined, not only it will insure base interoperability, but complete device automation becomes possible as well. Put another way, INDI standard properties are in essence properties that represent a clearly defined characteristic related to the operation of the device drivers.

For example, a very common standard property is `EQUATORIAL_EOD_COORD`. This property represents the telescope's current RA and DEC. Clients need to be aware of this property in order to, for example, draw the telescope's cross hair on the sky map. If you write a script to control a telescope, you know that any telescope supporting `EQUATORIAL_EOD_COORD` will behave in an expected manner when the property is invoked.

INDI clients are required to honor standard properties if when and they implement any functions associated with a particular standard property. Furthermore, INDI drivers employing standard properties should strictly adhere to the standard properties structure as defined here.

### General Properties

| Name             | Type   | Values           | Description                               |
| ---------------- | ------ | ---------------- | ----------------------------------------- |
| CONNECTION       | Switch | CONNECT          | Establish connection to the hardware      |
|                  |        | DISCONNECT       | Disconnect from the hardware              |
| DEVICE_PORT      | Text   | PORT             | Device connection port                    |
| TIME_LST         | Number | LST              | Local sidereal time HH:MM:SS              |
| TIME_UTC         | Text   | UTC              | UTC time in ISO 8601 format               |
|                  |        | OFFSET           | UTC offset, in hours +E                   |
| GEOGRAPHIC_COORD | Number | LAT              | Site latitude (-90 to +90) degrees +N     |
|                  |        | LONG             | Site longitude (0 to 360) degrees +E      |
|                  |        | ELEV             | Site elevation, meters                    |
| ATMOSPHERE       | Number | TEMPERATURE      | Kelvin                                    |
|                  |        | PRESSURE         | hPa                                       |
|                  |        | HUMIDITY         | Percentage %                              |
| UPLOAD_MODE      | Switch | UPLOAD_CLIENT    | Send BLOB to client                       |
|                  |        | UPLOAD_LOCAL     | Save BLOB locally                         |
|                  |        | UPLOAD_BOTH      | Send blob to client and save locally      |
| UPLOAD_SETTINGS  | Text   | UPLOAD_DIR       | Upload directory if BLOB is saved locally |
|                  |        | UPLOAD_PREFIX    | Prefix used when saving filename          |
| ACTIVE_DEVICES   | Text   | ACTIVE_TELESCOPE | Name of active devices.                   |
|                  |        | ACTIVE_CCD       |                                           |
|                  |        | ACTIVE_FILTER    |                                           |
|                  |        | ACTIVE_FOCUSER   |                                           |
|                  |        | ACTIVE_DOME      |                                           |
|                  |        | ACTIVE_GPS       |                                           |

`ACTIVE_DEVICES` is used to aid clients in automatically providing the users with a list of active devices (i.e. `CONNECTION` is `ON`) whenever needed. For example, a CCD driver may define `ACTIVE_DEVICES` property with one member: `ACTIVE_TELESCOPE`. Suppose that the client is also running LX200 Basic driver to control the telescope. If the telescope is connected, the client may automatically fill the `ACTIVE_TELESCOPE` field or provide a drop-down list of active telescopes to select from. Once set, the CCD driver may record, for example, the telescope name, RA, DEC, among other metadata once it captures an image. Therefore, `ACTIVE_DEVICES` is primarily used to link together different classes of devices to exchange information if required.

### Telescope Properties

Telescope standard properties define critical properties for the operation and control of the mount. In addition to the properties below, all properties in [INDI Alignment Subsystem](http://www.indilib.org/api/md_libs_indibase_alignment_alignment_white_paper.html) are considered standard properties as well and must be reserved in all implementations.

| Name             | Type   | Values | Description                             |
| ---------------- | ------ | ------ | --------------------------------------- |
| EQUATORIAL_COORD | Number |        | Equatorial astrometric J2000 coordinate |
|                  |        | RA     | J2000 RA, hours                         |
|                  |        | DEC    | J2000 Dec, degrees +N                   |
