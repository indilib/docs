---
title: Serial Connection
parent: Hardware Communications
grand_parent: Driver Development
nav_order: 2
permalink: /drivers/communications/serial/
---

# Serial Connection Plugin

The Serial Connection plugin handles RS-232 serial ports, USB-to-Serial adapters, and virtual serial ports. This is one of the most commonly used connection methods in INDI drivers.

## When to Use Serial

- RS-232 serial port devices
- USB-to-Serial adapters (FTDI, Prolific, CP2102, etc.)
- Virtual serial ports
- Most legacy astronomy equipment
- Arduino and microcontroller-based devices

## Header File

```cpp
#include <connectionplugins/connectionserial.h>
```

## Basic Setup

```cpp
class MyDriver : public INDI::DefaultDevice
{
public:
    bool initProperties() override
    {
        INDI::DefaultDevice::initProperties();

        // Connection plugins are created in initProperties(), once the device name is known.
        // Create the Serial Connection plugin
        serialConnection = new Connection::Serial(this);

        // Configure defaults (optional)
        serialConnection->setDefaultBaudRate(Connection::Serial::B_9600);
        serialConnection->setDefaultPort("/dev/ttyUSB0");

        // Register handshake function (optional)
        serialConnection->registerHandshake([&]() { return Handshake(); });

        // Register with the driver
        registerConnection(serialConnection);

        return true;
    }

private:
    Connection::Serial *serialConnection = nullptr;

    bool Handshake()
    {
        // Send test command to verify connection
        char response[32];
        if (sendCommand("*IDN?\n", response, sizeof(response)))
        {
            if (strstr(response, "MyDevice"))
            {
                LOG_INFO("Device handshake successful");
                return true;
            }
        }
        LOG_ERROR("Handshake failed");
        return false;
    }
};
```

## Communication Example

```cpp
bool MyDriver::sendCommand(const char *cmd, char *res, int reslen)
{
    // Get the serial port file descriptor
    int fd = serialConnection->getPortFD();
    if (fd < 0)
    {
        LOG_ERROR("Serial port not open");
        return false;
    }

    // Write command
    int nbytes_written = write(fd, cmd, strlen(cmd));
    if (nbytes_written < 0)
    {
        LOGF_ERROR("Write error: %s", strerror(errno));
        return false;
    }

    // Read response if expected
    if (res && reslen > 0)
    {
        int nbytes_read = read(fd, res, reslen - 1);
        if (nbytes_read < 0)
        {
            LOGF_ERROR("Read error: %s", strerror(errno));
            return false;
        }
        res[nbytes_read] = '\0';
    }

    return true;
}
```

## Available Baud Rates

The Serial plugin supports standard baud rates:

```cpp
Connection::Serial::B_1200    // 1200 bps
Connection::Serial::B_2400    // 2400 bps
Connection::Serial::B_4800    // 4800 bps
Connection::Serial::B_9600    // 9600 bps
Connection::Serial::B_19200   // 19200 bps
Connection::Serial::B_38400   // 38400 bps
Connection::Serial::B_57600   // 57600 bps
Connection::Serial::B_115200  // 115200 bps
Connection::Serial::B_230400  // 230400 bps
```

## Advanced: Reading with Timeout

```cpp
#include <poll.h>

bool MyDriver::readWithTimeout(char *buffer, int len, int timeout_ms)
{
    int fd = serialConnection->getPortFD();
    
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;
    
    int rc = poll(&pfd, 1, timeout_ms);
    if (rc < 0)
    {
        LOGF_ERROR("Poll error: %s", strerror(errno));
        return false;
    }
    else if (rc == 0)
    {
        LOG_ERROR("Read timeout");
        return false;
    }
    
    int nbytes = read(fd, buffer, len - 1);
    if (nbytes < 0)
    {
        LOGF_ERROR("Read error: %s", strerror(errno));
        return false;
    }
    
    buffer[nbytes] = '\0';
    return true;
}
```

## Binary Data Transfer

