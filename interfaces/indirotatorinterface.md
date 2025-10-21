---
title: Rotator Interface
nav_order: 12
parent: Device Interfaces
---

# Implementing the Rotator Interface

This guide provides a comprehensive overview of implementing the Rotator Interface in INDI drivers. It covers the basic structure of a rotator driver, how to implement the required methods, and how to handle device-specific functionality.

## Introduction to the Rotator Interface

The INDI Rotator Interface (`INDI::RotatorInterface`) provides a framework for implementing rotator functionality. A rotator can be an independent device or an embedded component within another device (e.g., a rotating focuser). Only absolute position rotators are supported. The angle is typically ranged from 0 to 360 degrees, increasing clockwise when looking at the back of the camera.

**IMPORTANT**: `initProperties()` must be called before any other function to properly initialize the rotator properties.
**IMPORTANT**: `processNumber()` must be called in your driver's `ISNewNumber()` function. Similarly, `processSwitch()` must be called in `ISNewSwitch()`.

## Prerequisites

Before implementing the Rotator Interface, you should have:

- Basic knowledge of C++ programming
- Understanding of the INDI protocol and architecture
- Familiarity with the device's communication protocol
- Development environment set up (compiler, build tools, etc.)
- INDI library installed

## Rotator Interface Structure

The Rotator Interface consists of several key components:

-   **Base Class**: `INDI::RotatorInterface` is the base class for all rotator drivers.
-   **RotatorCapability Enum**: Defines the various capabilities a rotator might possess.

### Base Class

The `INDI::RotatorInterface` base class provides functionality specific to devices that control rotation. It defines standard properties for controlling rotator movement, syncing, homing, and managing backlash.

### RotatorCapability Enum

The `RotatorCapability` enum defines a bitmask of capabilities that a rotator can support.

```cpp
enum
{
    ROTATOR_CAN_ABORT          = 1 << 0, /*!< Can the Rotator abort motion once started? */
    ROTATOR_CAN_HOME           = 1 << 1, /*!< Can the Rotator go to home position? */
    ROTATOR_CAN_SYNC           = 1 << 2, /*!< Can the Rotator sync to specific tick? */
    ROTATOR_CAN_REVERSE        = 1 << 3, /*!< Can the Rotator reverse direction? */
    ROTATOR_HAS_BACKLASH       = 1 << 4  /*!< Can the Rotatorer compensate for backlash? */
} RotatorCapability;
```

-   `ROTATOR_CAN_ABORT`: Indicates if the rotator can abort its current motion.
-   `ROTATOR_CAN_HOME`: Indicates if the rotator can move to a predefined home position.
-   `ROTATOR_CAN_SYNC`: Indicates if the rotator can be synchronized to a new angle without physical movement.
-   `ROTATOR_CAN_REVERSE`: Indicates if the rotator's direction of rotation can be reversed.
-   `ROTATOR_HAS_BACKLASH`: Indicates if the rotator supports backlash compensation.

### Key Methods

A driver implementing the `INDI::RotatorInterface` must override and implement the following virtual methods to provide rotator functionality:

-   `uint32_t GetCapability() const;`
    Returns the bitmask of capabilities supported by the rotator.

-   `void SetCapability(uint32_t cap);`
    Sets the rotator's capabilities. All capabilities must be initialized using this method.

-   `bool CanAbort();`, `bool CanHome();`, `bool CanSync();`, `bool CanReverse();`, `bool HasBacklash();`
    Convenience methods to check individual rotator capabilities.

The following protected methods are provided by the `RotatorInterface` for managing properties and controlling the rotator:

-   `explicit RotatorInterface(DefaultDevice *defaultDevice);`
    Constructor for the `RotatorInterface`.
    -   `defaultDevice`: A pointer to the `DefaultDevice` that owns this interface.

-   `void initProperties(const char *groupName);`
    Initializes the INDI properties related to the Rotator Interface. It is recommended to call this function within the driver's `initProperties()` method.
    -   `groupName`: Group or tab name to be used to define rotator properties (e.g., "Rotator").

-   `bool updateProperties();`
    Defines or deletes rotator properties based on the connection status of the default device.
    -   Returns `true` if successful, `false` otherwise.

-   `bool processNumber(const char *dev, const char *name, double values[], char *names[], int n);`
    Processes incoming client requests for rotator number properties (e.g., target angle, backlash steps). This should be called in your driver's `ISNewNumber()` function.

