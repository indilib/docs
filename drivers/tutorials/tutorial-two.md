---
title: Tutorial Two - Simple Telescope
nav_order: 2
parent: INDI Tutorials
grand_parent: Driver Development
---

# Tutorial Two: Simple Telescope

This tutorial demonstrates how to create a simple telescope simulator that can perform GOTO operations. It builds on the concepts introduced in [Tutorial One](tutorial-one.md) and introduces more advanced INDI functionality.

## What You'll Learn

- How to create a telescope driver by inheriting from `INDI::Telescope`
- How to implement GOTO functionality
- How to simulate telescope movement
- How to report telescope status to clients

## Source Code

### simplescope.h

```cpp
/*
   INDI Developers Manual
   Tutorial #2

   "Simple Telescope Driver"

   We develop a simple telescope simulator.

   Refer to README which contains instruction on how to build this driver and use it
   with an INDI-compatible client.

*/

/** \file simplescope.h
    \brief Construct a basic INDI telescope device that simulates GOTO commands.
    \author Jasem Mutlaq

    \example simplescope.h
    A simple GOTO telescope that simulator slewing operation.
*/

#pragma once

#include "inditelescope.h"

class SimpleScope : public INDI::Telescope
{
    public:
        SimpleScope();

    protected:
        bool Handshake() override;

        const char *getDefaultName() override;
        bool initProperties() override;

        // Telescope specific functions
        bool ReadScopeStatus() override;
        bool Goto(double, double) override;
        bool Abort() override;

    private:
        double currentRA {0};
        double currentDEC {90};
        double targetRA {0};
        double targetDEC {0};

        // Debug channel to write mount logs to
        // Default INDI::Logger debugging/logging channel are Message, Warn, Error, and Debug
        // Since scope information can be _very_ verbose we create another channel SCOPE specifically
        // for extra debug logs. This way the user can turn it on/off as desired.
        uint8_t DBG_SCOPE { INDI::Logger::DBG_IGNORE };

        // slew rate degrees/s
        static const uint8_t SLEW_RATE = 3;
};
```

### simplescope.cpp

