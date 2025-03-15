---
sort: 7
---

# INDI Client Development

This guide provides a comprehensive overview of developing client applications that communicate with INDI drivers. It covers the INDI client API, how to connect to and control INDI devices, and best practices for client development.

## Introduction to INDI Client Development

INDI (Instrument-Neutral Distributed Interface) is a protocol for controlling astronomical equipment. While INDI drivers handle the communication with the hardware devices, INDI clients provide the user interface and high-level functionality for controlling these devices.

INDI clients can range from simple command-line tools to complex graphical applications. They can be used to control a single device or to orchestrate multiple devices for complex astronomical tasks such as imaging, guiding, and focusing.

The INDI client API provides a set of classes and functions for connecting to INDI servers, discovering devices, and controlling device properties. This guide will walk you through the process of developing INDI clients using the C++ client API.

## INDI Client Architecture

The INDI client architecture consists of the following components:

- **INDI Server**: A standalone process that manages the communication between INDI drivers and clients.
- **INDI Drivers**: Processes that communicate with hardware devices and expose their functionality through INDI properties.
- **INDI Clients**: Applications that connect to the INDI server, discover devices, and control device properties.

The communication between INDI clients and drivers is mediated by the INDI server, which routes messages between them. Clients connect to the server using TCP/IP sockets, and the server forwards messages to the appropriate drivers.

## INDI Client API

The INDI client API is provided by the `libindiclient` library, which is part of the INDI distribution. The API provides a set of classes for connecting to INDI servers, discovering devices, and controlling device properties.

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

## Creating a Basic INDI Client

Let's create a simple INDI client that connects to an INDI server, discovers devices, and prints their properties.

### Step 1: Create a Client Class

First, we need to create a class that inherits from `INDI::BaseClient` and overrides the necessary methods:

```cpp
#include <libindi/baseclient.h>
#include <libindi/basedevice.h>
#include <iostream>

class SimpleClient : public INDI::BaseClient
{
public:
    SimpleClient() {}
    ~SimpleClient() {}

protected:
    // Override the server connection methods
    void newDevice(INDI::BaseDevice *dp) override
    {
        std::cout << "New device: " << dp->getDeviceName() << std::endl;
    }

    void removeDevice(INDI::BaseDevice *dp) override
    {
        std::cout << "Device removed: " << dp->getDeviceName() << std::endl;
    }

    void newProperty(INDI::Property *property) override
    {
        std::cout << "New property: " << property->getName() << " for device " << property->getDeviceName() << std::endl;
    }

    void removeProperty(INDI::Property *property) override
    {
        std::cout << "Property removed: " << property->getName() << " for device " << property->getDeviceName() << std::endl;
    }

    void newMessage(INDI::BaseDevice *dp, int messageID) override
    {
        std::cout << "New message from " << dp->getDeviceName() << ": " << dp->messageQueue(messageID) << std::endl;
    }

    void serverConnected() override
    {
        std::cout << "Server connected" << std::endl;
    }

    void serverDisconnected(int exit_code) override
    {
        std::cout << "Server disconnected with exit code " << exit_code << std::endl;
    }
};
```

### Step 2: Connect to the INDI Server

Now, let's create a main function that creates a client object and connects to the INDI server:

```cpp
int main(int argc, char *argv[])
{
    SimpleClient client;

    // Connect to the INDI server
    std::cout << "Connecting to INDI server..." << std::endl;
    if (!client.connectServer())
    {
        std::cerr << "Failed to connect to INDI server" << std::endl;
        return 1;
    }

    // Wait for user input to disconnect
    std::cout << "Press Enter to disconnect" << std::endl;
    std::cin.get();

    // Disconnect from the INDI server
    std::cout << "Disconnecting from INDI server..." << std::endl;
    client.disconnectServer();

    return 0;
}
```

### Step 3: Compile and Run the Client

To compile the client, you need to link against the `libindiclient` library:

```bash
g++ -o simple_client simple_client.cpp -lindiclient
```

To run the client:

```bash
./simple_client
```

This simple client will connect to the INDI server running on the local machine, discover devices, and print their properties. It will then wait for user input before disconnecting from the server.

