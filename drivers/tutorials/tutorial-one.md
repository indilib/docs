---
title: Tutorial One - Hello INDI
nav_order: 1
parent: INDI Tutorials
grand_parent: Driver Development
---

# Tutorial One: Hello INDI

This tutorial demonstrates how to create the most basic INDI device driver. It's a minimal implementation that only provides the essential functionality required by the INDI protocol.

## What You'll Learn

- How to create a basic INDI device driver
- The minimal required functions for an INDI driver
- How to handle connect and disconnect operations

## Source Code

### simpledevice.h

```cpp
/*
INDI Developers Manual
   Tutorial #1

   "Hello INDI"

   We construct a most basic (and useless) device driver to illustate INDI.

   Refer to README which contains instruction on how to build this driver and use it
   with an INDI-compatible client.

*/

/** \file simpledevice.h
    \brief Construct a basic INDI device with only one property to connect and disconnect.
    \author Jasem Mutlaq

    \example simpledevice.h
    A very minimal device! It also allows you to connect/disconnect and performs no other functions.
*/

#pragma once

#include "defaultdevice.h"

class SimpleDevice : public INDI::DefaultDevice
{
    public:
        SimpleDevice() = default;

    protected:
        bool Connect() override;
        bool Disconnect() override;
        const char *getDefaultName() override;
};
```

### simpledevice.cpp

```cpp
/*
INDI Developers Manual
   Tutorial #1

   "Hello INDI"

   We construct a most basic (and useless) device driver to illustrate INDI.

   Refer to README which contains instruction on how to build this driver and use it
   with an INDI-compatible client.

*/

/** \file simpledevice.cpp
    \brief Construct a basic INDI device with only one property to connect and disconnect.
    \author Jasem Mutlaq

    \example simpledevice.cpp
    A very minimal device! It also allows you to connect/disconnect and performs no other functions.
*/

#include "simpledevice.h"

#include <memory>

std::unique_ptr<SimpleDevice> simpleDevice(new SimpleDevice());

/**************************************************************************************
** Client is asking us to establish connection to the device
***************************************************************************************/
bool SimpleDevice::Connect()
{
    LOG_INFO("Simple device connected successfully!");
    return true;
}

/**************************************************************************************
** Client is asking us to terminate connection to the device
***************************************************************************************/
bool SimpleDevice::Disconnect()
{
    LOG_INFO("Simple device disconnected successfully!");
    return true;
}

/**************************************************************************************
** INDI is asking us for our default device name
***************************************************************************************/
const char *SimpleDevice::getDefaultName()
{
    return "Simple Device";
}
```

## Code Explanation

This driver is the simplest possible INDI device. Let's break down what's happening:

1. **Class Definition**: The `SimpleDevice` class inherits from `INDI::DefaultDevice`, which provides the basic functionality required by all INDI devices.

2. **Required Overrides**:

   - `Connect()`: Called when a client requests to connect to the device. In a real driver, this would establish a connection to the physical hardware.
   - `Disconnect()`: Called when a client requests to disconnect from the device. In a real driver, this would close the connection to the physical hardware.
   - `getDefaultName()`: Returns the default name of the device, which is used by clients to identify it.

3. **Device Instance**: The line `std::unique_ptr<SimpleDevice> simpleDevice(new SimpleDevice());` creates a global instance of the device. This is required for the INDI server to find and use the driver.

## Running the Tutorial

To run this tutorial:

1. Build the tutorial using CMake:

   ```bash
   cd /path/to/indi/build
   make
   ```

2. Run the INDI server with the tutorial driver:

   ```bash
   indiserver -v ./tutorial_one
   ```

3. Connect to the server using an INDI client (like KStars/EKOS or INDI Control Panel).

4. You should see the "Simple Device" in the client, and you can connect to it.

## Next Steps

This tutorial demonstrates the absolute minimum required for an INDI driver. In [Tutorial Two](tutorial-two.md), we'll build on this foundation to create a simple telescope simulator with more functionality.
