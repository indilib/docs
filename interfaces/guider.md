---
title: Guider Interface
nav_order: 4
parent: Device Interfaces
---

# Implementing the Guider Interface

This guide explains how to implement the guider interface, which lets clients send timed guide pulses to a device with an ST4 guide port or an equivalent pulse-guiding command.

## Introduction to the Guider Interface

The INDI Guider Interface (`INDI::GuiderInterface`, aliased as `GI`) provides a standardized way for clients such as PHD2 and KStars/Ekos to send guide corrections. Each correction is a pulse of a given duration, in milliseconds, in one of four directions:

| Direction | Axis              | Method         |
| --------- | ----------------- | -------------- |
| North     | Declination +     | `GuideNorth()` |
| South     | Declination −     | `GuideSouth()` |
| East      | Right Ascension + | `GuideEast()`  |
| West      | Right Ascension − | `GuideWest()`  |

The guider interface is rarely a device on its own. It is usually added to:

- **Mounts** that support pulse guiding over their serial or network protocol
- **Cameras** with an ST4 port. `INDI::CCD` already includes the guider interface; set the `CCD_HAS_ST4_PORT` capability and implement the four `Guide*()` methods. See the [Camera Interface](ccd-interface.md).
- **Standalone ST4 adapters**, such as the Arduino ST4 driver

## Prerequisites

Before implementing the guider interface, you should have:

- Basic knowledge of C++ programming
- Understanding of the INDI protocol and architecture
- Familiarity with the device's communication protocol
- Development environment set up (compiler, build tools, etc.)
- INDI library installed

## Standard Properties

| Property                   | Type   | Elements                         | Description                                          |
| -------------------------- | ------ | -------------------------------- | ---------------------------------------------------- |
| `TELESCOPE_TIMED_GUIDE_NS` | Number | `TIMED_GUIDE_N`, `TIMED_GUIDE_S` | Guide north or south for this many milliseconds      |
| `TELESCOPE_TIMED_GUIDE_WE` | Number | `TIMED_GUIDE_W`, `TIMED_GUIDE_E` | Guide west or east for this many milliseconds        |

A client sets one element of a property to a non-zero duration to send a pulse. The members are `GuideNSNP` and `GuideWENP`.

## Key Methods

### Interface Methods

Call these from your driver:

| Function               | Where to call it from your driver                               |
| ---------------------- | --------------------------------------------------------------- |
| `GI::initProperties()` | `initProperties()`, passing the group (tab) for the properties  |
| `GI::updateProperties()` | `updateProperties()`, both when connected and when disconnected |
| `GI::processNumber()`  | `ISNewNumber()`. It returns `true` if it handled the property.  |

Also add `GUIDER_INTERFACE` to your driver interface flags with `setDriverInterface()`, so clients know the device can guide.

### Virtual Methods to Override

```cpp
virtual IPState GuideNorth(uint32_t ms) = 0;
virtual IPState GuideSouth(uint32_t ms) = 0;
virtual IPState GuideEast(uint32_t ms) = 0;
virtual IPState GuideWest(uint32_t ms) = 0;
```

Each method sends a pulse of `ms` milliseconds and returns:

| Return value | Meaning                                                                                                   |
| ------------ | --------------------------------------------------------------------------------------------------------- |
| `IPS_OK`     | The pulse is complete. Appropriate when the method waits for very short pulses, or the device times the pulse itself and does not report completion. |
| `IPS_BUSY`   | The pulse has started and is in progress. You must call `GuideComplete()` when it ends.                    |
| `IPS_ALERT`  | The pulse failed.                                                                                         |

- `virtual void GuideComplete(INDI_EQ_AXIS axis)`:
  Call it when a pulse that returned `IPS_BUSY` has finished. Pass `AXIS_DE` for north/south pulses and `AXIS_RA` for east/west pulses. It sets the corresponding guide property back to idle, which tells the client it can send the next pulse.

## Example Implementation

