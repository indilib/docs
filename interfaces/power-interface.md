---
title: Power Interface
nav_order: 8
parent: Device Interfaces
---

# Implementing the Power Interface

This guide provides a comprehensive overview of implementing the power interface in INDI drivers. It covers the basic structure of a power driver, how to implement the required methods, and how to handle device-specific functionality.

## Introduction to the Power Interface

The INDI Power Interface (`INDI::PowerInterface`) is designed for devices that manage power distribution, such as power boxes, dew heater controllers, and USB hubs. It provides a standardized way for INDI drivers to expose and control various power-related functionalities, including DC outputs, dew heater ports, variable voltage outputs, voltage/current monitoring, and USB port control.

The power interface is implemented by inheriting from the `INDI::PowerInterface` base class, which provides a set of standard properties and methods for controlling power devices. By implementing this interface, your driver can be used with any INDI client that supports power devices.

## Prerequisites

Before implementing the power interface, you should have:

- Basic knowledge of C++ programming
- Understanding of the INDI protocol and architecture
- Familiarity with the device's communication protocol
- Development environment set up (compiler, build tools, etc.)
- INDI library installed

## Power Interface Structure

The power interface consists of several key components:

- **Base Class**: `INDI::PowerInterface` is the base class for all power drivers.
- **Standard Properties**: A set of standard properties for controlling power devices.
- **Virtual Methods**: A set of virtual methods that must be implemented by the driver.
- **Helper Methods**: A set of helper methods for common power operations.

### Base Class

The `INDI::PowerInterface` base class inherits from `INDI::DefaultDevice` and provides additional functionality specific to power devices. It defines standard properties for power control, dew control, variable voltage control, and more.

### Power Capabilities

The `PowerCapability` enum defines the features supported by a power device. A driver sets these capabilities to inform clients about the device's functionalities.

| Capability Flag             | Description                                   |
| :-------------------------- | :-------------------------------------------- |
| `POWER_HAS_DC_OUT`          | Device has 12V DC outputs.                    |
| `POWER_HAS_DEW_OUT`         | Device has DEW outputs for dew heaters.       |
| `POWER_HAS_VARIABLE_OUT`    | Device has variable voltage outputs.          |
| `POWER_HAS_VOLTAGE_SENSOR`  | Device has voltage monitoring.                |
| `POWER_HAS_OVERALL_CURRENT` | Device has overall current monitoring.        |
| `POWER_HAS_PER_PORT_CURRENT`| Device has per-port current monitoring.       |
| `POWER_HAS_LED_TOGGLE`      | Device can toggle power LEDs.                 |
| `POWER_HAS_AUTO_DEW`        | Device has automatic dew control.             |
| `POWER_HAS_POWER_CYCLE`     | Device can cycle power to all ports.          |
| `POWER_HAS_USB_TOGGLE`      | Device can toggle power to specific USB ports.|

### Key Methods

A driver implementing the `INDI::PowerInterface` must override and implement the following virtual methods to control the power device:

- `void initProperties(const char *groupName, size_t nPowerPorts = 0, size_t nDewPorts = 0, size_t nVariablePorts = 0, size_t nAutoDewPorts = 0, size_t nUSBPorts = 0)`:
  Initializes the INDI properties related to the power interface. This should be called within the driver's `initProperties()` method.
  - `groupName`: The name of the group or tab where power properties will be displayed in the client.
  - `nPowerPorts`: Number of 12V DC power ports.
  - `nDewPorts`: Number of DEW/Dew heater ports.
  - `nVariablePorts`: Number of variable voltage ports.
  - `nAutoDewPorts`: Number of Auto Dew ports.
  - `nUSBPorts`: Number of USB ports that can be toggled.

- `bool SetPowerPort(size_t port, bool enabled)`:
  Sets the state (on/off) of a specific 12V DC power port.
  - `port`: Port number (0-based index).
  - `enabled`: `true` to turn on, `false` to turn off.

- `bool SetDewPort(size_t port, bool enabled, double dutyCycle)`:
  Sets the duty cycle for a DEW/Dew heater port.
  - `port`: Port number (0-based index).
  - `enabled`: `true` to enable the port, `false` to disable.
  - `dutyCycle`: Duty cycle value (0-100%).

- `bool SetVariablePort(size_t port, bool enabled, double voltage)`:
  Sets the voltage for a variable voltage output port.
  - `port`: Port number (0-based index).
  - `enabled`: `true` to enable the port, `false` to disable.
  - `voltage`: Target voltage.

