---
title: Dust Cap Interface
nav_order: 11
parent: Device Interfaces
---

# Implementing the Dust Cap Interface

This guide provides a comprehensive overview of implementing the Dust Cap Interface in INDI drivers. It covers the basic structure of a dust cap driver, how to implement the required methods, and how to handle device-specific functionality for controlling a dust cover.

## Introduction to the Dust Cap Interface

The INDI Dust Cap Interface (`INDI::DustCapInterface`) provides a framework for implementing remotely controlled dust covers or caps for astronomical instruments. This interface allows drivers to expose functionality for parking (closing) and unparking (opening) the dust cap, and optionally aborting its motion.

**IMPORTANT**:
- `initProperties()` must be called before any other function to initialize the Dust Cap properties.
- `updateProperties()` must be called in your driver's `updateProperties()` function.
- `processSwitch()` must be called in your driver's `ISNewSwitch()` function.

## Prerequisites

Before implementing the Dust Cap Interface, you should have:

- Basic knowledge of C++ programming
- Understanding of the INDI protocol and architecture
- Familiarity with the device's communication protocol
- Development environment set up (compiler, build tools, etc.)
- INDI library installed

## Dust Cap Interface Structure

The Dust Cap Interface consists of several key components:

- **Base Class**: `INDI::DustCapInterface` is the base class for all dust cap drivers.
- **Actions**: Defines standard actions like Park and Unpark.
- **Capabilities**: Allows drivers to declare additional supported functionalities.

### Base Class

The `INDI::DustCapInterface` base class provides functionality specific to devices that control a dust cap. It defines standard properties for controlling the cap's state and reporting its status.

### Enums

The interface defines enums for standard actions and capabilities:

#### Cap Actions

```cpp
enum
{
    CAP_PARK,
    CAP_UNPARK
};
```
- `CAP_PARK`: Represents the action to close the dust cap.
- `CAP_UNPARK`: Represents the action to open the dust cap.

#### DustCapCapability

This enum defines optional capabilities that a dust cap device might support. These capabilities are passed during `initProperties`.

```cpp
enum
{
    CAN_ABORT           = 1 << 0,   /** Can the dust cap abort motion? */
    CAN_SET_POSITION    = 1 << 1,   /** Can the dust go to a specific angular position? UNUSED */
    CAN_SET_LIMITS      = 1 << 2,   /** Can the dust set the minimum and maximum ranges for Park and Unpark? UNUSED */
} DustCapCapability;
```
- `CAN_ABORT`: Indicates if the dust cap mechanism can abort its current motion.
- `CAN_SET_POSITION`: (UNUSED) Indicates if the dust cap can be moved to a specific angular position.
- `CAN_SET_LIMITS`: (UNUSED) Indicates if the dust cap can set minimum and maximum ranges for its Park and Unpark positions.

### Key Methods

A driver implementing the `INDI::DustCapInterface` must override and implement the following virtual methods to control the dust cap:

-   `virtual IPState ParkCap();`
    **This is a crucial method that your driver must implement.** It is responsible for commanding the dust cap to the "parked" (closed) position.
    -   Returns `IPS_OK` if the command completes immediately, `IPS_BUSY` if the command is in progress, or `IPS_ALERT` if an error occurs.

-   `virtual IPState UnParkCap();`
    **This is a crucial method that your driver must implement.** It is responsible for commanding the dust cap to the "unparked" (open) position.
    -   Returns `IPS_OK` if the command completes immediately, `IPS_BUSY` if the command is in progress, or `IPS_ALERT` if an error occurs.

-   `virtual IPState AbortCap();`
    **This is a crucial method that your driver must implement if `CAN_ABORT` capability is declared.** It is responsible for stopping any ongoing motion of the dust cap.
    -   Returns `IPS_OK` if the command completes immediately, `IPS_BUSY` if the command is in progress, or `IPS_ALERT` if an error occurs.

The following protected methods are provided by the `DustCapInterface` for managing properties:

-   `DustCapInterface(DefaultDevice *device);`
    Constructor for the `DustCapInterface`.
    -   `device`: A pointer to the `DefaultDevice` that owns this interface.

-   `virtual ~DustCapInterface() = default;`
    Destructor for the `DustCapInterface`.

-   `void initProperties(const char *group, uint32_t capabilities = 0);`
    Initializes the INDI properties related to the Dust Cap Interface. It is recommended to call this function within the driver's `initProperties()` method.
    -   `group`: Group or tab name to be used to define dust cap properties (e.g., "Dust Cap Control").
    -   `capabilities`: A bitmask of `DustCapCapability` flags indicating additional features supported by the dust cap (default: 0, no extra capabilities).

-   `bool updateProperties();`
    Defines or deletes dust cap properties based on the connection status of the default device.
    -   Returns `true` if all is OK, `false` otherwise.

-   `bool processSwitch(const char *dev, const char *name, ISState *states, char *names[], int n);`
    Processes incoming client requests for dust cap switch properties (e.g., Park/Unpark commands, Abort). Drivers should call this if a property name matches a dust cap property.

### Member Variables

The `INDI::DustCapInterface` class includes the following important member variables:

-   `INDI::PropertySwitch ParkCapSP;`
    A `PropertySwitch` object representing the Park/Unpark commands for the dust cap. It typically has two switches: `CAP_PARK` and `CAP_UNPARK`.

