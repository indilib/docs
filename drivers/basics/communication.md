---
title: Device Communication
nav_order: 4
parent: Basics
---

# Device Communication

Communication with hardware devices is a fundamental aspect of INDI drivers. INDI provides several connection plugins to simplify this process, supporting various communication protocols including serial, network, and USB connections.

## Connection Plugins

INDI provides a flexible connection framework based on plugins, which are modular components that implement specific connection protocols. The main advantages of using connection plugins are:

- **Modularity**: Each connection plugin is a self-contained module that can be used by any driver
- **Reusability**: Connection plugins can be reused across multiple drivers, reducing code duplication
- **Flexibility**: Drivers can support multiple connection methods without implementing each one from scratch
- **Standardization**: Connection plugins provide a standardized way to handle device connections

The available connection plugins include:

- **Serial Connection**: For devices that connect via serial ports (RS-232, USB-to-Serial, etc.)
- **TCP Connection**: For devices that connect via TCP/IP networks
- **UDP Connection**: For devices that communicate using UDP datagrams

## Serial Connections

Most astronomical devices communicate over serial connections, and INDI handles much of the complexity for you.

### Setting Up a Serial Connection

Let's add this to our header file:

```cpp
namespace Connection
{
    class Serial;
}
```

And this to our class header definition:

```cpp
private: // serial connection
    bool Handshake();
    bool sendCommand(const char *cmd);
    int PortFD{-1};

    Connection::Serial *serialConnection{nullptr};
```

Then we need to add this include to our cpp file:

```cpp
#include "libindi/connectionplugins/connectionserial.h"
```

Then add some new lines to `initProperties`:

```cpp
    // Add debug/simulation/etc controls to the driver.
    addAuxControls();

    serialConnection = new Connection::Serial(this);
    serialConnection->registerHandshake([&]() { return Handshake(); });
    serialConnection->setDefaultBaudRate(Connection::Serial::B_57600);
    serialConnection->setDefaultPort("/dev/ttyACM0");
    registerConnection(serialConnection);
```

This is pretty straightforward - we're registering a new serial connection. `DefaultDevice` will use it to connect when the user clicks the `Connect` button, then call `Handshake` when it is connected.

### Implementing the Handshake

The handshake function is called after the connection is established to verify communication with the device:

```cpp
bool MyCustomDriver::Handshake()
{
    if (isSimulation())
    {
        LOGF_INFO("Connected successfully to simulated %s.", getDeviceName());
        return true;
    }

    // TODO: Any initial communication needed with our device; we have an active
    // connection with a valid file descriptor called PortFD. This file descriptor
    // can be used with the tty_* functions in indicom.h

    return true;
}
```

Here we get a reference to the file descriptor that we can use in the `tty_*` functions exposed in `libindi/indicom.h`.

### Sending Commands

Here's an example `sendCommand` method, but this will be specific to your device's protocol:

```cpp
bool MyCustomDriver::sendCommand(const char *cmd)
{
    int nbytes_read = 0, nbytes_written = 0, tty_rc = 0;
    char res[8] = {0};
    LOGF_DEBUG("CMD <%s>", cmd);

    if (!isSimulation())
    {
        tcflush(PortFD, TCIOFLUSH);
        if ((tty_rc = tty_write_string(PortFD, cmd, &nbytes_written)) != TTY_OK)
        {
            char errorMessage[MAXRBUF];
            tty_error_msg(tty_rc, errorMessage, MAXRBUF);
            LOGF_ERROR("Serial write error: %s", errorMessage);
            return false;
        }
    }

    if (isSimulation())
    {
        strncpy(res, "OK#", 8);
        nbytes_read = 3;
    }
    else
    {
        if ((tty_rc = tty_read_section(PortFD, res, '#', 1, &nbytes_read)) != TTY_OK)
        {
            char errorMessage[MAXRBUF];
            tty_error_msg(tty_rc, errorMessage, MAXRBUF);
            LOGF_ERROR("Serial read error: %s", errorMessage);
            return false;
        }
    }

    res[nbytes_read - 1] = '\0';
    LOGF_DEBUG("RES <%s>", res);

    return true;
}
```

## Network Connections

For devices that communicate over networks, INDI provides TCP and UDP connection plugins.

### TCP Connection

TCP connections are used for devices that connect via TCP/IP networks, such as networked cameras, mounts, or other astronomical devices.