Here's a simplified standalone ST4 adapter. It starts a pulse, returns `IPS_BUSY`, and uses an `INDI::Timer` to stop the pulse and call `GuideComplete()`. The hypothetical `sendCommand()` sends a command to the device.

```cpp
#include <libindi/defaultdevice.h>
#include <libindi/indiguiderinterface.h>
#include <libindi/inditimer.h>

#include <cstring>

class MyST4 : public INDI::DefaultDevice, public INDI::GuiderInterface
{
    public:
        MyST4() : INDI::GuiderInterface(this)
        {
            setVersion(1, 0);

            m_NSTimer.setSingleShot(true);
            m_WETimer.setSingleShot(true);

            // Stop the pulse and tell the client it is complete.
            m_NSTimer.callOnTimeout([this]()
            {
                sendCommand("STOP_NS");
                GuideComplete(AXIS_DE);
            });
            m_WETimer.callOnTimeout([this]()
            {
                sendCommand("STOP_WE");
                GuideComplete(AXIS_RA);
            });
        }

        const char *getDefaultName() override
        {
            return "My ST4";
        }

        bool initProperties() override
        {
            INDI::DefaultDevice::initProperties();

            GI::initProperties(MAIN_CONTROL_TAB);
            setDriverInterface(AUX_INTERFACE | GUIDER_INTERFACE);

            addAuxControls();
            return true;
        }

        bool updateProperties() override
        {
            INDI::DefaultDevice::updateProperties();
            GI::updateProperties();
            return true;
        }

        bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) override
        {
            if (GI::processNumber(dev, name, values, names, n))
                return true;

            return INDI::DefaultDevice::ISNewNumber(dev, name, values, names, n);
        }

        IPState GuideNorth(uint32_t ms) override
        {
            return startPulse("NORTH", ms, m_NSTimer);
        }

        IPState GuideSouth(uint32_t ms) override
        {
            return startPulse("SOUTH", ms, m_NSTimer);
        }

        IPState GuideEast(uint32_t ms) override
        {
            return startPulse("EAST", ms, m_WETimer);
        }

        IPState GuideWest(uint32_t ms) override
        {
            return startPulse("WEST", ms, m_WETimer);
        }

    private:
        IPState startPulse(const char *direction, uint32_t ms, INDI::Timer &timer)
        {
            // A new pulse on the same axis replaces the previous one.
            timer.stop();

            if (!sendCommand(direction))
                return IPS_ALERT;

            timer.start(ms);
            return IPS_BUSY;
        }

        bool sendCommand(const char *cmd);

        INDI::Timer m_NSTimer;
        INDI::Timer m_WETimer;
};
```

## Adding Guiding to a Mount

To add pulse guiding to a mount driver, inherit from `INDI::GuiderInterface` alongside `INDI::Telescope` in the same way, and add `GUIDER_INTERFACE` to the driver interface:

```cpp
class MyMount : public INDI::Telescope, public INDI::GuiderInterface
{
    public:
        MyMount() : INDI::GuiderInterface(this) {}

        bool initProperties() override
        {
            INDI::Telescope::initProperties();
            GI::initProperties(GUIDE_TAB);
            setDriverInterface(getDriverInterface() | GUIDER_INTERFACE);
            return true;
        }

        // Call GI::updateProperties() from updateProperties(), and GI::processNumber()
        // from ISNewNumber() before passing the request to INDI::Telescope.
};
```

If the mount's protocol has a pulse-guide command that times the pulse itself, send the command and return `IPS_OK`. Otherwise, time the pulse in the driver as in the example above.

## Real-World Examples

The [INDI library](https://github.com/indilib/indi) contains several guider implementations:

- **Arduino ST4** (`drivers/auxiliary/arduino_st4.cpp`): A minimal standalone ST4 adapter.
- **LX200 Telescope** (`drivers/telescope/lx200telescope.cpp`): Pulse guiding over a mount protocol, shared by the LX200-based mount drivers.
- **CCD Simulator** (`drivers/ccd/ccd_simulator.cpp`): Guiding through a camera's ST4 port.
