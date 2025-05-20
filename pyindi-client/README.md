---
title: PyIndi Client
nav_order: 10
has_children: true
permalink: /pyindi-client/
---

# PyIndi-Client

The `PyIndi-Client` library provides Python bindings for the INDI (Instrument-Neutral Distributed Interface) protocol, enabling seamless control of astronomical devices like telescopes, CCDs, filter wheels, focusers, and more. It bridges Python applications and the INDI server using SWIG-generated interfaces.

## Overview

`PyIndi-Client` allows developers to:

* Connect to a running INDI server
* Communicate with various INDI-compatible devices
* Monitor and control device properties and states
* Automate observation tasks using Python scripts

It is ideal for astronomers and developers seeking to build custom observatory control software in Python.

## Installation

### Prerequisites

Ensure the following dependencies are installed:

**Ubuntu/Debian:**

```bash
sudo apt-get install python3-dev python3-setuptools libindi-dev swig libcfitsio-dev libnova-dev
```

**Fedora:**

```bash
sudo dnf install python3-devel python3-setuptools libindi-devel swig libcfitsio-devel libnova-devel
```

### Installation Methods

#### From APT Repository (Ubuntu-based):

```bash
sudo add-apt-repository ppa:mutlaqja/ppa
sudo apt update
sudo apt install python3-indi-client
```

#### From PyPI:

```bash
pip install pyindi-client
```

#### From Source:

```bash
git clone https://github.com/indilib/pyindi-client.git
cd pyindi-client
python3 setup.py install
```

If you encounter build errors, ensure `libindiclient.a` is accessible. You may need to update the `libindisearchpaths` in `setup.py` accordingly.

## Basic Usage

In the following simple example, an INDI 2.0.0 client class is defined giving
the implementation of the virtual INDI client functions. This is not
mandatory. This class is instantiated once, and after defining server
host and port in this object, a list of devices together with their
properties is printed on the console.

```python
# for logging
import sys
import time
import logging
# import the PyIndi module
import PyIndi

# The IndiClient class which inherits from the module PyIndi.BaseClient class
# Note that all INDI constants are accessible from the module as PyIndi.CONSTANTNAME
class IndiClient(PyIndi.BaseClient):
    def __init__(self):
        super(IndiClient, self).__init__()
        self.logger = logging.getLogger('IndiClient')
        self.logger.info('creating an instance of IndiClient')

    def newDevice(self, d):
        '''Emmited when a new device is created from INDI server.'''
        self.logger.info(f"new device {d.getDeviceName()}")

    def removeDevice(self, d):
        '''Emmited when a device is deleted from INDI server.'''
        self.logger.info(f"remove device {d.getDeviceName()}")

    def newProperty(self, p):
        '''Emmited when a new property is created for an INDI driver.'''
        self.logger.info(f"new property {p.getName()} as {p.getTypeAsString()} for device {p.getDeviceName()}")

    def updateProperty(self, p):
        '''Emmited when a new property value arrives from INDI server.'''
        self.logger.info(f"update property {p.getName()} as {p.getTypeAsString()} for device {p.getDeviceName()}")

    def removeProperty(self, p):
        '''Emmited when a property is deleted for an INDI driver.'''
        self.logger.info(f"remove property {p.getName()} as {p.getTypeAsString()} for device {p.getDeviceName()}")

    def newMessage(self, d, m):
        '''Emmited when a new message arrives from INDI server.'''
        self.logger.info(f"new Message {d.messageQueue(m)}")

    def serverConnected(self):
        '''Emmited when the server is connected.'''
        self.logger.info(f"Server connected ({self.getHost()}:{self.getPort()})")

    def serverDisconnected(self, code):
        '''Emmited when the server gets disconnected.'''
        self.logger.info(f"Server disconnected (exit code = {code},{self.getHost()}:{self.getPort()})")

logging.basicConfig(format = '%(asctime)s %(message)s', level = logging.INFO)

# Create an instance of the IndiClient class and initialize its host/port members
indiClient=IndiClient()
indiClient.setServer("localhost", 7624)

# Connect to server
print("Connecting and waiting 1 sec")
if not indiClient.connectServer():
     print(f"No indiserver running on {indiClient.getHost()}:{indiClient.getPort()} - Try to run")
     print("  indiserver indi_simulator_telescope indi_simulator_ccd")
     sys.exit(1)

# Waiting for discover devices
time.sleep(1)

# Print list of devices. The list is obtained from the wrapper function getDevices as indiClient is an instance
# of PyIndi.BaseClient and the original C++ array is mapped to a Python List. Each device in this list is an
# instance of PyIndi.BaseDevice, so we use getDeviceName to print its actual name.
print("List of devices")
deviceList = indiClient.getDevices()
for device in deviceList:
    print(f"   > {device.getDeviceName()}")

# Print all properties and their associated values.
print("List of Device Properties")
for device in deviceList:

    print(f"-- {device.getDeviceName()}")
    genericPropertyList = device.getProperties()

    for genericProperty in genericPropertyList:
        print(f"   > {genericProperty.getName()} {genericProperty.getTypeAsString()}")

        if genericProperty.getType() == PyIndi.INDI_TEXT:
            for widget in PyIndi.PropertyText(genericProperty):
                print(f"       {widget.getName()}({widget.getLabel()}) = {widget.getText()}")

        if genericProperty.getType() == PyIndi.INDI_NUMBER:
            for widget in PyIndi.PropertyNumber(genericProperty):
                print(f"       {widget.getName()}({widget.getLabel()}) = {widget.getValue()}")

        if genericProperty.getType() == PyIndi.INDI_SWITCH:
            for widget in PyIndi.PropertySwitch(genericProperty):
                print(f"       {widget.getName()}({widget.getLabel()}) = {widget.getStateAsString()}")

        if genericProperty.getType() == PyIndi.INDI_LIGHT:
            for widget in PyIndi.PropertyLight(genericProperty):
                print(f"       {widget.getLabel()}({widget.getLabel()}) = {widget.getStateAsString()}")

        if genericProperty.getType() == PyIndi.INDI_BLOB:
            for widget in PyIndi.PropertyBlob(genericProperty):
                print(f"       {widget.getName()}({widget.getLabel()}) = <blob {widget.getSize()} bytes>")

# Disconnect from the indiserver
print("Disconnecting")
indiClient.disconnectServer()
```

