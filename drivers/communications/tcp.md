---
title: TCP Connection
parent: Hardware Communications
grand_parent: Driver Development
nav_order: 3
permalink: /drivers/communications/tcp/
---

# TCP Connection Plugin

The TCP Connection plugin handles TCP/IP network connections for devices accessible over local networks or the internet.

## When to Use TCP

- Network-enabled cameras and devices
- WiFi-connected equipment
- Cloud-based telescope mounts
- Devices with Ethernet interfaces
- Remote observatory equipment

## Header File

```cpp
#include <connectionplugins/connectiontcp.h>
```

## Basic Setup

```cpp
class MyDriver : public INDI::DefaultDevice
{
public:
    MyDriver()
    {
        setConnectionMode(CONNECTION_TCP);
        
        tcpConnection = new Connection::TCP(this);
        tcpConnection->setDefaultHost("192.168.1.100");
        tcpConnection->setDefaultPort(9999);
        tcpConnection->registerHandshake([&]() { return Handshake(); });
        
        registerConnection(tcpConnection);
    }

private:
    Connection::TCP *tcpConnection = nullptr;
};
```

## Communication Example

```cpp
bool MyDriver::sendCommand(const char *cmd, char *res, int reslen)
{
    int fd = tcpConnection->getPortFD();
    if (fd < 0)
    {
        LOG_ERROR("TCP socket not open");
        return false;
    }

    // Write and read operations are identical to serial
    int nbytes_written = write(fd, cmd, strlen(cmd));
    if (nbytes_written < 0)
    {
        LOGF_ERROR("Write error: %s", strerror(errno));
        return false;
    }

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

## Best Practices

- **Connection Timeouts**: Set appropriate timeouts for network operations
- **Keep-Alive**: Consider TCP keep-alive for long-running connections
- **Reconnection**: Implement automatic reconnection logic
- **Error Handling**: Network errors are common, handle gracefully
- **Firewall**: Document required ports in driver documentation

## Troubleshooting

### Connection Refused
- Verify device IP address and port
- Check firewall settings
- Ensure device is powered on and network-accessible

### Connection Timeout
- Ping device to verify network connectivity
- Check for network congestion
- Verify correct port number

## Related Guides

- [Connection Plugins Overview](connection-plugins/)
- [Best Practices](best-practices/)
- [Troubleshooting](troubleshooting/)
