---
title: Telescope Interface
nav_order: 2
parent: Device Interfaces
---

# Implementing the Telescope Interface

This guide provides a comprehensive overview of implementing the telescope (mount) interface in INDI drivers. It covers the structure of a mount driver, the capabilities a mount can advertise, the methods you need to implement, and how to report the mount's position and state to clients.

## Introduction to the Telescope Interface

The telescope interface is implemented by inheriting from the `INDI::Telescope` base class (defined in `inditelescope.h`). It provides a standardized way for clients to slew, sync, track, park, and manually move a mount.

`INDI::Telescope` handles most of the work that is common to all mounts:

- Defining the standard coordinate, motion, tracking, parking, time, and location properties
- Validating client requests and calling the matching virtual method in your driver
- Serial and TCP [connection plugins](../drivers/communications/connection-plugins.md), and calling your `Handshake()` after connecting
- Polling the mount by calling `ReadScopeStatus()` at the polling period
- Storing park positions and status in `~/.indi/ParkData.xml`
- Joystick control, dome locking policy, and pier side simulation

Your driver only needs to talk to the mount: send commands and read back its position and state.

## Prerequisites

Before implementing the telescope interface, you should have:

- Basic knowledge of C++ programming
- Understanding of the INDI protocol and architecture
- Familiarity with the mount's communication protocol
- Development environment set up (compiler, build tools, etc.)
- INDI library installed

## Telescope Interface Structure

The telescope interface consists of several key components:

- **Base Class**: `INDI::Telescope` inherits from `INDI::DefaultDevice`.
- **Capabilities**: A bitmask that tells the base class which features your mount supports.
- **Mount State**: `TrackState` holds the current state of the mount, which your driver keeps up to date.
- **Virtual Methods**: Methods that your driver overrides to command the mount.

### TelescopeCapability Enum

Call `SetTelescopeCapability(capabilities, slewRateCount)` in your driver's constructor to declare what your mount can do:

```cpp
enum
{
    TELESCOPE_CAN_GOTO                 = 1 << 0,  /** Can the telescope go to specific coordinates? */
    TELESCOPE_CAN_SYNC                 = 1 << 1,  /** Can the telescope sync to specific coordinates? */
    TELESCOPE_CAN_PARK                 = 1 << 2,  /** Can the telescope park? */
    TELESCOPE_CAN_ABORT                = 1 << 3,  /** Can the telescope abort motion? */
    TELESCOPE_HAS_TIME                 = 1 << 4,  /** Does the telescope have configurable date and time settings? */
    TELESCOPE_HAS_LOCATION             = 1 << 5,  /** Does the telescope have configuration location settings? */
    TELESCOPE_HAS_PIER_SIDE            = 1 << 6,  /** Does the telescope have pier side property? */
    TELESCOPE_HAS_PEC                  = 1 << 7,  /** Does the telescope have PEC playback? */
    TELESCOPE_HAS_TRACK_MODE           = 1 << 8,  /** Does the telescope have track modes (sidereal, lunar, solar..etc)? */
    TELESCOPE_CAN_CONTROL_TRACK        = 1 << 9,  /** Can the telescope engage and disengage tracking? */
    TELESCOPE_HAS_TRACK_RATE           = 1 << 10, /** Does the telescope have custom track rates? */
    TELESCOPE_HAS_PIER_SIDE_SIMULATION = 1 << 11, /** Does the telescope simulate the pier side property? */
    TELESCOPE_CAN_TRACK_SATELLITE      = 1 << 12, /** Can the telescope track satellites? */
    TELESCOPE_CAN_FLIP                 = 1 << 13, /** Does the telescope have a command for flipping? */
    TELESCOPE_CAN_HOME_FIND            = 1 << 14, /** Can the telescope find home position? */
    TELESCOPE_CAN_HOME_SET             = 1 << 15, /** Can the telescope set the current position as the new home position? */
    TELESCOPE_CAN_HOME_GO              = 1 << 16, /** Can the telescope slew to home position? */
} TelescopeCapability;
```

`slewRateCount` is the number of manual slew rates the mount supports. If it is 4 or more, the base class creates `TELESCOPE_SLEW_RATE` with that many rates, from slowest to fastest. If it is less than 4, no slew rate property is defined. Manual motion and slew rate properties are only defined for mounts with `TELESCOPE_CAN_GOTO`.

