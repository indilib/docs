# INDI Client Development

An INDI client is defined as any software that communicates with either INDI server and/or drivers using the [INDI XML Protocol](../protocol/README.md). While clients may communicate with INDI drivers directly, in practice this is uncommon and all existing clients communicate with drivers via the [INDI server](../indiserver/README.md).

Since INDI is an XML-based protocol, it supports a wide-variety of methods to communicate with the INDI server and drivers. You can even use Telnet to communicate with INDI server and type in the XML messages directly!

To test this simple method, launch INDI server in one terminal window:

```bash
indiserver -v indi_simulator_telescope
```

This will launch INDI server on the default port 7624 and runs a single driver: Telescope Simulator. You should see the following output:

```
2017-03-01T06:51:25: Driver indi_simulator_telescope: pid=2496 rfd=3 wfd=6 efd=7
2017-03-01T06:51:25: listening to port 7624 on fd 4
2017-03-01T06:51:25: Driver indi_simulator_telescope: snooping on GPS Simulator.GEOGRAPHIC_COORD
2017-03-01T06:51:25: Driver indi_simulator_telescope: snooping on GPS Simulator.TIME_UTC
2017-03-01T06:51:25: Driver indi_simulator_telescope: snooping on Dome Simulator.DOME_PARK
2017-03-01T06:51:25: Driver indi_simulator_telescope: snooping on Dome Simulator.DOME_SHUTTER
```

Now open another terminal window and use Telnet to connect to the INDI server:

```bash
telnet localhost 7624
```

After connecting, simply type in the following to get a list of properties:

```xml
<getProperties version="1.7"/>
```

You should immediately see properties getting defined (only part of the properties are listed below):

```xml
<defswitchvector device="Telescope Simulator" name="CONNECTION" label="Connection" group="Main Control" state="Idle" perm="rw" rule="OneOfMany" timeout="60" timestamp="2017-03-01T06:53:45">
    <defswitch name="CONNECT" label="Connect">
Off
    </defswitch>
    <defswitch name="DISCONNECT" label="Disconnect">
On
    </defswitch>
</defswitchvector>
<deftextvector device="Telescope Simulator" name="DRIVER_INFO" label="Driver Info" group="Options" state="Idle" perm="ro" timeout="60" timestamp="2017-03-01T06:53:45">
    <deftext name="DRIVER_NAME" label="Name">
Telescope Simulator
    </deftext>
    <deftext name="DRIVER_EXEC" label="Exec">
indi_simulator_telescope
    </deftext>
    <deftext name="DRIVER_VERSION" label="Version">
1.0
    </deftext>
    <deftext name="DRIVER_INTERFACE" label="Interface">
5
    </deftext>
</deftextvector>
```

Using Telnet can be useful to test INDI server, but not useful to create a client application. For this there are multiple options:


* C/C++ Application Development: [INDI::BaseClient](http://www.indilib.org/api/classINDI_1_1BaseClient.html) for POSIX systems and [INDI::BaseClientQt](http://www.indilib.org/api/classINDI_1_1BaseClientQt.html) for cross-platform development based on the Qt5 toolkit.
* C# Application Development: [INDI Sharp](https://sourceforge.net/projects/indisharp/) project consists of a .NET/Mono assembly containing a simple INDI client and a WinForm that permits to connect to astronomical equipment INDI server, and to use drivers connected at it.
* Python Application Development: [PyINDI](https://indilib.org/support/tutorials/166-installing-and-using-the-python-pyndi-client-on-raspberry-pi.html) provides all the functionality requires to communicate with INDI server and drivers.
* Others: You can investigate [many existing INDI clients](https://indilib.org/about/clients.html), some of which are Open Source.

Officially, INDI Library provides BaseClient and BaseClientQt5 classes that encapsulates the most common functions required for a minimal INDI client. It supports:


* Connecting to INDI servers.
* Creation & manipulation of virtual devices: The class keeps a list of devices as they arrive from INDI server. It creates a virtual device to manage setting and retrieving the driver properties and status.
* Event notifications: The class sends event notification when:
    * New device is created.
    * New property vector is defined.
    * New property vector is set.
    * New message from driver.
    * Device property is removed.
    * Device is removed.
    * Server connected/disconnected.

INDI is shipped with a simple client example. To find more information, read the [INDI Client Development Tutorial](tutorial.md).