#### Setting Up a TCP Connection

```cpp
#include "libindi/connectionplugins/connectiontcp.h"

// In your class definition
private:
    Connection::TCP *tcpConnection{nullptr};
    int PortFD{-1};
    bool Handshake();
```

```cpp
// In your constructor or initProperties
tcpConnection = new Connection::TCP(this);
tcpConnection->setDefaultHost("192.168.1.100");
tcpConnection->setDefaultPort(9999);
tcpConnection->registerHandshake([&]() { return Handshake(); });
registerConnection(tcpConnection);
```

### UDP Connection

UDP connections are used for devices that communicate using UDP datagrams.

#### Setting Up a UDP Connection

```cpp
#include "libindi/connectionplugins/connectionudp.h"

// In your class definition
private:
    Connection::UDP *udpConnection{nullptr};
    int PortFD{-1};
    bool Handshake();
```

```cpp
// In your constructor or initProperties
udpConnection = new Connection::UDP(this);
udpConnection->setDefaultHost("192.168.1.100");
udpConnection->setDefaultPort(9999);
udpConnection->registerHandshake([&]() { return Handshake(); });
registerConnection(udpConnection);
```

## USB Connections

For USB devices, there is no specific connection plugin as you are expected to use libusb directly. INDI provides the `INDI::USBDevice` class that you can inherit from to simplify USB communication.

```cpp
#include "libindi/usb/usb.h"

class MyUSBDriver : public INDI::DefaultDevice, public INDI::USBDevice
{
public:
    MyUSBDriver();
    virtual ~MyUSBDriver() = default;

    // DefaultDevice overrides
    virtual bool initProperties() override;
    virtual bool updateProperties() override;
    virtual bool Connect() override;
    virtual bool Disconnect() override;
    virtual const char *getDefaultName() override;

private:
    // USB-specific methods
    bool findUSBDevice();
    bool configureUSBDevice();
};
```

## Supporting Multiple Connection Methods

INDI drivers can support multiple connection methods by registering multiple connection plugins. This allows users to choose the most appropriate connection method for their setup.

```cpp
// In your constructor or initProperties
setConnectionMode(CONNECTION_SERIAL | CONNECTION_TCP);

// Create and register the Serial Connection plugin
serialConnection = new Connection::Serial(this);
serialConnection->registerHandshake([&]() { return Handshake(); });
registerConnection(serialConnection);

// Create and register the TCP Connection plugin
tcpConnection = new Connection::TCP(this);
tcpConnection->registerHandshake([&]() { return Handshake(); });
registerConnection(tcpConnection);
```

When supporting multiple connection methods, you need to handle each connection type appropriately in your driver's methods:

```cpp
bool MyCustomDriver::sendCommand(const char *cmd, char *res, int reslen)
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
```

## Properties Defined After Connecting

If you have properties that you want defined only when you are connected, you'll need to override the `updateProperties` method:

```cpp
bool MyCustomDriver::updateProperties()
{
    INDI::DefaultDevice::updateProperties();

    if (isConnected())
    {
        // Add the properties to the driver when we connect.
        defineProperty(&SayHelloSP);
        defineProperty(&WhatToSayTP);
    }
    else
    {
        // And remove them when we disconnect.
        deleteProperty(SayHelloSP.name);
        deleteProperty(WhatToSayTP.name);
    }

    return true;
}
```

Remember, you can call `defineProperty` any time, not just in `initProperties` or `updateProperties`. You could query the capabilities of your device first, or call it in response to user interaction.

## Custom Connection Plugins

If the built-in connection plugins don't meet your needs, you can create custom connection plugins by inheriting from the `Connection::Interface` class and implementing the required methods. This is useful for devices with proprietary communication protocols.

## Best Practices

When implementing device communication in your INDI drivers, follow these best practices:

1. **Use the appropriate connection plugin** for your device
2. **Configure the plugin properly** with default values that make sense for your device
3. **Implement a handshake function** to verify the connection to the device
4. **Handle connection errors gracefully** and provide informative error messages
5. **Support multiple connection methods** if your device can be connected in different ways
6. **Check the connection status** before sending commands to the device
7. **Close the connection properly** when disconnecting from the device
8. **Document the supported connection methods** in your driver's documentation

Now that we have established communication with our device, let's learn how to perform periodic operations in [loops](loops.md).
