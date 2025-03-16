---
title: Client Development
nav_order: 8
has_children: true
permalink: /clients/
---

# INDI Client Development

This guide provides a comprehensive overview of developing client applications that communicate with INDI drivers. It covers the INDI client API, how to connect to and control INDI devices, and best practices for client development.

## Introduction to INDI Client Development

An INDI client is defined as any software that communicates with either INDI server and/or drivers using the [INDI XML Protocol](../protocol/README.md). While clients may communicate with INDI drivers directly, in practice this is uncommon and all existing clients communicate with drivers via the [INDI server](../indiserver/README.md).

INDI clients can range from simple command-line tools to complex graphical applications. They can be used to control a single device or to orchestrate multiple devices for complex astronomical tasks such as imaging, guiding, and focusing.

The INDI client API provides a set of classes and functions for connecting to INDI servers, discovering devices, and controlling device properties.

## INDI Client Architecture

The INDI client architecture consists of the following components:

- **INDI Server**: A standalone process that manages the communication between INDI drivers and clients.
- **INDI Drivers**: Processes that communicate with hardware devices and expose their functionality through INDI properties.
- **INDI Clients**: Applications that connect to the INDI server, discover devices, and control device properties.

The communication between INDI clients and drivers is mediated by the INDI server, which routes messages between them. Clients connect to the server using TCP/IP sockets, and the server forwards messages to the appropriate drivers.

## Testing with Telnet

Since INDI is an XML-based protocol, it supports a wide variety of methods to communicate with the INDI server and drivers. You can even use Telnet to communicate with INDI server and type in the XML messages directly!

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

## INDI Client API

The INDI client API is provided by the `libindiclient` library, which is part of the INDI distribution. The API provides a set of classes for connecting to INDI servers, discovering devices, and controlling device properties.

### Client Development Options

Using Telnet can be useful to test INDI server, but not useful to create a client application. For this there are multiple options:

- **C/C++ Application Development**: [INDI::BaseClient](http://www.indilib.org/api/classINDI_1_1BaseClient.html) for POSIX systems and [INDI::BaseClientQt](http://www.indilib.org/api/classINDI_1_1BaseClientQt.html) for cross-platform development based on the Qt5 toolkit.
- **Python Application Development**: [PyINDI](https://indilib.org/support/tutorials/166-installing-and-using-the-python-pyndi-client-on-raspberry-pi.html) provides all the functionality required to communicate with INDI server and drivers.
- **Others**: You can investigate [many existing INDI clients](https://indilib.org/about/clients.html), some of which are Open Source.

Officially, INDI Library provides BaseClient and BaseClientQt5 classes that encapsulate the most common functions required for a minimal INDI client. They support:

- Connecting to INDI servers.
- Creation & manipulation of virtual devices: The class keeps a list of devices as they arrive from INDI server. It creates a virtual device to manage setting and retrieving the driver properties and status.
- Event notifications: The class sends event notification when:
  - New device is created.
  - New property vector is defined.
  - New property vector is set.
  - New message from driver.
  - Device property is removed.
  - Device is removed.
  - Server connected/disconnected.

### Key Classes

- **INDI::BaseClient**: The base class for INDI clients. It provides methods for connecting to INDI servers, sending commands to devices, and receiving updates from devices.
- **INDI::BaseDevice**: Represents an INDI device. It provides methods for accessing device properties and sending commands to the device.
- **INDI::Property**: Represents an INDI property. It provides methods for accessing property values and sending updates to the device.
- **INDI::PropertyNumber**: Represents a number property.
- **INDI::PropertyText**: Represents a text property.
- **INDI::PropertySwitch**: Represents a switch property.
- **INDI::PropertyLight**: Represents a light property.
- **INDI::PropertyBLOB**: Represents a BLOB (Binary Large Object) property.

### Connection Flow

The typical flow for an INDI client is as follows:

1. Create a client object that inherits from `INDI::BaseClient`.
2. Connect to the INDI server.
3. Discover devices and their properties.
4. Control device properties to perform desired actions.
5. Receive updates from devices.
6. Disconnect from the INDI server.

## INDI Client Implementation Approaches

There are two main approaches to implementing INDI clients: the traditional approach using virtual method overrides and the modern approach using callbacks. Both approaches are valid, and the choice depends on your specific requirements and programming style.

### Traditional Approach

The traditional approach involves subclassing `INDI::BaseClient` and overriding virtual methods to handle device and property events:

```cpp
class MyClient : public INDI::BaseClient
{
protected:
    // Override the server connection methods
    void newDevice(INDI::BaseDevice dp) override
    {
        // Called when a new device is discovered
        std::cout << "New device: " << dp.getDeviceName() << std::endl;
    }

    void removeDevice(INDI::BaseDevice dp) override
    {
        // Called when a device is removed
        std::cout << "Device removed: " << dp.getDeviceName() << std::endl;
    }

    void newProperty(INDI::Property property) override
    {
        // Called when a new property is defined
        std::cout << "New property: " << property.getName() << " for device " << property.getDeviceName() << std::endl;
    }

    void updateProperty(INDI::Property property) override
    {
        // Called when a property is updated
        std::cout << "Property updated: " << property.getName() << " for device " << property.getDeviceName() << std::endl;
    }

    void removeProperty(INDI::Property property) override
    {
        // Called when a property is removed
        std::cout << "Property removed: " << property.getName() << " for device " << property.getDeviceName() << std::endl;
    }

    void newMessage(INDI::BaseDevice dp, int messageID) override
    {
        // Called when a new message is received from a device
        std::cout << "New message from " << dp.getDeviceName() << ": " << dp.messageQueue(messageID) << std::endl;
    }

    void serverConnected() override
    {
        // Called when the client connects to the server
        std::cout << "Server connected" << std::endl;
    }

    void serverDisconnected(int exit_code) override
    {
        // Called when the client disconnects from the server
        std::cout << "Server disconnected with exit code " << exit_code << std::endl;
    }
};
```

This approach is useful for more complex clients that need to maintain state and handle multiple devices and properties. Here's an example of a client that controls a roll-off roof:

```cpp
class RoofClient : public INDI::BaseClient
{
public:
    RoofClient(const std::string &input, const std::string &output)
        : m_Input(input), m_Output(output) {}

    bool openRoof()
    {
        auto device = getDevice(m_Output.c_str());
        if (!device || !device.isConnected())
            return false;

        // Send command to open roof
        auto property = device.getSwitch("DIGITAL_OUTPUT_1");
        if (property)
        {
            property.reset();
            property[1].setState(ISS_ON);
            sendNewSwitch(property);
            return true;
        }
        return false;
    }

protected:
    void newDevice(INDI::BaseDevice dp) override
    {
        if (dp.isDeviceNameMatch(m_Input) && dp.isConnected())
            m_InputReady = true;

        if (dp.isDeviceNameMatch(m_Output) && dp.isConnected())
            m_OutputReady = true;
    }

    void updateProperty(INDI::Property property) override
    {
        // Check for property updates
        if (property.isNameMatch("DIGITAL_INPUT_1") && property.isDeviceNameMatch(m_Input))
        {
            // Update roof state based on input
            auto isOpen = property.getSwitch()[1].getState() == ISS_ON;
            if (m_StateCallback)
                m_StateCallback(isOpen);
        }
    }

private:
    std::string m_Input, m_Output;
    bool m_InputReady = false, m_OutputReady = false;
    std::function<void(bool)> m_StateCallback;
};
```

### Modern Approach with Callbacks

The modern approach uses the `watchDevice` and `watchProperty` methods to register callbacks for device and property events. This approach is more event-driven and allows for more flexible programming:

```cpp
class MyClient : public INDI::BaseClient
{
public:
    MyClient();
    ~MyClient() = default;

public:
    void setTemperature(double value);
    void takeExposure(double seconds);

protected:
    void newMessage(INDI::BaseDevice baseDevice, int messageID) override;

private:
    INDI::BaseDevice mSimpleCCD;
};

MyClient::MyClient()
{
    // wait for the availability of the device
    watchDevice("Simple CCD", [this](INDI::BaseDevice device)
    {
        mSimpleCCD = device; // save device

        // wait for the availability of the "CONNECTION" property
        device.watchProperty("CONNECTION", [this](INDI::Property)
        {
            IDLog("Connecting to INDI Driver...\n");
            connectDevice("Simple CCD");
        }, INDI::BaseDevice::WATCH_NEW);

        // wait for the availability of the "CCD_TEMPERATURE" property
        device.watchProperty("CCD_TEMPERATURE", [this](INDI::PropertyNumber property)
        {
            if (mSimpleCCD.isConnected())
            {
                IDLog("CCD is connected.\n");
                setTemperature(-20);
            }

            // call lambda function if property changed
            property.onUpdate([property, this]()
            {
                IDLog("Receiving new CCD Temperature: %g C\n", property[0].getValue());
                if (property[0].getValue() == -20)
                {
                    IDLog("CCD temperature reached desired value!\n");
                    takeExposure(1);
                }
            });
        }, INDI::BaseDevice::WATCH_NEW);

        // call if updated of the "CCD1" property - simplified way
        device.watchProperty("CCD1", [](INDI::PropertyBlob property)
        {
            // Save FITS file to disk
            std::ofstream myfile;

            myfile.open("ccd_simulator.fits", std::ios::out | std::ios::binary);
            myfile.write(static_cast<char *>(property[0].getBlob()), property[0].getBlobLen());
            myfile.close();

            IDLog("Received image saved as ccd_simulator.fits\n");
        }, INDI::BaseDevice::WATCH_UPDATE);
    });
}
```

### Choosing the Right Approach

Both approaches have their strengths:

- **Traditional Approach**: Better for complex clients that need to maintain state and handle multiple devices and properties. It provides more control over the client's behavior and is more suitable for integration into larger applications.

- **Modern Approach**: More concise and easier to use for simple clients. It's more event-driven and allows for more flexible programming with callbacks. It's particularly useful for quick prototyping and simple applications.

You can also mix both approaches in the same client, using the traditional approach for the overall structure and the modern approach for specific functionality.

## Creating a Basic INDI Client

Let's create a simple INDI client using the modern approach that connects to an INDI server, discovers devices, and controls device properties.

### Step 1: Create a Client Class

First, we need to create a class that inherits from `INDI::BaseClient`:

```cpp
#include <libindi/baseclient.h>
#include <libindi/basedevice.h>
#include <iostream>
#include <fstream>

class MyClient : public INDI::BaseClient
{
public:
    MyClient();
    ~MyClient() = default;

public:
    void setTemperature(double value);
    void takeExposure(double seconds);

protected:
    void newMessage(INDI::BaseDevice baseDevice, int messageID) override;

private:
    INDI::BaseDevice mSimpleCCD;
};
```

### Step 2: Implement the Client Class

Now, let's implement the client class:

```cpp
MyClient::MyClient()
{
    // wait for the availability of the device
    watchDevice("Simple CCD", [this](INDI::BaseDevice device)
    {
        mSimpleCCD = device; // save device

        // wait for the availability of the "CONNECTION" property
        device.watchProperty("CONNECTION", [this](INDI::Property)
        {
            IDLog("Connecting to INDI Driver...\n");
            connectDevice("Simple CCD");
        }, INDI::BaseDevice::WATCH_NEW);

        // wait for the availability of the "CCD_TEMPERATURE" property
        device.watchProperty("CCD_TEMPERATURE", [this](INDI::PropertyNumber property)
        {
            if (mSimpleCCD.isConnected())
            {
                IDLog("CCD is connected.\n");
                setTemperature(-20);
            }

            // call lambda function if property changed
            property.onUpdate([property, this]()
            {
                IDLog("Receiving new CCD Temperature: %g C\n", property[0].getValue());
                if (property[0].getValue() == -20)
                {
                    IDLog("CCD temperature reached desired value!\n");
                    takeExposure(1);
                }
            });
        }, INDI::BaseDevice::WATCH_NEW);

        // call if updated of the "CCD1" property - simplified way
        device.watchProperty("CCD1", [](INDI::PropertyBlob property)
        {
            // Save FITS file to disk
            std::ofstream myfile;

            myfile.open("ccd_simulator.fits", std::ios::out | std::ios::binary);
            myfile.write(static_cast<char *>(property[0].getBlob()), property[0].getBlobLen());
            myfile.close();

            IDLog("Received image saved as ccd_simulator.fits\n");
        }, INDI::BaseDevice::WATCH_UPDATE);
    });
}

void MyClient::setTemperature(double value)
{
    INDI::PropertyNumber ccdTemperature = mSimpleCCD.getProperty("CCD_TEMPERATURE");

    if (!ccdTemperature.isValid())
    {
        IDLog("Error: unable to find Simple CCD CCD_TEMPERATURE property...\n");
        return;
    }

    IDLog("Setting temperature to %g C.\n", value);
    ccdTemperature[0].setValue(value);
    sendNewProperty(ccdTemperature);
}

void MyClient::takeExposure(double seconds)
{
    INDI::PropertyNumber ccdExposure = mSimpleCCD.getProperty("CCD_EXPOSURE");

    if (!ccdExposure.isValid())
    {
        IDLog("Error: unable to find CCD Simulator CCD_EXPOSURE property...\n");
        return;
    }

    // Take a 1 second exposure
    IDLog("Taking a %g second exposure.\n", seconds);
    ccdExposure[0].setValue(seconds);
    sendNewProperty(ccdExposure);
}

void MyClient::newMessage(INDI::BaseDevice baseDevice, int messageID)
{
    if (!baseDevice.isDeviceNameMatch("Simple CCD"))
        return;

    IDLog("Receiving message from Server:\n"
          "    %s\n\n",
          baseDevice.messageQueue(messageID).c_str());
}
```

### Step 3: Create the Main Function

Now, let's create a main function that creates a client object and connects to the INDI server:

```cpp
int main(int, char *[])
{
    MyClient myClient;
    myClient.setServer("localhost", 7624);

    myClient.connectServer();

    myClient.setBLOBMode(B_ALSO, "Simple CCD", nullptr);

    myClient.enableDirectBlobAccess("Simple CCD", nullptr);

    std::cout << "Press Enter key to terminate the client.\n";
    std::cin.ignore();
}
```

### Step 4: Compile and Run the Client

To compile the client, you need to link against the `libindiclient` library:

```bash
g++ -o my_client my_client.cpp -lindiclient
```

To run the client:

```bash
./my_client
```

This client will connect to the INDI server running on the local machine, connect to the "Simple CCD" device, set the temperature to -20°C, and take a 1-second exposure when the temperature reaches the target value. It will then save the received image to a file.

## INDI Client API Reference

Here are the key concepts and methods available in the INDI client API:

### Watching for Devices

The `watchDevice` method allows you to register a callback that will be called when a device becomes available:

```cpp
watchDevice("Device Name", [](INDI::BaseDevice device)
{
    // Device is available
});
```

### Watching for Properties

The `watchProperty` method allows you to register a callback that will be called when a property becomes available or is updated:

```cpp
device.watchProperty("Property Name", [](INDI::Property property)
{
    // Property is available or updated
}, INDI::BaseDevice::WATCH_NEW); // or WATCH_UPDATE or WATCH_REMOVE
```

You can also use type-specific callbacks:

```cpp
device.watchProperty("Number Property", [](INDI::PropertyNumber property)
{
    // Number property is available or updated
}, INDI::BaseDevice::WATCH_NEW);

device.watchProperty("Switch Property", [](INDI::PropertySwitch property)
{
    // Switch property is available or updated
}, INDI::BaseDevice::WATCH_NEW);

device.watchProperty("Text Property", [](INDI::PropertyText property)
{
    // Text property is available or updated
}, INDI::BaseDevice::WATCH_NEW);

device.watchProperty("Light Property", [](INDI::PropertyLight property)
{
    // Light property is available or updated
}, INDI::BaseDevice::WATCH_NEW);

device.watchProperty("BLOB Property", [](INDI::PropertyBlob property)
{
    // BLOB property is available or updated
}, INDI::BaseDevice::WATCH_NEW);
```

### Property Updates

You can register a callback to be called when a property is updated:

```cpp
property.onUpdate([property]()
{
    // Property has been updated
});
```

### Accessing Property Values

You can access property values using array-like syntax:

```cpp
// Number property
double value = numberProperty[0].getValue();
numberProperty[0].setValue(newValue);

// Switch property
bool isOn = switchProperty[0].getState() == ISS_ON;
switchProperty[0].setState(ISS_ON);

// Text property
std::string text = textProperty[0].getText();
textProperty[0].setText(newText);

// BLOB property
void *blob = blobProperty[0].getBlob();
size_t blobSize = blobProperty[0].getBlobLen();
std::string format = blobProperty[0].getFormat();
```

### Sending Property Updates

To send property updates to the server:

```cpp
sendNewProperty(property);
```

Or for specific property types:

```cpp
sendNewNumber(numberProperty);
sendNewSwitch(switchProperty);
sendNewText(textProperty);
```

### Direct BLOB Access

For efficient handling of large binary data (like images), you can enable direct BLOB access:

```cpp
enableDirectBlobAccess("Device Name", "Property Name");
```

## Advanced Topics

For more advanced topics, including:

- Example: Controlling a CCD Camera
- Asynchronous Operation
- Error Handling
- BLOB Handling
- Multi-Device Control

Please refer to the [INDI Client Development Tutorial](tutorial.md).

## Resources

- [INDI Library API Documentation](https://www.indilib.org/api/index.html)
- [INDI Protocol Specification](https://www.indilib.org/develop/developer-manual/104-indi-protocol.html)
- [INDI Client Development Tutorial](tutorial.md)
- [PyINDI Documentation](https://indilib.org/support/tutorials/166-installing-and-using-the-python-pyndi-client-on-raspberry-pi.html)
- [INDI Clients](https://indilib.org/about/clients.html)
