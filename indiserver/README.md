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

```
Usage: indiserver [options] driver [driver ...]
Purpose: server for local and remote INDI drivers
Options:
 -l d     : log driver messages to <d>/YYYY-MM-DD.islog
 -m m     : kill client if gets more than this many MB behind, default 128
 -d m     : drop streaming blobs if client gets more than this many MB behind, default 5. 0 to disable
 -u path  : Path for the local connection socket (abstract), default /tmp/indiserver
 -p p     : alternate IP port, default 7624
 -r r     : maximum driver restarts on error, default 10
 -f path  : Path to fifo for dynamic startup and shutdown of drivers.
 -v       : show key events, no traffic
 -vv      : -v + key message content
 -vvv     : -vv + complete xml
driver    : executable or [device]@host[:port]
```

For example, to start the INDI server on port 8000 with verbose output:

```bash
indiserver -p 8000 -v indi_simulator_ccd
```

### FIFO Mode for Dynamic Driver Startup and Shutdown

The INDI server supports a FIFO (First In, First Out) mode for dynamic driver startup and shutdown. This allows you to add or remove drivers while the server is running, without having to restart the server.

To enable FIFO mode, use the `-f` option followed by the path to the FIFO file:

```bash
indiserver -f /tmp/indififo indi_simulator_ccd
```

Once the server is running in FIFO mode, you can add or remove drivers by writing commands to the FIFO file:

- To start a new driver, write `start driver_name` to the FIFO file:

  ```bash
  echo "start indi_simulator_telescope" > /tmp/indififo
  ```

- To stop a running driver, write `stop driver_name` to the FIFO file:
  ```bash
  echo "stop indi_simulator_ccd" > /tmp/indififo
  ```

This is particularly useful for applications that need to dynamically manage drivers based on user input or other events.

For more detailed information about INDI in FIFO mode, see the [INDI Server documentation](https://indilib.org/develop/developer-manual/92-indi-server.html).

### Driver Reliability

The INDI server can automatically restart drivers that crash or exit with an error. By default, the server will restart a driver up to 10 times if it exits with an error. You can change this limit using the `-r` option:

```bash
indiserver -r 5 indi_simulator_ccd
```

This sets the maximum number of driver restarts to 5.

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

## Local Connection Socket

The INDI server uses a local connection socket for communication between the server and drivers. By default, this socket is located at `/tmp/indiserver`. You can specify a different path using the `-u` option:

```bash
indiserver -u /path/to/socket indi_simulator_ccd
```

This can be useful if you need to run multiple INDI servers on the same machine, or if you need to use a different path for security reasons.

## Server Security

By default, the INDI server only accepts connections from the local machine. For secure remote access, consider using SSH tunneling or [EkosLive](https://ekoslive.com):

```bash
ssh -L 7624:localhost:7624 user@remote_host
```

Then connect to the local port 7624, which will be forwarded to the remote INDI server.

## Server Logging

The INDI server can log driver messages to a directory. To enable logging:

```bash
indiserver -l /var/log/indi indi_simulator_ccd
```

This will create log files in the specified directory with names in the format `YYYY-MM-DD.islog`.

## Server Performance

The INDI server is designed to be lightweight and efficient. However, when dealing with large BLOBs (e.g., images from cameras), it can consume significant resources. The server provides options to manage this:

- `-m m`: Kill client if it gets more than `m` MB behind (default: 128)
- `-d m`: Drop streaming blobs if client gets more than `m` MB behind (default: 5, 0 to disable)

For example:

```bash
indiserver -m 64 -d 10 indi_simulator_ccd
```

This kills clients that get more than 64 MB behind and drops streaming blobs if clients get more than 10 MB behind.

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
