---
title: Server
nav_order: 4
has_children: true
permalink: /indiserver/
---

# INDI Server

The INDI server (`indiserver`) is a standalone process that manages the communication between INDI drivers and clients. This section provides detailed information about the INDI server, its features, and how to use it.

## Introduction to INDI Server

The INDI server is a key component of the INDI architecture. It acts as a hub that connects clients to drivers, providing:

- Network access to INDI drivers
- Device discovery
- Data routing between clients and drivers
- Resource management
- Logging and debugging

The INDI server can run multiple drivers simultaneously, allowing clients to control multiple devices through a single connection. It handles all the network communication, so drivers don't need to implement their own network code.

## Server Behavior

An INDI Server presents the behavior of a Client to all Devices it runs, and the behavior of all those Devices to all connected Clients. In its simplest form, each command the Server receives from any Device is sent to all Clients, and each command the Server receives from any Client is sent to all Devices.

INDI Servers must take special precautions to deal with large BLOBs:

- They must maintain and honor the enableBLOB state for each Client
- They may drop BLOBs if they arrive faster than slow recipients can handle
- They must take care not to block while writing large BLOBs

It is possible to build INDI Servers such that they can be chained together, allowing control across a network to be distributed and take advantage of multiprocessing.

## Installing INDI Server

The INDI server is part of the INDI library package. To install it:

### Ubuntu/Debian

```bash
sudo apt-get install indi-bin
```

### Fedora

```bash
sudo dnf install indi-bin
```

### macOS (using Homebrew)

```bash
brew install indi-bin
```

### From Source

```bash
git clone https://github.com/indilib/indi.git
cd indi
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr ..
make
sudo make install
```

## Running INDI Server

To start the INDI server with one or more drivers:

```bash
indiserver [options] driver1 [driver2 ...]
```

For example, to start the INDI server with the CCD simulator driver:

```bash
indiserver indi_simulator_ccd
```

To start the INDI server with multiple drivers:

```bash
indiserver indi_simulator_ccd indi_simulator_telescope indi_simulator_focus
```

### Server Options

The INDI server supports several command-line options:

- `-p port`: Listen on the given port (default: 7624)
- `-r`: Allow remote connections (default: only local connections)
- `-v`: Enable verbose output
- `-m MB`: Limit memory used by each driver to MB megabytes
- `-f path`: Log driver messages to a file
- `-l dir`: Load driver(s) from the given directory
- `-d`: Enable debug output
- `-s`: Force driver to use syslog for logging
- `-n`: No auto-connect when using driver selection
- `-c config`: Load a configuration file
- `-w`: Wait for all drivers to be ready before accepting connections
- `-u`: Run drivers as a specific user
- `-g`: Run drivers as a specific group

For example, to start the INDI server on port 8000 with verbose output:

```bash
indiserver -p 8000 -v indi_simulator_ccd
```

### Driver Selection

The INDI server can dynamically load drivers based on client requests. To enable driver selection:

```bash
indiserver -l /usr/share/indi
```

With driver selection enabled, clients can request specific drivers to be loaded by sending a `<getProperties device="driver_name"/>` message. The server will search for a driver with the given name in the specified directory and load it if found.

## Connecting to INDI Server

Clients can connect to the INDI server using TCP/IP, typically on port 7624. Once connected, they can:

- Discover available devices
- Get and set device properties
- Receive property updates
- Process binary data (e.g., images from cameras)

For example, to connect to a local INDI server using the `indi_getprop` command-line tool:

```bash
indi_getprop -h localhost -p 7624
```

## Server Configuration

The INDI server can be configured using a configuration file. The configuration file specifies which drivers to load and their options.

Example configuration file:

```xml
<INDIServer>
  <Driver name="CCD Simulator">
    <executable>indi_simulator_ccd</executable>
    <version>1.0</version>
  </Driver>
  <Driver name="Telescope Simulator">
    <executable>indi_simulator_telescope</executable>
    <version>1.0</version>
  </Driver>
</INDIServer>
```

To load a configuration file:

```bash
indiserver -c config.xml
```

## Server Security

By default, the INDI server only accepts connections from the local machine. To allow remote connections, use the `-r` option:

```bash
indiserver -r indi_simulator_ccd
```

For more secure remote access, consider using SSH tunneling:

```bash
ssh -L 7624:localhost:7624 user@remote_host
```

Then connect to the local port 7624, which will be forwarded to the remote INDI server.

## Server Logging

The INDI server can log driver messages to a file or to syslog. To log to a file:

```bash
indiserver -f /var/log/indi.log indi_simulator_ccd
```

To log to syslog:

```bash
indiserver -s indi_simulator_ccd
```

## Server Performance

The INDI server is designed to be lightweight and efficient. However, when dealing with large BLOBs (e.g., images from cameras), it can consume significant resources. To limit the memory used by each driver:

```bash
indiserver -m 64 indi_simulator_ccd
```

This limits each driver to 64 MB of memory.

## Server Extensions

The INDI server can be extended with additional functionality through plugins or custom implementations. Some examples of server extensions include:

- **INDI Web Manager**: A web interface for managing INDI servers and drivers
- **INDI Supervisor**: A tool for monitoring and restarting INDI servers and drivers
- **INDI Multiplexer**: A server that can connect to multiple INDI servers and present them as a single server

## Server Implementation

The INDI server is implemented in C++ and is part of the INDI library. The source code is available in the [INDI GitHub repository](https://github.com/indilib/indi).

The server implementation handles:

- Network communication using TCP/IP sockets
- XML parsing and generation
- Process management for drivers
- Resource management
- Error handling and recovery

For more information about the server implementation, see the [INDI Library API Documentation](https://www.indilib.org/api/index.html).

## Server Guide

For more detailed information about using the INDI server, see the [INDI Server Guide](indiserver-guide.md).

## Conclusion

The INDI server is a critical component of the INDI ecosystem, providing the glue that connects clients to drivers. By understanding how the server works and how to configure it, you can build robust and efficient INDI-based systems for controlling astronomical equipment.
