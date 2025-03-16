---
title: Connection Plugins
nav_order: 4
parent: Driver Development
permalink: /drivers/connection-plugins/
---

# INDI Connection Plugins

This guide provides a comprehensive overview of the connection plugins available in INDI. It covers how to use these plugins to connect to devices, how to implement custom connection plugins, and best practices for handling device connections.

## Introduction to Connection Plugins

INDI provides a flexible connection framework that allows drivers to connect to devices using different communication methods. The connection framework is based on plugins, which are modular components that implement specific connection protocols.

The main advantages of using connection plugins are:

- **Modularity**: Each connection plugin is a self-contained module that can be used by any driver.
- **Reusability**: Connection plugins can be reused across multiple drivers, reducing code duplication.
- **Flexibility**: Drivers can support multiple connection methods without having to implement each one from scratch.
- **Standardization**: Connection plugins provide a standardized way to handle device connections, making drivers more consistent and easier to use.

## Available Connection Plugins

INDI provides several built-in connection plugins for common communication protocols:

- **Serial Connection**: For devices that connect via serial ports (RS-232, USB-to-Serial, etc.).
- **TCP Connection**: For devices that connect via TCP/IP networks.
- **UDP Connection**: For devices that communicate using UDP datagrams.

Each plugin provides a set of properties and methods for configuring and establishing connections to devices.

## Using Connection Plugins in Drivers

To use connection plugins in your driver, you need to:

1. Include the appropriate header files.
2. Create instances of the connection plugins you want to use.
3. Register the plugins with the driver.
4. Configure the plugins as needed.
5. Use the plugins to connect to and communicate with the device.

Let's look at how to use each of the available connection plugins.

### Serial Connection

The Serial Connection plugin is used for devices that connect via serial ports, such as RS-232 ports, USB-to-Serial adapters, or virtual serial ports.

#### Including the Header File

```cpp
#include <connectionplugins/connectionserial.h>
```

#### Creating and Registering the Plugin

```cpp
class MyDriver : public INDI::DefaultDevice
{
public:
    MyDriver()
    {
        // Set the default connection mode to Serial
        setConnectionMode(CONNECTION_SERIAL);

        // Create the Serial Connection plugin
        serialConnection = new Connection::Serial(this);

        // Register the plugin with the driver
        registerConnection(serialConnection);
    }

private:
    // Serial Connection plugin instance
    Connection::Serial *serialConnection = nullptr;
};
```

#### Configuring the Plugin

```cpp
MyDriver::MyDriver()
{
    // Set the default connection mode to Serial
    setConnectionMode(CONNECTION_SERIAL);

    // Create the Serial Connection plugin
    serialConnection = new Connection::Serial(this);

    // Configure the plugin
    serialConnection->setDefaultBaudRate(Connection::Serial::B_9600);
    serialConnection->setDefaultPort("/dev/ttyUSB0");

    // Register a handshake function (optional)
    serialConnection->registerHandshake([&]() { return Handshake(); });

    // Register the plugin with the driver
    registerConnection(serialConnection);
}

// Handshake function to verify the connection
bool MyDriver::Handshake()
{
    // Send a test command to the device
    char response[32];
    if (sendCommand("PING\r\n", response, sizeof(response)))
    {
        // Check if the response is valid
        if (strstr(response, "PONG"))
        {
            LOG_INFO("Device responded to handshake");
            return true;
        }
    }

    LOG_ERROR("Handshake failed");
    return false;
}
```

#### Using the Plugin