| Capability                        | Properties defined                                   | Methods to override                  |
| --------------------------------- | ---------------------------------------------------- | ------------------------------------ |
| _(always)_                        | `EQUATORIAL_EOD_COORD`, `TELESCOPE_MOUNT_TYPE`       | `ReadScopeStatus()`                  |
| `TELESCOPE_CAN_GOTO`              | `ON_COORD_SET` (`TRACK`, `SLEW`), `TARGET_EOD_COORD`, `TELESCOPE_MOTION_NS`, `TELESCOPE_MOTION_WE`, `TELESCOPE_REVERSE_MOTION`, `TELESCOPE_SLEW_RATE` | `Goto()`, `MoveNS()`, `MoveWE()` |
| `TELESCOPE_CAN_SYNC`              | `ON_COORD_SET` (`SYNC`)                              | `Sync()`                             |
| `TELESCOPE_CAN_FLIP`              | `ON_COORD_SET` (`FLIP`)                              | `Flip()`                             |
| `TELESCOPE_CAN_ABORT`             | `TELESCOPE_ABORT_MOTION`                             | `Abort()`                            |
| `TELESCOPE_CAN_PARK`              | `TELESCOPE_PARK`, and `TELESCOPE_PARK_POSITION` and `TELESCOPE_PARK_OPTION` depending on the park data type | `Park()`, `UnPark()`, `SetCurrentPark()`, `SetDefaultPark()` |
| `TELESCOPE_HAS_TIME`              | `TIME_UTC`                                           | `updateTime()`                       |
| `TELESCOPE_HAS_LOCATION`          | `GEOGRAPHIC_COORD`                                   | `updateLocation()`                   |
| `TELESCOPE_HAS_TRACK_MODE`        | `TELESCOPE_TRACK_MODE`                               | `SetTrackMode()`                     |
| `TELESCOPE_CAN_CONTROL_TRACK`     | `TELESCOPE_TRACK_STATE`                              | `SetTrackEnabled()`                  |
| `TELESCOPE_HAS_TRACK_RATE`        | `TELESCOPE_TRACK_RATE`                               | `SetTrackRate()`                     |
| `TELESCOPE_HAS_PIER_SIDE`         | `TELESCOPE_PIER_SIDE`                                | Call `setPierSide()` in `ReadScopeStatus()` |
| `TELESCOPE_HAS_PEC`               | `PEC`                                                | Call `setPECState()`                 |
| `TELESCOPE_CAN_HOME_FIND`, `_SET`, `_GO` | `TELESCOPE_HOME`                              | `ExecuteHomeAction()`                |
| `TELESCOPE_CAN_TRACK_SATELLITE`   | `SAT_TLE_TEXT`, `SAT_PASS_WINDOW`, `SAT_TRACKING_STAT` | —                                  |

