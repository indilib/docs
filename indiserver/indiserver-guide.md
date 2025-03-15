---
title: INDI Server Guide
nav_order: 1
parent: INDI Server
---

# INDI Server Guide

This guide provides a comprehensive overview of the INDI server, including its installation, configuration, and usage. It is intended for developers and users who want to set up and manage INDI servers for controlling astronomical equipment.

## Introduction to the INDI Server

The INDI server (`indiserver`) is a standalone process that manages the communication between INDI drivers and clients. It acts as a mediator, forwarding messages between clients and drivers, and handling the connection and disconnection of devices.

The INDI server is designed to be lightweight, reliable, and easy to use. It can run on a variety of platforms, including Linux, macOS, and Windows (via WSL), and can be configured to meet the specific needs of different astronomical setups.

## Installation

### Linux

On most Linux distributions, the INDI server can be installed from the package repositories:

#### Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install indi-bin
```

#### Fedora

```bash
sudo dnf install indi
```

#### Arch Linux

```bash
sudo pacman -S indi
```

### macOS

On macOS, the INDI server can be installed using Homebrew:

```bash
brew tap indilib/indi
brew install indi-bin
```

### Building from Source

If you need the latest version or if the INDI server is not available in your package repositories, you can build it from source:

```bash
# Install dependencies
sudo apt-get install build-essential cmake git libnova-dev libcfitsio-dev libusb-1.0-0-dev zlib1g-dev libgsl-dev

# Clone the repository
git clone https://github.com/indilib/indi.git

# Build and install
cd indi
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release ..
make
sudo make install
```

## Basic Usage

### Starting the INDI Server

To start the INDI server with one or more drivers, use the following command:

```bash
indiserver [options] driver1 [driver2 ...]
```

For example, to start the INDI server with the CCD Simulator and Telescope Simulator drivers:

```bash
indiserver indi_simulator_ccd indi_simulator_telescope
```

### Command-Line Options

The INDI server supports several command-line options:

- `-p port`: Specify the port number (default: 7624)
- `-r host:port`: Connect to a remote INDI server
- `-v`: Enable verbose output
- `-vv`: Enable more verbose output
- `-vvv`: Enable very verbose output (including all XML traffic)
- `-m mb`: Limit the maximum memory used by the server (in megabytes)
- `-f path`: Log to a file instead of stdout
- `-l dir`: Load drivers from the specified directory
- `-d`: Enable driver debugging
- `-s`: Simulate hardware (all drivers)
- `-n`: No auto-connection (all drivers)
- `-c config`: Load a configuration file

For example, to start the INDI server on port 8000 with verbose output:

```bash
indiserver -p 8000 -v indi_simulator_ccd
```

### Connecting to the INDI Server

INDI clients can connect to the INDI server using TCP/IP sockets. By default, the INDI server listens on port 7624 on all network interfaces.

To connect to a local INDI server, clients can use the hostname `localhost` or the IP address `127.0.0.1`:

```
hostname: localhost
port: 7624
```

To connect to a remote INDI server, clients need to know the hostname or IP address of the server:

```
hostname: remote-server.example.com
port: 7624
```

## Advanced Configuration

### Running Multiple INDI Servers

You can run multiple INDI servers on the same machine by specifying different port numbers:

```bash
# Start the first INDI server on the default port (7624)
indiserver indi_simulator_ccd &

# Start the second INDI server on port 7625
indiserver -p 7625 indi_simulator_telescope &
```

### Remote INDI Servers

The INDI server can connect to remote INDI servers using the `-r` option:

```bash
# Connect to a remote INDI server at remote-server.example.com:7624
indiserver -r remote-server.example.com:7624
```

This allows you to create a network of INDI servers, where one server can forward messages to other servers.

### Logging

The INDI server can log its output to a file instead of stdout using the `-f` option:

```bash
# Log to a file
indiserver -f /var/log/indi.log indi_simulator_ccd
```

You can also enable different levels of verbosity using the `-v`, `-vv`, and `-vvv` options:

```bash
# Enable verbose output
indiserver -v indi_simulator_ccd

# Enable more verbose output
indiserver -vv indi_simulator_ccd