## Controlling INDI Devices

Now that we have a basic client that can connect to the INDI server and discover devices, let's extend it to control device properties.

### Connecting to a Device

To connect to a device, we need to send a connection request to the device. This is done by setting the `CONNECTION` property of the device:

```cpp
void SimpleClient::connectDevice(const char *deviceName)
{
    INDI::BaseDevice *device = getDevice(deviceName);
    if (!device)
    {
        std::cerr << "Device " << deviceName << " not found" << std::endl;
        return;
    }

    // Find the CONNECTION property
    INDI::Property *connectionProperty = device->getProperty("CONNECTION");
    if (!connectionProperty)
    {
        std::cerr << "CONNECTION property not found for device " << deviceName << std::endl;
        return;
    }

    // Set the CONNECT switch to ON
    ISwitchVectorProperty *connectionSwitchVector = connectionProperty->getSwitch();
    ISwitch *connectSwitch = IUFindSwitch(connectionSwitchVector, "CONNECT");
    if (!connectSwitch)
    {
        std::cerr << "CONNECT switch not found for device " << deviceName << std::endl;
        return;
    }

    // Turn on the CONNECT switch
    connectSwitch->s = ISS_ON;

    // Send the updated property to the server
    sendNewSwitch(connectionSwitchVector);
}
```

### Setting a Number Property

To set a number property, we need to find the property and update its value:

```cpp
void SimpleClient::setNumberProperty(const char *deviceName, const char *propertyName, const char *elementName, double value)
{
    INDI::BaseDevice *device = getDevice(deviceName);
    if (!device)
    {
        std::cerr << "Device " << deviceName << " not found" << std::endl;
        return;
    }

    // Find the property
    INDI::Property *property = device->getProperty(propertyName);
    if (!property)
    {
        std::cerr << "Property " << propertyName << " not found for device " << deviceName << std::endl;
        return;
    }

    // Get the number vector
    INumberVectorProperty *numberVector = property->getNumber();
    if (!numberVector)
    {
        std::cerr << "Property " << propertyName << " is not a number property" << std::endl;
        return;
    }

    // Find the element
    INumber *element = IUFindNumber(numberVector, elementName);
    if (!element)
    {
        std::cerr << "Element " << elementName << " not found in property " << propertyName << std::endl;
        return;
    }

    // Set the value
    element->value = value;

    // Send the updated property to the server
    sendNewNumber(numberVector);
}
```

### Setting a Switch Property

To set a switch property, we need to find the property and update its state:

```cpp
void SimpleClient::setSwitchProperty(const char *deviceName, const char *propertyName, const char *elementName)
{
    INDI::BaseDevice *device = getDevice(deviceName);
    if (!device)
    {
        std::cerr << "Device " << deviceName << " not found" << std::endl;
        return;
    }

    // Find the property
    INDI::Property *property = device->getProperty(propertyName);
    if (!property)
    {
        std::cerr << "Property " << propertyName << " not found for device " << deviceName << std::endl;
        return;
    }

    // Get the switch vector
    ISwitchVectorProperty *switchVector = property->getSwitch();
    if (!switchVector)
    {
        std::cerr << "Property " << propertyName << " is not a switch property" << std::endl;
        return;
    }

    // Find the element
    ISwitch *element = IUFindSwitch(switchVector, elementName);
    if (!element)
    {
        std::cerr << "Element " << elementName << " not found in property " << propertyName << std::endl;
        return;
    }

    // Set the state based on the rule
    if (switchVector->r == ISR_1OFMANY)
    {
        // For ISR_1OFMANY, turn off all switches and turn on the selected one
        IUResetSwitch(switchVector);
        element->s = ISS_ON;
    }
    else if (switchVector->r == ISR_ATMOST1)
    {
        // For ISR_ATMOST1, turn off all switches if the selected one is already on, otherwise turn it on
        if (element->s == ISS_ON)
        {
            IUResetSwitch(switchVector);
        }
        else
        {
            IUResetSwitch(switchVector);
            element->s = ISS_ON;
        }
    }
    else // ISR_NOFMANY
    {
        // For ISR_NOFMANY, toggle the state of the selected switch
        element->s = (element->s == ISS_ON) ? ISS_OFF : ISS_ON;
    }

    // Send the updated property to the server
    sendNewSwitch(switchVector);
}
```