-   `bool processSwitch(const char *dev, const char *name, ISState *states, char *names[], int n);`
    Processes incoming client requests for rotator switch properties (e.g., abort, home, reverse, backlash enable). This should be called in your driver's `ISNewSwitch()` function.

-   `virtual IPState MoveRotator(double angle) = 0;`
    **This is a crucial method that your driver must implement.** It commands the rotator to move to a specific target angle.
    -   `angle`: Target angle in degrees (0-360).
    -   Returns `IPS_OK` if motion is completed, `IPS_BUSY` if motion is in progress, `IPS_ALERT` on error.

-   `virtual bool SyncRotator(double angle);`
    Sets the current angle of the rotator as the supplied angle *without* physical movement.
    -   `angle`: Desired new angle.
    -   Returns `true` if successful, `false` otherwise. (Default implementation returns `false`).

-   `virtual IPState HomeRotator();`
    Commands the rotator to move to its home position.
    -   Returns `IPS_OK` if motion is completed, `IPS_BUSY` if motion is in progress, `IPS_ALERT` on error. (Default implementation returns `IPS_ALERT`).

-   `virtual bool ReverseRotator(bool enabled);`
    Reverses the direction of the rotator. Clockwise (CW) is usually the normal direction, and counter-clockwise (CCW) is the reversed direction.
    -   `enabled`: If `true`, reverse direction. If `false`, revert to normal direction.
    -   Returns `true` if successful, `false` otherwise. (Default implementation returns `false`).

-   `virtual bool AbortRotator();`
    Aborts any ongoing rotator motion.
    -   Returns `true` if successful, `false` otherwise. (Default implementation returns `false`).

-   `virtual bool SetRotatorBacklash(int32_t steps);`
    Sets the rotator backlash compensation value.
    -   `steps`: Value in absolute steps to compensate.
    -   Returns `true` if successful, `false` otherwise. (Default implementation returns `false`).

-   `virtual bool SetRotatorBacklashEnabled(bool enabled);`
    Enables or disables the rotator backlash compensation.
    -   `enabled`: Flag to enable or disable backlash compensation.
    -   Returns `true` if successful, `false` otherwise. (Default implementation returns `false`).

-   `bool saveConfigItems(FILE * fp);`
    Saves rotator properties defined in the interface to the configuration file.
    -   `fp`: Pointer to the configuration file.
    -   Always returns `true`.

### Member Variables

The `INDI::RotatorInterface` class includes the following important member variables:

-   `INDI::PropertyNumber GotoRotatorNP;`
    A `PropertyNumber` for setting the target angle for rotator movement.

-   `INDI::PropertyNumber SyncRotatorNP;`
    A `PropertyNumber` for synchronizing the rotator to a new angle.

-   `INDI::PropertySwitch AbortRotatorSP;`
    A `PropertySwitch` to abort rotator motion.

-   `INDI::PropertySwitch HomeRotatorSP;`
    A `PropertySwitch` to command the rotator to its home position.

-   `INDI::PropertySwitch ReverseRotatorSP;`
    A `PropertySwitch` to reverse the rotator's direction.

-   `INDI::PropertySwitch RotatorBacklashSP;`
    A `PropertySwitch` to enable or disable backlash compensation.

-   `INDI::PropertyNumber RotatorBacklashNP;`
    A `PropertyNumber` for setting the backlash compensation steps.

-   `INDI::PropertyNumber RotatorLimitsNP;`
    A `PropertyNumber` for defining rotator limits.

-   `double m_RotatorOffset;`
    An internal offset for rotator position.

-   `uint32_t rotatorCapability;`
    Stores the bitmask of the rotator's capabilities.

-   `DefaultDevice *m_defaultDevice;`
    A pointer to the `DefaultDevice` that owns this interface.

### Using the INDI::Rotator Base Class

For drivers that are primarily rotators, it is often more convenient to inherit from `INDI::Rotator` (defined in `indirotator.h`) instead of directly from `INDI::DefaultDevice` and `INDI::RotatorInterface`. The `INDI::Rotator` class extends `INDI::RotatorInterface` and provides additional functionalities and boilerplate code, simplifying driver development:

-   **Presets**: It includes `PresetNP` (for defining preset angles) and `PresetGotoSP` (for initiating moves to presets).
-   **Connection Management**: It handles serial and TCP connection plugins, so you don't have to implement them yourself. This includes properties for setting connection modes and managing the underlying communication.
-   **Handshake**: Provides a virtual `Handshake()` method to verify communication with the device.

