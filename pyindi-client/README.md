---
title: PyIndi Client
nav_order: 10
has_children: false
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
* `newBLOB(blob)`: Triggered when a binary large object is received.
* `newSwitch(switchVector)`, `newText(textVector)`, `newNumber(numberVector)`, `newLight(lightVector)`: Property value updates.

### Property Classes

#### INumber, INumberVectorProperty

Control numerical properties such as focus position or CCD temperature.

#### ISwitch, ISwitchVectorProperty

Manage toggle and selection properties (e.g., turning tracking on/off).

#### IText, ITextVectorProperty

Send or receive text-based commands or data.

#### IBLOB, IBLOBVectorProperty

Used for transmitting image or binary data from devices.

## Advanced Examples

### Setting a Property

```python
ccd = client.getDevice("CCD Simulator")
prop = ccd.getNumber("CCD_EXPOSURE")
prop["CCD_EXPOSURE_VALUE"].value = 5.0
client.sendNewNumber(prop)
```

### Enabling BLOB Mode

```python
client.setBLOBMode(PyIndi.B_ALSO, "CCD Simulator", None)
```

### Switching Telescope Tracking On

```python
telescope = client.getDevice("Telescope Simulator")
switch_vector = telescope.getSwitch("TELESCOPE_TRACK_STATE")
switch_vector["TRACK_ON"].s = PyIndi.ISS_ON
switch_vector["TRACK_OFF"].s = PyIndi.ISS_OFF
client.sendNewSwitch(switch_vector)
```

### Slewing Telescope to a Target

```python
telescope = client.getDevice("Telescope Simulator")
coords = telescope.getNumber("EQUATORIAL_EOD_COORD")
coords["RA"].value = 5.5      # RA in hours
coords["DEC"].value = -10.0   # DEC in degrees
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
ccd_exposure["CCD_EXPOSURE_VALUE"].value = 2.0
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
coords["RA"].value = 6.0
coords["DEC"].value = -5.0
client.sendNewNumber(coords)

# Wait for settling
time.sleep(10)

# Capture image
ccd = client.getDevice("CCD Simulator")
exposure = ccd.getNumber("CCD_EXPOSURE")
exposure["CCD_EXPOSURE_VALUE"].value = 3.0
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


# Migration to INDI Core 2.0.0

## For all property

Applies to all properties available in the library (Switch/Number/Text/Light/Blob)

New implementation since INDI Core 1.9.x

```python
# set
anyProperty.setDeviceName("Some device")
anyProperty.setName("Some name")
anyProperty.setLabel("Some label")
anyProperty.setGroupName("Some group")
anyProperty.setState(PyIndi.IPS_IDLE)
anyProperty.setTimestamp("123")
anyProperty.setPermission(PyIndi.IP_RO) # no effect for Light Property
anyProperty.setTimeout(123)             # no effect for Light Property

anyProperty[0].setName("Some name of widget")
anyProperty[0].setLabel("Some label of widget")

# get
device    = anyProperty.getDeviceName()
name      = anyProperty.getName()
label     = anyProperty.getLabel()
group     = anyProperty.getGroupName()
state     = anyProperty.getState()
timestamp = anyProperty.getTimestamp()
perm      = anyProperty.getPermission() # returns IP_RO for Light Property
timeout   = anyProperty.getTimeout()    # returns 0 for Light Property

name      = anyProperty[0].getName()
label     = anyProperty[0].getLabel()

# auxiliary functions
if anyProperty.isNameMatch("Some name"):
    # anyProperty.getName() is equal to "Some name"
    pass

if anyProperty.isLabelMatch("Some label"):
    # anyProperty.getLabel() is equal to "Some label"
    pass

if not anyProperty.isValid():
    # e.g. PyIndi.Property() is not valid because type is unknown
    # PyIndi.PropertyText(somePropertySwitch) is also not valid because type
    # is mismatch (invalid cast)
    pass