## API Overview

### `BaseClient`

The main interface to the INDI server.

#### Methods:

* `setServer(hostname, port)`: Set the target server address.
* `connectServer()`: Connect to the INDI server.
* `disconnectServer()`: Disconnect from the server.
* `getDevices()`: Returns a list of connected INDI devices.
* `getDevice(name)`: Fetch a device by name.

#### Callbacks (to override):

* `newProperty(property)`: Triggered when a new property is defined.
* `removeProperty(property)`: Triggered when a property is removed.
* `updateProperty(property)`: Triggered when a property is updated.

### Property Classes

#### INDI::PropertyNumber

Control numerical properties such as focus position or CCD temperature.

#### INDI::PropertySwitch

Manage toggle and selection properties (e.g., turning tracking on/off).

#### INDI::PropertyText

Send or receive text-based commands or data.

#### INDI::PropertyBLOB

Used for transmitting image or binary data from devices.

## Advanced Examples

### Setting a Property
You can set the value directly if you know the element (widget) index or try to find it first, then set its value

```python
ccd = client.getDevice("CCD Simulator")
prop = ccd.getNumber("CCD_EXPOSURE")
# Set by index
prop[0].setValue(-5)
# OR Find Widget and Set value
widget = prop.findWidgetByName("CCD_TEMPERATURE_VALUE")
widget.setValue(-5)
# After adjusting property value, send back to client
client.sendNewNumber(prop)
```

### NewProperty and UpdateProperty
You may want to define how your client reacts when the INDI server sends notifications about new or updated properties.

```python
import PyIndi
import logging

class MyIndiClient(PyIndi.BaseClient):
    def __init__(self):
        super(MyIndiClient, self).__init__()
        self.logger = logging.getLogger('MyIndiClient')

    def newDevice(self, d):
        self.logger.info(f"New device connected: {d.getDeviceName()}")

    def newProperty(self, p):
        self.logger.info(f"New property '{p.getName()}' for device '{p.getDeviceName()}' of type '{p.getTypeAsString()}'")
        # You might want to store this property object or process its initial state here
        if p.getName() == "CCD_TEMPERATURE":
            number_property = PyIndi.PropertyNumber(p)
            for element in number_property:
                self.logger.info(f"  {element.name} ({element.label}) = {element.value}")
        elif p.getName() == "CCD_EXPOSURE":
            number_property = PyIndi.PropertyNumber(p)
            for element in number_property:
                self.logger.info(f"  {element.name} ({element.label}) = {element.value}")
                # You could set an initial exposure time here, for example:
                # element.value = 10.0
                # self.sendNewNumber(number_property)

    def updateProperty(self, p):
        self.logger.info(f"Property '{p.getName()}' for device '{p.getDeviceName()}' updated")
        if p.getName() == "CCD_TEMPERATURE":
            number_property = PyIndi.PropertyNumber(p)
            for element in number_property:
                self.logger.info(f"  {element.name} ({element.label}) = {element.value}")
                # You could react to temperature changes here
        elif p.getName() == "CCD_EXPOSURE":
            number_property = PyIndi.PropertyNumber(p)
            for element in number_property:
                self.logger.info(f"  {element.name} ({element.label}) = {element.value}")
                # You might check if the exposure has finished here

    # ... other necessary methods like newMessage, serverConnected, serverDisconnected, etc.

if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)
    indi_client = MyIndiClient()
    indi_client.setServer("localhost", 7624)  # Replace with your INDI server address and port

    if not indi_client.connectServer():
        print("Could not connect to INDI server!")
        exit(1)

    try:
        while True:
            pass  # Keep the client running to receive events
    except KeyboardInterrupt:
        indi_client.disconnectServer()
```

