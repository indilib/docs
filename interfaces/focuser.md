---
title: Focuser Interface
nav_order: 3
parent: Device Interfaces
---

# Implementing the Focuser Interface

This guide provides a comprehensive overview of implementing the focuser interface in INDI drivers. It covers the basic structure of a focuser driver, the capabilities a focuser can advertise, the methods you need to implement, and how to report motion progress back to clients.

## Introduction to the Focuser Interface

The INDI Focuser Interface (`INDI::FocuserInterface`, aliased as `FI`) provides a standardized way for INDI clients to move a focuser, query its position, and configure features such as backlash compensation, motion reversal, and speed. Clients such as KStars/Ekos rely on this interface for manual focusing and for automated focus routines, so a correct implementation is essential for autofocus to work reliably.

A focuser can be an independent device, or an embedded component within another device (for example, a focus motor port on a camera, mount, or power box).

The interface supports three kinds of focusers:

- **Absolute Position**: Focusers that know their absolute position in steps on power up. Clients can command them to go to a specific position. By definition, position 0 (zero) is where the focuser is completely _retracted_ (closest to the OTA), and the position increases as the focuser moves _outward_. Negative positions are not accepted.
- **Relative Position**: Focusers that can move a specific number of steps inward or outward, but have no information on their absolute position on power up.
- **DC Motor**: Focusers without any position feedback. The only way to control them is by moving in a direction at a given speed for a given duration, or until stopped.

## Prerequisites

Before implementing the focuser interface, you should have:

- Basic knowledge of C++ programming
- Understanding of the INDI protocol and architecture
- Familiarity with the device's communication protocol
- Development environment set up (compiler, build tools, etc.)
- INDI library installed

## Focuser Interface Structure

The focuser interface consists of several key components:

- **Base Classes**: `INDI::FocuserInterface` provides the focuser properties and logic. `INDI::Focuser` combines it with `INDI::DefaultDevice` for standalone focusers.
- **Capabilities**: A bitmask that tells the interface which features your focuser supports. It determines which properties are defined and which methods you need to implement.
- **Standard Properties**: Properties for position, motion direction, speed, timers, limits, sync, reverse, backlash, and abort.
- **Virtual Methods**: Methods that your driver overrides to command the hardware.

### Choosing a Base Class

There are two ways to add focuser functionality to a driver:

| Use case                                                                 | Inherit from                                          | Header                    |
| ------------------------------------------------------------------------ | ----------------------------------------------------- | ------------------------- |
| A standalone focuser device                                              | `INDI::Focuser`                                       | `indifocuser.h`           |
| A focuser embedded in another device (camera, mount, power box, etc.)    | Your device base class **and** `INDI::FocuserInterface` | `indifocuserinterface.h`  |

For standalone focusers, always use `INDI::Focuser`. It wires up the focuser interface for you and adds:

- **Connection Management**: Serial and TCP connection plugins, selectable with `setSupportedConnections()`. After a successful connection, the file descriptor is available in `PortFD`.
- **Handshake**: A virtual `Handshake()` method, called after the connection is established, to verify communication with the device.
- **Presets**: Three preset positions (`PresetNP`) and the corresponding goto buttons (`PresetGotoSP`), defined for absolute focusers.
- **Joystick Support**: Mapping of joystick buttons to _Focus In_, _Focus Out_, and _Abort Focus_.
- **Standard Controls**: Debug and polling period controls, and the `FOCUSER_INTERFACE` driver interface flag.