stringState  = anyProperty.getStateAsString()            # returns Idle/Ok/Busy/Alert
stringPerm   = anyProperty.getPermissionAsString()       # returns ro/wo/rw
someWidget   = anyProperty.findWidgetByName("Some name") # returns widget with `Some name` name
```

Deprecated implementation

```python
# set
anyProperty.device    = "Some device"
anyProperty.name      = "Some name"
anyProperty.label     = "Some label"
anyProperty.group     = "Some group"
anyProperty.s         = PyIndi.IPS_IDLE
anyProperty.timestamp = "123"
anyProperty.p         = PyIndi.IP_RO # except Light Property
anyProperty.timeout   = 123          # except Light Property

anyProperty[0].name   = "Some name of widget"
anyProperty[0].label  = "Some label of widget"

# get
device    = anyProperty.device
name      = anyProperty.name
label     = anyProperty.label
group     = anyProperty.group
state     = anyProperty.s
timestamp = anyProperty.timestamp
perm      = anyProperty.p       # except Light Property
timeout   = anyProperty.timeout # except Light Property

name  = anyProperty[0].name
label = anyProperty[0].label
```

## Property Text

New implementation since INDI Core 1.9.x

```python
# set
textProperty[0].setText("Some text")

# get
text  = textProperty[0].getText()
```

Deprecated implementation

```python
# set
textProperty[0].text  = "Some text"

# get
text  = textProperty[0].text
```

## Property Number

New implementation since INDI Core 1.9.x

```python
# set
numberProperty[0].setFormat("Some format")
numberProperty[0].setMin(0)
numberProperty[0].setMax(1000)
numberProperty[0].setMinMax(0, 1000) # simplification
numberProperty[0].setStep(1)
numberProperty[0].setValue(123)

# get
format = numberProperty[0].getFormat()
min    = numberProperty[0].getMin()
max    = numberProperty[0].getMax()
step   = numberProperty[0].getStep()
value  = numberProperty[0].getValue()
```

Deprecated implementation

```python
# set
numberProperty[0].format = "Some format"
numberProperty[0].min = 0
numberProperty[0].max = 1000
numberProperty[0].step = 1
numberProperty[0].value = 123

# get
format = numberProperty[0].format
min    = numberProperty[0].min
max    = numberProperty[0].max
step   = numberProperty[0].step
value  = numberProperty[0].value
```

## Property Switch

New implementation since INDI Core 1.9.x

```python
# set
switchProperty.setRule(PyIndi.ISR_NOFMANY)
switchProperty[0].setState(PyIndi.ISS_ON)

# get
rule  = switchProperty.getRule()
state = switchProperty[0].getState()

# auxiliary functions
stringRule  = switchProperty.getRuleAsString()     # returns OneOfMany/AtMostOne/AnyOfMany
stringState = switchProperty[0].getStateAsString() # returns On/Off
switchProperty.reset()                             # reset all widget switches to Off
switchProperty.findOnSwitchIndex()                 # find index of Widget with On state
switchProperty.findOnSwitch()                      # returns widget with On state
```

Deprecated implementation

```python
# set
switchProperty.r = PyIndi.ISR_NOFMANY
switchProperty[0].s = PyIndi.ISS_ON

# get
rule  = switchProperty.r
state = switchProperty[0].s
```

## Property Light

New implementation since INDI Core 1.9.x

```python
# set
lightProperty[0].setState(PyIndi.IPS_BUSY)

# get
lightState = lightProperty[0].getState()

# auxiliary functions
stringState = lightProperty[0].getStateAsString() # returns Idle/Ok/Busy/Alert
```

Deprecated implementation

```python
# set
lightProperty[0].s = PyIndi.IPS_BUSY

# get
lightState = lightProperty[0].s
```

## Property Blob

New implementation since INDI Core 1.9.x

```python
# set
blobProperty[0].setFormat("Some format")
blobProperty[0].setBlob("Some blob")
blobProperty[0].setBlobLen(123)
blobProperty[0].setSize(123)