-   `INDI::PropertySwitch AbortCapSP;`
    A `PropertySwitch` object representing the Abort command for the dust cap. This property is defined only if the `CAN_ABORT` capability is set.

-   `DefaultDevice *m_DefaultDevice;`
    A pointer to the `DefaultDevice` that owns this interface.

-   `uint32_t m_Capabilities;`
    A bitmask storing the capabilities of the dust cap, as passed during `initProperties()`.

### Example Implementation

Here's a simplified example of how a driver might implement the `INDI::DustCapInterface`, drawing inspiration from the `alto.cpp` driver. This example focuses on the core structure and omits complex hardware interaction details for clarity.

```cpp
#include "indibase.h"
#include "indidustcapinterface.h"
#include <iostream>
#include <thread> // For simulating motion delay
#include <chrono> // For simulating motion delay

// Forward declaration of a dummy DefaultDevice for the example
class MyDustCapDevice;

class MyDustCapDevice : public INDI::DefaultDevice, public INDI::DustCapInterface
{
public:
    MyDustCapDevice() : INDI::DefaultDevice(), INDI::DustCapInterface(this)
    {
        setDriverInterface(DUSTCAP_INTERFACE);
    }

    virtual const char *getDefaultName() override
    {
        return "MyDustCap";
    }

    virtual bool initProperties() override
    {
        INDI::DefaultDevice::initProperties();

        // Initialize Dust Cap properties. "Dust Cap Control" for the group name.
        // Declare CAN_ABORT capability.
        DI::initProperties("Dust Cap Control", DI::CAN_ABORT);

        return true;
    }

    virtual bool updateProperties() override
    {
        INDI::DefaultDevice::updateProperties();
        if (isConnected())
        {
            // Define Dust Cap properties when connected
            DI::updateProperties();
            // Start a timer for periodic status checks if needed
            SetTimer(getCurrentPollingPeriod());
        }
        else
        {
            // Delete Dust Cap properties when disconnected
            DI::updateProperties();
        }
        return true;
    }

    // Implement the crucial ParkCap method
    virtual IPState ParkCap() override
    {
        LOG_INFO("Dust Cap: Parking (closing)...");
        // Simulate hardware action that takes time
        // In a real driver, this would send a command to hardware
        // and the TimerHit would monitor its completion.
        std::thread([this]() {
            std::this_thread::sleep_for(std::chrono::seconds(5)); // Simulate 5 seconds to close
            m_isParked = true;
            LOG_INFO("Dust Cap: Parked (closed).");
            // Update property state after completion
            ParkCapSP[DI::CAP_PARK].setState(ISS_ON);
            ParkCapSP[DI::CAP_UNPARK].setState(ISS_OFF);
            ParkCapSP.setState(IPS_OK);
            IDSetSwitch(&ParkCapSP, nullptr);
        }).detach();

        ParkCapSP.setState(IPS_BUSY); // Indicate motion in progress
        IDSetSwitch(&ParkCapSP, nullptr);
        return IPS_BUSY;
    }

    // Implement the crucial UnParkCap method
    virtual IPState UnParkCap() override
    {
        LOG_INFO("Dust Cap: Unparking (opening)...");
        // Simulate hardware action that takes time
        std::thread([this]() {
            std::this_thread::sleep_for(std::chrono::seconds(5)); // Simulate 5 seconds to open
            m_isParked = false;
            LOG_INFO("Dust Cap: Unparked (opened).");
            // Update property state after completion
            ParkCapSP[DI::CAP_PARK].setState(ISS_OFF);
            ParkCapSP[DI::CAP_UNPARK].setState(ISS_ON);
            ParkCapSP.setState(IPS_OK);
            IDSetSwitch(&ParkCapSP, nullptr);
        }).detach();

        ParkCapSP.setState(IPS_BUSY); // Indicate motion in progress
        IDSetSwitch(&ParkCapSP, nullptr);
        return IPS_BUSY;
    }

    // Implement the crucial AbortCap method (since CAN_ABORT was declared)
    virtual IPState AbortCap() override
    {
        LOG_INFO("Dust Cap: Aborting motion.");
        // In a real driver, this would send an abort command to hardware.
        // For this simulation, we'll just instantly "stop" it.
        // A real abort might also need to stop the simulated thread.
        ParkCapSP.setState(IPS_ALERT); // Indicate motion was interrupted
        IDSetSwitch(&ParkCapSP, nullptr);
        return IPS_OK;
    }

    // You would typically forward DUSTCAP_* properties to DI::processSwitch
    virtual bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n) override
    {
        if (dev && !strcmp(dev, getDeviceName()))
        {
            if (DI::processSwitch(dev, name, states, names, n))
                return true;
        }
        return INDI::DefaultDevice::ISNewSwitch(dev, name, states, names, n);
    }

    // TimerHit for periodic updates or monitoring motion
    virtual void TimerHit() override
    {
        if (!isConnected())
        {
            SetTimer(getCurrentPollingPeriod());
            return;
        }

        // In a real driver, you might poll hardware for current status
        // and update ParkCapSP state if it's IPS_BUSY and motion has completed.
        // For this example, the thread updates the state directly.

        SetTimer(getCurrentPollingPeriod());
    }

private:
    bool m_isParked {true}; // Simulate current state of the dust cap
};

// This is typically how an INDI driver is instantiated
// static MyDustCapDevice myDustCapDevice;
