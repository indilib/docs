---
title: Filter Wheel Interface
nav_order: 10
parent: Device Interfaces
---

# Implementing the Filter Wheel Interface

This guide provides a comprehensive overview of implementing the filter wheel interface in INDI drivers. It covers the basic structure of a filter wheel driver, how to implement the required methods, and how to handle device-specific functionality.

## Introduction to the Filter Wheel Interface

The INDI Filter Wheel Interface (`INDI::FilterInterface`) is designed for devices that manage optical filters, such as filter wheels. It provides a standardized way for INDI clients to control filter selection, query current filter position, and manage filter names. A filter wheel can be an independent device or an embedded component within another device (e.g., a CCD camera).

## Prerequisites

Before implementing the filter wheel interface, you should have:

- Basic knowledge of C++ programming
- Understanding of the INDI protocol and architecture
- Familiarity with the device's communication protocol
- Development environment set up (compiler, build tools, etc.)
- INDI library installed

## Filter Wheel Interface Structure

The filter wheel interface consists of several key components:

- **Base Class**: `INDI::FilterInterface` is the base class for all filter wheel drivers.
- **Standard Properties**: Properties for controlling filter slot selection and managing filter names.
- **Virtual Methods**: A set of virtual methods that must be implemented by the driver.
- **Helper Methods**: Methods for common filter wheel operations.

### Base Class

The `INDI::FilterInterface` base class inherits from `INDI::DefaultDevice` and provides additional functionality specific to filter wheels. It defines standard properties for filter slot selection and filter name management.

### Key Methods

A driver implementing the `INDI::FilterInterface` must override and implement the following virtual methods to control the filter wheel:

-   `void initProperties(const char *groupName)`:
    Initializes the INDI properties related to the filter wheel interface. This should be called within the driver's `initProperties()` method.
    -   `groupName`: The name of the group or tab where filter wheel properties will be displayed in the client.

-   `bool updateProperties()`:
    Defines or deletes filter wheel properties based on the connection status of the base device.

-   `bool processNumber(const char *dev, const char *name, double values[], char *names[], int n)`:
    **Important**: This method must be called in your driver's `ISNewNumber()` function. It processes incoming client requests for filter slot selection and calls `SelectFilter()` accordingly.

-   `bool processText(const char *dev, const char *name, char *texts[], char *names[], int n)`:
    Processes incoming client requests for filter name properties. Drivers should call this if a property name matches a filter name property.

-   `virtual int QueryFilter() = 0`:
    **Pure virtual method.** Your driver must implement this to return the current filter position (1-based index).

-   `virtual bool SelectFilter(int position) = 0`:
    **Pure virtual method.** Your driver must implement this to select a new filter position.
    -   `position`: The target filter position (1-based index).
    -   Returns `true` if the operation is successful, `false` otherwise.

-   `void SelectFilterDone(int newpos)`:
    Your child class must call this function when the hardware successfully finishes selecting a new filter wheel position.
    -   `newpos`: The new position of the filter wheel (1-based index).

-   `virtual bool SetFilterNames()`:
    Sets filter names as defined by the client for each filter position. The desired filter names are stored in the `FilterNameTP` property. Filter names should be saved in hardware if possible. The default implementation saves them in the configuration file.

-   `virtual bool GetFilterNames()`:
    Obtains a list of filter names from the hardware and initializes the `FilterNameTP` property. The function should check for the number of filters available in the filter wheel and build the `FilterNameTP` property accordingly. The default implementation loads the filter names from the configuration file.

### Example Implementation

Here's a simplified example of how a driver might implement the `INDI::FilterInterface`. This example focuses on the core structure and omits complex serial communication details for clarity.

```cpp
#include "indibase.h"
#include "indifilterinterface.h"
#include <iostream>
#include <vector>
#include <string>

// Forward declaration of a dummy DefaultDevice for the example
class MyFilterWheel;

class MyFilterWheel : public INDI::DefaultDevice, public INDI::FilterInterface
{
public:
    MyFilterWheel() : INDI::DefaultDevice(), INDI::FilterInterface(this)
    {
        setDriverInterface(FILTER_INTERFACE);
        // Initialize current filter position
        CurrentFilter = 1;
        TargetFilter = 1;
    }

    virtual bool initProperties() override
    {
        INDI::DefaultDevice::initProperties();

        // Initialize Filter Wheel properties. "Filter Wheel" for the group name.
        initProperties("Filter Wheel");

        // Set the min/max for the filter slot property.
        // Assuming a 5-position filter wheel for this example.
        FilterSlotNP[0].setMin(1);
        FilterSlotNP[0].setMax(5); // 5 filter positions
                
        return true;
    }

    virtual bool updateProperties() override
    {
        INDI::DefaultDevice::updateProperties();
        if (isConnected())
        {
            // Define Filter Wheel properties when connected
            INDI::FilterInterface::updateProperties();
        }
        else
        {
            // Delete Filter Wheel properties when disconnected
            INDI::FilterInterface::updateProperties();
        }
        return true;
    }

    virtual const char *getDefaultName() override
    {
        return "MyFilterWheel";
    }

    // Implement the pure virtual QueryFilter method
    virtual int QueryFilter() override
    {
        std::cout << "Querying current filter position: " << CurrentFilter << std::endl;
        // In a real driver, you would query the hardware for the current position
        return CurrentFilter;
    }

    // Implement the pure virtual SelectFilter method
    virtual bool SelectFilter(int position) override
    {
        if (position < FilterSlotNP[0].getMin() || position > FilterSlotNP[0].getMax())
        {
            std::cerr << "Invalid filter position: " << position << std::endl;
            return false;
        }

        TargetFilter = position;
        std::cout << "Attempting to select filter position: " << TargetFilter << std::endl;
        // In a real driver, you would send a command to your hardware to move the filter wheel
        // For simulation, we'll just "complete" it immediately
        // In a real driver, this would be called from a separate thread or a timer after the hardware movement is done.
        SelectFilterDone(TargetFilter);
        return true; // Assume command sent successfully
    }

    // Override ISNewNumber to process filter slot changes
    virtual bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) override
    {
        if (dev && !strcmp(dev, getDeviceName()))
        {
            // Important: Call FilterInterface's processNumber for filter slot changes
            if (INDI::FilterInterface::processNumber(dev, name, values, names, n))
                return true;
        }
        return INDI::DefaultDevice::ISNewNumber(dev, name, values, names, n);
    }

    // Override ISNewText to process filter name changes
    virtual bool ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n) override
    {
        if (dev && !strcmp(dev, getDeviceName()))
        {
            // Important: Call FilterInterface's processText for filter name changes
            if (INDI::FilterInterface::processText(dev, name, texts, names, n))
                return true;
        }
        return INDI::DefaultDevice::ISNewText(dev, name, texts, names, n);
    }

    // Other INDI::DefaultDevice methods would also be implemented here
};

```