# get
format  = blobProperty[0].getFormat()
blob    = blobProperty[0].getBlob()
bloblen = blobProperty[0].getBlobLen()
size    = blobProperty[0].getSize()
```

Deprecated implementation

```python
# set
blobProperty[0].format  = "Some format"
blobProperty[0].blob    = "Some blob"
blobProperty[0].bloblen = 123
blobProperty[0].size    = 123

# get
format  = blobProperty[0].format
blob    = blobProperty[0].blob
bloblen = blobProperty[0].bloblen
size    = blobProperty[0].size
```

## BaseClient virual methods

As of INDI Core 2.0.0, there are no features like:

- newText
- newSwitch
- newNumber
- newLight
- newBlob

They have been replaced with a single `updateProperty` function to differentiate the behavior between `newProperty` and to provide more memory control in the INDI Core library itself.

It is not required to implement all functions. Please use as needed.

```python
class IndiClient(PyIndi.BaseClient):
    def __init__(self):
        super(IndiClient, self).__init__()

    # Emmited when a new device is created from INDI server.
    def newDevice(self, dev):
        pass

    # Emmited when a device is deleted from INDI server.
    def removeDevice(self, dev):
        pass

    # Emmited when a new property is created for an INDI driver.
    def newProperty(self, genericProperty):
        pass

    # Emmited when a new property value arrives from INDI server.
    def updateProperty(self, genericProperty):
        pass

    # Emmited when a property is deleted for an INDI driver.
    def removeProperty(self, genericProperty):
        pass

    # Emmited when a new message arrives from INDI server.
    def newMessage(self, device, messageId):
        pass

    # Emmited when the server is connected.
    def serverConnected(self):
        pass

    # Emmited when the server gets disconnected.
    def serverDisconnected(self, exitCode):
        pass
```

The function `newProperty` and `updateProperty` provide in argument a generic type in which you can read common information for all properties such as name, group, device, type, state. To read values associated with a specific type, e.g. `Switch`, you need to perform cast.

```python
switchProperty = PyIndi.PropertySwitch(genericProperty)
```

If successful, the function `switchProperty.isValid()` will return `true`.

A similar mechanism existed before version 2.0.0 for the `newProperty` function.

### BaseClient Example

With Python, migration can be simple. Nothing stands in the way of creating a function for a given property type and naming it the same as for the INDI Core 1.x.x version

```python
class IndiClient(PyIndi.BaseClient):
    def __init__(self):
        super(IndiClient, self).__init__()

    # Call functions in old style
    def updateProperty(self, prop):
        if prop.getType() == PyIndi.INDI_NUMBER:
            self.newNumber(PyIndi.PropertyNumber(prop))
        elif prop.getType() == PyIndi.INDI_SWITCH:
            self.newSwitch(PyIndi.PropertySwitch(prop))
        elif prop.getType() == PyIndi.INDI_TEXT:
            self.newText(PyIndi.PropertyText(prop))
        elif prop.getType() == PyIndi.INDI_LIGHT:
            self.newLight(PyIndi.PropertyLight(prop))
        elif prop.getType() == PyIndi.INDI_BLOB:
            self.newBLOB(PyIndi.PropertyBlob(prop)[0])

    # The methods work again in INDI Core 2.0.0!
    def newBLOB(self, prop):
        print(f"new BLOB {prop.getName()}")
    def newSwitch(self, prop):
        print(f"new Switch {prop.getName()} for device {prop.getDeviceName()}")
    def newNumber(self, prop):
        print(f"new Number {prop.getName()} for device {prop.getDeviceName()}")
    def newText(self, prop):
        print(f"new Text {prop.getName()} for device {prop.getDeviceName()}")
    def newLight(self, prop):
        print(f"new Light {prop.getName()} for device {prop.getDeviceName()}")
```

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