```cpp
/*
   INDI Developers Manual
   Tutorial #2

   "Simple Telescope Driver"

   We develop a simple telescope simulator.

   Refer to README which contains instruction on how to build this driver and use it
   with an INDI-compatible client.

*/

/** \file simplescope.cpp
    \brief Construct a basic INDI telescope device that simulates GOTO commands.
    \author Jasem Mutlaq

    \example simplescope.cpp
    A simple GOTO telescope that simulator slewing operation.
*/

#include "simplescope.h"

#include "indicom.h"

#include <cmath>
#include <memory>

static std::unique_ptr<SimpleScope> simpleScope(new SimpleScope());

SimpleScope::SimpleScope()
{
    // We add an additional debug level so we can log verbose scope status
    DBG_SCOPE = INDI::Logger::getInstance().addDebugLevel("Scope Verbose", "SCOPE");
}

/**************************************************************************************
** We init our properties here. The only thing we want to init are the Debug controls
***************************************************************************************/
bool SimpleScope::initProperties()
{
    // ALWAYS call initProperties() of parent first
    INDI::Telescope::initProperties();

    // Add Debug control so end user can turn debugging/logging on and off
    addDebugControl();

    // Enable simulation mode so that serial connection in INDI::Telescope does not try
    // to attempt to perform a physical connection to the serial port.
    setSimulation(true);

    // Set telescope capabilities. 0 is for the the number of slew rates that we support. We have none for this simple driver.
    SetTelescopeCapability(TELESCOPE_CAN_GOTO | TELESCOPE_CAN_ABORT, 0);

    return true;
}

/**************************************************************************************
** INDI is asking us to check communication with the device via a handshake
***************************************************************************************/
bool SimpleScope::Handshake()
{
    // When communicating with a real mount we check here if commands are received
    // and acknolowedged by the mount. For SimpleScope we simply return true.
    return true;
}

/**************************************************************************************
** INDI is asking us for our default device name
***************************************************************************************/
const char *SimpleScope::getDefaultName()
{
    return "Simple Scope";
}

/**************************************************************************************
** Client is asking us to slew to a new position
***************************************************************************************/
bool SimpleScope::Goto(double ra, double dec)
{
    targetRA  = ra;
    targetDEC = dec;
    char RAStr[64] = {0}, DecStr[64] = {0};

    // Parse the RA/DEC into strings
    fs_sexa(RAStr, targetRA, 2, 3600);
    fs_sexa(DecStr, targetDEC, 2, 3600);

    // Mark state as slewing
    TrackState = SCOPE_SLEWING;

    // Inform client we are slewing to a new position
    LOGF_INFO("Slewing to RA: %s - DEC: %s", RAStr, DecStr);

    // Success!
    return true;
}

/**************************************************************************************
** Client is asking us to abort our motion
***************************************************************************************/
bool SimpleScope::Abort()
{
    return true;
}

/**************************************************************************************
** Client is asking us to report telescope status
***************************************************************************************/
bool SimpleScope::ReadScopeStatus()
{
    static struct timeval ltv
    {
        0, 0
    };
    struct timeval tv
    {
        0, 0
    };
    double dt = 0, da_ra = 0, da_dec = 0, dx = 0, dy = 0;
    int nlocked;

    /* update elapsed time since last poll, don't presume exactly POLLMS */
    gettimeofday(&tv, nullptr);

    if (ltv.tv_sec == 0 && ltv.tv_usec == 0)
        ltv = tv;

    dt  = tv.tv_sec - ltv.tv_sec + (tv.tv_usec - ltv.tv_usec) / 1e6;
    ltv = tv;

    // Calculate how much we moved since last time
    da_ra  = SLEW_RATE * dt;
    da_dec = SLEW_RATE * dt;

    /* Process per current state. We check the state of EQUATORIAL_EOD_COORDS_REQUEST and act accordingly */
    switch (TrackState)
    {
        case SCOPE_SLEWING:
            // Wait until we are "locked" into position for both RA & DEC axis
            nlocked = 0;

            // Calculate diff in RA
            dx = targetRA - currentRA;

            // If diff is very small i.e. smaller than how much we changed since last time then we reached target RA.
            if (fabs(dx) * 15. <= da_ra)
            {
                currentRA = targetRA;
                nlocked++;
            }
            // Otherwise increase RA
            else if (dx > 0)
                currentRA += da_ra / 15.;
            // Otherwise decrease RA
            else
                currentRA -= da_ra / 15.;

            // Calculate diff in DEC
            dy = targetDEC - currentDEC;

            // If diff is very small i.e. smaller than how much we changed since last time then we reached target DEC.
            if (fabs(dy) <= da_dec)
            {
                currentDEC = targetDEC;
                nlocked++;
            }
            // Otherwise increase DEC
            else if (dy > 0)
                currentDEC += da_dec;
            // Otherwise decrease DEC
            else
                currentDEC -= da_dec;

            // Let's check if we reached position for both RA/DEC
            if (nlocked == 2)
            {
                // Let's set state to TRACKING
                TrackState = SCOPE_TRACKING;

                LOG_INFO("Telescope slew is complete. Tracking...");
            }
            break;

        default:
            break;
    }

    char RAStr[64] = {0}, DecStr[64] = {0};

    // Parse the RA/DEC into strings
    fs_sexa(RAStr, currentRA, 2, 3600);
    fs_sexa(DecStr, currentDEC, 2, 3600);

    DEBUGF(DBG_SCOPE, "Current RA: %s Current DEC: %s", RAStr, DecStr);

    NewRaDec(currentRA, currentDEC);
    return true;
}
```

## Code Explanation

This driver simulates a simple telescope that can perform GOTO operations. Let's break down the key components:

1. **Class Definition**: The `SimpleScope` class inherits from `INDI::Telescope`, which provides the basic functionality required for telescope devices.

2. **Properties Initialization**:

   - In `initProperties()`, we initialize the telescope properties and set the telescope capabilities.
   - We enable simulation mode to avoid attempting physical connections.
   - We set the telescope capabilities to indicate it can perform GOTO operations and abort slews.

3. **GOTO Implementation**:

   - The `Goto(double ra, double dec)` function is called when a client requests a slew to a new position.
   - It stores the target coordinates and sets the telescope state to `SCOPE_SLEWING`.

4. **Status Reporting**:

   - The `ReadScopeStatus()` function is called periodically to update the telescope status.
   - It simulates the telescope movement by gradually changing the current coordinates towards the target coordinates.
   - When the telescope reaches the target position, it changes the state to `SCOPE_TRACKING`.

5. **Debugging**:
   - We create a custom debug channel (`DBG_SCOPE`) for verbose telescope status logging.
   - This allows users to enable detailed logging specifically for telescope operations.

## Running the Tutorial

To run this tutorial:

1. Build the tutorial using CMake:

   ```bash
   cd /path/to/indi/build
   make
   ```

2. Run the INDI server with the tutorial driver:

   ```bash
   indiserver -v ./tutorial_two
   ```

3. Connect to the server using an INDI client (like KStars/EKOS or INDI Control Panel).

4. You should see the "Simple Scope" in the client, and you can connect to it and send GOTO commands.

## Next Steps

This tutorial demonstrates a simple telescope simulator. In [Tutorial Three](tutorial-three.md), we'll build a CCD simulator that can generate and transmit FITS images.
