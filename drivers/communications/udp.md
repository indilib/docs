---
title: UDP Connection
parent: Hardware Communications
grand_parent: Driver Development
nav_order: 4
permalink: /drivers/communications/udp/
---

# UDP Connection

INDI does not have a separate UDP connection plugin. UDP is a mode of the [TCP connection plugin](tcp.md) (`Connection::TCP`), which can open either a TCP stream socket or a UDP datagram socket to the configured host and port.

## When to Use UDP

- Devices whose network protocol is UDP-based, such as some WiFi mounts and dome controllers
- Low-latency request/response protocols where each command fits in a single datagram

Several drivers in the [INDI library](https://github.com/indilib/indi) use UDP this way, including the SkyWatcher Alt-Az WiFi mount (`skywatcherAPIMount.cpp`), Gemini (`lx200gemini.cpp`), Astro-Physics (`lx200ap_v2.cpp`), and NexDome Beaver (`nexdome_beaver.cpp`).

## Header File

```cpp
#include <connectionplugins/connectiontcp.h>
```

## Basic Setup

Create a `Connection::TCP` plugin and call `setConnectionType()` with `Connection::TCP::TYPE_UDP`:

```cpp
class MyDriver : public INDI::DefaultDevice
{
public:
    bool initProperties() override
    {
        INDI::DefaultDevice::initProperties();

        // Connection plugins are created in initProperties(), once the device name is known.
        udpConnection = new Connection::TCP(this);
        udpConnection->setDefaultHost("192.168.4.1");
        udpConnection->setDefaultPort(11880);

        // Use a UDP datagram socket instead of a TCP stream socket.
        udpConnection->setConnectionType(Connection::TCP::TYPE_UDP);

        udpConnection->registerHandshake([&]() { return Handshake(); });
        registerConnection(udpConnection);

        return true;
    }

private:
    Connection::TCP *udpConnection = nullptr;
    int PortFD = -1;

    bool Handshake()
    {
        PortFD = udpConnection->getPortFD();

        // Read each reply as a whole datagram (see below).
        tty_set_generic_udp_format(1);

        // Send a command and check the reply to verify the device responds.
        return true;
    }
};
```

`setConnectionType()` sets the default. The plugin also defines a `CONNECTION_TYPE` switch property (with `TCP` and `UDP` elements) on the Connection tab, so users can change the type, and the choice is saved in the driver's configuration. If a saved configuration exists, it takes precedence over the default.

## Communication Example

The plugin connects the UDP socket to the configured host and port, so you can use the `tty_*` functions from `indicom.h` as for TCP. Each write sends one datagram.

Reading needs one extra step. By default, `tty_read_section()` reads one byte at a time, and on a UDP socket each read consumes a whole datagram, so the rest of the reply would be lost. Call `tty_set_generic_udp_format(1)` once the connection is established, as in the `Handshake()` above, so that `tty_read()` and `tty_read_section()` read each reply as a complete datagram. If your driver also supports serial or TCP connections, enable it only when the UDP connection is active, and pass `0` otherwise.

```cpp
bool MyDriver::sendCommand(const char *cmd, char *res, int reslen)
{
    int nbytes_written = 0, nbytes_read = 0;

    int rc = tty_write_string(PortFD, cmd, &nbytes_written);
    if (rc != TTY_OK)
    {
        char errstr[MAXRBUF] = {0};
        tty_error_msg(rc, errstr, MAXRBUF);
        LOGF_ERROR("Write error: %s", errstr);
        return false;
    }

    if (res == nullptr || reslen <= 0)
        return true;

    // Wait up to 3 seconds for a '#'-terminated reply.
    rc = tty_read_section(PortFD, res, '#', 3, &nbytes_read);
    if (rc != TTY_OK)
    {
        char errstr[MAXRBUF] = {0};
        tty_error_msg(rc, errstr, MAXRBUF);
        LOGF_ERROR("Read error: %s", errstr);
        return false;
    }

    res[nbytes_read - 1] = '\0'; // Replace the terminator
    return true;
}
```

## Best Practices

- **No Connection State**: UDP is connectionless. A successful connection only means the socket was created, so always verify the device responds in `Handshake()`.
- **Packet Loss**: Datagrams can be lost. Use timeouts on every read and retry critical commands.
- **Size Limits**: Keep each command and reply within a single datagram.
- **Firewall**: Make sure replies from the device are allowed back to the INDI host.

## Related Guides

- [Connection Plugins Overview](connection-plugins.md)
- [TCP Connection](tcp.md) - For reliable connections
- [INDI Driver Development Best Practices](../best-practices.md)