### Setting a Text Property

To set a text property, we need to find the property and update its value:

```cpp
void SimpleClient::setTextProperty(const char *deviceName, const char *propertyName, const char *elementName, const char *text)
{
    INDI::BaseDevice *device = getDevice(deviceName);
    if (!device)
    {
        std::cerr << "Device " << deviceName << " not found" << std::endl;
        return;
    }

    // Find the property
    INDI::Property *property = device->getProperty(propertyName);
    if (!property)
    {
        std::cerr << "Property " << propertyName << " not found for device " << deviceName << std::endl;
        return;
    }

    // Get the text vector
    ITextVectorProperty *textVector = property->getText();
    if (!textVector)
    {
        std::cerr << "Property " << propertyName << " is not a text property" << std::endl;
        return;
    }

    // Find the element
    IText *element = IUFindText(textVector, elementName);
    if (!element)
    {
        std::cerr << "Element " << elementName << " not found in property " << propertyName << std::endl;
        return;
    }

    // Set the text
    IUSaveText(element, text);

    // Send the updated property to the server
    sendNewText(textVector);
}
```

### Receiving Property Updates

To receive property updates, we need to override the `newNumber`, `newSwitch`, `newText`, and `newBLOB` methods of the `INDI::BaseClient` class:

```cpp
void SimpleClient::newNumber(INumberVectorProperty *nvp)
{
    std::cout << "Number property updated: " << nvp->name << " for device " << nvp->device << std::endl;

    // Print the values of all elements
    for (int i = 0; i < nvp->nnp; i++)
    {
        std::cout << "  " << nvp->np[i].name << ": " << nvp->np[i].value << std::endl;
    }
}

void SimpleClient::newSwitch(ISwitchVectorProperty *svp)
{
    std::cout << "Switch property updated: " << svp->name << " for device " << svp->device << std::endl;

    // Print the state of all elements
    for (int i = 0; i < svp->nsp; i++)
    {
        std::cout << "  " << svp->sp[i].name << ": " << (svp->sp[i].s == ISS_ON ? "On" : "Off") << std::endl;
    }
}

void SimpleClient::newText(ITextVectorProperty *tvp)
{
    std::cout << "Text property updated: " << tvp->name << " for device " << tvp->device << std::endl;

    // Print the values of all elements
    for (int i = 0; i < tvp->ntp; i++)
    {
        std::cout << "  " << tvp->tp[i].name << ": " << tvp->tp[i].text << std::endl;
    }
}

void SimpleClient::newBLOB(IBLOB *bp)
{
    std::cout << "BLOB property updated: " << bp->name << " for device " << bp->bvp->device << std::endl;
    std::cout << "  Size: " << bp->size << " bytes" << std::endl;
    std::cout << "  Format: " << bp->format << std::endl;
}
```

## Example: Controlling a CCD Camera

Let's create a more complex example that controls a CCD camera. This example will connect to the camera, set the exposure time, take an image, and save it to a file.

