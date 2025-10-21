---
title: Input Interface
nav_order: 10
parent: Device Interfaces
---

# Implementing the Input Interface

This guide provides a comprehensive overview of implementing the Input Interface in INDI drivers. It covers the basic structure of an input driver, how to implement the required methods, and how to handle device-specific functionality for digital and analog inputs.

## Introduction to the Input Interface

The INDI Input Interface (`INDI::InputInterface`) provides a framework for implementing digital and analog input functionality in INDI devices. This is particularly useful for devices like web-enabled observatory controllers and General Purpose Input/Output (GPIO) systems, where monitoring the state of physical inputs is crucial. A typical observatory controller might support both `InputInterface` and `OutputInterface`.

**IMPORTANT**: The `initProperties()` method must be called before any other function to properly initialize the input properties.

## Prerequisites

Before implementing the Input Interface, you should have:

- Basic knowledge of C++ programming
- Understanding of the INDI protocol and architecture
- Familiarity with the device's communication protocol
- Development environment set up (compiler, build tools, etc.)
- INDI library installed

## Input Interface Structure

The Input Interface consists of several key components:

- **Base Class**: `INDI::InputInterface` is the base class for all input drivers.
- **Input Types**: Supports both digital and analog inputs.
- **Input State**: Defines the possible states for an input (On/Off).

### Base Class

The `INDI::InputInterface` base class provides functionality specific to devices that need to report digital and analog input states. It defines standard properties for input states and labels.

### InputState Enum

The `InputState` enum defines the possible boolean states for an input, regardless of whether the input is active low or high.

```cpp
typedef enum
{
    Off,     /*!< Input is off. */
    On,      /*!< Input is on. */
} InputState;
```

-   `Off`: Indicates the input is in an 'off' state (e.g., open circuit for a relay).
-   `On`: Indicates the input is in an 'on' state (e.g., closed circuit for a relay).

### Key Methods

A driver implementing the `INDI::InputInterface` must override and implement the following virtual methods to provide input data:

-   `virtual bool UpdateDigitalInputs() = 0;`
    **This is a crucial method that your driver must implement.** It is responsible for updating the state of all digital inputs from the device or service.
    -   Returns `true` if the operation is successful, `false` otherwise.

-   `virtual bool UpdateAnalogInputs() = 0;`
    **This is a crucial method that your driver must implement.** It is responsible for updating the state of all analog inputs from the device or service.
    -   Returns `true` if the operation is successful, `false` otherwise.

The following protected methods are provided by the `InputInterface` for managing properties:

-   `explicit InputInterface(DefaultDevice *defaultDevice);`
    Constructor for the `InputInterface`.
    -   `defaultDevice`: A pointer to the `DefaultDevice` that owns this interface.

-   `~InputInterface();`
    Destructor for the `InputInterface`.

-   `void initProperties(const char *groupName, uint8_t digital, uint8_t analog, const std::string &digitalPrefix = "Digital", const std::string &analogPrefix = "Analog");`
    Initializes the INDI properties related to the Input Interface. It is recommended to call this function within the driver's `initProperties()` method.
    -   `groupName`: Group or tab name to be used to define input properties (e.g., "Inputs").
    -   `digital`: Number of digital inputs.
    -   `analog`: Number of analog inputs.
    -   `digitalPrefix`: Prefix used to label digital inputs (default: "Digital").
    -   `analogPrefix`: Prefix used to label analog inputs (default: "Analog").

-   `bool updateProperties();`
    Defines or deletes input properties based on the connection status of the default device.
    -   Returns `true` if all is OK, `false` otherwise.

-   `bool processText(const char *dev, const char *name, char *texts[], char *names[], int n);`
    Processes incoming client requests for text properties related to the input interface (e.g., input labels).

-   `bool saveConfigItems(FILE *fp);`
    Saves input labels in the configuration file.
    -   `fp`: Pointer to the configuration file.
    -   Always returns `true`.

### Member Variables

The `INDI::InputInterface` class includes the following important member variables:

-   `std::vector<INDI::PropertySwitch> DigitalInputsSP;`
    A vector of `PropertySwitch` objects representing the digital inputs.

-   `std::vector<INDI::PropertyNumber> AnalogInputsNP;`
    A vector of `PropertyNumber` objects representing the analog inputs.

-   `INDI::PropertyText DigitalInputLabelsTP;`
    A `PropertyText` object used to manage labels for digital inputs.

-   `INDI::PropertyText AnalogInputLabelsTP;`
    A `PropertyText` object used to manage labels for analog inputs.

-   `bool m_DigitalInputLabelsConfig;`
    Indicates whether digital input labels were successfully loaded from the configuration file.

