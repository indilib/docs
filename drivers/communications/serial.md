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
    MyDriver()
    {
        // Set default connection mode to Serial
        setConnectionMode(CONNECTION_SERIAL);

        // Create the Serial Connection plugin
        serialConnection = new Connection::Serial(this);

        // Configure defaults (optional)
        serialConnection->setDefaultBaudRate(Connection::Serial::B_9600);
        serialConnection->setDefaultPort("/dev/ttyUSB0");

        // Register handshake function (optional)
        serialConnection->registerHandshake([&]() { return Handshake(); });

        // Register with the driver
        registerConnection(serialConnection);
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

- [Connection Plugins Overview](connection-plugins/)
- [Best Practices](best-practices/)
- [Troubleshooting](troubleshooting/)
- [System Setup](system-setup/)