When embedding a focuser in another device, you must call the interface methods yourself, as described in [Embedding a Focuser in Another Device](#embedding-a-focuser-in-another-device).

### FocuserCapability Enum

Call `SetCapability()` in your driver's constructor to declare what your focuser can do. The capabilities are bit flags that you combine with `|`:

```cpp
enum
{
    FOCUSER_CAN_ABS_MOVE       = 1 << 0, /*!< Can the focuser move by absolute position? */
    FOCUSER_CAN_REL_MOVE       = 1 << 1, /*!< Can the focuser move by relative position? */
    FOCUSER_CAN_ABORT          = 1 << 2, /*!< Is it possible to abort focuser motion? */
    FOCUSER_CAN_REVERSE        = 1 << 3, /*!< Is it possible to reverse focuser motion? */
    FOCUSER_CAN_SYNC           = 1 << 4, /*!< Can the focuser sync to a custom position */
    FOCUSER_HAS_VARIABLE_SPEED = 1 << 5, /*!< Can the focuser move in different configurable speeds? */
    FOCUSER_HAS_BACKLASH       = 1 << 6  /*!< Can the focuser compensate for backlash? */
} FocuserCapability;
```

You can query capabilities at any time with `CanAbsMove()`, `CanRelMove()`, `CanAbort()`, `CanReverse()`, `CanSync()`, `HasVariableSpeed()`, and `HasBacklash()`.

Each capability determines which properties are defined when the device connects, and which methods you must override:

| Capability                   | Properties defined                       | Method to override                                   |
| ---------------------------- | ---------------------------------------- | ---------------------------------------------------- |
| _(always)_                   | `FOCUS_MOTION`                           | —                                                    |
| `FOCUSER_CAN_ABS_MOVE`       | `ABS_FOCUS_POSITION`, `FOCUS_MAX`        | `MoveAbsFocuser()`, optionally `SetFocuserMaxPosition()` |
| `FOCUSER_CAN_REL_MOVE`       | `REL_FOCUS_POSITION`                     | `MoveRelFocuser()`                                   |
| `FOCUSER_CAN_ABORT`          | `FOCUS_ABORT_MOTION`                     | `AbortFocuser()`                                     |
| `FOCUSER_CAN_SYNC`           | `FOCUS_SYNC`                             | `SyncFocuser()`                                      |
| `FOCUSER_CAN_REVERSE`        | `FOCUS_REVERSE_MOTION`                   | `ReverseFocuser()`                                   |
| `FOCUSER_HAS_VARIABLE_SPEED` | `FOCUS_SPEED`, and `FOCUS_TIMER` if the focuser cannot move to an absolute position | `SetFocuserSpeed()`, `MoveFocuser()` |
| `FOCUSER_HAS_BACKLASH`       | `FOCUS_BACKLASH_TOGGLE`, `FOCUS_BACKLASH_STEPS` | `SetFocuserBacklash()`, optionally `SetFocuserBacklashEnabled()` |

Typical combinations are:

- **Absolute stepper focuser**: `FOCUSER_CAN_ABS_MOVE | FOCUSER_CAN_REL_MOVE | FOCUSER_CAN_ABORT`, plus `FOCUSER_CAN_SYNC`, `FOCUSER_CAN_REVERSE`, and `FOCUSER_HAS_BACKLASH` if the hardware supports them.
- **Relative-only focuser**: `FOCUSER_CAN_REL_MOVE | FOCUSER_CAN_ABORT`.
- **DC focuser with speed control**: `FOCUSER_HAS_VARIABLE_SPEED | FOCUSER_CAN_ABORT`.
- **Simple DC focuser (no speed control)**: `FOCUSER_CAN_ABORT`. Selecting a direction in `FOCUS_MOTION` starts motion until it is aborted.

> **Note:** Always set `FOCUSER_CAN_ABORT` if the hardware can stop motion. Clients use `FOCUS_ABORT_MOTION` to stop the focuser, and the property is only defined when this capability is set.

## Standard Properties

The interface defines the following standard properties. They are created in `initProperties()` and defined or deleted in `updateProperties()` according to the capabilities. See [Standard Properties](../drivers/standard-properties.md#focuser-properties) for the full list of standard property names.

| Property                | Member             | Type   | Elements                            | Description                                                                                                                                         |
| ----------------------- | ------------------ | ------ | ----------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------- |
| `FOCUS_MOTION`          | `FocusMotionSP`    | Switch | `FOCUS_INWARD`, `FOCUS_OUTWARD`     | Direction used by relative and timed moves. For simple DC focusers, selecting a direction starts continuous motion.                                 |
| `ABS_FOCUS_POSITION`    | `FocusAbsPosNP`    | Number | `FOCUS_ABSOLUTE_POSITION`           | Current absolute position in steps. Setting it moves the focuser to that position.                                                                  |
| `REL_FOCUS_POSITION`    | `FocusRelPosNP`    | Number | `FOCUS_RELATIVE_POSITION`           | Move this many steps in the `FOCUS_MOTION` direction. Must be greater than zero.                                                                     |
| `FOCUS_MAX`             | `FocusMaxPosNP`    | Number | `FOCUS_MAX_VALUE`                   | Maximum travel in steps. Changing it updates the limits of the absolute, relative, sync, and preset properties. Defaults to 100,000.                |
| `FOCUS_SPEED`           | `FocusSpeedNP`     | Number | `FOCUS_SPEED_VALUE`                 | Focuser speed. Defaults to a 0–255 range; adjust the range to match your hardware.                                                                   |
| `FOCUS_TIMER`           | `FocusTimerNP`     | Number | `FOCUS_TIMER_VALUE`                 | Move in the `FOCUS_MOTION` direction at `FOCUS_SPEED` for this many milliseconds.                                                                    |
| `FOCUS_SYNC`            | `FocusSyncNP`      | Number | `FOCUS_SYNC_VALUE`                  | Set the current position to this value without moving.                                                                                              |
| `FOCUS_REVERSE_MOTION`  | `FocusReverseSP`   | Switch | `INDI_ENABLED`, `INDI_DISABLED`     | Reverse the default motor direction.                                                                                                                |
| `FOCUS_BACKLASH_TOGGLE` | `FocusBacklashSP`  | Switch | `INDI_ENABLED`, `INDI_DISABLED`     | Enable or disable backlash compensation.                                                                                                            |
| `FOCUS_BACKLASH_STEPS`  | `FocusBacklashNP`  | Number | `FOCUS_BACKLASH_VALUE`              | Backlash compensation in steps. Can only be set while backlash compensation is enabled.                                                            |
| `FOCUS_ABORT_MOTION`    | `FocusAbortSP`     | Switch | `ABORT`                             | Abort any focuser motion.                                                                                                                           |

`INDI::Focuser` additionally defines `Presets` (`PresetNP`) and `Goto` (`PresetGotoSP`) for absolute focusers.

### Temperature

Temperature is not part of the focuser interface, but many focusers have a temperature probe. If yours does, define a number property named `FOCUS_TEMPERATURE` with an element named `TEMPERATURE` in degrees Celsius. Clients such as Ekos look for this property to record temperature with focus runs and to trigger refocusing on temperature changes.

```cpp
INDI::PropertyNumber TemperatureNP {1};

// In initProperties()
TemperatureNP[0].fill("TEMPERATURE", "Celsius", "%6.2f", -50., 70., 0., 0.);
TemperatureNP.fill(getDeviceName(), "FOCUS_TEMPERATURE", "Temperature",
                   MAIN_CONTROL_TAB, IP_RO, 0, IPS_IDLE);
```

## Key Methods

### Interface Methods

These methods are provided by `INDI::FocuserInterface`. When you inherit from `INDI::Focuser`, they are already called for you. When you embed a focuser in another device, you must call them from your driver.

- `void SetCapability(uint32_t cap)`:
  Sets the focuser capabilities. Call it in the constructor, before properties are initialized.

- `void initProperties(const char *groupName)`:
  Initializes the focuser properties. Call it from your driver's `initProperties()`.
  - `groupName`: The group or tab where focuser properties are displayed in the client.

- `bool updateProperties()`:
  Defines or deletes focuser properties based on the connection status and capabilities of the device. Call it from your driver's `updateProperties()`, whether connected or not.

- `bool processNumber(const char *dev, const char *name, double values[], char *names[], int n)`:
  Processes client requests for focuser number properties and calls the matching virtual method (`MoveAbsFocuser()`, `MoveRelFocuser()`, `MoveFocuser()`, `SyncFocuser()`, and so on). Call it from your driver's `ISNewNumber()`. It returns `true` if the property was handled.

- `bool processSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)`:
  Processes client requests for focuser switch properties (motion, abort, reverse, backlash). Call it from your driver's `ISNewSwitch()`. It returns `true` if the property was handled.

- `bool saveConfigItems(FILE *fp)`:
  Saves the focuser maximum position, reverse setting, and backlash settings to the configuration file. Call it from your driver's `saveConfigItems()`.

### Virtual Methods to Override

Override the methods that correspond to the capabilities you set. The default implementations either fail (returning `IPS_ALERT` or `false`) or, for `SetFocuserMaxPosition()`, succeed without doing anything.

- `virtual IPState MoveAbsFocuser(uint32_t targetTicks)`:
  Moves the focuser to an absolute position. Required for `FOCUSER_CAN_ABS_MOVE`. The interface validates that `targetTicks` is within the limits of `ABS_FOCUS_POSITION` before calling it.

- `virtual IPState MoveRelFocuser(FocusDirection dir, uint32_t ticks)`:
  Moves the focuser a number of steps inward (`FOCUS_INWARD`) or outward (`FOCUS_OUTWARD`). Required for `FOCUSER_CAN_REL_MOVE`. If the focuser can also move to an absolute position, the interface checks that the move stays within limits before calling it.

- `virtual IPState MoveFocuser(FocusDirection dir, int speed, uint16_t duration)`:
  Moves the focuser in a direction at a speed for `duration` milliseconds. A `duration` of 0 means move until aborted. Required for DC focusers. It is called when the client sets `FOCUS_TIMER`, and, for simple DC focusers without speed, position, or relative control, when the client selects a direction in `FOCUS_MOTION`.

- `virtual bool AbortFocuser()`:
  Stops all focuser motion. Required for `FOCUSER_CAN_ABORT`. On success, the interface sets any busy position properties back to idle.

- `virtual bool SyncFocuser(uint32_t ticks)`:
  Sets the current position to `ticks` without moving the focuser. Required for `FOCUSER_CAN_SYNC`. On success, the interface updates `ABS_FOCUS_POSITION` to the new value.

- `virtual bool ReverseFocuser(bool enabled)`:
  Reverses the default motor direction when `enabled` is `true`, and restores it when `false`. Required for `FOCUSER_CAN_REVERSE`.

- `virtual bool SetFocuserSpeed(int speed)`:
  Sets the focuser speed. Required for `FOCUSER_HAS_VARIABLE_SPEED`.

- `virtual bool SetFocuserMaxPosition(uint32_t ticks)`:
  Sets the maximum travel limit in the hardware. Override it only if the hardware stores a limit. The default implementation does nothing and returns `true`, so the limit is still enforced in software.

- `virtual bool SetFocuserBacklash(int32_t steps)`:
  Sets the backlash compensation value in the hardware. Required for `FOCUSER_HAS_BACKLASH`.

- `virtual bool SetFocuserBacklashEnabled(bool enabled)`:
  Enables or disables backlash compensation. The default implementation calls `SetFocuserBacklash()` with the current `FOCUS_BACKLASH_STEPS` value when enabled, and with 0 when disabled. Override it only if the hardware has a separate enable flag.

> **Warning:** `INDI::Focuser` overrides `SetFocuserMaxPosition()` to update the limits of the preset properties. If you override it in a driver derived from `INDI::Focuser`, call `INDI::Focuser::SetFocuserMaxPosition(ticks)` (or `SyncPresets(ticks)`) so the presets stay in sync.

## Reporting Motion

Focuser moves usually take time. The three `Move*` methods return an `IPState` that tells the interface how the move went:

| Return value | Meaning                                                                                  |
| ------------ | ---------------------------------------------------------------------------------------- |
| `IPS_OK`     | The move has finished and the focuser is at the requested position.                      |
| `IPS_BUSY`   | The move has started and is in progress. Your driver reports completion later.           |
| `IPS_ALERT`  | The move failed. The interface sets the property to the alert state and logs an error.   |

Most hardware should return `IPS_BUSY` immediately after the move command is accepted, and should not block until motion finishes. While the move is in progress, poll the hardware in `TimerHit()`:

1. Read the current position and update `FocusAbsPosNP[0]` so clients can follow the motion.
2. When the focuser stops, set `FocusAbsPosNP` and `FocusRelPosNP` to `IPS_OK` and send them to the client.

Clients such as Ekos wait for `ABS_FOCUS_POSITION` (or `REL_FOCUS_POSITION`) to change from `IPS_BUSY` to `IPS_OK` before taking the next autofocus frame. If you never change the state back to `IPS_OK`, autofocus will stall until it times out.

> **Important:** When a move finishes, always set `FocusAbsPosNP[0]` to the actual position reported by the hardware rather than the requested target. The two can differ, for example after an abort or a backlash move.

If `MoveAbsFocuser()` returns `IPS_OK`, the interface updates `ABS_FOCUS_POSITION` to the requested value. If `MoveRelFocuser()` returns `IPS_OK`, the interface sends `ABS_FOCUS_POSITION` to the client but does not change its value, so you must update `FocusAbsPosNP[0]` yourself before returning.

## Position Limits

The absolute position range is 0 to `FOCUS_MAX_VALUE`. When a client changes `FOCUS_MAX`, the interface calls `SetFocuserMaxPosition()` and, if it succeeds, updates the limits and step sizes of `ABS_FOCUS_POSITION`, `REL_FOCUS_POSITION`, and `FOCUS_SYNC`. The value is saved to the configuration file, so it is restored on the next connection.

If the hardware reports its own maximum position, read it in `Handshake()` and update the limits:

```cpp
FocusMaxPosNP[0].setValue(maxSteps);
FocusAbsPosNP[0].setMax(maxSteps);
FocusSyncNP[0].setMax(maxSteps);
FocusRelPosNP[0].setMax(maxSteps / 2);
FocusAbsPosNP.updateMinMax();
FocusSyncNP.updateMinMax();
FocusRelPosNP.updateMinMax();
```

## Example Implementation

Here's a simplified example of a standalone absolute focuser that inherits from `INDI::Focuser`. Serial communication is hidden behind the hypothetical helpers `sendCommand()` and `readValue()`; replace them with your device's protocol.

### Header (`my_focuser.h`)

```cpp
#pragma once

#include <libindi/indifocuser.h>

class MyFocuser : public INDI::Focuser
{
    public:
        MyFocuser();

        const char *getDefaultName() override;
        bool initProperties() override;
        bool updateProperties() override;

    protected:
        bool Handshake() override;
        void TimerHit() override;

        IPState MoveAbsFocuser(uint32_t targetTicks) override;
        IPState MoveRelFocuser(FocusDirection dir, uint32_t ticks) override;
        bool AbortFocuser() override;
        bool SyncFocuser(uint32_t ticks) override;
        bool ReverseFocuser(bool enabled) override;
        bool SetFocuserBacklash(int32_t steps) override;

    private:
        bool readPosition();
        bool readTemperature();
        bool isMoving();

        bool sendCommand(const char *cmd);
        bool readValue(const char *cmd, int &value);

        INDI::PropertyNumber TemperatureNP {1};

        uint32_t m_TargetPosition {0};
};
```

### Implementation (`my_focuser.cpp`)

```cpp
#include "my_focuser.h"

#include <libindi/indicom.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <memory>

static std::unique_ptr<MyFocuser> myFocuser(new MyFocuser());

MyFocuser::MyFocuser()
{
    setVersion(1, 0);

    // Only serial is supported by this device.
    setSupportedConnections(CONNECTION_SERIAL);

    SetCapability(FOCUSER_CAN_ABS_MOVE |
                  FOCUSER_CAN_REL_MOVE |
                  FOCUSER_CAN_ABORT    |
                  FOCUSER_CAN_SYNC     |
                  FOCUSER_CAN_REVERSE  |
                  FOCUSER_HAS_BACKLASH);
}

const char *MyFocuser::getDefaultName()
{
    return "My Focuser";
}

bool MyFocuser::initProperties()
{
    // Initializes the focuser interface, presets, and connection plugins.
    INDI::Focuser::initProperties();

    TemperatureNP[0].fill("TEMPERATURE", "Celsius", "%6.2f", -50., 70., 0., 0.);
    TemperatureNP.fill(getDeviceName(), "FOCUS_TEMPERATURE", "Temperature",
                       MAIN_CONTROL_TAB, IP_RO, 0, IPS_IDLE);

    // Hardware travel range.
    FocusMaxPosNP[0].setValue(50000);
    FocusAbsPosNP[0].setMax(50000);
    FocusRelPosNP[0].setMax(25000);
    FocusSyncNP[0].setMax(50000);

    addAuxControls();

    return true;
}

bool MyFocuser::updateProperties()
{
    // Defines or deletes the focuser interface properties.
    INDI::Focuser::updateProperties();

    if (isConnected())
    {
        defineProperty(TemperatureNP);

        readPosition();
        readTemperature();
    }
    else
    {
        deleteProperty(TemperatureNP);
    }

    return true;
}

bool MyFocuser::Handshake()
{
    if (isSimulation())
        return true;

    // PortFD is set by the base class before Handshake() is called.
    int firmware = 0;
    if (!readValue("VERSION", firmware))
    {
        LOG_ERROR("Failed to communicate with the focuser. Check the port and baud rate.");
        return false;
    }

    LOGF_INFO("Focuser firmware version %d.", firmware);
    return true;
}

IPState MyFocuser::MoveAbsFocuser(uint32_t targetTicks)
{
    char cmd[32] = {0};
    snprintf(cmd, sizeof(cmd), "GOTO %u", targetTicks);

    if (!sendCommand(cmd))
        return IPS_ALERT;

    m_TargetPosition = targetTicks;

    // Motion has started. TimerHit() reports when it finishes.
    return IPS_BUSY;
}

IPState MyFocuser::MoveRelFocuser(FocusDirection dir, uint32_t ticks)
{
    int32_t current = static_cast<int32_t>(FocusAbsPosNP[0].getValue());
    int32_t target  = current + (dir == FOCUS_INWARD ? -1 : 1) * static_cast<int32_t>(ticks);

    // The interface already checks limits for absolute focusers, but clamp to be safe.
    target = std::max(0, std::min(target, static_cast<int32_t>(FocusAbsPosNP[0].getMax())));

    // Returning IPS_BUSY marks both REL_FOCUS_POSITION and ABS_FOCUS_POSITION as busy.
    return MoveAbsFocuser(static_cast<uint32_t>(target));
}

bool MyFocuser::AbortFocuser()
{
    return sendCommand("STOP");
}

bool MyFocuser::SyncFocuser(uint32_t ticks)
{
    char cmd[32] = {0};
    snprintf(cmd, sizeof(cmd), "SYNC %u", ticks);
    return sendCommand(cmd);
}

bool MyFocuser::ReverseFocuser(bool enabled)
{
    return sendCommand(enabled ? "REVERSE 1" : "REVERSE 0");
}

bool MyFocuser::SetFocuserBacklash(int32_t steps)
{
    char cmd[32] = {0};
    snprintf(cmd, sizeof(cmd), "BACKLASH %d", steps);
    return sendCommand(cmd);
}

void MyFocuser::TimerHit()
{
    if (!isConnected())
        return;

    // Only send the position to the client when it changes.
    double lastPosition = FocusAbsPosNP[0].getValue();
    if (readPosition() && std::fabs(lastPosition - FocusAbsPosNP[0].getValue()) > 0)
        FocusAbsPosNP.apply();

    double lastTemperature = TemperatureNP[0].getValue();
    if (readTemperature() && std::fabs(lastTemperature - TemperatureNP[0].getValue()) >= 0.1)
        TemperatureNP.apply();

    // Report completion of any move in progress.
    if (FocusAbsPosNP.getState() == IPS_BUSY || FocusRelPosNP.getState() == IPS_BUSY)
    {
        if (!isMoving())
        {
            FocusAbsPosNP.setState(IPS_OK);
            FocusRelPosNP.setState(IPS_OK);
            FocusAbsPosNP.apply();
            FocusRelPosNP.apply();
            LOG_INFO("Focuser reached requested position.");
        }
    }

    SetTimer(getCurrentPollingPeriod());
}

bool MyFocuser::readPosition()
{
    int position = 0;
    if (!readValue("POSITION", position))
        return false;

    FocusAbsPosNP[0].setValue(position);
    return true;
}

bool MyFocuser::readTemperature()
{
    int tenthsOfDegree = 0;
    if (!readValue("TEMPERATURE", tenthsOfDegree))
        return false;

    TemperatureNP[0].setValue(tenthsOfDegree / 10.0);
    TemperatureNP.setState(IPS_OK);
    return true;
}

bool MyFocuser::isMoving()
{
    int moving = 0;
    return readValue("MOVING", moving) && moving == 1;
}

// sendCommand() and readValue() implement the device protocol using PortFD and the
// tty_* functions from indicom.h. See the Serial Communication guide for details.
```

Note that `INDI::Focuser::ISNewNumber()` and `ISNewSwitch()` already call `FI::processNumber()` and `FI::processSwitch()`. You only need to override `ISNewNumber()` or `ISNewSwitch()` to handle your own custom properties. When you do, check your own properties first and then call the `INDI::Focuser` implementation.

The [indi_dummy_focuser](https://github.com/indilib/docs/tree/master/drivers/examples/indi_dummy_focuser) example in this project is a minimal driver template that you can start from.

## Embedding a Focuser in Another Device

To add a focuser to a device that is not primarily a focuser, inherit from `INDI::FocuserInterface` alongside your main base class and call the interface methods at the following points:

| Function                 | Where to call it from your driver                                   |
| ------------------------ | ------------------------------------------------------------------- |
| `FI::SetCapability()`    | Constructor                                                         |
| `FI::initProperties()`   | `initProperties()`                                                  |
| `FI::updateProperties()` | `updateProperties()`, both when connected and when disconnected     |
| `FI::processNumber()`    | `ISNewNumber()`                                                     |
| `FI::processSwitch()`    | `ISNewSwitch()`                                                     |
| `FI::saveConfigItems()`  | `saveConfigItems()`                                                 |

You must also add `FOCUSER_INTERFACE` to the driver interface flags, so clients know the device has a focuser.

```cpp
#include <libindi/defaultdevice.h>
#include <libindi/indifocuserinterface.h>

#include <cstring>

class MyPowerBox : public INDI::DefaultDevice, public INDI::FocuserInterface
{
    public:
        MyPowerBox() : INDI::FocuserInterface(this)
        {
            setVersion(1, 0);
            FI::SetCapability(FOCUSER_CAN_ABS_MOVE | FOCUSER_CAN_REL_MOVE | FOCUSER_CAN_ABORT);
        }

        const char *getDefaultName() override
        {
            return "My Power Box";
        }

        bool initProperties() override
        {
            INDI::DefaultDevice::initProperties();

            // Place the focuser properties on their own tab.
            FI::initProperties("Focuser");

            setDriverInterface(AUX_INTERFACE | FOCUSER_INTERFACE);

            addAuxControls();
            return true;
        }

        bool updateProperties() override
        {
            INDI::DefaultDevice::updateProperties();

            // Defines the properties when connected, and deletes them when disconnected.
            FI::updateProperties();

            return true;
        }

        bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) override
        {
            if (dev && !strcmp(dev, getDeviceName()))
            {
                if (strstr(name, "FOCUS"))
                    return FI::processNumber(dev, name, values, names, n);
            }

            return INDI::DefaultDevice::ISNewNumber(dev, name, values, names, n);
        }

        bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n) override
        {
            if (dev && !strcmp(dev, getDeviceName()))
            {
                if (strstr(name, "FOCUS"))
                    return FI::processSwitch(dev, name, states, names, n);
            }

            return INDI::DefaultDevice::ISNewSwitch(dev, name, states, names, n);
        }

    protected:
        bool saveConfigItems(FILE *fp) override
        {
            INDI::DefaultDevice::saveConfigItems(fp);
            FI::saveConfigItems(fp);
            return true;
        }

        IPState MoveAbsFocuser(uint32_t targetTicks) override;
        IPState MoveRelFocuser(FocusDirection dir, uint32_t ticks) override;
        bool AbortFocuser() override;
};
```

Unlike `INDI::Focuser`, the interface alone does not provide presets, connection plugins, or joystick mapping. Report motion progress from `TimerHit()` as described in [Reporting Motion](#reporting-motion).

Examples of drivers with embedded focusers in the [INDI library](https://github.com/indilib/indi) include the Pegasus Ultimate Power Box (`drivers/power/pegasus_upb.cpp`) and the Celestron mount driver (`drivers/telescope/celestrongps.cpp`).

## Best Practices

- **Never block in a move method**: Send the move command, return `IPS_BUSY`, and report completion from `TimerHit()`. Blocking stalls the whole driver, including abort requests.
- **Always finish what you start**: Every move that returns `IPS_BUSY` must eventually be set to `IPS_OK` (on success) or `IPS_ALERT` (on failure). Autofocus routines depend on it.
- **Report the real position**: Read the position from the hardware rather than assuming the target was reached.
- **Keep position semantics consistent**: Position 0 is fully retracted, and inward moves decrease the position. Use `FOCUS_REVERSE_MOTION` to handle motors that are mounted the other way around, instead of inverting directions in your driver.
- **Throttle updates**: Only send position and temperature updates when the values change, to avoid flooding clients.
- **Support simulation**: Check `isSimulation()` so the driver can be tested without hardware.
- **Save user settings**: Call `FI::saveConfigItems()` (done for you by `INDI::Focuser`) so the maximum position, reverse, and backlash settings persist between sessions.

## Real-World Examples

The [INDI library](https://github.com/indilib/indi/tree/master/drivers/focuser) contains many focuser drivers that can serve as references:

- **Focuser Simulator** (`focus_simulator.cpp`): Simulates absolute, relative, and DC focusers. Useful for testing clients.
- **MoonLite** (`moonlite.cpp`): A classic serial absolute focuser with temperature compensation.
- **Robo Focus** (`robofocus.cpp`): An absolute focuser with backlash and maximum travel settings.
