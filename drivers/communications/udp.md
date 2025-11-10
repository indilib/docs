---
title: UDP Connection
parent: Hardware Communications
grand_parent: Driver Development
nav_order: 4
permalink: /drivers/communications/udp/
---

# UDP Connection Plugin

The UDP Connection plugin handles UDP datagram communication for devices that use connectionless protocols.

## When to Use UDP

- Discovery services
- Streaming data applications
- Low-latency communication
- Broadcast/multicast applications
- Devices using UDP protocols

## Header File

```cpp
#include <connectionplugins/connectionudp.h>
```

## Basic Setup

```cpp
class MyDriver : public INDI::DefaultDevice
{
public:
    MyDriver()
    {
        setConnectionMode(CONNECTION_UDP);
        
        udpConnection = new Connection::UDP(this);
        udpConnection->setDefaultHost("192.168.1.100");
        udpConnection->setDefaultPort(9999);
        udpConnection->registerHandshake([&]() { return Handshake(); });
        
        registerConnection(udpConnection);
    }

private:
    Connection::UDP *udpConnection = nullptr;
};
```

## Communication Example

```cpp
bool MyDriver::sendCommand(const char *cmd, char *res, int reslen)
{
    int fd = udpConnection->getPortFD();
    if (fd < 0)
    {
        LOG_ERROR("UDP socket not open");
        return false;
    }

    struct sockaddr_in *remoteAddr = udpConnection->getRemoteAddr();
    if (!remoteAddr)
    {
        LOG_ERROR("Remote address not set");
        return false;
    }

    // Send datagram
    int nbytes_written = sendto(fd, cmd, strlen(cmd), 0, 
                                (struct sockaddr *)remoteAddr, sizeof(*remoteAddr));
    if (nbytes_written < 0)
    {
        LOGF_ERROR("Sendto error: %s", strerror(errno));
        return false;
    }

    // Receive response
    if (res && reslen > 0)
    {
        struct sockaddr_in fromAddr;
        socklen_t fromLen = sizeof(fromAddr);
        int nbytes_read = recvfrom(fd, res, reslen - 1, 0, 
                                   (struct sockaddr *)&fromAddr, &fromLen);
        if (nbytes_read < 0)
        {
            LOGF_ERROR("Recvfrom error: %s", strerror(errno));
            return false;
        }
        res[nbytes_read] = '\0';
    }

    return true;
}
```

## Best Practices

- **No Connection State**: UDP is connectionless, handle accordingly
- **Packet Loss**: Implement retry logic for critical operations
- **Size Limits**: UDP datagrams have size limitations (typically ~65KB max)
- **Timeouts**: Always use timeouts for receive operations
- **Firewall**: Consider both directions for firewall rules

## Related Guides

- [Connection Plugins Overview](connection-plugins/)
- [TCP Connection](tcp/) - For reliable connections
- [Best Practices](best-practices/)
