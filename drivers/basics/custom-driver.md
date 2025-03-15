---
sort: 2
---

# Creating Custom INDI Drivers

This guide provides a comprehensive overview of creating custom INDI drivers for devices that aren't already supported by the INDI library. It covers the basic structure of an INDI driver, how to implement the required methods, and how to handle device-specific functionality.

## Introduction to INDI Drivers

INDI (Instrument-Neutral Distributed Interface) drivers are software components that communicate with hardware devices and expose their functionality through INDI properties. They act as a bridge between the INDI server and the hardware, translating INDI protocol messages into device-specific commands and vice versa.

Creating a custom INDI driver involves:

1. Understanding the device's communication protocol
2. Implementing the INDI driver interface
3. Defining properties to represent the device's functionality
4. Handling property updates and device commands
5. Building and testing the driver

## Prerequisites

Before creating a custom INDI driver, you should have:

- Basic knowledge of C++ programming
- Understanding of the INDI protocol and architecture
- Familiarity with the device's communication protocol
- Development environment set up (compiler, build tools, etc.)
- INDI library installed

## Driver Structure

An INDI driver consists of several key components:

- **Header File**: Contains the class declaration, including member variables and method declarations.
- **Implementation File**: Contains the implementation of the methods declared in the header file.
- **Main File**: Contains the `main()` function that creates an instance of the driver and runs it.
- **CMakeLists.txt**: Contains the build configuration for the driver.
- **XML File**: Contains the driver metadata for the INDI server.

### Base Classes

INDI provides several base classes that you can inherit from to create your driver:

- **[INDI::DefaultDevice](https://www.indilib.org/api/classINDI_1_1DefaultDevice.html)**: The base class for all INDI devices. Use this if your device doesn't fit into any of the specialized categories.
- **[INDI::CCD](https://www.indilib.org/api/classINDI_1_1CCD.html)**: For CCD cameras and other imaging devices.
- **[INDI::Telescope](https://www.indilib.org/api/classINDI_1_1Telescope.html)**: For telescope mounts and other pointing devices.
- **[INDI::Focuser](https://www.indilib.org/api/classINDI_1_1Focuser.html)**: For focusers and other focusing devices.
- **[INDI::FilterWheel](https://www.indilib.org/api/classINDI_1_1FilterWheel.html)**: For filter wheels and other filter selection devices.
- **[INDI::Dome](https://www.indilib.org/api/classINDI_1_1Dome.html)**: For domes and other enclosure devices.
- **[INDI::Weather](https://www.indilib.org/api/classINDI_1_1Weather.html)**: For weather stations and other environmental monitoring devices.
- **[INDI::GPS](https://www.indilib.org/api/classINDI_1_1GPS.html)**: For GPS and other location devices.
- **[INDI::Rotator](https://www.indilib.org/api/classINDI_1_1Rotator.html)**: For rotators and other rotation devices.
- **[INDI::Detector](https://www.indilib.org/api/classINDI_1_1Detector.html)**: For detectors and other sensing devices.
- **INDI::AUX**: For auxiliary interfaces and other miscellaneous devices.

Choose the base class that best matches your device's functionality. If your device doesn't fit into any of these categories, use `INDI::DefaultDevice`.

## Creating a Basic Driver

Let's create a simple INDI driver for a hypothetical device called "MyDevice". This device has a simple serial interface and supports basic commands for turning on/off and setting a value.

### Step 1: Create the Header File

Create a file named `mydevicedriver.h` with the following content:

```cpp
#pragma once

#include <defaultdevice.h>

class MyDeviceDriver : public INDI::DefaultDevice
{
public:
    MyDeviceDriver();
    virtual ~MyDeviceDriver() = default;

    // DefaultDevice overrides
    virtual const char *getDefaultName() override;
    virtual bool initProperties() override;
    virtual bool updateProperties() override;
    virtual bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) override;
    virtual bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n) override;
    virtual void TimerHit() override;

    // Connection overrides
    virtual bool Connect() override;
    virtual bool Disconnect() override;

private:
    // Helper methods
    bool sendCommand(const char *cmd, char *res = nullptr, int reslen = 0);
    bool readResponse(char *res, int reslen);

    // Properties
    INDI::PropertySwitch PowerSP {2};
    // Use built-in INDI_ENABLED and INDI_DISABLED for toggle switches

    INDI::PropertyNumber ValueNP {1};
    // No need for enum when property size is 1

    // Connection
    int PortFD = -1;
};
```

### Step 2: Create the Implementation File

Create a file named `mydevicedriver.cpp` with the following content:

```cpp
#include "mydevicedriver.h"

#include <memory>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <connectionplugins/connectionserial.h>

// We declare an auto pointer to MyDeviceDriver
static std::unique_ptr<MyDeviceDriver> mydevice(new MyDeviceDriver());

MyDeviceDriver::MyDeviceDriver()
{
    // Set the driver version
    setVersion(1, 0);

    // Set up the connection
    setConnectionMode(CONNECTION_SERIAL);
    serialConnection = new Connection::Serial(this);
    serialConnection->registerHandshake([&]() { return Handshake(); });
    serialConnection->setDefaultBaudRate(Connection::Serial::B_9600);
    serialConnection->setDefaultPort("/dev/ttyUSB0");
    registerConnection(serialConnection);
}

const char *MyDeviceDriver::getDefaultName()
{
    return "My Device";
}

bool MyDeviceDriver::initProperties()
{
    // Initialize the parent's properties
    INDI::DefaultDevice::initProperties();

    // Initialize power switch
    PowerSP[INDI_ENABLED].fill("POWER_ON", "On", ISS_OFF);
    PowerSP[INDI_DISABLED].fill("POWER_OFF", "Off", ISS_ON);
    PowerSP.fill(getDeviceName(), "POWER", "Power", MAIN_CONTROL_TAB, IP_RW, ISR_1OFMANY, 60, IPS_IDLE);

    // Initialize value number
    ValueNP[0].fill("VALUE", "Value", "%6.2f", 0, 100, 1, 50);
    ValueNP.fill(getDeviceName(), "DEVICE_VALUE", "Value", MAIN_CONTROL_TAB, IP_RW, 60, IPS_IDLE);

    // Add debug, simulation, and configuration controls
    addAuxControls();

    return true;
}

bool MyDeviceDriver::updateProperties()
{
    // Call the parent's updateProperties
    INDI::DefaultDevice::updateProperties();

    if (isConnected())
    {
        // Define properties when connected
        defineProperty(PowerSP);
        defineProperty(ValueNP);

        // Start the timer
        SetTimer(POLLMS);
    }
    else
    {
        // Delete properties when disconnected
        deleteProperty(PowerSP);
        deleteProperty(ValueNP);
    }

    return true;
}

bool MyDeviceDriver::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    // Check if the message is for this device
    if (!strcmp(dev, getDeviceName()))
    {
        // Check if the message is for the value property
        if (!strcmp(name, ValueNP.name))
        {
            // Update the property values
            ValueNP.update(values, names, n);

            // Send the new value to the device
            char cmd[32];
            snprintf(cmd, sizeof(cmd), "SET_VALUE %.2f\r\n", ValueNP[0].getValue());
            if (!sendCommand(cmd))
            {
                LOG_ERROR("Failed to set value");
                ValueNP.setState(IPS_ALERT);
                ValueNP.apply();
                return false;
            }

            // Value set successfully
            LOG_INFO("Value set successfully");
            ValueNP.setState(IPS_OK);
            ValueNP.apply();
            return true;
        }
    }

    // If the message is not for this device or property, call the parent's ISNewNumber
    return INDI::DefaultDevice::ISNewNumber(dev, name, values, names, n);
}

bool MyDeviceDriver::ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    // Check if the message is for this device
    if (!strcmp(dev, getDeviceName()))
    {
        // Check if the message is for the power property
        if (!strcmp(name, PowerSP.name))
        {
            // Update the property values
            PowerSP.update(states, names, n);

            // Send the power command to the device
            const char *cmd = (PowerSP[INDI_ENABLED].getState() == ISS_ON) ? "POWER_ON\r\n" : "POWER_OFF\r\n";
            if (!sendCommand(cmd))
            {
                LOG_ERROR("Failed to set power state");
                PowerSP.setState(IPS_ALERT);
                PowerSP.apply();
                return false;
            }

            // Power state set successfully
            LOG_INFO("Power state set successfully");
            PowerSP.setState(IPS_OK);
            PowerSP.apply();
            return true;
        }
    }

    // If the message is not for this device or property, call the parent's ISNewSwitch
    return INDI::DefaultDevice::ISNewSwitch(dev, name, states, names, n);
}

void MyDeviceDriver::TimerHit()
{
    // Check if the device is connected
    if (!isConnected())
        return;

    // Get the current value from the device
    char res[32];
    if (sendCommand("GET_VALUE\r\n", res, sizeof(res)))
    {
        // Parse the response
        float value;
        if (sscanf(res, "VALUE=%f", &value) == 1)
        {
            // Update the value property
            ValueNP[0].setValue(value);
            ValueNP.setState(IPS_OK);
            ValueNP.apply();
        }
    }

    // Get the current power state from the device
    if (sendCommand("GET_POWER\r\n", res, sizeof(res)))
    {
        // Parse the response
        if (strstr(res, "POWER=ON"))
        {
            // Update the power property
            PowerSP[INDI_ENABLED].setState(ISS_ON);
            PowerSP[INDI_DISABLED].setState(ISS_OFF);
            PowerSP.setState(IPS_OK);
            PowerSP.apply();
        }
        else if (strstr(res, "POWER=OFF"))
        {
            // Update the power property
            PowerSP[INDI_ENABLED].setState(ISS_OFF);
            PowerSP[INDI_DISABLED].setState(ISS_ON);
            PowerSP.setState(IPS_OK);
            PowerSP.apply();
        }
    }

    // Set the timer for the next update
    SetTimer(POLLMS);
}

bool MyDeviceDriver::Connect()
{
    // Call the parent's Connect method
    bool result = INDI::DefaultDevice::Connect();

    if (result)
    {
        // Get the file descriptor for the serial port
        PortFD = serialConnection->getPortFD();

        // Send a test command to verify the connection
        if (!sendCommand("PING\r\n"))
        {
            LOG_ERROR("Failed to communicate with the device");
            return false;
        }

        LOG_INFO("Device connected successfully");
    }

    return result;
}

bool MyDeviceDriver::Disconnect()
{
    // Close the serial port
    if (PortFD > 0)
    {
        close(PortFD);
        PortFD = -1;
    }

    // Call the parent's Disconnect method
    return INDI::DefaultDevice::Disconnect();
}

bool MyDeviceDriver::sendCommand(const char *cmd, char *res, int reslen)
{
    // Check if the port is open
    if (PortFD < 0)
    {
        LOG_ERROR("Serial port not open");
        return false;
    }

    // Write the command
    int nbytes_written = write(PortFD, cmd, strlen(cmd));
    if (nbytes_written < 0)
    {
        LOGF_ERROR("Error writing to device: %s", strerror(errno));
        return false;
    }

    // If no response is expected, return success
    if (res == nullptr || reslen <= 0)
        return true;

    // Read the response
    if (!readResponse(res, reslen))
    {
        LOG_ERROR("Error reading response from device");
        return false;
    }

    return true;
}

bool MyDeviceDriver::readResponse(char *res, int reslen)
{
    // Check if the port is open
    if (PortFD < 0)
    {
        LOG_ERROR("Serial port not open");
        return false;
    }

    // Read the response
    int nbytes_read = read(PortFD, res, reslen - 1);
    if (nbytes_read < 0)
    {
        LOGF_ERROR("Error reading from device: %s", strerror(errno));
        return false;
    }

    // Null-terminate the response
    res[nbytes_read] = '\0';

    return true;
}
```

### Step 3: Create the Main File

Create a file named `main.cpp` with the following content:

```cpp
#include "mydevicedriver.h"

int main(int argc, char *argv[])
{
    // Create and initialize the driver
    std::unique_ptr<MyDeviceDriver> mydevice(new MyDeviceDriver());

    // Set the driver version
    mydevice->setVersion(1, 0);

    // Start the driver
    mydevice->ISGetProperties(nullptr);

    // Run the driver
    return mydevice->run();
}
```

### Step 4: Create the CMakeLists.txt File

Create a file named `CMakeLists.txt` with the following content:

```cmake
cmake_minimum_required(VERSION 3.0)
project(indi-mydevice CXX C)

include(GNUInstallDirs)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")

find_package(INDI REQUIRED)
find_package(Nova REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${INDI_INCLUDE_DIR})
include_directories(${NOVA_INCLUDE_DIR})

include(CMakeCommon)

add_executable(indi_mydevice mydevicedriver.cpp main.cpp)

target_link_libraries(indi_mydevice ${INDI_LIBRARIES} ${NOVA_LIBRARIES})

install(TARGETS indi_mydevice RUNTIME DESTINATION bin)
```

### Step 5: Create the XML File

Create a file named `indi_mydevice.xml` with the following content:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<driversList>
   <devGroup group="Auxiliary">
      <device label="My Device" manufacturer="INDI">
         <driver name="My Device">indi_mydevice</driver>
         <version>1.0</version>
      </device>
   </devGroup>
</driversList>
```

### Step 6: Build the Driver

To build the driver, create a `build` directory and run CMake:

```bash
mkdir build
cd build
cmake ..
make
```

### Step 7: Install the Driver

To install the driver, run:

```bash
sudo make install
```

This will install the driver executable to `/usr/bin` and the XML file to `/usr/share/indi`.

### Step 8: Test the Driver

To test the driver, start the INDI server with your driver:

```bash
indiserver -v indi_mydevice
```

Then, connect to the INDI server using an INDI client, such as the INDI Control Panel:

```bash
indi_control_panel
```

## Advanced Topics

### Device Interfaces

INDI provides several device interfaces that you can implement to expose specific functionality:

- **[CCD Interface](https://www.indilib.org/api/classINDI_1_1CCD.html)**: For CCD cameras and other imaging devices.
- **[Telescope Interface](https://www.indilib.org/api/classINDI_1_1Telescope.html)**: For telescope mounts and other pointing devices.
- **[Focuser Interface](https://www.indilib.org/api/classINDI_1_1Focuser.html)**: For focusers and other focusing devices.
- **[Filter Wheel Interface](https://www.indilib.org/api/classINDI_1_1FilterWheel.html)**: For filter wheels and other filter selection devices.
- **[Dome Interface](https://www.indilib.org/api/classINDI_1_1Dome.html)**: For domes and other enclosure devices.
- **[Weather Interface](https://www.indilib.org/api/classINDI_1_1Weather.html)**: For weather stations and other environmental monitoring devices.
- **[GPS Interface](https://www.indilib.org/api/classINDI_1_1GPS.html)**: For GPS and other location devices.
- **[Rotator Interface](https://www.indilib.org/api/classINDI_1_1Rotator.html)**: For rotators and other rotation devices.
- **[Detector Interface](https://www.indilib.org/api/classINDI_1_1Detector.html)**: For detectors and other sensing devices.
- **AUX Interface**: For auxiliary interfaces and other miscellaneous devices.
- **[Output Interface](https://www.indilib.org/api/classINDI_1_1OutputInterface.html)**: For devices that provide output control functionality.
- **[Guider Interface](https://www.indilib.org/api/classINDI_1_1GuiderInterface.html)**: For autoguider devices.
- **[Dustcap Interface](https://www.indilib.org/api/classINDI_1_1DustCapInterface.html)**: For dust cap devices.
- **[Lightbox Interface](https://www.indilib.org/api/classINDI_1_1LightBoxInterface.html)**: For light box devices.
- **[Correlator Interface](https://www.indilib.org/api/classINDI_1_1Correlator.html)**: For correlator devices.
- **[Spectrograph Interface](https://www.indilib.org/api/classINDI_1_1Spectrograph.html)**: For spectrograph devices.

To implement a device interface, inherit from the corresponding base class and implement the required methods.

### Connection Plugins

INDI provides several connection plugins that you can use to connect to devices:

- **[Serial Connection](https://www.indilib.org/api/classConnection_1_1Serial.html)**: For devices that connect via serial ports.
- **[TCP Connection](https://www.indilib.org/api/classConnection_1_1TCP.html)**: For devices that connect via TCP/IP.

To use a connection plugin, create an instance of the plugin and register it with the driver:

```cpp
// Set up the serial connection
setConnectionMode(CONNECTION_SERIAL);
serialConnection = new Connection::Serial(this);
serialConnection->registerHandshake([&]() { return Handshake(); });
serialConnection->setDefaultBaudRate(Connection::Serial::B_9600);
serialConnection->setDefaultPort("/dev/ttyUSB0");
registerConnection(serialConnection);
```

### Property Types

INDI supports several property types:

- **[Number](https://www.indilib.org/api/classINDI_1_1PropertyNumber.html)**: Represents a numeric value.
- **[Text](https://www.indilib.org/api/classINDI_1_1PropertyText.html)**: Represents a text value.
- **[Switch](https://www.indilib.org/api/classINDI_1_1PropertySwitch.html)**: Represents a boolean value or a set of mutually exclusive options.
- **[Light](https://www.indilib.org/api/classINDI_1_1PropertyLight.html)**: Represents a status indicator.
- **[BLOB](https://www.indilib.org/api/classINDI_1_1PropertyBlob.html)**: Represents binary data.

Each property type has its own set of functions for creating, updating, and deleting properties.

### Simulation Mode

INDI drivers should support simulation mode, which allows users to test the driver without connecting to the actual hardware. To implement simulation mode, check the `isSimulation()` flag and provide simulated responses:

```cpp
bool MyDeviceDriver::sendCommand(const char *cmd, char *res, int reslen)
{
    // Check if we're in simulation mode
    if (isSimulation())
    {
        // Simulate a response
        if (res && reslen > 0)
        {
            if (!strcmp(cmd, "GET_VALUE\r\n"))
                snprintf(res, reslen, "VALUE=%.2f\r\n", ValueNP[0].getValue());
            else if (!strcmp(cmd, "GET_POWER\r\n"))
                snprintf(res, reslen, "POWER=%s\r\n", (PowerSP[INDI_ENABLED].getState() == ISS_ON) ? "ON" : "OFF");
            else if (!strcmp(cmd, "PING\r\n"))
                snprintf(res, reslen, "PONG\r\n");
            else
                snprintf(res, reslen, "ERROR\r\n");
        }
        return true;
    }

    // Real command code
    // ...
}
```

### Debug Mode

INDI drivers should support debug mode, which provides additional logging information for troubleshooting. To implement debug mode, use the `DEBUGF` and `DEBUG` macros:

```cpp
bool MyDeviceDriver::sendCommand(const char *cmd, char *res, int reslen)
{
    // Log the command in debug mode
    DEBUGF(INDI::Logger::DBG_DEBUG, "Sending command: %s", cmd);

    // Send the command
    // ...

    // Log the response in debug mode
    if (res && reslen > 0)
        DEBUGF(INDI::Logger::DBG_DEBUG, "Received response: %s", res);

    return true;
}
```

### Configuration

INDI drivers should support saving and loading configuration. To implement configuration support, override the `saveConfigItems` method:

```cpp
bool MyDeviceDriver::saveConfigItems(FILE *fp)
{
    // Call the parent's saveConfigItems
    INDI::DefaultDevice::saveConfigItems(fp);

    // Save custom configuration items
    ValueNP.save(fp);
    PowerSP.save(fp);

    return true;
}
```

### Error Handling

INDI drivers should handle errors gracefully and provide informative error messages. Use the `LOG_ERROR` and `LOGF_ERROR` macros to log errors:

```cpp
bool MyDeviceDriver::sendCommand(const char *cmd, char *res, int reslen)
{
    // Check if the port is open
    if (PortFD < 0)
    {
        LOG_ERROR("Serial port not open");
        return false;
    }

    // Write the command
    int nbytes_written = write(PortFD, cmd, strlen(cmd));
    if (nbytes_written < 0)
    {
        LOGF_ERROR("Error writing to device: %s", strerror(errno));
        return false;
    }

    // ...
}
```

## Best Practices

When creating custom INDI drivers, follow these best practices:

- **Use the appropriate base class** for your device.
- **Implement simulation mode** to allow testing without hardware.
- **Provide informative error messages** to help users troubleshoot issues.
- **Handle connection and disconnection gracefully** to avoid resource leaks.
- **Update property states** to reflect the current state of the device.
- **Use appropriate property types** for different kinds of data.
- **Follow the INDI naming conventions** for properties and elements.
- **Document your driver** to help users understand how to use it.
- **Test your driver thoroughly** with different clients and configurations.

## Conclusion

Creating a custom INDI driver involves implementing the INDI driver interface, defining properties to represent the device's functionality, and handling property updates and device commands. By following the steps and best practices outlined in this guide, you can create robust and feature-rich INDI drivers for your devices.

For more information, refer to the [INDI Library Documentation](https://www.indilib.org/api/index.html) and the [INDI Driver Development Guide](https://www.indilib.org/develop/developer-manual/100-driver-development.html).