```cpp
#include <libindi/baseclient.h>
#include <libindi/basedevice.h>
#include <fstream>
#include <iostream>
#include <chrono>
#include <thread>

class CCDClient : public INDI::BaseClient
{
public:
    CCDClient() : connected(false), exposureComplete(false) {}
    ~CCDClient() {}

    // Connect to the INDI server and the CCD device
    bool connectCCD(const char *deviceName)
    {
        // Connect to the INDI server
        if (!connectServer())
        {
            std::cerr << "Failed to connect to INDI server" << std::endl;
            return false;
        }

        // Wait for the device to be discovered
        std::cout << "Waiting for device " << deviceName << "..." << std::endl;
        for (int i = 0; i < 10; i++)
        {
            if (getDevice(deviceName))
                break;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // Check if the device was discovered
        if (!getDevice(deviceName))
        {
            std::cerr << "Device " << deviceName << " not found" << std::endl;
            return false;
        }

        // Connect to the device
        std::cout << "Connecting to device " << deviceName << "..." << std::endl;
        connectDevice(deviceName);

        // Wait for the device to connect
        for (int i = 0; i < 10; i++)
        {
            if (connected)
                break;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // Check if the device is connected
        if (!connected)
        {
            std::cerr << "Failed to connect to device " << deviceName << std::endl;
            return false;
        }

        std::cout << "Connected to device " << deviceName << std::endl;
        return true;
    }

    // Take an exposure
    bool takeExposure(const char *deviceName, double exposureTime)
    {
        INDI::BaseDevice *device = getDevice(deviceName);
        if (!device)
        {
            std::cerr << "Device " << deviceName << " not found" << std::endl;
            return false;
        }

        // Find the CCD_EXPOSURE property
        INDI::Property *exposureProperty = device->getProperty("CCD_EXPOSURE");
        if (!exposureProperty)
        {
            std::cerr << "CCD_EXPOSURE property not found for device " << deviceName << std::endl;
            return false;
        }

        // Get the number vector
        INumberVectorProperty *exposureVector = exposureProperty->getNumber();
        if (!exposureVector)
        {
            std::cerr << "CCD_EXPOSURE is not a number property" << std::endl;
            return false;
        }

        // Find the exposure element
        INumber *exposureElement = IUFindNumber(exposureVector, "CCD_EXPOSURE_VALUE");
        if (!exposureElement)
        {
            std::cerr << "CCD_EXPOSURE_VALUE element not found in CCD_EXPOSURE property" << std::endl;
            return false;
        }

        // Set the exposure time
        exposureElement->value = exposureTime;

        // Reset the exposure complete flag
        exposureComplete = false;

        // Send the updated property to the server
        std::cout << "Starting exposure of " << exposureTime << " seconds..." << std::endl;
        sendNewNumber(exposureVector);

        // Wait for the exposure to complete
        std::cout << "Waiting for exposure to complete..." << std::endl;
        for (int i = 0; i < int(exposureTime) + 10; i++)
        {
            if (exposureComplete)
                break;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // Check if the exposure completed
        if (!exposureComplete)
        {
            std::cerr << "Exposure did not complete within the expected time" << std::endl;
            return false;
        }

        std::cout << "Exposure complete" << std::endl;
        return true;
    }

    // Save the image to a file
    bool saveImage(const char *filename)
    {
        if (!imageData.empty())
        {
            std::ofstream file(filename, std::ios::binary);
            if (file.is_open())
            {
                file.write(imageData.data(), imageData.size());
                file.close();
                std::cout << "Image saved to " << filename << std::endl;
                return true;
            }
            else
            {
                std::cerr << "Failed to open file " << filename << " for writing" << std::endl;
                return false;
            }
        }
        else
        {
            std::cerr << "No image data available" << std::endl;
            return false;
        }
    }

protected:
    // Override the server connection methods
    void newDevice(INDI::BaseDevice *dp) override
    {
        std::cout << "New device: " << dp->getDeviceName() << std::endl;
    }

    void newProperty(INDI::Property *property) override
    {
        // Check if this is the CONNECTION property
        if (strcmp(property->getName(), "CONNECTION") == 0)
        {
            // Get the switch vector
            ISwitchVectorProperty *connectionSwitchVector = property->getSwitch();
            if (connectionSwitchVector)
            {
                // Find the CONNECT switch
                ISwitch *connectSwitch = IUFindSwitch(connectionSwitchVector, "CONNECT");
                if (connectSwitch)
                {
                    // Check if the device is connected
                    connected = (connectSwitch->s == ISS_ON);
                }
            }
        }
    }

    void newSwitch(ISwitchVectorProperty *svp) override
    {
        // Check if this is the CONNECTION property
        if (strcmp(svp->name, "CONNECTION") == 0)
        {
            // Find the CONNECT switch
            ISwitch *connectSwitch = IUFindSwitch(svp, "CONNECT");
            if (connectSwitch)
            {
                // Check if the device is connected
                connected = (connectSwitch->s == ISS_ON);
            }
        }
    }

    void newNumber(INumberVectorProperty *nvp) override
    {
        // Check if this is the CCD_EXPOSURE property
        if (strcmp(nvp->name, "CCD_EXPOSURE") == 0)
        {
            // Find the CCD_EXPOSURE_VALUE element
            INumber *exposureElement = IUFindNumber(nvp, "CCD_EXPOSURE_VALUE");
            if (exposureElement)
            {
                // Check if the exposure is complete
                if (exposureElement->value == 0)
                {
                    exposureComplete = true;
                }
            }
        }
    }

    void newBLOB(IBLOB *bp) override
    {
        std::cout << "Received BLOB: " << bp->name << " for device " << bp->bvp->device << std::endl;
        std::cout << "  Size: " << bp->size << " bytes" << std::endl;
        std::cout << "  Format: " << bp->format << std::endl;

        // Save the image data
        imageData.assign(static_cast<char *>(bp->blob), static_cast<char *>(bp->blob) + bp->size);
    }

    void serverConnected() override
    {
        std::cout << "Server connected" << std::endl;
    }

    void serverDisconnected(int exit_code) override
    {
        std::cout << "Server disconnected with exit code " << exit_code << std::endl;
    }

private:
    bool connected;
    bool exposureComplete;
    std::vector<char> imageData;
};

int main(int argc, char *argv[])
{
    // Check command line arguments
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <device_name> [exposure_time]" << std::endl;
        return 1;
    }

    const char *deviceName = argv[1];
    double exposureTime = (argc > 2) ? std::stod(argv[2]) : 1.0;

    CCDClient client;

    // Connect to the CCD device
    if (!client.connectCCD(deviceName))
    {
        return 1;
    }

    // Take an exposure
    if (!client.takeExposure(deviceName, exposureTime))
    {
        return 1;
    }

    // Save the image
    if (!client.saveImage("image.fits"))
    {
        return 1;
    }

    // Disconnect from the INDI server
    std::cout << "Disconnecting from INDI server..." << std::endl;
    client.disconnectServer();

    return 0;
}
```