-   `bool m_AnalogInputLabelsConfig;`
    Indicates whether analog input labels were successfully loaded from the configuration file.

-   `DefaultDevice *m_defaultDevice;`
    A pointer to the `DefaultDevice` that owns this interface.

### Example Implementation

Here's a simplified example of how a driver might implement the `INDI::InputInterface`. This example focuses on the core structure and omits complex hardware interaction details for clarity.

```cpp
#include "indibase.h"
#include "indiinputinterface.h"
#include <iostream>
#include <vector>

// Forward declaration of a dummy DefaultDevice for the example
class MyInputDevice;

class MyInputDevice : public INDI::DefaultDevice, public INDI::InputInterface
{
public:
    MyInputDevice() : INDI::DefaultDevice(), INDI::InputInterface(this)
    {
        setDriverInterface(GENERAL_INTERFACE); // Or a more specific interface if applicable
    }

    virtual const char *getDefaultName() override
    {
        return "MyInputDevice";
    }

    virtual bool initProperties() override
    {
        INDI::DefaultDevice::initProperties();

        // Initialize Input properties. "Inputs" for the group name, 2 digital, 1 analog.
        // Using default prefixes "Digital" and "Analog".
        initProperties("Inputs", 2, 1);

        return true;
    }

    virtual bool updateProperties() override
    {
        INDI::DefaultDevice::updateProperties();
        if (isConnected())
        {
            // Define Input properties when connected
            INDI::InputInterface::updateProperties();
            // Start a timer to periodically update input data
            SetTimer(getCurrentPollingPeriod());
        }
        else
        {
            // Delete Input properties when disconnected
            INDI::InputInterface::updateProperties();
        }
        return true;
    }

    // Implement the crucial UpdateDigitalInputs method
    virtual bool UpdateDigitalInputs() override
    {
        // Simulate reading digital input states from hardware
        // For example, read from GPIO pins
        // In a real driver, you would read actual hardware states
        static bool digitalState1 = false;
        static bool digitalState2 = true;

        // Update the PropertySwitch values
        if (DigitalInputsSP.size() > 0)
        {
            DigitalInputsSP[0].setState(digitalState1 ? INDI::InputInterface::On : INDI::InputInterface::Off);
            IDSetSwitch(&DigitalInputsSP[0], nullptr); // Notify clients
        }
        if (DigitalInputsSP.size() > 1)
        {
            DigitalInputsSP[1].setState(digitalState2 ? INDI::InputInterface::On : INDI::InputInterface::Off);
            IDSetSwitch(&DigitalInputsSP[1], nullptr); // Notify clients
        }

        std::cout << "Digital Inputs updated: Digital #1=" << (digitalState1 ? "On" : "Off")
                  << ", Digital #2=" << (digitalState2 ? "On" : "Off") << std::endl;

        // Toggle states for next update (for simulation)
        digitalState1 = !digitalState1;
        digitalState2 = !digitalState2;

        return true; // Indicate successful update
    }

    // Implement the crucial UpdateAnalogInputs method
    virtual bool UpdateAnalogInputs() override
    {
        // Simulate reading analog input states from hardware
        // For example, read from an ADC
        // In a real driver, you would read actual hardware values
        static double analogValue1 = 0.5; // Value between 0.0 and 1.0

        // Update the PropertyNumber values
        if (AnalogInputsNP.size() > 0)
        {
            AnalogInputsNP[0].setValue(analogValue1);
            IDSetNumber(&AnalogInputsNP[0], nullptr); // Notify clients
        }

        std::cout << "Analog Inputs updated: Analog #1=" << analogValue1 << std::endl;

        // Change value for next update (for simulation)
        analogValue1 += 0.1;
        if (analogValue1 > 1.0) analogValue1 = 0.0;

        return true; // Indicate successful update
    }

    // Other INDI::DefaultDevice methods would also be implemented here
    // e.g., ISNewSwitch, ISNewNumber, ISNewText for processing client requests
    // You would typically forward relevant properties to InputInterface::processText
    virtual bool ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n) override
    {
        if (dev && !strcmp(dev, getDeviceName()))
        {
            if (INDI::InputInterface::processText(dev, name, texts, names, n))
                return true;
        }
        return INDI::DefaultDevice::ISNewText(dev, name, texts, names, n);
    }

    // TimerHit for periodic updates
    virtual void TimerHit() override
    {
        if (!isConnected())
        {
            SetTimer(getCurrentPollingPeriod());
            return;
        }

        // Update digital and analog inputs
        UpdateDigitalInputs();
        UpdateAnalogInputs();

        SetTimer(getCurrentPollingPeriod());
    }
};

// This is typically how an INDI driver is instantiated
// static MyInputDevice myInputDevice;
