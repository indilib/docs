---
title: Output Interface
nav_order: 11
parent: Device Interfaces
---

# Implementing the Output Interface

This guide provides a comprehensive overview of implementing the Output Interface in INDI drivers. It covers the basic structure of an output driver, how to implement the required methods, and how to handle device-specific functionality for digital outputs.

## Introduction to the Output Interface

The INDI Output Interface (`INDI::OutputInterface`) provides a framework for implementing digital boolean output (On/Off) functionality in INDI devices. This is particularly useful for devices like web-enabled output controllers and General Purpose Input/Output (GPIO) systems, where controlling the state of physical outputs (e.g., relays, LEDs) is crucial.

**IMPORTANT**: The `initProperties()` method must be called before any other function to properly initialize the output properties.

## Prerequisites

Before implementing the Output Interface, you should have:

- Basic knowledge of C++ programming
- Understanding of the INDI protocol and architecture
- Familiarity with the device's communication protocol
- Development environment set up (compiler, build tools, etc.)
- INDI library installed

## Output Interface Structure

The Output Interface consists of several key components:

-   **Base Class**: `INDI::OutputInterface` is the base class for all output drivers.
-   **Output State**: Defines the possible states for an output (On/Off).

### Base Class

The `INDI::OutputInterface` base class provides functionality specific to devices that need to control digital outputs. It defines standard properties for output states, labels, and pulse durations.

### OutputState Enum

The `OutputState` enum defines the possible boolean states for an output.

```cpp
typedef enum
{
    Off,     /*!< Output is off. For Relays, it is open circuit. */
    On,      /*!< Output is on. For Relays, it is closed circuit. */
} OutputState;
```

-   `Off`: Indicates the output is in an 'off' state (e.g., open circuit for a relay).
-   `On`: Indicates the output is in an 'on' state (e.g., closed circuit for a relay).

### Key Methods

A driver implementing the `INDI::OutputInterface` must override and implement the following virtual methods to control output devices:

-   `virtual bool UpdateDigitalOutputs() = 0;`
    **This is a crucial method that your driver must implement.** It is responsible for updating the state of all digital outputs from the device or service.
    -   Returns `true` if the operation is successful, `false` otherwise.
    -   **Note**: `UpdateDigitalOutputs` should be called periodically (e.g., in the child's `TimerHit` or a custom timer function) or when an interrupt/trigger warrants updating the digital outputs. Only updated properties that had a change in status since the last call should be sent to clients to reduce unnecessary updates. Implementation can be polling or event-driven depending on hardware capabilities.

-   `virtual bool CommandOutput(uint32_t index, OutputState command) = 0;`
    **This is a crucial method that your driver must implement.** It is responsible for sending a command to a specific output.
    -   `index`: The zero-based index of the output to command.
    -   `command`: The desired `OutputState` (On or Off).
    -   Returns `true` if the operation is successful, `false` otherwise.

The following protected methods are provided by the `OutputInterface` for managing properties:

-   `explicit OutputInterface(DefaultDevice *defaultDevice);`
    Constructor for the `OutputInterface`.
    -   `defaultDevice`: A pointer to the `DefaultDevice` that owns this interface.

-   `~OutputInterface();`
    Destructor for the `OutputInterface`.

-   `void initProperties(const char *groupName, uint8_t outputs, const std::string &prefix = "Output");`
    Initializes the INDI properties related to the Output Interface. It is recommended to call this function within the driver's `initProperties()` method.
    -   `groupName`: Group or tab name to be used to define output properties (e.g., "Outputs").
    -   `outputs`: Number of outputs.
    -   `prefix`: Prefix used to label outputs (default: "Output").

-   `bool updateProperties();`
    Defines or deletes output properties based on the connection status of the default device.
    -   Returns `true` if all is OK, `false` otherwise.

-   `bool processSwitch(const char *dev, const char *name, ISState states[], char *names[], int n);`
    Processes incoming client requests for switch properties related to the output interface (e.g., toggling outputs).

-   `bool processText(const char *dev, const char *name, char *texts[], char *names[], int n);`
    Processes incoming client requests for text properties related to the output interface (e.g., output labels).

-   `bool processNumber(const char *dev, const char *name, double values[], char *names[], int n);`
    Processes incoming client requests for number properties related to the output interface (e.g., pulse duration).

-   `bool saveConfigItems(FILE *fp);`
    Saves output labels in the configuration file.
    -   `fp`: Pointer to the configuration file.
    -   Always returns `true`.

### Member Variables

The `INDI::OutputInterface` class includes the following important member variables:

-   `std::vector<INDI::PropertySwitch> DigitalOutputsSP;`
    A vector of `PropertySwitch` objects representing the digital outputs.

-   `INDI::PropertyText DigitalOutputLabelsTP;`
    A `PropertyText` object used to manage labels for digital outputs.

-   `std::vector<INDI::PropertyNumber> PulseDurationNP;`
    A vector of `PropertyNumber` objects for setting pulse durations for outputs.

-   `bool m_DigitalOutputLabelsConfig;`
    Indicates whether digital output labels were successfully loaded from the configuration file.

-   `DefaultDevice *m_defaultDevice;`
    A pointer to the `DefaultDevice` that owns this interface.

### Example Implementation

Here's a simplified example of how a driver might implement the `INDI::OutputInterface`. This example focuses on the core structure and omits complex hardware interaction details for clarity.

```cpp
#include "indibase.h"
#include "indioutputinterface.h"
#include <iostream>
#include <vector>

// Forward declaration of a dummy DefaultDevice for the example
class MyOutputDevice;

class MyOutputDevice : public INDI::DefaultDevice, public INDI::OutputInterface
{
public:
    MyOutputDevice() : INDI::DefaultDevice(), INDI::OutputInterface(this)
    {
        setDriverInterface(GENERAL_INTERFACE); // Or a more specific interface if applicable
    }

    virtual const char *getDefaultName() override
    {
        return "MyOutputDevice";
    }

    virtual bool initProperties() override
    {
        INDI::DefaultDevice::initProperties();

        // Initialize Output properties. "Outputs" for the group name, 4 outputs.
        // Using default prefix "Output".
        initProperties("Outputs", 4);

        return true;
    }

    virtual bool updateProperties() override
    {
        INDI::DefaultDevice::updateProperties();
        if (isConnected())
        {
            // Define Output properties when connected
            INDI::OutputInterface::updateProperties();
            // Start a timer to periodically update output data (if needed for status feedback)
            SetTimer(getCurrentPollingPeriod());
        }
        else
        {
            // Delete Output properties when disconnected
            INDI::OutputInterface::updateProperties();
        }
        return true;
    }

    // Implement the crucial UpdateDigitalOutputs method
    virtual bool UpdateDigitalOutputs() override
    {
        // In a real driver, you would read the actual state of the outputs from hardware
        // and update the DigitalOutputsSP vector accordingly.
        // For this example, we'll just log the current states.
        std::cout << "Updating Digital Outputs status:" << std::endl;
        for (size_t i = 0; i < DigitalOutputsSP.size(); ++i)
        {
            std::cout << "  Output #" << (i + 1) << ": "
                      << (DigitalOutputsSP[i].getState() == INDI::OutputInterface::On ? "On" : "Off")
                      << std::endl;
            // If the state changed, you would call IDSetSwitch(&DigitalOutputsSP[i], nullptr);
        }
        return true; // Indicate successful update
    }

    // Implement the crucial CommandOutput method
    virtual bool CommandOutput(uint32_t index, OutputState command) override
    {
        if (index >= DigitalOutputsSP.size())
        {
            IDLog("CommandOutput: Invalid output index %u", index);
            return false;
        }

        // In a real driver, you would send the command to the hardware
        // For this example, we'll just simulate the command and update the property.
        DigitalOutputsSP[index].setState(command);
        IDSetSwitch(&DigitalOutputsSP[index], nullptr); // Notify clients of the new state

        std::cout << "Commanded Output #" << (index + 1) << " to "
                  << (command == INDI::OutputInterface::On ? "On" : "Off") << std::endl;

        return true; // Indicate successful command
    }

    // Other INDI::DefaultDevice methods would also be implemented here
    // e.g., ISNewSwitch, ISNewNumber, ISNewText for processing client requests
    // You would typically forward relevant properties to OutputInterface::processSwitch/processText/processNumber
    virtual bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n) override
    {
        if (dev && !strcmp(dev, getDeviceName()))
        {
            if (INDI::OutputInterface::processSwitch(dev, name, states, names, n))
                return true;
        }
        return INDI::DefaultDevice::ISNewSwitch(dev, name, states, names, n);
    }

    virtual bool ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n) override
    {
        if (dev && !strcmp(dev, getDeviceName()))
        {
            if (INDI::OutputInterface::processText(dev, name, texts, names, n))
                return true;
        }
        return INDI::DefaultDevice::ISNewText(dev, name, texts, names, n);
    }

    virtual bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) override
    {
        if (dev && !strcmp(dev, getDeviceName()))
        {
            if (INDI::OutputInterface::processNumber(dev, name, values, names, n))
                return true;
        }
        return INDI::DefaultDevice::ISNewNumber(dev, name, values, names, n);
    }

    // TimerHit for periodic updates
    virtual void TimerHit() override
    {
        if (!isConnected())
        {
            SetTimer(getCurrentPollingPeriod());
            return;
        }

        // Update digital outputs (e.g., read their actual state from hardware)
        UpdateDigitalOutputs();

        SetTimer(getCurrentPollingPeriod());
    }
};

// This is typically how an INDI driver is instantiated
// static MyOutputDevice myOutputDevice;