To compile this example:

```bash
g++ -o ccd_client ccd_client.cpp -lindiclient
```

To run the example:

```bash
./ccd_client "CCD Simulator" 2.0
```

This will connect to the CCD Simulator device, take a 2-second exposure, and save the image to a file named `image.fits`.

## Advanced Topics

### Asynchronous Operation

The examples above use a synchronous approach, where the client waits for operations to complete before proceeding. In a real application, you might want to use an asynchronous approach, where the client continues to process events while waiting for operations to complete.

To implement an asynchronous approach, you can use a separate thread for the INDI client, or you can use an event loop to process INDI events:

```cpp
#include <libindi/baseclient.h>
#include <libindi/basedevice.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>

class AsyncClient : public INDI::BaseClient
{
public:
    AsyncClient() : connected(false), running(false) {}
    ~AsyncClient()
    {
        // Stop the event loop if it's running
        if (running)
        {
            running = false;
            if (eventThread.joinable())
                eventThread.join();
        }
    }

    // Start the event loop
    void startEventLoop()
    {
        running = true;
        eventThread = std::thread(&AsyncClient::eventLoop, this);
    }

    // Stop the event loop
    void stopEventLoop()
    {
        running = false;
        if (eventThread.joinable())
            eventThread.join();
    }

protected:
    // Event loop
    void eventLoop()
    {
        while (running)
        {
            // Process INDI events
            // ...

            // Sleep for a short time to avoid busy waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    // Override the server connection methods
    void newDevice(INDI::BaseDevice *dp) override
    {
        std::cout << "New device: " << dp->getDeviceName() << std::endl;
    }

    void newProperty(INDI::Property *property) override
    {
        // ...
    }

    // Other overrides...

private:
    bool connected;
    std::atomic<bool> running;
    std::thread eventThread;
};
```

### Error Handling

In a real application, you need to handle errors that may occur during INDI operations. Here are some common error scenarios and how to handle them:

- **Connection Errors**: If the client fails to connect to the INDI server, you should provide a clear error message and allow the user to retry or configure the connection.
- **Device Errors**: If a device reports an error (e.g., a property state is `IPS_ALERT`), you should display the error message to the user and take appropriate action.
- **Timeout Errors**: If an operation takes too long to complete, you should provide a timeout mechanism and allow the user to cancel the operation.

Here's an example of how to handle property state changes:

```cpp
void AsyncClient::newNumber(INumberVectorProperty *nvp)
{
    // Check the property state
    switch (nvp->s)
    {
        case IPS_IDLE:
            std::cout << "Property " << nvp->name << " is idle" << std::endl;
            break;
        case IPS_OK:
            std::cout << "Property " << nvp->name << " is OK" << std::endl;
            break;
        case IPS_BUSY:
            std::cout << "Property " << nvp->name << " is busy" << std::endl;
            break;
        case IPS_ALERT:
            std::cerr << "Property " << nvp->name << " is in alert state" << std::endl;
            // Handle the error
            // ...
            break;
    }
}
```

### BLOB Handling

BLOB (Binary Large Object) properties are used to transfer binary data, such as images, between INDI drivers and clients. Handling BLOBs requires special attention due to their potentially large size.

By default, INDI clients do not receive BLOBs. To enable BLOB reception, you need to call the `enableBLOB` method:

```cpp
// Enable BLOB reception for all properties of the device
client.enableBLOB(true, "CCD Simulator");

// Enable BLOB reception for a specific property
client.enableBLOB(true, "CCD Simulator", "CCD1");
```

When a BLOB is received, the `newBLOB` method is called with a pointer to the BLOB data:

```cpp
void AsyncClient::newBLOB(IBLOB *bp)
{
    std::cout << "Received BLOB: " << bp->name << " for device " << bp->bvp->device << std::endl;
    std::cout << "  Size: " << bp->size << " bytes" << std::endl;
    std::cout << "  Format: " << bp->format << std::endl;

    // Process the BLOB data
    // ...
}
```

### Multi-Device Control

In many astronomical applications, you need to control multiple devices simultaneously. For example, you might want to control a telescope, a CCD camera, and a filter wheel to perform automated imaging.

To control multiple devices, you can use a single INDI client that connects to all the devices and coordinates their operations:

```cpp
class MultiDeviceClient : public INDI::BaseClient
{
public:
    MultiDeviceClient() {}
    ~MultiDeviceClient() {}

    // Connect to multiple devices
    bool connectDevices(const std::vector<std::string> &deviceNames)
    {
        // Connect to the INDI server
        if (!connectServer())
        {
            std::cerr << "Failed to connect to INDI server" << std::endl;
            return false;
        }

        // Wait for the devices to be discovered
        std::cout << "Waiting for devices..." << std::endl;
        for (int i = 0; i < 10; i++)
        {
            bool allDevicesFound = true;
            for (const auto &deviceName : deviceNames)
            {
                if (!getDevice(deviceName.c_str()))
                {
                    allDevicesFound = false;
                    break;
                }
            }
            if (allDevicesFound)
                break;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // Check if all devices were discovered
        for (const auto &deviceName : deviceNames)
        {
            if (!getDevice(deviceName.c_str()))
            {
                std::cerr << "Device " << deviceName << " not found" << std::endl;
                return false;
            }
        }

        // Connect to each device
        for (const auto &deviceName : deviceNames)
        {
            std::cout << "Connecting to device " << deviceName << "..." << std::endl;
            connectDevice(deviceName.c_str());
        }

        // Wait for all devices to connect
        for (int i = 0; i < 10; i++)
        {
            bool allDevicesConnected = true;
            for (const auto &deviceName : deviceNames)
            {
                INDI::BaseDevice *device = getDevice(deviceName.c_str());
                if (!device || !device->isConnected())
                {
                    allDevicesConnected = false;
                    break;
                }
            }
            if (allDevicesConnected)
                break;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // Check if all devices are connected
        for (const auto &deviceName : deviceNames)
        {
            INDI::BaseDevice *device = getDevice(deviceName.c_str());
            if (!device || !device->isConnected())
            {
                std::cerr << "Failed to connect to device " << deviceName << std::endl;
                return false;
            }
            std::
```