# Enable very verbose output (including all XML traffic)
indiserver -vvv indi_simulator_ccd
```

### Memory Limits

The INDI server can limit the maximum memory used by each driver using the `-m` option:

```bash
# Limit each driver to 64 MB of memory
indiserver -m 64 indi_simulator_ccd
```

This can be useful to prevent memory leaks or excessive memory usage by drivers.

### Driver Debugging

The INDI server can enable debugging for all drivers using the `-d` option:

```bash
# Enable driver debugging
indiserver -d indi_simulator_ccd
```

This is equivalent to enabling the `DEBUG` property for each driver.

### Simulation Mode

The INDI server can start all drivers in simulation mode using the `-s` option:

```bash
# Start all drivers in simulation mode
indiserver -s indi_simulator_ccd
```

This is equivalent to enabling the `SIMULATION` property for each driver.

### No Auto-Connection

The INDI server can disable auto-connection for all drivers using the `-n` option:

```bash
# Disable auto-connection for all drivers
indiserver -n indi_simulator_ccd
```

This is equivalent to disabling the `CONNECTION_MODE` property for each driver.

### Configuration Files

The INDI server can load a configuration file using the `-c` option:

```bash
# Load a configuration file
indiserver -c /etc/indi/indi.conf
```

The configuration file can contain command-line options and driver specifications, one per line. For example:

```
# INDI Server Configuration
-p 7624
-v
-f /var/log/indi.log
indi_simulator_ccd
indi_simulator_telescope
```

## INDI Server Architecture

The INDI server follows a client-server architecture:

- **INDI Server**: A standalone process that manages the communication between INDI drivers and clients.
- **INDI Drivers**: Processes that communicate with hardware devices and expose their functionality through INDI properties.
- **INDI Clients**: Applications that connect to the INDI server, discover devices, and control device properties.

The INDI server uses TCP/IP sockets for communication with clients and standard input/output streams for communication with drivers. When a client connects to the server, the server creates a new socket for the client and forwards messages between the client and the appropriate drivers.

### Message Flow

The message flow between clients, the server, and drivers is as follows:

1. Clients connect to the INDI server using TCP/IP sockets.
2. The server forwards messages from clients to the appropriate drivers.
3. Drivers process the messages and send responses back to the server.
4. The server forwards the responses to the appropriate clients.

### Driver Management

The INDI server manages the lifecycle of drivers:

1. When the server starts, it launches the specified drivers as separate processes.
2. The server communicates with drivers using standard input/output streams.
3. If a driver crashes or exits, the server can restart it automatically.
4. When the server exits, it terminates all driver processes.

## INDI Server Security

The INDI server does not provide built-in authentication or encryption. By default, it listens on all network interfaces, which can pose a security risk if the server is exposed to the internet.

To secure the INDI server, consider the following measures:

- **Firewall**: Configure your firewall to restrict access to the INDI server port (default: 7624).
- **VPN**: Use a VPN to secure the connection between clients and the server.
- **SSH Tunneling**: Use SSH tunneling to encrypt the connection between clients and the server.
- **Reverse Proxy**: Use a reverse proxy with SSL/TLS to secure the connection between clients and the server.

### SSH Tunneling

SSH tunneling is a simple way to secure the connection between clients and the INDI server. To create an SSH tunnel:

```bash
# On the client machine
ssh -L 7624:localhost:7624 user@remote-server.example.com
```

This creates a tunnel from port 7624 on the client machine to port 7624 on the remote server. Clients can then connect to `localhost:7624` on the client machine, and the connection will be forwarded to the INDI server on the remote machine through the encrypted SSH tunnel.

## INDI Web Manager

The INDI Web Manager is a web-based interface for managing INDI servers. It provides a user-friendly way to start, stop, and configure INDI drivers, and to monitor the status of the INDI server.

### Installation

To install the INDI Web Manager:

```bash
sudo apt-get install indi-web
```

### Usage

To start the INDI Web Manager:

```bash
indi-web
```

By default, the INDI Web Manager listens on port 8624. You can access it by opening a web browser and navigating to:

```
http://localhost:8624
```

The INDI Web Manager provides the following features:

- **Driver Management**: Start, stop, and configure INDI drivers.
- **Server Monitoring**: Monitor the status of the INDI server and connected clients.
- **Profile Management**: Create and manage profiles for different astronomical setups.
- **Remote Access**: Control the INDI server from a remote location.

## INDI Server in Docker

The INDI server can be run in a Docker container, which provides isolation and portability. This can be useful for deploying the INDI server on different platforms or for testing purposes.

### Building the Docker Image

To build a Docker image for the INDI server:

```bash
# Create a Dockerfile
cat > Dockerfile << 'EOF'
FROM ubuntu:20.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    indi-bin \
    indi-full \
    && rm -rf /var/lib/apt/lists/*

# Expose the INDI server port
EXPOSE 7624

# Set the entrypoint
ENTRYPOINT ["indiserver"]

# Set the default command
CMD ["-v", "indi_simulator_ccd", "indi_simulator_telescope"]
EOF

# Build the image
docker build -t indiserver .
```

### Running the Docker Container

To run the INDI server in a Docker container:

```bash
# Run the container
docker run -p 7624:7624 indiserver
```

This starts the INDI server with the CCD Simulator and Telescope Simulator drivers, and maps port 7624 from the container to port 7624 on the host.

To specify different drivers or options:

```bash
# Run the container with custom options
docker run -p 7624:7624 indiserver -vv indi_simulator_ccd
```

## Troubleshooting

### Common Issues

#### INDI Server Won't Start

If the INDI server won't start, check the following:

- **Port Conflict**: Make sure no other process is using the same port.
- **Driver Path**: Make sure the driver executables are in the system PATH or specify the full path.
- **Permissions**: Make sure you have permission to run the INDI server and drivers.

#### Clients Can't Connect

If clients can't connect to the INDI server, check the following:

- **Server Running**: Make sure the INDI server is running.
- **Firewall**: Make sure the firewall allows connections to the INDI server port.
- **Network Configuration**: Make sure the client can reach the server on the network.

#### Drivers Won't Connect to Devices

If drivers won't connect to devices, check the following:

- **Device Connected**: Make sure the device is connected to the computer.
- **Device Permissions**: Make sure you have permission to access the device.
- **Driver Configuration**: Make sure the driver is configured correctly.

### Debugging

To debug the INDI server, use the verbose output options:

```bash
# Enable verbose output
indiserver -v indi_simulator_ccd

# Enable more verbose output
indiserver -vv indi_simulator_ccd

# Enable very verbose output (including all XML traffic)
indiserver -vvv indi_simulator_ccd
```

You can also enable debugging for specific drivers:

```bash
# Enable debugging for a specific driver
indiserver -d indi_simulator_ccd
```

## Conclusion

The INDI server is a powerful tool for controlling astronomical equipment. By understanding its installation, configuration, and usage, you can set up and manage INDI servers for your specific needs.

For more information, refer to the [INDI Server Documentation](https://www.indilib.org/develop/developer-manual/107-indi-server.html) and the [INDI Library Documentation](https://www.indilib.org/api/index.html).
