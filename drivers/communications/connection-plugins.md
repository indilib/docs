---
title: Connection Plugins Overview
parent: Hardware Communications
grand_parent: Driver Development
nav_order: 1
permalink: /drivers/communications/connection-plugins/
---

# Connection Plugins Framework

Connection plugins provide a standardized, modular framework for implementing common communication protocols in INDI drivers. These plugins handle connection management, property definitions, and provide consistent user interfaces.

## Key Advantages

- **Modularity**: Self-contained components that can be used by any driver
- **Reusability**: Share code across multiple drivers, reducing duplication
- **Flexibility**: Support multiple connection methods without implementing each from scratch
- **Standardization**: Consistent connection properties and behavior across drivers
- **Configuration**: Automatic saving and loading of connection settings

## Available Connection Plugins

INDI provides several built-in connection plugins:

| Plugin | Header | Use Case |
|--------|--------|----------|
| **Serial** | `connectionserial.h` | RS-232, USB-Serial adapters |
| **TCP** | `connectiontcp.h` | Network devices using TCP |
| **UDP** | `connectionudp.h` | Network devices using UDP |
| **I2C** | `connectioni2c.h` | I2C bus devices on Linux |

## Basic Workflow

The typical workflow for using connection plugins:

1. **Include** the appropriate header file
2. **Create** an instance of the connection plugin
3. **Register** the plugin with your driver
4. **Configure** default settings (optional)
5. **Register** a handshake function (optional)
6. **Use** the plugin's file descriptor for communication

## Simple Example

```cpp
#include <connectionplugins/connectionserial.h>

class MyDriver : public INDI::DefaultDevice
{
public:
    MyDriver()
    {
        // Set default connection mode
        setConnectionMode(CONNECTION_SERIAL);

        // Create and configure plugin
        serialConnection = new Connection::Serial(this);
        serialConnection->setDefaultBaudRate(Connection::Serial::B_9600);
        serialConnection->setDefaultPort("/dev/ttyUSB0");

        // Register handshake (optional)
        serialConnection->registerHandshake([&]() { return Handshake(); });

        // Register with driver
        registerConnection(serialConnection);
    }

private:
    Connection::Serial *serialConnection = nullptr;

    bool Handshake()
    {
        // Verify device responds correctly
        char response[32];
        if (sendCommand("*IDN?\n", response, sizeof(response)))
        {
            if (strstr(response, "MyDevice"))
                return true;
        }
        return false;
    }
};
```

## Using the Connection

Once connected, use the file descriptor for communication:

```cpp
bool MyDriver::sendCommand(const char *cmd, char *res, int reslen)
{
    // Get the file descriptor
    int fd = serialConnection->getPortFD();
    if (fd < 0)
    {
        LOG_ERROR("Connection not open");
        return false;
    }

    // Write command
    int nbytes = write(fd, cmd, strlen(cmd));
    if (nbytes < 0)
    {
        LOGF_ERROR("Write error: %s", strerror(errno));
        return false;
    }

    // Read response if expected
    if (res && reslen > 0)
    {
        nbytes = read(fd, res, reslen - 1);
        if (nbytes < 0)
        {
            LOGF_ERROR("Read error: %s", strerror(errno));
            return false;
        }
        res[nbytes] = '\0';
    }

    return true;
}
```

## Supporting Multiple Connection Methods

You can register multiple connection plugins to give users flexibility:

```cpp
class MyDriver : public INDI::DefaultDevice
{
public:
    MyDriver()
    {
        // Default to serial
        setConnectionMode(CONNECTION_SERIAL);

        // Register multiple options
        serialConnection = new Connection::Serial(this);
        registerConnection(serialConnection);

        tcpConnection = new Connection::TCP(this);
        registerConnection(tcpConnection);
    }

private:
    Connection::Serial *serialConnection = nullptr;
    Connection::TCP *tcpConnection = nullptr;

    bool sendCommand(const char *cmd, char *res, int reslen)
    {
        // Route based on active connection
        int activeConnection = getActiveConnection();
        
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
};
```

## Handshake Functions

Handshake functions verify that the device is responding correctly after connection:

```cpp
bool MyDriver::Handshake()
{
    // Send identification command
    char response[64];
    if (!sendCommand("*IDN?\n", response, sizeof(response)))
    {
        LOG_ERROR("Failed to send identification command");
        return false;
    }

    // Check response
    if (!strstr(response, "Expected Device Name"))
    {
        LOGF_ERROR("Unexpected response: %s", response);
        return false;
    }

    LOG_INFO("Device handshake successful");
    return true;
}
```

## Configuration Management

Connection plugins automatically save and load configuration:

```cpp
// Settings are automatically saved when connection succeeds
// No additional code needed for basic configuration

// For custom settings, save in saveConfigItems():
bool MyDriver::saveConfigItems(FILE *fp)
{
    INDI::DefaultDevice::saveConfigItems(fp);
    
    // Plugin configuration is saved automatically
    // Add any custom configuration here
    
    return true;
}
```

## Next Steps

- **[Serial Connection](serial/)** - Detailed serial communication guide
- **[TCP Connection](tcp/)** - TCP/IP network communication
- **[UDP Connection](udp/)** - UDP datagram communication
- **[I2C Connection](i2c/)** - I2C bus communication
- **[Creating Custom Plugins](custom-plugins/)** - Build your own plugin

## Additional Resources

- [INDI API Documentation](https://www.indilib.org/api/)
- [Connection Plugin Source Code](https://github.com/indilib/indi/tree/master/libs/indibase/connectionplugins)
- [Example Drivers](https://github.com/indilib/indi/tree/master/drivers)