```cpp
bool MyDriver::sendCommand(const char *cmd, char *res, int reslen)
{
    // Get the file descriptor for the serial port
    int fd = serialConnection->getPortFD();
    if (fd < 0)
    {
        LOG_ERROR("Serial port not open");
        return false;
    }

    // Write the command
    int nbytes_written = write(fd, cmd, strlen(cmd));
    if (nbytes_written < 0)
    {
        LOGF_ERROR("Error writing to device: %s", strerror(errno));
        return false;
    }

    // If no response is expected, return success
    if (res == nullptr || reslen <= 0)
        return true;

    // Read the response
    int nbytes_read = read(fd, res, reslen - 1);
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

### TCP Connection

The TCP Connection plugin is used for devices that connect via TCP/IP networks, such as networked cameras, mounts, or other astronomical devices.

#### Including the Header File

```cpp
#include <connectionplugins/connectiontcp.h>
```

#### Creating and Registering the Plugin

```cpp
class MyDriver : public INDI::DefaultDevice
{
public:
    MyDriver()
    {
        // Set the default connection mode to TCP
        setConnectionMode(CONNECTION_TCP);

        // Create the TCP Connection plugin
        tcpConnection = new Connection::TCP(this);

        // Register the plugin with the driver
        registerConnection(tcpConnection);
    }

private:
    // TCP Connection plugin instance
    Connection::TCP *tcpConnection = nullptr;
};
```

#### Configuring the Plugin

```cpp
MyDriver::MyDriver()
{
    // Set the default connection mode to TCP
    setConnectionMode(CONNECTION_TCP);

    // Create the TCP Connection plugin
    tcpConnection = new Connection::TCP(this);

    // Configure the plugin
    tcpConnection->setDefaultHost("192.168.1.100");
    tcpConnection->setDefaultPort(9999);

    // Register a handshake function (optional)
    tcpConnection->registerHandshake([&]() { return Handshake(); });

    // Register the plugin with the driver
    registerConnection(tcpConnection);
}

