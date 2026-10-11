---
title: Dome Interface
nav_order: 12
parent: Device Interfaces
---

# Implementing the Dome Interface

This guide provides a comprehensive overview of implementing the dome interface in INDI drivers. It covers the structure of a dome driver, the capabilities a dome can advertise, the methods you need to implement, and how to report dome and shutter state back to clients.

## Introduction to the Dome Interface

The INDI Dome Interface (`INDI::Dome`, defined in `indidome.h`) is designed for devices that control observatory enclosures, including rotating domes and roll-off roofs. It supports:

- **Absolute and relative position domes**, which can rotate to a given azimuth
- **Open-loop domes** without position feedback, which can only rotate clockwise or counter-clockwise until stopped
- **Roll-off roofs**, where parking means closing the roof and unparking means opening it
- **Shutters** that can be opened and closed electronically

`INDI::Dome` handles most of the work that is common to all domes:

- Defining the standard motion, position, shutter, and parking properties
- Validating client requests (for example, rejecting motion while parked) and calling the matching virtual method in your driver
- Serial and TCP [connection plugins](../drivers/communications/connection-plugins.md), and calling your `Handshake()` after connecting
- Storing park positions and status in `~/.indi/ParkData.xml`
- **Slaving**: following the mount, as described in [Slaving](#slaving)
- The mount locking policy, presets, and joystick control

Your driver only needs to talk to the dome hardware: start and stop rotation, open and close the shutter, and report the azimuth and status.

## Prerequisites

Before implementing the dome interface, you should have:

- Basic knowledge of C++ programming
- Understanding of the INDI protocol and architecture
- Familiarity with the device's communication protocol
- Development environment set up (compiler, build tools, etc.)
- INDI library installed

## Dome Interface Structure

The dome interface consists of several key components:

- **Base Class**: `INDI::Dome` inherits from `INDI::DefaultDevice`.
- **Capabilities**: A bitmask that tells the base class which features your dome supports.
- **Dome and Shutter State**: The current state of the dome and shutter, which your driver keeps up to date.
- **Virtual Methods**: Methods that your driver overrides to command the hardware.

### Dome Capabilities

Call `SetDomeCapability()` in your driver's constructor to declare what your dome can do. The capabilities are bit flags that you combine with `|`:

| Capability Flag           | Value    | Description                                                                 |
| ------------------------- | -------- | --------------------------------------------------------------------------- |
| `DOME_CAN_ABORT`          | `1 << 0` | Dome motion can be aborted.                                                 |
| `DOME_CAN_ABS_MOVE`       | `1 << 1` | Dome can move to an absolute azimuth.                                       |
| `DOME_CAN_REL_MOVE`       | `1 << 2` | Dome can move a number of degrees from its current position. Positive values are clockwise, negative values are counter-clockwise. |
| `DOME_CAN_PARK`           | `1 << 3` | Dome can park and unpark itself. For roll-off roofs, this closes and opens the roof. |
| `DOME_CAN_SYNC`           | `1 << 4` | Dome can sync to an arbitrary azimuth.                                      |
| `DOME_HAS_SHUTTER`        | `1 << 5` | Dome has a shutter that can be opened and closed electronically.            |
| `DOME_HAS_VARIABLE_SPEED` | `1 << 6` | Dome can rotate at different configurable speeds.                           |
| `DOME_HAS_BACKLASH`       | `1 << 7` | Dome can compensate for backlash.                                           |

Each capability determines which properties are defined when the dome connects, and which methods you must override:

| Capability                | Properties defined                                                      | Methods to override                       |
| ------------------------- | ----------------------------------------------------------------------- | ----------------------------------------- |
| _(always)_                | `DOME_MOTION`                                                           | `Move()`                                  |
| `DOME_CAN_ABS_MOVE`       | `ABS_DOME_POSITION`, presets, and the slaving properties (`DOME_AUTOSYNC`, `DOME_PARAMS`, `DOME_MEASUREMENTS`, `DM_OTA_SIDE`) | `MoveAbs()`         |
| `DOME_CAN_REL_MOVE`       | `REL_DOME_POSITION`                                                     | `MoveRel()`                               |
| `DOME_CAN_ABORT`          | `DOME_ABORT_MOTION`                                                     | `Abort()`                                 |
| `DOME_CAN_SYNC`           | `DOME_SYNC`                                                             | `Sync()`                                  |
| `DOME_CAN_PARK`           | `DOME_PARK`, and `DOME_PARK_POSITION` and `DOME_PARK_OPTION` unless the park data type is `PARK_NONE` | `Park()`, `UnPark()`, and `SetCurrentPark()` and `SetDefaultPark()` for custom park positions |
| `DOME_HAS_SHUTTER`        | `DOME_SHUTTER`, `DOME_SHUTTER_PARK_POLICY`                              | `ControlShutter()`                        |
| `DOME_HAS_VARIABLE_SPEED` | `DOME_SPEED`                                                            | `SetSpeed()`                              |
| `DOME_HAS_BACKLASH`       | `DOME_BACKLASH_TOGGLE`, `DOME_BACKLASH_STEPS`                           | `SetBacklash()`, optionally `SetBacklashEnabled()` |

See [Standard Properties](../drivers/standard-properties.md#dome-properties) for the elements of each property.

### Connection

By default, `INDI::Dome` offers both serial and TCP connections. To change this, call `setDomeConnection()` in your constructor with `CONNECTION_SERIAL`, `CONNECTION_TCP`, both, or `CONNECTION_NONE`. After a successful connection, the base class sets `PortFD` to the open file descriptor, calls your `Handshake()`, and starts the polling timer.

### Dome State

The dome state tells clients what the dome is doing. Read it with `getDomeState()`, and change it with `setDomeState()`, which also updates the related properties:

| State            | Meaning                         | Effect of `setDomeState()`                                    |
| ---------------- | ------------------------------- | ------------------------------------------------------------- |
| `DOME_IDLE`      | Not moving                      | Busy motion and position properties become idle.              |
| `DOME_MOVING`    | Rotating                        | —                                                             |
| `DOME_SYNCED`    | Reached the requested position  | Busy motion and position properties become OK.                |
| `DOME_PARKING`   | Moving to the park position     | `DOME_PARK` becomes busy.                                     |
| `DOME_UNPARKING` | Unparking                       | `DOME_PARK` becomes busy.                                     |
| `DOME_PARKED`    | Parked                          | `DOME_PARK` becomes OK, with `PARK` on.                       |
| `DOME_UNPARKED`  | Unparked                        | `DOME_PARK` becomes OK, with `UNPARK` on.                     |
| `DOME_UNKNOWN`   | State is unknown                | `DOME_PARK` becomes idle.                                     |
| `DOME_ERROR`     | Dome has errors                 | `DOME_PARK` becomes alert.                                    |

To finish parking or unparking, call `SetParked(true)` or `SetParked(false)` instead: it sets the parked or unparked state and also updates the park data file.

The shutter state works the same way, with `getShutterState()` and `setShutterState()`: `SHUTTER_OPENED`, `SHUTTER_CLOSED`, `SHUTTER_MOVING`, `SHUTTER_UNKNOWN`, and `SHUTTER_ERROR`. `setShutterState()` updates `DOME_SHUTTER` and sends it to clients.

## Key Methods

### Virtual Methods to Override

The base class validates each client request before calling these methods, and updates the property states based on the value you return. Most methods return an `IPState`:

| Return value | Meaning                                                                                    |
| ------------ | ------------------------------------------------------------------------------------------ |
| `IPS_OK`     | The operation is complete.                                                                 |
| `IPS_BUSY`   | The operation has started and is in progress. Your driver reports completion later.        |
| `IPS_ALERT`  | The operation failed.                                                                      |

- `virtual IPState Move(DomeDirection dir, DomeMotionCommand operation)`:
  Start (`MOTION_START`) or stop (`MOTION_STOP`) rotating clockwise (`DOME_CW`) or counter-clockwise (`DOME_CCW`).

- `virtual IPState MoveAbs(double az)`:
  Rotate to azimuth `az` in degrees. Required for `DOME_CAN_ABS_MOVE`.

- `virtual IPState MoveRel(double azDiff)`:
  Rotate `azDiff` degrees from the current position: positive is clockwise, negative is counter-clockwise. Required for `DOME_CAN_REL_MOVE`.

- `virtual bool Sync(double az)`:
  Set the current azimuth to `az` without moving. Required for `DOME_CAN_SYNC`.

- `virtual bool Abort()`:
  Stop all motion. Required for `DOME_CAN_ABORT`.

- `virtual IPState Park()` / `virtual IPState UnPark()`:
  Park or unpark the dome. For roll-off roofs, close or open the roof. Required for `DOME_CAN_PARK`. When parking or unparking finishes, call `SetParked(true)` or `SetParked(false)`.

- `virtual IPState ControlShutter(ShutterOperation operation)`:
  Open (`SHUTTER_OPEN`) or close (`SHUTTER_CLOSE`) the shutter. Required for `DOME_HAS_SHUTTER`. When the shutter finishes moving, call `setShutterState()`.

- `virtual bool SetSpeed(double rpm)`:
  Set the rotation speed in RPM, without starting motion. Required for `DOME_HAS_VARIABLE_SPEED`.

- `virtual bool SetBacklash(int32_t steps)` / `virtual bool SetBacklashEnabled(bool enabled)`:
  Set the backlash compensation in steps, and enable or disable it. Required for `DOME_HAS_BACKLASH`.

- `virtual bool SetCurrentPark()` / `virtual bool SetDefaultPark()`:
  Called when the client chooses to use the current position or the default position as the park position. Update the park position with `SetAxis1Park()`.

- `virtual bool Handshake()`:
  Called after the connection is established. Verify that the dome responds, and return `false` if it does not.

- `virtual void ActiveDevicesUpdated()`:
  Called when the client changes `ACTIVE_DEVICES`, so you can snoop on the new devices.

### Helper Methods

- `void SetDomeCapability(uint32_t cap)`: Sets the dome capabilities.
- `void setDomeConnection(const uint8_t &value)`: Sets the supported connection types.
- `void setDomeState(const DomeState &value)` / `DomeState getDomeState() const`: Set or get the dome state.
- `void setShutterState(const ShutterState &value)` / `ShutterState getShutterState() const`: Set or get the shutter state.
- `void SetParkDataType(DomeParkData type)`: Sets how park positions are stored. See [Parking](#parking).
- `bool InitPark()`: Loads park data from `~/.indi/ParkData.xml`. Call it once the dome is connected.
- `bool isParked()` / `void SetParked(bool isparked)`: Get or set the park status. `SetParked()` also updates the dome state, `DOME_PARK`, and the park data file.
- `SetAxis1Park()`, `SetAxis1ParkDefault()`, `GetAxis1Park()`, `GetAxis1ParkDefault()`: Set and get the current and default park azimuth (or encoder position).
- `bool isLocked()`: Returns `true` if the mount locking policy prevents the dome from parking.

## Example Implementation

Here's a simplified example of a serial dome with absolute positioning and a shutter. It uses a hypothetical text protocol in which each command and reply ends with `#`; replace the commands with your dome's protocol.

### Header (`my_dome.h`)

```cpp
#pragma once

#include <libindi/indidome.h>

class MyDome : public INDI::Dome
{
    public:
        MyDome();

        const char *getDefaultName() override;
        bool initProperties() override;
        bool updateProperties() override;

    protected:
        bool Handshake() override;
        void TimerHit() override;

        IPState Move(DomeDirection dir, DomeMotionCommand operation) override;
        IPState MoveAbs(double az) override;
        bool Abort() override;
        IPState Park() override;
        IPState UnPark() override;
        IPState ControlShutter(ShutterOperation operation) override;
        bool SetCurrentPark() override;
        bool SetDefaultPark() override;

    private:
        bool readStatus(double &az, bool &isMoving, int &shutter);
        bool sendCommand(const char *cmd, char *res = nullptr, int reslen = 0);
};
```

### Implementation (`my_dome.cpp`)

```cpp
#include "my_dome.h"

#include <libindi/indicom.h>

#include <cmath>
#include <cstring>
#include <memory>
#include <termios.h>

static std::unique_ptr<MyDome> myDome(new MyDome());

MyDome::MyDome()
{
    setVersion(1, 0);

    // This dome only has a serial port.
    setDomeConnection(CONNECTION_SERIAL);

    SetDomeCapability(DOME_CAN_ABORT | DOME_CAN_ABS_MOVE | DOME_CAN_PARK | DOME_HAS_SHUTTER);
}

const char *MyDome::getDefaultName()
{
    return "My Dome";
}

bool MyDome::initProperties()
{
    INDI::Dome::initProperties();

    // The dome parks at an azimuth angle that the user can change.
    SetParkDataType(PARK_AZ);

    addAuxControls();
    return true;
}

bool MyDome::updateProperties()
{
    INDI::Dome::updateProperties();

    if (isConnected())
    {
        // Load the park status and position once connected.
        if (InitPark())
        {
            SetAxis1ParkDefault(0);
        }
        else
        {
            SetAxis1Park(0);
            SetAxis1ParkDefault(0);
        }
    }

    return true;
}

bool MyDome::Handshake()
{
    // PortFD is set by the base class before Handshake() is called.
    char res[32] = {0};
    if (!sendCommand("VERSION#", res, sizeof(res)))
    {
        LOG_ERROR("Dome is not responding. Check the port and baud rate.");
        return false;
    }

    LOGF_INFO("Connected to dome controller %s.", res);
    return true;
}

IPState MyDome::Move(DomeDirection dir, DomeMotionCommand operation)
{
    if (operation == MOTION_STOP)
        return sendCommand("STOP#") ? IPS_OK : IPS_ALERT;

    return sendCommand(dir == DOME_CW ? "ROTATE_CW#" : "ROTATE_CCW#") ? IPS_BUSY : IPS_ALERT;
}

IPState MyDome::MoveAbs(double az)
{
    char cmd[32] = {0};
    snprintf(cmd, sizeof(cmd), "GOTO %.1f#", az);

    // Motion has started. TimerHit() reports when it finishes.
    return sendCommand(cmd) ? IPS_BUSY : IPS_ALERT;
}

bool MyDome::Abort()
{
    return sendCommand("STOP#");
}

IPState MyDome::Park()
{
    // Honor the shutter park policy chosen by the user. Calling the base class
    // Dome::ControlShutter() updates DOME_SHUTTER and the shutter state for us.
    if (HasShutter() && ShutterParkPolicySP[SHUTTER_CLOSE_ON_PARK].getState() == ISS_ON)
        Dome::ControlShutter(SHUTTER_CLOSE);

    char cmd[32] = {0};
    snprintf(cmd, sizeof(cmd), "GOTO %.1f#", GetAxis1Park());
    return sendCommand(cmd) ? IPS_BUSY : IPS_ALERT;
}

IPState MyDome::UnPark()
{
    if (HasShutter() && ShutterParkPolicySP[SHUTTER_OPEN_ON_UNPARK].getState() == ISS_ON)
        Dome::ControlShutter(SHUTTER_OPEN);

    // This dome does not need to move to unpark.
    return IPS_OK;
}

IPState MyDome::ControlShutter(ShutterOperation operation)
{
    if (!sendCommand(operation == SHUTTER_OPEN ? "SHUTTER_OPEN#" : "SHUTTER_CLOSE#"))
        return IPS_ALERT;

    // The shutter takes time to move. TimerHit() reports when it finishes.
    return IPS_BUSY;
}

bool MyDome::SetCurrentPark()
{
    SetAxis1Park(DomeAbsPosNP[0].getValue());
    return true;
}

bool MyDome::SetDefaultPark()
{
    SetAxis1Park(GetAxis1ParkDefault());
    return true;
}

void MyDome::TimerHit()
{
    if (!isConnected())
        return;

    double az = 0;
    bool isMoving = false;
    int shutter = 0;

    if (readStatus(az, isMoving, shutter))
    {
        // Report the azimuth when it changes.
        if (std::fabs(az - DomeAbsPosNP[0].getValue()) > 0.1)
        {
            DomeAbsPosNP[0].setValue(az);
            DomeAbsPosNP.apply();
        }

        // Report the end of rotation.
        if (getDomeState() == DOME_MOVING && !isMoving)
        {
            setDomeState(DOME_SYNCED);
            LOG_INFO("Dome reached requested azimuth.");
        }
        else if (getDomeState() == DOME_PARKING && !isMoving)
        {
            SetParked(true);
        }

        // Report the end of shutter motion. shutter: 0 = closed, 1 = open, 2 = moving.
        if (getShutterState() == SHUTTER_MOVING && shutter != 2)
            setShutterState(shutter == 1 ? SHUTTER_OPENED : SHUTTER_CLOSED);
    }

    SetTimer(getCurrentPollingPeriod());
}

bool MyDome::readStatus(double &az, bool &isMoving, int &shutter)
{
    // Reply format: "<azimuth> <moving> <shutter>#"
    char res[32] = {0};
    if (!sendCommand("STATUS#", res, sizeof(res)))
        return false;

    int moving = 0;
    if (sscanf(res, "%lf %d %d", &az, &moving, &shutter) != 3)
        return false;

    isMoving = (moving == 1);
    return true;
}

bool MyDome::sendCommand(const char *cmd, char *res, int reslen)
{
    int nbytes_written = 0, nbytes_read = 0;
    char errstr[MAXRBUF] = {0};

    tcflush(PortFD, TCIOFLUSH);

    int rc = tty_write_string(PortFD, cmd, &nbytes_written);
    if (rc != TTY_OK)
    {
        tty_error_msg(rc, errstr, MAXRBUF);
        LOGF_ERROR("Write error: %s", errstr);
        return false;
    }

    char buffer[64] = {0};
    rc = tty_read_section(PortFD, buffer, '#', 3, &nbytes_read);
    if (rc != TTY_OK)
    {
        tty_error_msg(rc, errstr, MAXRBUF);
        LOGF_ERROR("Read error: %s", errstr);
        return false;
    }

    buffer[nbytes_read - 1] = '\0'; // Remove the terminator
    if (res && reslen > 0)
        snprintf(res, reslen, "%s", buffer);

    return true;
}
```

Note that `MoveAbs()`, `Park()`, and `ControlShutter()` do not change property states: the base class does that based on the value they return. The driver only reports completion from `TimerHit()`, with `setDomeState()`, `SetParked()`, and `setShutterState()`.

There is no `main()` function: drivers are linked against `libindidriver`, which provides it. See [Creating Custom INDI Drivers](../drivers/basics/custom-driver.md) for the CMake setup and driver XML file.

## Advanced Topics

### Parking

`SetParkDataType()` selects how the park position is stored and presented to the user. Call it in `initProperties()`:

| Park data type    | Park position                                                                 |
| ----------------- | ----------------------------------------------------------------------------- |
| `PARK_NONE`       | No park position. Only the park status (open or closed) is stored. Use this for roll-off roofs. |
| `PARK_AZ`         | Azimuth angle in degrees.                                                     |
| `PARK_AZ_ENCODER` | Azimuth encoder steps.                                                        |

For `PARK_AZ` and `PARK_AZ_ENCODER`, the base class defines `DOME_PARK_POSITION` and `DOME_PARK_OPTION`. Call `InitPark()` once the dome is connected, as in the example above, to load the stored park status and position.

The `DOME_SHUTTER_PARK_POLICY` property lets users choose to close the shutter when parking and open it when unparking. The base class only stores the user's choice: your `Park()` and `UnPark()` must check `ShutterParkPolicySP` and operate the shutter, as in the example.

### Roll-Off Roofs

A roll-off roof is a dome that can only park (close) and unpark (open). Set `DOME_CAN_PARK` (and `DOME_CAN_ABORT` if the roof can be stopped), use `PARK_NONE`, and implement `Park()`, `UnPark()`, and `Move()`. `Park()` closes the roof and `UnPark()` opens it. Call `SetParked()` when the roof's limit switches report it is fully closed or open.

See the Roll-Off Simulator (`drivers/dome/roll_off.cpp`) and Universal ROR (`drivers/dome/universal_ror.cpp`) drivers for complete examples.

### Mount Locking Policy

`MOUNT_POLICY`, set by the user, controls how the dome interacts with the mount:

- `MOUNT_IGNORED`: The dome can park whatever the mount's state.
- `MOUNT_LOCKS`: The dome cannot park while the mount is unparked. The base class enforces this by checking `isLocked()` before calling your `Park()`.

The mount is the one named in `ACTIVE_TELESCOPE` in the `ACTIVE_DEVICES` property.

### Slaving

For domes that can move to an absolute azimuth, the base class can keep the dome slit in front of the telescope. It snoops the active mount's coordinates and calls `MoveAbs()` when:

- The mount starts slewing to a new target (`TARGET_EOD_COORD`), or
- The difference between the dome and mount azimuth exceeds the auto-sync threshold set in `DOME_PARAMS` (0.5 degrees by default).

The user enables slaving with `DOME_AUTOSYNC`. To account for the mount not being at the dome's center, the user enters the dome geometry in `DOME_MEASUREMENTS` (dome radius, shutter width, mount displacement north, east, and up from the dome center, and the distance from the optical axis to the mount center, all in meters), and the side of the mount the telescope is on in `DM_OTA_SIDE`. You do not need to implement anything for slaving, other than `MoveAbs()`.

### Backlash

For domes with backlash compensation in the hardware, set `DOME_HAS_BACKLASH` and implement `SetBacklash()`. The default `SetBacklashEnabled()` calls `SetBacklash()` with the configured steps when enabled, and with 0 when disabled.

## Best Practices

- **Never block** in a motion or shutter method: start the operation, return `IPS_BUSY`, and report completion from `TimerHit()`.
- **Do not change property states** in methods called by the base class. Return the right `IPState` and let the base class update them.
- **Always finish what you start**: Every `IPS_BUSY` must eventually lead to `setDomeState()`, `SetParked()`, or `setShutterState()`.
- **Do not modify properties from other threads.** If you use a separate thread, hand results back to `TimerHit()` instead.
- **Use the connection plugins** and `PortFD` instead of opening the port yourself.
- **Support simulation**: Check `isSimulation()` so the driver can be tested without hardware.

## Real-World Examples

The [INDI library](https://github.com/indilib/indi/tree/master/drivers/dome) contains many dome drivers that can serve as references:

- **Dome Simulator** (`dome_simulator.cpp`): An absolute position dome with a shutter.
- **Roll-Off Simulator** (`roll_off.cpp`): A roll-off roof.
- **NexDome Beaver** (`nexdome_beaver.cpp`): A full-featured production dome driver with a shutter and TCP/UDP support.