By inheriting from `INDI::Rotator`, you can focus more on the device-specific commands and less on the generic INDI property management and connection setup.

### Example Implementation

Here's a simplified example of how a driver might implement the `INDI::RotatorInterface`. This example focuses on the core structure and omits complex hardware interaction details for clarity.

```cpp
#include "indibase.h"
#include "indirotatorinterface.h"
#include <iostream>
#include <thread>
#include <chrono>

// Forward declaration of a dummy DefaultDevice for the example
class MyRotatorDevice;

class MyRotatorDevice : public INDI::DefaultDevice, public INDI::RotatorInterface
{
public:
    MyRotatorDevice() : INDI::DefaultDevice(), INDI::RotatorInterface(this)
    {
        setDriverInterface(ROTATOR_INTERFACE);
        // Set capabilities: Can Abort, Can Home, Can Sync, Has Backlash
        SetCapability(RI::ROTATOR_CAN_ABORT | RI::ROTATOR_CAN_HOME | RI::ROTATOR_CAN_SYNC | RI::ROTATOR_HAS_BACKLASH);
    }

    virtual const char *getDefaultName() override
    {
        return "MyRotatorDevice";
    }

    virtual bool initProperties() override
    {
        INDI::DefaultDevice::initProperties();

        // Initialize Rotator properties. "Rotator" for the group name.
        initProperties("Rotator");

        // Set default values for properties
        GotoRotatorNP[0].setValue(0); // Initial target angle
        SyncRotatorNP[0].setValue(0); // Initial sync angle
        RotatorBacklashNP[0].setValue(0); // Initial backlash steps

        return true;
    }

    virtual bool updateProperties() override
    {
        INDI::DefaultDevice::updateProperties();
        if (isConnected())
        {
            // Define Rotator properties when connected
            INDI::RotatorInterface::updateProperties();
        }
        else
        {
            // Delete Rotator properties when disconnected
            INDI::RotatorInterface::updateProperties();
        }
        return true;
    }

    // Implement the crucial MoveRotator method
    virtual IPState MoveRotator(double angle) override
    {
        LOG_INFO("Moving rotator to %.2f degrees...", angle);
        // In a real driver, you would send the command to hardware and start motion.
        // For simulation, we'll set state to BUSY and let TimerHit handle completion.
        GotoRotatorNP[0].setValue(angle); // Set target value
        GotoRotatorNP.setState(IPS_BUSY);
        GotoRotatorNP.apply();

        // Store target angle for TimerHit to track
        m_targetAngle = angle;
        m_isMoving = true;

        return IPS_BUSY; // Indicate motion in progress
    }

    // Implement SyncRotator
    virtual bool SyncRotator(double angle) override
    {
        if (!CanSync())
        {
            LOG_ERROR("Rotator does not support synchronization.");
            return false;
        }
        LOG_INFO("Synchronizing rotator to %.2f degrees...", angle);
        // In a real driver, you would update the internal position without moving hardware
        GotoRotatorNP[0].setValue(angle);
        GotoRotatorNP.setState(IPS_OK); // Sync is usually instantaneous
        GotoRotatorNP.apply();
        return true;
    }

    // Implement HomeRotator
    virtual IPState HomeRotator() override
    {
        if (!CanHome())
        {
            LOG_ERROR("Rotator does not support homing.");
            return IPS_ALERT;
        }
        LOG_INFO("Homing rotator...");
        // In a real driver, you would send homing command to hardware.
        // For simulation, set state to BUSY and let TimerHit handle completion.
        HomeRotatorSP[0].setState(ISS_ON); // Set the individual switch element to ON
        HomeRotatorSP.setState(IPS_BUSY);
        HomeRotatorSP.apply();

        m_isHoming = true;

        return IPS_BUSY;
    }

    // Implement AbortRotator
    virtual bool AbortRotator() override
    {
        if (!CanAbort())
        {
            LOG_ERROR("Rotator does not support aborting motion.");
            return false;
        }
        LOG_INFO("Aborting rotator motion.");
        // In a real driver, you would send an abort command to hardware
        if (m_isMoving)
        {
            m_isMoving = false;
            GotoRotatorNP.setState(IPS_ALERT); // Motion aborted
            GotoRotatorNP.apply();
        }
        if (m_isHoming)
        {
            m_isHoming = false;
            HomeRotatorSP[0].setState(ISS_OFF); // Turn off homing switch element
            HomeRotatorSP.setState(IPS_ALERT); // Homing aborted
            HomeRotatorSP.apply();
        }
        return true;
    }

    // Implement SetRotatorBacklash
    virtual bool SetRotatorBacklash(int32_t steps) override
    {
        if (!HasBacklash())
        {
            LOG_ERROR("Rotator does not support backlash compensation.");
            return false;
        }
        LOG_INFO("Setting rotator backlash to %d steps.", steps);
        // In a real driver, you would store this value and apply it during motion
        RotatorBacklashNP[0].setValue(steps);
        RotatorBacklashNP.setState(IPS_OK);
        RotatorBacklashNP.apply();
        return true;
    }

    // Implement SetRotatorBacklashEnabled
    virtual bool SetRotatorBacklashEnabled(bool enabled) override
    {
        if (!HasBacklash())
        {
            LOG_ERROR("Rotator does not support backlash compensation.");
            return false;
        }
        LOG_INFO("Rotator backlash compensation %s.", enabled ? "enabled" : "disabled");
        // In a real driver, you would enable/disable backlash compensation
        RotatorBacklashSP[0].setState(enabled ? ISS_ON : ISS_OFF); // Set the individual switch element
        RotatorBacklashSP.setState(IPS_OK);
        RotatorBacklashSP.apply();
        return true;
    }

    // Other INDI::DefaultDevice methods would also be implemented here
    // You would typically forward relevant properties to RotatorInterface::processNumber/processSwitch
    virtual bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) override
    {
        if (dev && !strcmp(dev, getDeviceName()))
        {
            if (INDI::RotatorInterface::processNumber(dev, name, values, names, n))
                return true;
        }
        return INDI::DefaultDevice::ISNewNumber(dev, name, values, names, n);
    }

    virtual bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n) override
    {
        if (dev && !strcmp(dev, getDeviceName()))
        {
            if (INDI::RotatorInterface::processSwitch(dev, name, states, names, n))
                return true;
        }
        return INDI::DefaultDevice::ISNewSwitch(dev, name, states, names, n);
    }

    // TimerHit for periodic updates (e.g., to report current position if not moving)
    virtual void TimerHit() override
    {
        if (!isConnected())
        {
            SetTimer(getCurrentPollingPeriod());
            return;
        }

        // Simulate motion completion for MoveRotator
        if (m_isMoving)
        {
            // In a real driver, you would query hardware for current position
            // For this example, we simulate reaching the target after some time
            static int moveCounter = 0;
            moveCounter++;
            if (moveCounter >= 5) // Simulate 5 timer hits to reach target
            {
                LOG_INFO("Rotator reached %.2f degrees.", m_targetAngle);
                GotoRotatorNP[0].setValue(m_targetAngle); // Update current position to target
                GotoRotatorNP.setState(IPS_OK);
                GotoRotatorNP.apply();
                m_isMoving = false;
                moveCounter = 0;
            }
            else
            {
                // Simulate intermediate position updates if desired
                double currentSimulatedAngle = GotoRotatorNP[0].getValue() + (m_targetAngle - GotoRotatorNP[0].getValue()) / (5.0 - moveCounter);
                GotoRotatorNP[0].setValue(currentSimulatedAngle);
                GotoRotatorNP.apply();
            }
        }

        // Simulate homing completion for HomeRotator
        if (m_isHoming)
        {
            // In a real driver, you would query hardware for homing status
            // For this example, we simulate completion after some time
            static int homeCounter = 0;
            homeCounter++;
            if (homeCounter >= 10) // Simulate 10 timer hits for homing
            {
                LOG_INFO("Rotator homed.");
                GotoRotatorNP[0].setValue(0); // Assuming home is 0 degrees
                GotoRotatorNP.setState(IPS_OK);
                GotoRotatorNP.apply();
                HomeRotatorSP[0].setState(ISS_OFF); // Turn off homing switch element
                HomeRotatorSP.setState(IPS_OK);
                HomeRotatorSP.apply();
                m_isHoming = false;
                homeCounter = 0;
            }
        }

        SetTimer(getCurrentPollingPeriod());
    }

private:
    double m_targetAngle {0};
    bool m_isMoving {false};
    bool m_isHoming {false};
};

// This is typically how an INDI driver is instantiated
// static MyRotatorDevice myRotatorDevice;