// Handshake function to verify the connection
bool MyDriver::Handshake()
{
    // Send a test command to the device
    char response[32];
    if (sendCommand("PING\r\n", response, sizeof(response)))
    {
        // Check if the response is valid
        if (strstr(response, "PONG"))
        {
            LOG_INFO("Device responded to handshake");
            return true;
        }
    }

    LOG_ERROR("Handshake failed");
    return false;
}
```

#### Using the Plugin

```cpp
bool MyDriver::sendCommand(const char *cmd, char *res, int reslen)
{
    // Get the file descriptor for the TCP socket
    int fd = tcpConnection->getPortFD();
    if (fd < 0)
    {
        LOG_ERROR("TCP socket not open");
        return false;
    }

    // Write the command
    int nbytes_written = write(fd, cmd, strlen(cmd));
    if (nbytes_written < 0)
    {
        LOGF_ERROR("Error writing to device: %s", strerror(errno));
        return false;
    }

    // If no response is expected, return success
    if (res == nullptr || reslen <= 0)
        return true;

    // Read the response
    int nbytes_read = read(fd, res, reslen - 1);
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

### UDP Connection

The UDP Connection plugin is used for devices that communicate using UDP datagrams, such as some networked cameras or other devices that use UDP for communication.

#### Including the Header File

```cpp
#include <connectionplugins/connectionudp.h>
```

#### Creating and Registering the Plugin

```cpp
class MyDriver : public INDI::DefaultDevice
{
public:
    MyDriver()
    {
        // Set the default connection mode to UDP
        setConnectionMode(CONNECTION_UDP);

        // Create the UDP Connection plugin
        udpConnection = new Connection::UDP(this);

        // Register the plugin with the driver
        registerConnection(udpConnection);
    }

private:
    // UDP Connection plugin instance
    Connection::UDP *udpConnection = nullptr;
};
```

#### Configuring the Plugin

```cpp
MyDriver::MyDriver()
{
    // Set the default connection mode to UDP
    setConnectionMode(CONNECTION_UDP);

    // Create the UDP Connection plugin
    udpConnection = new Connection::UDP(this);

    // Configure the plugin
    udpConnection->setDefaultHost("192.168.1.100");
    udpConnection->setDefaultPort(9999);

    // Register a handshake function (optional)
    udpConnection->registerHandshake([&]() { return Handshake(); });

    // Register the plugin with the driver
    registerConnection(udpConnection);
}

// Handshake function to verify the connection
bool MyDriver::Handshake()
{
    // Send a test command to the device
    char response[32];
    if (sendCommand("PING\r\n", response, sizeof(response)))
    {
        // Check if the response is valid
        if (strstr(response, "PONG"))
        {
            LOG_INFO("Device responded to handshake");
            return true;
        }
    }

    LOG_ERROR("Handshake failed");
    return false;
}
```

#### Using the Plugin

```cpp
bool MyDriver::sendCommand(const char *cmd, char *res, int reslen)
{
    // Get the file descriptor for the UDP socket
    int fd = udpConnection->getPortFD();
    if (fd < 0)
    {
        LOG_ERROR("UDP socket not open");
        return false;
    }

    // Get the remote address
    struct sockaddr_in *remoteAddr = udpConnection->getRemoteAddr();
    if (remoteAddr == nullptr)
    {
        LOG_ERROR("Remote address not set");
        return false;
    }

    // Send the command
    int nbytes_written = sendto(fd, cmd, strlen(cmd), 0, (struct sockaddr *)remoteAddr, sizeof(*remoteAddr));
    if (nbytes_written < 0)
    {
        LOGF_ERROR("Error writing to device: %s", strerror(errno));
        return false;
    }

    // If no response is expected, return success
    if (res == nullptr || reslen <= 0)
        return true;

    // Read the response
    struct sockaddr_in fromAddr;
    socklen_t fromLen = sizeof(fromAddr);
    int nbytes_read = recvfrom(fd, res, reslen - 1, 0, (struct sockaddr *)&fromAddr, &fromLen);
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

## Supporting Multiple Connection Methods

INDI drivers can support multiple connection methods by registering multiple connection plugins. This allows users to choose the most appropriate connection method for their setup.

### Registering Multiple Plugins

```cpp
class MyDriver : public INDI::DefaultDevice
{
public:
    MyDriver()
    {
        // Set the default connection mode to Serial
        setConnectionMode(CONNECTION_SERIAL);

        // Create and register the Serial Connection plugin
        serialConnection = new Connection::Serial(this);
        registerConnection(serialConnection);

        // Create and register the TCP Connection plugin
        tcpConnection = new Connection::TCP(this);
        registerConnection(tcpConnection);
    }

private:
    // Connection plugin instances
    Connection::Serial *serialConnection = nullptr;
    Connection::TCP *tcpConnection = nullptr;
};
```

### Handling Different Connection Types

When supporting multiple connection methods, you need to handle each connection type appropriately in your driver's methods.

```cpp
bool MyDriver::sendCommand(const char *cmd, char *res, int reslen)
{
    // Get the active connection type
    int activeConnection = getActiveConnection();

    // Handle the command based on the active connection type
    switch (activeConnection)
    {
        case CONNECTION_SERIAL:
            return sendSerialCommand(cmd, res, reslen);

        case CONNECTION_TCP:
            return sendTCPCommand(cmd, res, reslen);

        default:
            LOG_ERROR("Unsupported connection type");
            return false;
    }
}

bool MyDriver::sendSerialCommand(const char *cmd, char *res, int reslen)
{
    // Get the file descriptor for the serial port
    int fd = serialConnection->getPortFD();
    if (fd < 0)
    {
        LOG_ERROR("Serial port not open");
        return false;
    }

    // Write the command
    int nbytes_written = write(fd, cmd, strlen(cmd));
    if (nbytes_written < 0)
    {
        LOGF_ERROR("Error writing to device: %s", strerror(errno));
        return false;
    }

    // If no response is expected, return success
    if (res == nullptr || reslen <= 0)
        return true;

    // Read the response
    int nbytes_read = read(fd, res, reslen - 1);
    if (nbytes_read < 0)
    {
        LOGF_ERROR("Error reading from device: %s", strerror(errno));
        return false;
    }

    // Null-terminate the response
    res[nbytes_read] = '\0';

    return true;
}

bool MyDriver::sendTCPCommand(const char *cmd, char *res, int reslen)
{
    // Get the file descriptor for the TCP socket
    int fd = tcpConnection->getPortFD();
    if (fd < 0)
    {
        LOG_ERROR("TCP socket not open");
        return false;
    }

    // Write the command
    int nbytes_written = write(fd, cmd, strlen(cmd));
    if (nbytes_written < 0)
    {
        LOGF_ERROR("Error writing to device: %s", strerror(errno));
        return false;
    }

    // If no response is expected, return success
    if (res == nullptr || reslen <= 0)
        return true;

    // Read the response
    int nbytes_read = read(fd, res, reslen - 1);
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

## Creating Custom Connection Plugins

If the built-in connection plugins don't meet your needs, you can create custom connection plugins by inheriting from the `Connection::Interface` class and implementing the required methods.

### Creating a Custom Connection Plugin

```cpp
#include <connectionplugins/connectioninterface.h>

namespace Connection
{

class Custom : public Interface
{
public:
    // Enum for property indices
    enum
    {
        DEVICE_PATH = 0,
        BAUD_RATE,
        PROTOCOL_VERSION,
        N_CUSTOM_PROPERTIES
    };

    Custom(INDI::DefaultDevice *dev);
    virtual ~Custom();

    // Interface implementation
    virtual bool Connect() override;
    virtual bool Disconnect() override;
    virtual void Activated() override;
    virtual void Deactivated() override;
    virtual std::string name() override { return "CONNECTION_CUSTOM"; }
    virtual std::string label() override { return "Custom Protocol"; }

    // Custom methods
    bool sendCommand(const char *cmd, char *res = nullptr, int reslen = 0);
    int getDeviceHandle() const { return deviceHandle; }

    // Getters for configuration
    const char *getDevicePath() const { return CustomTP[DEVICE_PATH].getText(); }
    int getBaudRate() const { return std::stoi(CustomTP[BAUD_RATE].getText()); }
    int getProtocolVersion() const { return std::stoi(CustomTP[PROTOCOL_VERSION].getText()); }

protected:
    // Property definitions
    INDI::PropertyText CustomTP {N_CUSTOM_PROPERTIES};

private:
    // Device handle for the connection
    int deviceHandle = -1;
};

} // namespace Connection
```

### Implementing the Custom Connection Plugin

```cpp
#include "connectioncustom.h"

namespace Connection
{

Custom::Custom(INDI::DefaultDevice *dev) : Interface(dev, CONNECTION_CUSTOM)
{
    // Initialize properties
    CustomTP[DEVICE_PATH].fill("DEVICE_PATH", "Device Path", "/dev/customdevice");
    CustomTP[BAUD_RATE].fill("BAUD_RATE", "Baud Rate", "115200");
    CustomTP[PROTOCOL_VERSION].fill("PROTOCOL_VERSION", "Protocol Version", "1");
    CustomTP.fill(getDeviceName(), "CUSTOM_SETTINGS", "Custom Protocol", OPTIONS_TAB, IP_RW, 60, IPS_IDLE);
}

Custom::~Custom()
{
    // No need to delete anything with the new property style
}

bool Custom::Connect()
{
    // Get the device path and settings
    const char *devicePath = getDevicePath();
    int baudRate = getBaudRate();
    int protocolVersion = getProtocolVersion();

    // Log the connection attempt
    LOGF_INFO("Connecting to %s at %d baud with protocol version %d", devicePath, baudRate, protocolVersion);

    // Open the device
    int fd = open(devicePath, O_RDWR);
    if (fd < 0)
    {
        LOGF_ERROR("Failed to open device: %s - %s", devicePath, strerror(errno));
        return false;
    }

    // Configure the device based on protocol version
    if (protocolVersion == 1)
    {
        // Protocol version 1 configuration
        // ...
    }
    else if (protocolVersion == 2)
    {
        // Protocol version 2 configuration
        // ...
    }
    else
    {
        LOGF_ERROR("Unsupported protocol version: %d", protocolVersion);
        close(fd);
        return false;
    }

    // Set the device handle
    deviceHandle = fd;

    LOGF_INFO("Connected to %s successfully", devicePath);
    return true;
}

bool Custom::Disconnect()
{
    // Check if we're connected
    if (deviceHandle < 0)
        return true;

    // Close the device
    close(deviceHandle);
    deviceHandle = -1;

    LOG_INFO("Disconnected from device");
    return true;
}

void Custom::Activated()
{
    // Define properties when the connection is activated
    defineProperty(CustomTP);
}

void Custom::Deactivated()
{
    // Delete properties when the connection is deactivated
    deleteProperty(CustomTP.getName());
}

bool Custom::sendCommand(const char *cmd, char *res, int reslen)
{
    // Check if we're connected
    if (deviceHandle < 0)
    {
        LOG_ERROR("Device not connected");
        return false;
    }

    // Get the protocol version
    int protocolVersion = getProtocolVersion();

    // Format the command based on protocol version
    std::string formattedCmd;
    if (protocolVersion == 1)
    {
        // Protocol version 1 command format: CMD\r\n
        formattedCmd = std::string(cmd) + "\r\n";
    }
    else if (protocolVersion == 2)
    {
        // Protocol version 2 command format: #CMD#\r\n
        formattedCmd = "#" + std::string(cmd) + "#\r\n";
    }
    else
    {
        LOGF_ERROR("Unsupported protocol version: %d", protocolVersion);
        return false;
    }

    // Write the command
    int nbytes_written = write(deviceHandle, formattedCmd.c_str(), formattedCmd.length());
    if (nbytes_written < 0)
    {
        LOGF_ERROR("Error writing to device: %s", strerror(errno));
        return false;
    }

    // If no response is expected, return success
    if (res == nullptr || reslen <= 0)
        return true;

    // Read the response
    int nbytes_read = read(deviceHandle, res, reslen - 1);
    if (nbytes_read < 0)
    {
        LOGF_ERROR("Error reading from device: %s", strerror(errno));
        return false;
    }

    // Null-terminate the response
    res[nbytes_read] = '\0';

    // Log the response in debug mode
    DEBUGF(INDI::Logger::DBG_DEBUG, "Received response: %s", res);

    return true;
}

} // namespace Connection
```

### Using the Custom Connection Plugin

```cpp
#include "connectioncustom.h"

class MyDriver : public INDI::DefaultDevice
{
public:
    MyDriver()
    {
        // Set the default connection mode to Custom
        setConnectionMode(CONNECTION_CUSTOM);

        // Create and register the Custom Connection plugin
        customConnection = new Connection::Custom(this);
        registerConnection(customConnection);
    }

private:
    // Custom Connection plugin instance
    Connection::Custom *customConnection = nullptr;
};
```

## Best Practices

When using connection plugins in your INDI drivers, follow these best practices:

- **Use the appropriate connection plugin** for your device. Choose the plugin that best matches the device's communication protocol.
- **Configure the plugin properly** with default values that make sense for your device.
- **Implement a handshake function** to verify the connection to the device.
- **Handle connection errors gracefully** and provide informative error messages.
- **Support multiple connection methods** if your device can be connected in different ways.
- **Use the plugin's methods** for communication rather than implementing your own.
- **Check the connection status** before sending commands to the device.
- **Close the connection properly** when disconnecting from the device.
- **Document the supported connection methods** in your driver's documentation.

## Conclusion

INDI connection plugins provide a flexible and reusable way to handle device connections in INDI drivers. By using these plugins, you can support multiple connection methods, handle connection errors gracefully, and provide a consistent user experience across different drivers.

For more information, refer to the [INDI Library Documentation](https://www.indilib.org/api/index.html) and the [INDI Driver Development Guide](https://www.indilib.org/develop/developer-manual/100-driver-development.html).