- `bool SetLEDEnabled(bool enabled)`:
  Enables or disables the power LEDs on the device.
  - `enabled`: `true` to enable LEDs, `false` to disable.

- `bool SetAutoDewEnabled(size_t port, bool enabled)`:
  Enables or disables automatic dew control for a specific port (if supported).
  - `port`: Port number (0-based index).
  - `enabled`: `true` to enable, `false` to disable.

- `bool CyclePower()`:
  Cycles power (off then on) to all connected ports.

- `bool SetUSBPort(size_t port, bool enabled)`:
  Sets the state (on/off) of a specific USB port.
  - `port`: Port number (0-based index).
  - `enabled`: `true` to turn on, `false` to turn off.

### Example Implementation

Here's a simplified example of how a driver might implement the `INDI::PowerInterface`. This example focuses on the core structure and omits complex serial communication details for clarity.

```cpp
#include "indibase.h"
#include "indipowerinterface.h"
#include <iostream>

// Forward declaration of a dummy DefaultDevice for the example
class MyPowerDevice;

class MyPowerDevice : public INDI::DefaultDevice, public INDI::PowerInterface
{
public:
    MyPowerDevice() : INDI::DefaultDevice(), INDI::PowerInterface(this)
    {
        setDriverInterface(POWER_INTERFACE);
    }

    virtual bool initProperties() override
    {
        INDI::DefaultDevice::initProperties();

        // Set capabilities for this dummy device
        // For example, it has DC outputs, Dew outputs, Voltage Sensor, and USB Toggle
        SetCapability(POWER_HAS_DC_OUT | POWER_HAS_DEW_OUT | POWER_HAS_VOLTAGE_SENSOR | POWER_HAS_USB_TOGGLE);

        // Initialize Power properties: 4 DC ports, 2 Dew ports, 0 Variable, 0 AutoDew, 2 USB ports
        initProperties("Power Control", 4, 2, 0, 0, 2);

        return true;
    }

    virtual bool updateProperties() override
    {
        INDI::DefaultDevice::updateProperties();
        if (isConnected())
        {
            // Define Power properties when connected
            INDI::PowerInterface::updateProperties();
        }
        else
        {
            // Delete Power properties when disconnected
            INDI::PowerInterface::updateProperties();
        }
        return true;
    }

    virtual const char *getDefaultName() override
    {
        return "MyPowerBox";
    }

    // Implement the virtual methods from INDI::PowerInterface
    virtual bool SetPowerPort(size_t port, bool enabled) override
    {
        std::cout << "Setting Power Port " << port << " to " << (enabled ? "ON" : "OFF") << std::endl;
        // Here you would send a command to your hardware to control the power port
        // For example: sendCommandToHardware("P" + std::to_string(port + 1) + ":" + (enabled ? "1" : "0"));
        return true; // Assume success
    }

    virtual bool SetDewPort(size_t port, bool enabled, double dutyCycle) override
    {
        std::cout << "Setting Dew Port " << port << " to " << (enabled ? "ON" : "OFF") << " with duty cycle " << dutyCycle << "%" << std::endl;
        // Here you would send a command to your hardware to control the dew port
        return true; // Assume success
    }

    virtual bool SetVariablePort(size_t port, bool enabled, double voltage) override
    {
        std::cout << "Setting Variable Port " << port << " to " << (enabled ? "ON" : "OFF") << " with voltage " << voltage << "V" << std::endl;
        // Here you would send a command to your hardware
        return true; // Assume success
    }

    virtual bool SetLEDEnabled(bool enabled) override
    {
        std::cout << "Setting LEDs to " << (enabled ? "ON" : "OFF") << std::endl;
        // Here you would send a command to your hardware
        return true; // Assume success
    }

    virtual bool SetAutoDewEnabled(size_t port, bool enabled) override
    {
        std::cout << "Setting Auto Dew for Port " << port << " to " << (enabled ? "ON" : "OFF") << std::endl;
        // Here you would send a command to your hardware
        return true; // Assume success
    }

    virtual bool CyclePower() override
    {
        std::cout << "Cycling Power to all ports" << std::endl;
        // Here you would send a command to your hardware
        return true; // Assume success
    }

    virtual bool SetUSBPort(size_t port, bool enabled) override
    {
        std::cout << "Setting USB Port " << port << " to " << (enabled ? "ON" : "OFF") << std::endl;
        // Here you would send a command to your hardware
        return true; // Assume success
    }

    // Other INDI::DefaultDevice methods would also be implemented here
    // e.g., ISNewSwitch, ISNewNumber, ISNewText for processing client requests
};
