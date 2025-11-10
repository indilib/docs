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

## Need Help?

- Review the [Troubleshooting Guide](troubleshooting/) for common issues
- Check the [INDI API Documentation](https://www.indilib.org/api/)
- Visit the [INDI Support Forum](https://indilib.org/forum.html)
- See the [Driver Development Guide](/drivers/) for general driver information
