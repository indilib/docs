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

```python
import PyIndi

class IndiClient(PyIndi.BaseClient):
    def __init__(self):
        super().__init__()

    def newProperty(self, prop):
        print(f"New property {prop.getName()} for device {prop.getDeviceName()}")

    def updateProperty(self, prop):
        print(f"Updated property {prop.getName()} for device {prop.getDeviceName()}")

client = IndiClient()
client.setServer("localhost", 7624)
client.connectServer()
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

### Source Examples:

```bash
git clone https://github.com/indilib/pyindi-client.git
cd pyindi-client/examples
```

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

## Docker Support

The repository includes a `Dockerfile` for containerized development and testing.

```bash
docker build -t pyindi-client .
docker run -it pyindi-client
```

## Contributing

We welcome contributions! Please submit issues or pull requests via the [GitHub repository](https://github.com/indilib/pyindi-client).

## License

`PyIndi-Client` is licensed under the GPLv3. See the [LICENSE](https://github.com/indilib/pyindi-client/blob/master/LICENSE) file for details.

