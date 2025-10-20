---
title: Lightbox Interface
nav_order: 11
parent: Device Interfaces
---

# Implementing the Lightbox Interface

This guide provides a comprehensive overview of implementing the lightbox interface in INDI drivers. It covers the basic structure of a lightbox driver, how to implement the required methods, and how to handle device-specific functionality.

## Introduction to the Lightbox Interface

The INDI Lightbox Interface (`INDI::LightBoxInterface`) is designed for devices that provide controllable illumination, such as flat field light boxes or simple on/off switches for lights. It provides a standardized way for INDI clients to control the light's state (on/off) and, if supported, its intensity (dimming).

## Prerequisites

Before implementing the lightbox interface, you should have:

- Basic knowledge of C++ programming
- Understanding of the INDI protocol and architecture
- Familiarity with the device's communication protocol
- Development environment set up (compiler, build tools, etc.)
- INDI library installed

## Lightbox Interface Structure

The lightbox interface consists of several key components:

-   **Base Class**: `INDI::LightBoxInterface` is the base class for all lightbox drivers.
-   **Capabilities**: Defines whether the lightbox supports dimming.
-   **Standard Properties**: Properties for controlling the light's state and intensity.
-   **Virtual Methods**: A set of virtual methods that must be implemented by the driver.
-   **Helper Methods**: Methods for common lightbox operations.

### Base Class

The `INDI::LightBoxInterface` base class inherits from `INDI::DefaultDevice` and provides additional functionality specific to lightbox devices. It defines standard properties for light control and intensity.

### Lightbox Capabilities

The `LightBoxCapability` enum defines the features supported by a lightbox device. A driver sets these capabilities to inform clients about the device's functionalities.

| Capability Flag | Description             |
| :-------------- | :---------------------- |
| `CAN_DIM`       | Device supports dimming. |

### Key Methods

A driver implementing the `INDI::LightBoxInterface` must override and implement the following virtual methods to control the lightbox:

-   `void initProperties(const char *group, uint32_t capabilities)`:
    Initializes the INDI properties related to the lightbox interface. This should be called within the driver's `initProperties()` method.
    -   `group`: Group or tab name to be used to define lightbox properties.
    -   `capabilities`: Lightbox capabilities (e.g., `CAN_DIM`).

-   `void ISGetProperties(const char *deviceName)`:
    This method should be called in your driver's `ISGetProperties` function to get lightbox properties.

-   `bool processSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)`:
    **Important**: This method must be called in your driver's `ISNewSwitch()` function. It processes incoming client requests for light on/off control.

-   `bool processNumber(const char *dev, const char *name, double values[], char *names[], int n)`:
    **Important**: This method must be called in your driver's `ISNewNumber()` function. It processes incoming client requests for light intensity control (if `CAN_DIM` is supported).

-   `bool processText(const char *dev, const char *name, char *texts[], char *names[], int n)`:
    **Important**: This method must be called in your driver's `ISNewText()` function. It processes incoming client requests for text properties related to the lightbox.

-   `bool updateProperties()`:
    Defines or deletes lightbox properties based on the connection status of the base device.

-   `bool saveConfigItems(FILE *fp)`:
    Saves lightbox properties to the configuration file.

-   `virtual bool SetLightBoxBrightness(uint16_t value)`:
    **Virtual method.** Must be implemented in the child class if dimming is supported (`CAN_DIM`). Sets the light level.
    -   `value`: Level of the lightbox (e.g., 0-4096).
    -   Returns `true` if successful, `false` otherwise.

-   `virtual bool EnableLightBox(bool enable)`:
    **Virtual method.** Must be implemented in the child class. Turns the lightbox on or off.
    -   `enable`: If `true`, turn on the light; otherwise, turn off.
    -   Returns `true` if successful, `false` otherwise.

### Example Implementation

Here's a simplified example of how a driver might implement the `INDI::LightBoxInterface`. This example focuses on the core structure and omits complex serial communication details for clarity.

```cpp
#include "indibase.h"
#include "indilightboxinterface.h"
#include <iostream>

// Forward declaration of a dummy DefaultDevice for the example
class MyLightBox;

class MyLightBox : public INDI::DefaultDevice, public INDI::LightBoxInterface
{
public:
    MyLightBox() : INDI::DefaultDevice(), INDI::LightBoxInterface(this)
    {
        setDriverInterface(LIGHTBOX_INTERFACE);
    }

    virtual bool initProperties() override
    {
        INDI::DefaultDevice::initProperties();

        // Initialize Lightbox properties. "Light Control" for the group name.
        // Declare that this lightbox supports dimming.
        initProperties("Light Control", CAN_DIM);

        // Set the min/max for the light intensity property.
        // Assuming a brightness range of 0-255 for this example.
        LightIntensityNP[0].setMin(0);
        LightIntensityNP[0].setMax(255);
        LightIntensityNP[0].setStep(1);

        return true;
    }

    virtual bool updateProperties() override
    {
        INDI::DefaultDevice::updateProperties();
        if (isConnected())
        {
            // Define Lightbox properties when connected
            INDI::LightBoxInterface::updateProperties();
        }
        else
        {
            // Delete Lightbox properties when disconnected
            INDI::LightBoxInterface::updateProperties();
        }
        return true;
    }

    virtual const char *getDefaultName() override
    {
        return "MyLightBox";
    }

    // Implement the virtual EnableLightBox method
    virtual bool EnableLightBox(bool enable) override
    {
        std::cout << "Setting Lightbox to " << (enable ? "ON" : "OFF") << std::endl;
        // In a real driver, you would send a command to your hardware to turn the light on/off
        return true; // Assume success
    }

    // Implement the virtual SetLightBoxBrightness method (since CAN_DIM is set)
    virtual bool SetLightBoxBrightness(uint16_t value) override
    {
        std::cout << "Setting Lightbox brightness to: " << value << std::endl;
        // In a real driver, you would send a command to your hardware to set the brightness
        return true; // Assume success
    }

    // Override ISNewSwitch to process light on/off changes
    virtual bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n) override
    {
        if (dev && !strcmp(dev, getDeviceName()))
        {
            // Important: Call LightBoxInterface's processSwitch for light control
            if (INDI::LightBoxInterface::processSwitch(dev, name, states, names, n))
                return true;
        }
        return INDI::DefaultDevice::ISNewSwitch(dev, name, states, names, n);
    }

    // Override ISNewNumber to process light intensity changes
    virtual bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) override
    {
        if (dev && !strcmp(dev, getDeviceName()))
        {
            // Important: Call LightBoxInterface's processNumber for intensity control
            if (INDI::LightBoxInterface::processNumber(dev, name, values, names, n))
                return true;
        }
        return INDI::DefaultDevice::ISNewNumber(dev, name, values, names, n);
    }

    // Other INDI::DefaultDevice methods would also be implemented here
};

// This is typically how an INDI driver is instantiated
// static MyLightBox myLightBox;
