---
title: Hardware Communications
nav_order: 4
parent: Driver Development
has_children: true
permalink: /drivers/communications/
---

# Hardware Communications Guide

This comprehensive guide covers all methods for communicating with hardware devices in INDI drivers. Whether you're connecting to a serial device, network equipment, USB hardware, GPIO pins, or I2C sensors, this guide will help you choose and implement the right communication method.

## Quick Reference: Choosing Your Communication Method

| Communication Type | Use When | Library/API | Example Devices | Documentation |
|-------------------|----------|-------------|-----------------|---------------|
| **Serial** | RS-232 or USB-Serial devices | Connection::Serial plugin | Mounts, focusers, cameras | [Serial Guide](serial/) |
| **TCP** | Network devices using TCP | Connection::TCP plugin | Network cameras, cloud mounts | [TCP Guide](tcp/) |
| **UDP** | Network devices using UDP | Connection::UDP plugin | Discovery services, streaming | [UDP Guide](udp/) |
| **I2C** | I2C bus devices | Connection::I2C plugin | Temperature sensors, ADCs, displays | [I2C Guide](i2c/) |
| **USB Bulk** | Direct USB control with bulk transfers | libusb | DSLR shutters, custom USB devices | [USB Guide](usb/) |
| **HID** | USB Human Interface Devices | hidapi | Filter wheels, focusers, keypads | [HID Guide](hid/) |
| **GPIO** | Raspberry Pi / Linux GPIO pins | libgpiod | Relays, switches, sensors, PWM | [GPIO Guide](gpio/) |
| **Modbus** | Industrial devices (PLCs, relays, sensors) | nanomodbus (built-in) | Relay boards, automation equipment | [Modbus Guide](modbus/) |
| **Web Protocols** | REST APIs, HTTP services, JSON data | cpp-httplib, nlohmann/json | Cloud devices, IoT, web APIs | [Web Protocols Guide](web-protocols/) |

## Communication Methods Overview

### Connection Plugins

Connection plugins provide a standardized, modular framework for common communication protocols:

- **[Connection Plugins Overview](connection-plugins/)** - Introduction to the plugin framework
- **[Serial Connection](serial/)** - RS-232 and USB-Serial communication
- **[TCP Connection](tcp/)** - Network devices using TCP/IP
- **[UDP Connection](udp/)** - Network devices using UDP datagrams
- **[I2C Connection](i2c/)** - I2C bus devices on Linux systems

**Advantages:**
- Modular and reusable across drivers
- Standardized user interface
- Built-in connection management
- Configuration persistence

### Direct Hardware Communication

For specialized hardware that doesn't fit the plugin model:

- **[USB Communication](usb/)** - Direct USB bulk transfers with libusb
- **[HID Communication](hid/)** - Human Interface Devices with hidapi
- **[GPIO Communication](gpio/)** - Digital I/O and PWM with libgpiod

**Advantages:**
- Full control over hardware interactions
- Support for specialized protocols
- Direct access to device features
- Platform-specific optimizations

### Application Layer Protocols

For modern web-based devices and industrial automation equipment:

- **[Web Protocols](web-protocols/)** - HTTP clients/servers and JSON handling
- **[Modbus](modbus/)** - Industrial automation protocol (RTU and TCP)

**Advantages:**
- Industry-standard protocols with wide device support
- Structured data models and well-defined operations
- Built-in libraries (nanomodbus included, cpp-httplib available)
- Suitable for both consumer IoT and industrial applications
- Extensible to other protocols (WebSocket, MQTT, etc.)

**Important:** The TCP Connection Plugin is designed for raw binary/text communication protocols. For HTTP/REST APIs, use the [Web Protocols Guide](web-protocols/). For Modbus-enabled industrial devices, use the [Modbus Guide](modbus/).

## Additional Resources

- **[Creating Custom Plugins](custom-plugins/)** - Build your own connection plugin
- **[Best Practices](best-practices/)** - Guidelines and platform considerations
- **[Troubleshooting](troubleshooting/)** - Common issues and solutions
- **[System Setup](system-setup/)** - Permissions, udev rules, and configuration

## Getting Started

1. **Choose your communication method** using the quick reference table above
2. **Read the specific guide** for your chosen method
3. **Review the examples** from actual INDI drivers
4. **Set up system permissions** following the [System Setup guide](system-setup/)
5. **Refer to [Best Practices](best-practices/)** for implementation guidelines

## Example Drivers

For complete working examples, see:

- **Serial/TCP/UDP**: Most INDI drivers in the main repository
- **USB**: `indi-gphoto/dsusbdriver.cpp` - DSLR shutter control
- **HID**: `indi-sx/sxwheel.cpp` - Starlight Xpress filter wheel
- **GPIO**: `indi-gpio/indi_gpio.cpp` - Raspberry Pi GPIO driver
- **I2C**: `indi/libs/indibase/connectionplugins/connectioni2c.cpp` - I2C plugin
- **Modbus**: `indi/drivers/io/waveshare_modbus_relay.cpp` - Modbus TCP relay control
- **Web Protocols**: 
  - HTTP Client: `indi/drivers/auxiliary/ipx800v4.cpp` - REST API consumer
  - HTTP Server: `indi/drivers/alpaca/indi_alpaca_server.cpp` - REST API provider
  - JSON Handling: `indi/drivers/auxiliary/alto.cpp` - JSON parsing examples

## Need Help?

- Review the [Troubleshooting Guide](troubleshooting/) for common issues
- Check the [INDI API Documentation](https://www.indilib.org/api/)
- Visit the [INDI Support Forum](https://indilib.org/forum.html)
- See the [Driver Development Guide](/drivers/) for general driver information