### Enabling BLOB Mode

```python
client.setBLOBMode(PyIndi.B_ALSO, "CCD Simulator", None)
```

### Switching Telescope Tracking On

```python
telescope = client.getDevice("Telescope Simulator")
switch_vector = telescope.getSwitch("TELESCOPE_TRACK_STATE")
# By index
switch_vector[0].setState(PyIndi.ISS_ON)
switch_vector[1].setState(PyIndi.ISS_OFF)
# OR Find Widget and Set value
widget = prop.findWidgetByName("TRACK_ON")
if widget:
    widget.setState(PyIndi.ISS_ON)
widget = prop.findWidgetByName("TRACK_OFF")
if widget:
    widget.setState(PyIndi.ISS_OFF)
client.sendNewSwitch(switch_vector)
```

### Slewing Telescope to a Target

```python
telescope = client.getDevice("Telescope Simulator")
coords = telescope.getNumber("EQUATORIAL_EOD_COORD")
coords[0].setValue(5.5)      # RA in hours
coords[1].setValue(-10.0)   # DEC in degrees
client.sendNewNumber(coords)
```

### Capturing and Saving CCD Image

```python
class MyClient(PyIndi.BaseClient):
    def newBLOB(self, bp):
        for b in bp:
            with open("image.fits", "wb") as f:
                f.write(b.getblob())
                print("Saved image.fits")

client = MyClient()
client.setServer("localhost", 7624)
client.connectServer()
client.setBLOBMode(PyIndi.B_ALSO, "CCD Simulator", None)

ccd = client.getDevice("CCD Simulator")
ccd_exposure = ccd.getNumber("CCD_EXPOSURE")
ccd_exposure[0].setValue(2.0)
client.sendNewNumber(ccd_exposure)
```

### Full Automation Script Example

```python
import time

class AutomationClient(PyIndi.BaseClient):
    def newBLOB(self, bp):
        for b in bp:
            filename = f"capture_{int(time.time())}.fits"
            with open(filename, "wb") as f:
                f.write(b.getblob())
            print(f"Saved {filename}")

client = AutomationClient()
client.setServer("localhost", 7624)
client.connectServer()
client.setBLOBMode(PyIndi.B_ALSO, "CCD Simulator", None)

# Slew telescope
telescope = client.getDevice("Telescope Simulator")
coords = telescope.getNumber("EQUATORIAL_EOD_COORD")
coords[0].value = 6.0
coords[1].value = -5.0
client.sendNewNumber(coords)

# Wait for settling
time.sleep(10)

# Capture image
ccd = client.getDevice("CCD Simulator")
exposure = ccd.getNumber("CCD_EXPOSURE")
exposure[0].value = 3.0
client.sendNewNumber(exposure)
```

### Notes

See the
[examples](https://github.com/indilib/pyindi-client/tree/master/examples)
for more simple demos of using **pyindi-client**.

See the [interface
file](https://github.com/indilib/pyindi-client/blob/master/indiclientpython.i)
for an insight of what is wrapped and how.

For documentation on the methods of INDI Client API, refer to the [INDI
C++ API documentation](http://www.indilib.org/api/index.html).

# INDI Version Compatibility

| Versions        | commit                        | pip                                                                                         |
| --------------- | ----------------------------- | ------------------------------------------------------------------------------------------- |
| v2.0.4 - latest | HEAD                          | `pip3 install 'git+https://github.com/indilib/pyindi-client.git'`                           |
| v2.0.0 - v2.0.3 | indilib/pyindi-client@674706f | `pip3 install 'git+https://github.com/indilib/pyindi-client.git@674706f#egg=pyindi-client'` |
| v1.9.9          | indilib/pyindi-client@ce808b7 | `pip3 install 'git+https://github.com/indilib/pyindi-client.git@ce808b7#egg=pyindi-client'` |
| v1.9.8          | indilib/pyindi-client@ffd939b | `pip3 install 'git+https://github.com/indilib/pyindi-client.git@ffd939b#egg=pyindi-client'` |

# Docker Support

The repository includes a `Dockerfile` for containerized development and testing.

```bash
docker build -t pyindi-client .
docker run -it pyindi-client
```

# Contributing

We welcome contributions! Please submit issues or pull requests via the [GitHub repository](https://github.com/indilib/pyindi-client).

# License

`PyIndi-Client` is licensed under the GPLv3. See the [LICENSE](https://github.com/indilib/pyindi-client/blob/master/LICENSE) file for details.