For binary data (images, firmware, etc.):

```cpp
bool MyDriver::sendBinaryData(const uint8_t *data, size_t len)
{
    int fd = serialConnection->getPortFD();
    size_t totalWritten = 0;
    
    while (totalWritten < len)
    {
        int nbytes = write(fd, data + totalWritten, len - totalWritten);
        if (nbytes < 0)
        {
            LOGF_ERROR("Write error: %s", strerror(errno));
            return false;
        }
        totalWritten += nbytes;
    }
    
    return true;
}
```

## Automatic Port Selection

USB serial adapters can show up under a different port name each time they are plugged in, so the serial plugin helps users connect without having to pick the right port by hand.

### System Ports

When the device connects, the plugin scans the system for serial ports and lists them in the `SYSTEM_PORTS` property, so users can pick a port with one click. On Linux, it lists the stable `/dev/serial/by-id/` links instead of `/dev/ttyUSBx` names when there is one for every adapter. Identical adapters (same USB vendor and product ID) may not get their own by-id links, and in that case the `/dev/ttyUSBx` names are listed instead.

If the port saved in the driver's configuration is no longer available (it is neither among the detected ports nor present on disk) and **exactly one** serial port is detected, the plugin uses that port instead. Custom or mapped ports that still exist, such as a udev symlink like `/dev/mount`, are always kept.

### Auto Search

The `DEVICE_AUTO_SEARCH` property lets users enable auto search. When enabled and connecting to the selected port fails, the plugin tries every other detected port, in random order so that several drivers searching at the same time don't collide, and calls your handshake on each one. On success, it saves the working port and, on Linux, turns auto search off again.

Auto search relies on your handshake to recognize the device, so make sure `Handshake()` returns `false` for any device that is not yours.

### Matching Ports by Name

Some devices include their name in the USB serial ID, for example `/dev/serial/by-id/usb-Pegasus_Astro_FlatMaster_...`. For these devices, call `setPortMatchPattern()` with a case-insensitive regular expression. When the saved port is not available, the plugin selects the first detected port whose path matches the pattern, without having to try a handshake on every port:

```cpp
bool MyDriver::initProperties()
{
    INDI::DefaultDevice::initProperties();

    serialConnection = new Connection::Serial(this);
    serialConnection->setDefaultBaudRate(Connection::Serial::B_115200);

    // Select the port whose ID contains "FlatMaster" if the saved port is gone.
    serialConnection->setPortMatchPattern("FlatMaster");

    serialConnection->registerHandshake([&]() { return Handshake(); });
    registerConnection(serialConnection);

    return true;
}
```

The pattern can match several names, such as `"303a|espressif|wheelly|usbmodem"`. If the pattern is empty (the default), no matching is done.

## Best Practices

- **Baud Rate**: Use the device's documented baud rate
- **Timeouts**: Always use timeouts for read operations
- **Flow Control**: Consider hardware flow control for high-speed transfers
- **Error Recovery**: Implement retry logic for transient errors
- **Buffer Flushing**: Flush buffers after errors or unexpected data
- **Line Endings**: Be consistent with line endings (\r\n, \n, etc.)

## Common Issues

### Permission Denied

```bash
# Add user to dialout group
sudo usermod -a -G dialout $USER
# Log out and log back in
```

### Device Not Found

```bash
# List serial devices
ls -l /dev/ttyUSB* /dev/ttyACM*

# Check dmesg for connection messages
dmesg | tail -20
```

### Wrong Baud Rate

Symptoms include:
- Garbled data
- No response from device
- Partial commands

Solution: Verify baud rate in device documentation

## Example Drivers

- Most INDI drivers use serial communication
- Check `indi-eqmod` for telescope control
- Check `indi-asi` for camera control
- Check any mount or focuser driver

## Related Guides

- [Connection Plugins Overview](connection-plugins.md)
- [Best Practices](../best-practices.md)
- [Troubleshooting](../../support/troubleshooting.md)
- [System Setup](system-setup.md)