See [Standard Properties](../drivers/standard-properties.md#telescope-properties) for the elements of each property.

### Connection

By default, `INDI::Telescope` offers both serial and TCP connections. To change this, call `setTelescopeConnection()` in your constructor with `CONNECTION_SERIAL`, `CONNECTION_TCP`, both, or `CONNECTION_NONE`. After a successful connection, the base class sets `PortFD` to the open file descriptor, calls your `Handshake()`, and starts the polling timer.

You can change the defaults of the connection plugins in `initProperties()`, after calling `INDI::Telescope::initProperties()`, through the `serialConnection` and `tcpConnection` members.

### Mount State

`TrackState` holds the state of the mount, and must always reflect what the mount is doing:

| State            | Meaning                                       |
| ---------------- | --------------------------------------------- |
| `SCOPE_IDLE`     | Not moving and not tracking                   |
| `SCOPE_SLEWING`  | Slewing to a target                           |
| `SCOPE_TRACKING` | Tracking                                      |
| `SCOPE_PARKING`  | Moving to the park position                   |
| `SCOPE_PARKED`   | Parked                                        |

The base class uses `TrackState` to set the state of `EQUATORIAL_EOD_COORD` (busy while slewing or parking, OK while tracking, idle otherwise) and of `TELESCOPE_TRACK_STATE`. Clients rely on these states to know when a slew has finished.

## Key Methods

### Virtual Methods to Override

- `virtual bool ReadScopeStatus() = 0`:
  **Required.** Called by the base class at every polling period. Read the mount's position and status, update `TrackState`, and call `NewRaDec()`. Return `false` if communication with the mount failed.

- `virtual bool Handshake()`:
  Called after the connection is established. Verify that the mount responds, and return `false` if it does not.

- `virtual bool Goto(double ra, double dec)`:
  Slew to JNow RA (hours) and DEC (degrees). Start the slew, set `TrackState = SCOPE_SLEWING`, and return `true`. Do not wait for the slew to finish.

- `virtual bool Sync(double ra, double dec)`:
  Tell the mount that it is currently pointing at JNow RA and DEC.

- `virtual bool Flip(double ra, double dec)`:
  Slew to RA and DEC and force a meridian flip.

- `virtual bool Abort()`:
  Stop all motion. The base class resets the motion, coordinate, and park properties, and restores `TrackState` if the mount was slewing or parking.

- `virtual bool MoveNS(INDI_DIR_NS dir, TelescopeMotionCommand command)` / `MoveWE(INDI_DIR_WE dir, TelescopeMotionCommand command)`:
  Start (`MOTION_START`) or stop (`MOTION_STOP`) manual motion north/south or west/east. Use `SlewRateSP.findOnSwitchIndex()` to get the selected rate.

- `virtual bool Park()` / `UnPark()`:
  Start parking and set `TrackState = SCOPE_PARKING`, or unpark the mount and call `SetParked(false)`.

- `virtual bool SetCurrentPark()` / `SetDefaultPark()`:
  Called when the client chooses to use the current position or the default position as the park position. Update the park position with `SetAxis1Park()` and `SetAxis2Park()`.

- `virtual bool SetTrackMode(uint8_t mode)`:
  Set the tracking mode. `mode` is the index of the mode in `TELESCOPE_TRACK_MODE`, in the order the modes were added with `AddTrackMode()`.

- `virtual bool SetTrackEnabled(bool enabled)`:
  Start or stop tracking.

- `virtual bool SetTrackRate(double raRate, double deRate)`:
  Set custom tracking rates in arcseconds per second.

- `virtual bool SetSlewRate(int index)`:
  Called when the client changes `TELESCOPE_SLEW_RATE`. The default implementation does nothing and returns `true`. Override it only if the mount needs a command to change rate; most drivers read the rate when motion starts.

- `virtual bool updateTime(ln_date *utc, double utc_offset)`:
  Send the UTC date and time and UTC offset (hours) to the mount.

- `virtual bool updateLocation(double latitude, double longitude, double elevation)`:
  Send the site location to the mount. Longitude is in degrees, 0 to 360, increasing eastward.

- `virtual IPState ExecuteHomeAction(TelescopeHomeAction action)`:
  Execute `HOME_FIND`, `HOME_SET`, or `HOME_GO`. Return `IPS_OK` if complete, `IPS_BUSY` if in progress, or `IPS_ALERT` on failure.

### Helper Methods

- `void SetTelescopeCapability(uint32_t cap, uint8_t slewRateCount)`: Sets the capabilities and number of slew rates.
- `void setTelescopeConnection(const uint8_t &value)`: Sets the supported connection types.
- `void NewRaDec(double ra, double dec)`: Reports the current JNow position to clients. Call it from `ReadScopeStatus()`. It only sends an update when the position or state changed.
- `void setPierSide(TelescopePierSide side)`: Reports `PIER_EAST`, `PIER_WEST`, or `PIER_UNKNOWN`.
- `int AddTrackMode(const char *name, const char *label, bool isDefault)`: Adds a tracking mode. Use the standard names `TRACK_SIDEREAL`, `TRACK_SOLAR`, `TRACK_LUNAR`, and `TRACK_CUSTOM`.
- `void SetParkDataType(TelescopeParkData type)`: Sets how park positions are stored. See [Parking](#parking).
- `bool InitPark()`: Loads park data from `~/.indi/ParkData.xml`.
- `bool isParked()` / `void SetParked(bool isparked)`: Get or set the park status. `SetParked()` also updates `TrackState`, `TELESCOPE_PARK`, and the park data file.
- `SetAxis1Park()`, `SetAxis2Park()`, `SetAxis1ParkDefault()`, `SetAxis2ParkDefault()`, `GetAxis1Park()`, `GetAxis2Park()`: Set and get the current and default park positions.
- `bool isLocked()`: Returns `true` if the dome locking policy prevents the mount from unparking.

## Example Implementation

Here's a simplified example of a serial GOTO mount. It uses a hypothetical text protocol in which each command and reply ends with `#`; replace the commands with your mount's protocol.

### Header (`my_mount.h`)

```cpp
#pragma once

#include <libindi/inditelescope.h>

class MyMount : public INDI::Telescope
{
    public:
        MyMount();

        const char *getDefaultName() override;
        bool initProperties() override;
        bool updateProperties() override;

    protected:
        bool Handshake() override;
        bool ReadScopeStatus() override;

        bool Goto(double ra, double dec) override;
        bool Sync(double ra, double dec) override;
        bool Abort() override;
        bool MoveNS(INDI_DIR_NS dir, TelescopeMotionCommand command) override;
        bool MoveWE(INDI_DIR_WE dir, TelescopeMotionCommand command) override;
        bool Park() override;
        bool UnPark() override;
        bool SetTrackMode(uint8_t mode) override;
        bool SetTrackEnabled(bool enabled) override;
        bool updateTime(ln_date *utc, double utc_offset) override;
        bool updateLocation(double latitude, double longitude, double elevation) override;

    private:
        bool sendCommand(const char *cmd, char *res = nullptr, int reslen = 0);
};
```

### Implementation (`my_mount.cpp`)

```cpp
#include "my_mount.h"

#include <libindi/indicom.h>

#include <cmath>
#include <cstring>
#include <memory>
#include <termios.h>

static std::unique_ptr<MyMount> myMount(new MyMount());

MyMount::MyMount()
{
    setVersion(1, 0);

    // This mount only has a serial port.
    setTelescopeConnection(CONNECTION_SERIAL);

    SetTelescopeCapability(TELESCOPE_CAN_GOTO |
                           TELESCOPE_CAN_SYNC |
                           TELESCOPE_CAN_ABORT |
                           TELESCOPE_CAN_PARK |
                           TELESCOPE_HAS_TIME |
                           TELESCOPE_HAS_LOCATION |
                           TELESCOPE_HAS_PIER_SIDE |
                           TELESCOPE_HAS_TRACK_MODE |
                           TELESCOPE_CAN_CONTROL_TRACK,
                           4);
}

const char *MyMount::getDefaultName()
{
    return "My Mount";
}

bool MyMount::initProperties()
{
    INDI::Telescope::initProperties();

    // Tracking modes, in the order of the TelescopeTrackMode enum.
    AddTrackMode("TRACK_SIDEREAL", "Sidereal", true);
    AddTrackMode("TRACK_SOLAR", "Solar");
    AddTrackMode("TRACK_LUNAR", "Lunar");

    // The mount parks to its own stored position, so we only track the park status.
    SetParkDataType(PARK_SIMPLE);

    serialConnection->setDefaultBaudRate(Connection::Serial::B_115200);

    addAuxControls();
    return true;
}

bool MyMount::updateProperties()
{
    INDI::Telescope::updateProperties();

    if (isConnected())
    {
        // Load the park status from the park data file.
        InitPark();
    }

    return true;
}

bool MyMount::Handshake()
{
    if (isSimulation())
        return true;

    // PortFD is set by the base class before Handshake() is called.
    char res[32] = {0};
    if (!sendCommand("VERSION#", res, sizeof(res)))
    {
        LOG_ERROR("Mount is not responding. Check the port and baud rate.");
        return false;
    }

    LOGF_INFO("Connected to mount firmware %s.", res);
    return true;
}

bool MyMount::ReadScopeStatus()
{
    char res[64] = {0};

    // Reply format: "<RA hours> <DEC degrees> <status> <pier>#"
    if (!sendCommand("STATUS#", res, sizeof(res)))
        return false;

    double ra = 0, dec = 0;
    int status = 0, pier = 0;
    if (sscanf(res, "%lf %lf %d %d", &ra, &dec, &status, &pier) != 4)
    {
        LOGF_ERROR("Unexpected status reply: %s", res);
        return false;
    }

    // status: 0 = idle, 1 = slewing, 2 = tracking, 3 = parking, 4 = parked
    switch (status)
    {
        case 0:
            TrackState = SCOPE_IDLE;
            break;
        case 1:
            TrackState = SCOPE_SLEWING;
            break;
        case 2:
            if (TrackState == SCOPE_SLEWING)
                LOG_INFO("Slew complete, tracking...");
            TrackState = SCOPE_TRACKING;
            break;
        case 3:
            TrackState = SCOPE_PARKING;
            break;
        case 4:
            // SetParked() updates TrackState, TELESCOPE_PARK, and the park data file.
            if (!isParked())
                SetParked(true);
            break;
    }

    setPierSide(pier == 0 ? PIER_EAST : PIER_WEST);

    // Report the position. The base class sets the property state from TrackState.
    NewRaDec(ra, dec);
    return true;
}

bool MyMount::Goto(double ra, double dec)
{
    char cmd[64] = {0};
    snprintf(cmd, sizeof(cmd), "GOTO %.6f %.6f#", ra, dec);
    if (!sendCommand(cmd))
        return false;

    TrackState = SCOPE_SLEWING;

    char raStr[16], decStr[16];
    fs_sexa(raStr, ra, 2, 3600);
    fs_sexa(decStr, dec, 2, 3600);
    LOGF_INFO("Slewing to RA: %s - DEC: %s", raStr, decStr);
    return true;
}

bool MyMount::Sync(double ra, double dec)
{
    char cmd[64] = {0};
    snprintf(cmd, sizeof(cmd), "SYNC %.6f %.6f#", ra, dec);
    if (!sendCommand(cmd))
        return false;

    NewRaDec(ra, dec);
    return true;
}

bool MyMount::Abort()
{
    return sendCommand("STOP#");
}

bool MyMount::MoveNS(INDI_DIR_NS dir, TelescopeMotionCommand command)
{
    if (command == MOTION_STOP)
        return sendCommand("STOP_NS#");

    // Rates 0 (slowest) to 3 (fastest).
    char cmd[32] = {0};
    snprintf(cmd, sizeof(cmd), "MOVE_%s %d#", dir == DIRECTION_NORTH ? "N" : "S",
             SlewRateSP.findOnSwitchIndex());
    return sendCommand(cmd);
}

bool MyMount::MoveWE(INDI_DIR_WE dir, TelescopeMotionCommand command)
{
    if (command == MOTION_STOP)
        return sendCommand("STOP_WE#");

    char cmd[32] = {0};
    snprintf(cmd, sizeof(cmd), "MOVE_%s %d#", dir == DIRECTION_WEST ? "W" : "E",
             SlewRateSP.findOnSwitchIndex());
    return sendCommand(cmd);
}

bool MyMount::Park()
{
    if (!sendCommand("PARK#"))
        return false;

    // ReadScopeStatus() calls SetParked(true) when the mount reports it is parked.
    TrackState = SCOPE_PARKING;
    LOG_INFO("Parking...");
    return true;
}

bool MyMount::UnPark()
{
    if (!sendCommand("UNPARK#"))
        return false;

    SetParked(false);
    return true;
}

bool MyMount::SetTrackMode(uint8_t mode)
{
    // mode is TRACK_SIDEREAL, TRACK_SOLAR, or TRACK_LUNAR, in the order added above.
    char cmd[32] = {0};
    snprintf(cmd, sizeof(cmd), "TRACK_MODE %d#", mode);
    return sendCommand(cmd);
}

bool MyMount::SetTrackEnabled(bool enabled)
{
    return sendCommand(enabled ? "TRACK_ON#" : "TRACK_OFF#");
}

bool MyMount::updateTime(ln_date *utc, double utc_offset)
{
    char cmd[64] = {0};
    snprintf(cmd, sizeof(cmd), "TIME %04d-%02d-%02dT%02d:%02d:%02.0f %.2f#",
             utc->years, utc->months, utc->days, utc->hours, utc->minutes, utc->seconds, utc_offset);
    return sendCommand(cmd);
}

bool MyMount::updateLocation(double latitude, double longitude, double elevation)
{
    // The mount expects longitude from -180 to 180.
    if (longitude > 180)
        longitude -= 360;

    char cmd[64] = {0};
    snprintf(cmd, sizeof(cmd), "SITE %.6f %.6f %.1f#", latitude, longitude, elevation);
    return sendCommand(cmd);
}

bool MyMount::sendCommand(const char *cmd, char *res, int reslen)
{
    if (isSimulation())
        return true;

    int nbytes_written = 0, nbytes_read = 0;
    char errstr[MAXRBUF] = {0};

    tcflush(PortFD, TCIOFLUSH);

    LOGF_DEBUG("CMD <%s>", cmd);
    int rc = tty_write_string(PortFD, cmd, &nbytes_written);
    if (rc != TTY_OK)
    {
        tty_error_msg(rc, errstr, MAXRBUF);
        LOGF_ERROR("Write error: %s", errstr);
        return false;
    }

    // Every command is acknowledged with a reply ending in '#'.
    char buffer[128] = {0};
    rc = tty_read_section(PortFD, buffer, '#', 3, &nbytes_read);
    if (rc != TTY_OK)
    {
        tty_error_msg(rc, errstr, MAXRBUF);
        LOGF_ERROR("Read error: %s", errstr);
        return false;
    }

    buffer[nbytes_read - 1] = '\0'; // Remove the terminator
    LOGF_DEBUG("RES <%s>", buffer);

    if (res && reslen > 0)
        snprintf(res, reslen, "%s", buffer);

    return true;
}
```

There is no `main()` function and no `TimerHit()` override: drivers are linked against `libindidriver`, which provides `main()`, and the base class's `TimerHit()` calls `ReadScopeStatus()` at every polling period. See [Creating Custom INDI Drivers](../drivers/basics/custom-driver.md) for the CMake setup and driver XML file.

## Advanced Topics

### Parking

`SetParkDataType()` selects how the park position is stored and presented to the user. Call it in `initProperties()`:

| Park data type        | Park position                                                        |
| --------------------- | -------------------------------------------------------------------- |
| `PARK_NONE`           | No custom park position.                                             |
| `PARK_SIMPLE`         | Only the park status is stored. The mount manages its own park position. |
| `PARK_HA_DEC`         | Hour angle (hours) and declination (degrees).                        |
| `PARK_AZ_ALT`         | Azimuth and altitude (degrees).                                      |
| `PARK_RA_DEC_ENCODER` | Hour angle and declination axis encoder steps.                       |
| `PARK_AZ_ALT_ENCODER` | Azimuth and altitude axis encoder steps.                             |
| `PARK_RA_DEC`         | Deprecated. Do not use.                                              |

For every type except `PARK_NONE` and `PARK_SIMPLE`, the base class defines `TELESCOPE_PARK_POSITION` and `TELESCOPE_PARK_OPTION`, and stores the position in `~/.indi/ParkData.xml`. Load it when the mount connects, and provide defaults:

```cpp
if (InitPark())
{
    // Park data was loaded. Only set the default park position.
    SetAxis1ParkDefault(-6);   // HA, hours
    SetAxis2ParkDefault(90);   // DEC, degrees
}
else
{
    // No park data yet. Set both the current and default park position.
    SetAxis1Park(-6);
    SetAxis2Park(90);
    SetAxis1ParkDefault(-6);
    SetAxis2ParkDefault(90);
}
```

`Park()` then slews to `GetAxis1Park()` and `GetAxis2Park()`, converted to the mount's coordinates. Implement `SetCurrentPark()` and `SetDefaultPark()` so users can change the park position:

```cpp
bool MyMount::SetCurrentPark()
{
    // Convert the current RA to hour angle (-12 to +12 hours).
    double lst = get_local_sidereal_time(LocationNP[LOCATION_LONGITUDE].getValue());
    double ha  = rangeHA(lst - EqNP[AXIS_RA].getValue());

    SetAxis1Park(ha);
    SetAxis2Park(EqNP[AXIS_DE].getValue());
    return true;
}

bool MyMount::SetDefaultPark()
{
    SetAxis1Park(GetAxis1ParkDefault());
    SetAxis2Park(GetAxis2ParkDefault());
    return true;
}
```

### Tracking Modes and Rates

Add tracking modes with `AddTrackMode()` in `initProperties()`, in the order of the `TelescopeTrackMode` enum (`TRACK_SIDEREAL`, `TRACK_SOLAR`, `TRACK_LUNAR`, `TRACK_CUSTOM`), and mark one as the default. `SetTrackMode()` receives the index of the selected mode.

If the mount supports custom rates, set `TELESCOPE_HAS_TRACK_RATE` and add a `TRACK_CUSTOM` mode. The base class calls `SetTrackRate()` with the RA and DEC rates in arcseconds per second when the client changes `TELESCOPE_TRACK_RATE`. The base class rejects rate changes that would reverse direction while tracking.

### Pier Side

If the mount reports which side of the pier it is on, set `TELESCOPE_HAS_PIER_SIDE` and call `setPierSide()` from `ReadScopeStatus()`. Clients use the pier side to handle meridian flips.

If the mount does not report it, set `TELESCOPE_HAS_PIER_SIDE_SIMULATION` instead, and the base class calculates the expected pier side from the hour angle.

### Pulse Guiding

To support guiding from clients such as PHD2 or Ekos, also inherit from `INDI::GuiderInterface`. See the [Guider Interface](guider.md).

### Alignment

For better GOTO accuracy, especially with Alt-Az mounts and imperfectly polar-aligned equatorial mounts, a driver can use the INDI Alignment Subsystem. It stores sync points and uses math plugins to transform between celestial and mount coordinates. See the [INDI Alignment Subsystem](alignment-subsystem.md).

### Custom Properties

You can add properties that are specific to your mount. For example, a guide rate:

```cpp
// In the class declaration
INDI::PropertyNumber GuideRateNP {2};

// In initProperties()
GuideRateNP[0].fill("GUIDE_RATE_WE", "W/E Rate", "%.1f", 0, 1, 0.1, 0.5);
GuideRateNP[1].fill("GUIDE_RATE_NS", "N/S Rate", "%.1f", 0, 1, 0.1, 0.5);
GuideRateNP.fill(getDeviceName(), "GUIDE_RATE", "Guiding Rate", MOTION_TAB, IP_RW, 0, IPS_IDLE);

// In updateProperties()
if (isConnected())
    defineProperty(GuideRateNP);
else
    deleteProperty(GuideRateNP);

// In ISNewNumber(), before calling INDI::Telescope::ISNewNumber()
if (dev != nullptr && !strcmp(dev, getDeviceName()) && GuideRateNP.isNameMatch(name))
{
    char cmd[32] = {0};
    snprintf(cmd, sizeof(cmd), "GUIDE_RATE %.1f %.1f#", values[0], values[1]);
    if (sendCommand(cmd))
    {
        GuideRateNP.update(values, names, n);
        GuideRateNP.setState(IPS_OK);
    }
    else
        GuideRateNP.setState(IPS_ALERT);

    GuideRateNP.apply();
    return true;
}
```

## Best Practices

- **Never block** in `Goto()`, `Park()`, or other commands: start the operation, set `TrackState`, and let `ReadScopeStatus()` report progress.
- **Keep `TrackState` accurate**: Clients wait for `EQUATORIAL_EOD_COORD` to leave the busy state before starting the next step of a sequence.
- **Report JNow coordinates**: `Goto()`, `Sync()`, and `NewRaDec()` all use JNow (epoch of date) RA in hours and DEC in degrees.
- **Use the connection plugins** and `PortFD` instead of opening the port yourself.
- **Support simulation**: Check `isSimulation()` so the driver can be tested without hardware.

## Real-World Examples

The [INDI library](https://github.com/indilib/indi/tree/master/drivers/telescope) contains many mount drivers that can serve as references:

- **Telescope Simulator** (`telescope_simulator.cpp`): Implements nearly every feature of the interface, including parking, tracking modes, pier side, and guiding.
- **LX200 Telescope** (`lx200telescope.cpp`): The base for the many LX200-compatible mount drivers.
- **iOptron** (`ieqpro.cpp`): A full-featured equatorial mount driver.
