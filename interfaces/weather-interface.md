---
title: Weather Interface
nav_order: 9
parent: Device Interfaces
---

# Implementing the Weather Interface

This guide provides a comprehensive overview of implementing the weather interface in INDI drivers. It covers the basic structure of a weather driver, how to implement the required methods, and how to handle device-specific functionality.

## Introduction to the Weather Interface

The INDI Weather Interface (`INDI::WeatherInterface`) provides a comprehensive framework for implementing weather monitoring and observatory safety systems. It allows devices to report weather conditions and determine if the environment is safe for observatory operations. This interface can be implemented as an independent weather station device or as weather-related reports within another device (e.g., an observatory control system).

## Prerequisites

Before implementing the weather interface, you should have:

- Basic knowledge of C++ programming
- Understanding of the INDI protocol and architecture
- Familiarity with the device's communication protocol
- Development environment set up (compiler, build tools, etc.)
- INDI library installed

## Weather Interface Structure

The weather interface consists of several key components:

- **Base Class**: `INDI::WeatherInterface` is the base class for all weather drivers.
- **Parameter Management**: For various weather measurements (temperature, humidity, wind speed, etc.).
- **Safety Monitoring**: Configurable safe/warning/danger zones for each parameter and critical parameter monitoring.
- **Evaluation Models**: Two evaluation models for parameters (standard and flipped).
- **Update Mechanisms**: Automatic periodic updates and manual refresh capability.

### Base Class

The `INDI::WeatherInterface` base class inherits from `INDI::DefaultDevice` and provides additional functionality specific to weather devices. It defines standard properties for weather parameters, safety status, and update controls.

### Weather Parameters

Weather parameters represent physical measurements such as Temperature, Humidity, Wind speed, Cloud cover, Rain detection, Sky quality, etc. Each parameter has configurable ranges that determine its state:

- **OK range**: Values considered safe for operation.
- **Warning range**: Values approaching unsafe conditions.
- **Danger range**: Values unsafe for operation.

### Parameter Evaluation Models

The interface supports two models for evaluating parameters:

1.  **Standard model (default)**:
    ```
    min--+         +-low-%    high-%-+        +--max
         |         |                 |        |
         v         v                 v        v
         [         (                 )        ]
    danger     warning       good         warning   danger
    ```
    -   Values outside min/max limits = DANGER (`IPS_ALERT`)
    -   Values in warning zones = WARNING (`IPS_BUSY`)
    -   Values in the middle safe zone = GOOD (`IPS_OK`)

2.  **Flipped model**:
    ```
    min--+         +-low-%    high-%-+        +--max
         |         |                 |        |
         v         v                 v        v
         [         (                 )        ]
    good       warning      danger        warning   good
    ```
    -   Values outside min/max limits = GOOD (`IPS_OK`)
    -   Values in warning zones = WARNING (`IPS_BUSY`)
    -   Values in the middle zone = DANGER (`IPS_ALERT`)

The flipped model is useful for parameters where extreme values indicate good conditions (e.g., cloud sensors where very low/high readings indicate clear skies).

### Key Methods

A driver implementing the `INDI::WeatherInterface` must override and implement the following virtual methods to provide weather data:

-   `void initProperties(const char *statusGroup, const char *paramsGroup)`:
    Initializes the INDI properties related to the weather interface. This should be called within the driver's `initProperties()` method.
    -   `statusGroup`: Group name for status properties (e.g., overall safety).
    -   `paramsGroup`: Group name for individual weather parameter properties.

-   `bool updateProperties()`:
    Defines or deletes weather properties based on the connection status of the base device.

-   `bool processNumber(const char *dev, const char *name, double values[], char *names[], int n)`:
    Processes incoming client requests for weather number properties. Drivers should call this if a property name matches a weather property (e.g., `WEATHER_*`).

-   `bool processSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)`:
    Processes incoming client requests for weather switch properties (e.g., refresh, override). Drivers should call this if a property name matches a weather property.

-   `virtual IPState updateWeather()`:
    **This is a crucial method that your driver must implement.** It is responsible for updating the raw values of your weather parameters from the device or service. This method should *not* change the state of any property, as that is handled by the `WeatherInterface` itself. It should only update the raw values using `setParameterValue()`.
    -   Returns `IPS_OK` if data is valid, `IPS_BUSY` if update is in progress, `IPS_ALERT` if there is an error.

-   `void addParameter(std::string name, std::string label, double numMinOk, double numMaxOk, double percWarning, bool flipWarning = false)`:
    Adds a physical weather measurable parameter to the weather driver.
    -   `name`: Internal name of the parameter.
    -   `label`: Display label for the GUI.
    -   `numMinOk`, `numMaxOk`: Minimum and maximum values for the "OK" range.
    -   `percWarning`: Percentage for the "Warning" zone (e.g., 10 for 10%).
    -   `flipWarning`: If `true`, uses the flipped evaluation model.

-   `bool setCriticalParameter(std::string name)`:
    Sets a parameter as critical. The state of critical parameters affects the overall weather driver state, signaling clients to take appropriate action.

-   `void setParameterValue(std::string name, double value)`:
    Updates the raw value of a specific weather parameter. This is typically called within your `updateWeather()` implementation.

### Example Implementation

Here's a simplified example of how a driver might implement the `INDI::WeatherInterface`. This example focuses on the core structure and omits complex serial communication details for clarity.

```cpp
#include "indibase.h"
#include "indiweatherinterface.h"
#include <iostream>
#include <random> // For simulating sensor data

// Forward declaration of a dummy DefaultDevice for the example
class MyWeatherStation;

class MyWeatherStation : public INDI::DefaultDevice, public INDI::WeatherInterface
{
public:
    MyWeatherStation() : INDI::DefaultDevice(), INDI::WeatherInterface(this)
    {
        setDriverInterface(WEATHER_INTERFACE);
    }

    virtual bool initProperties() override
    {
        INDI::DefaultDevice::initProperties();

        // Initialize Weather properties. "Weather Status" for overall status, "Environment" for parameters.
        initProperties("Weather Status", "Environment");

        // Add weather parameters
        // Temperature: OK between 10-30C, 15% warning zone
        addParameter("WEATHER_TEMPERATURE", "Temperature (C)", 10, 30, 15);
        // Humidity: OK between 30-70%, 10% warning zone
        addParameter("WEATHER_HUMIDITY", "Humidity (%)", 30, 70, 10);
        // Cloud Cover: Flipped model, OK if very low or very high (clear), DANGER in middle
        addParameter("WEATHER_CLOUD", "Cloud Cover (%)", 20, 80, 10, true);
        // Wind Speed: OK below 20 km/h, 20% warning zone
        addParameter("WEATHER_WIND_SPEED", "Wind Speed (km/h)", 0, 20, 20);

        // Set critical parameters that affect overall safety
        setCriticalParameter("WEATHER_TEMPERATURE");
        setCriticalParameter("WEATHER_CLOUD");

        return true;
    }

    virtual bool updateProperties() override
    {
        INDI::DefaultDevice::updateProperties();
        if (isConnected())
        {
            // Define Weather properties when connected
            INDI::WeatherInterface::updateProperties();
            // Start a timer to periodically update weather data
            SetTimer(getCurrentPollingPeriod());
        }
        else
        {
            // Delete Weather properties when disconnected
            INDI::WeatherInterface::updateProperties();
        }
        return true;
    }

    virtual const char *getDefaultName() override
    {
        return "MyWeatherStation";
    }

    // Implement the crucial updateWeather method
    virtual IPState updateWeather() override
    {
        // Simulate reading sensor data
        static std::default_random_engine generator;
        static std::uniform_real_distribution<double> temp_dist(5.0, 35.0); // Temp between 5 and 35
        static std::uniform_real_distribution<double> hum_dist(20.0, 80.0); // Humidity between 20 and 80
        static std::uniform_real_distribution<double> cloud_dist(0.0, 100.0); // Cloud between 0 and 100
        static std::uniform_real_distribution<double> wind_dist(0.0, 30.0); // Wind between 0 and 30

        double currentTemp = temp_dist(generator);
        double currentHum = hum_dist(generator);
        double currentCloud = cloud_dist(generator);
        double currentWind = wind_dist(generator);

        // Update the raw parameter values
        setParameterValue("WEATHER_TEMPERATURE", currentTemp);
        setParameterValue("WEATHER_HUMIDITY", currentHum);
        setParameterValue("WEATHER_CLOUD", currentCloud);
        setParameterValue("WEATHER_WIND_SPEED", currentWind);

        std::cout << "Weather updated: Temp=" << currentTemp << "C, Hum=" << currentHum << "%, Cloud=" << currentCloud << "%, Wind=" << currentWind << "km/h" << std::endl;

        // The WeatherInterface will automatically check critical parameters and update overall state
        return IPS_OK; // Indicate successful data update
    }

    // Other INDI::DefaultDevice methods would also be implemented here
    // e.g., ISNewSwitch, ISNewNumber, ISNewText for processing client requests
    // You would typically forward WEATHER_* properties to WI::processSwitch/processNumber
    virtual bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n) override
    {
        if (dev && !strcmp(dev, getDeviceName()))
        {
            if (WI::processSwitch(dev, name, states, names, n))
                return true;
        }
        return INDI::DefaultDevice::ISNewSwitch(dev, name, states, names, n);
    }

    virtual bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) override
    {
        if (dev && !strcmp(dev, getDeviceName()))
        {
            if (WI::processNumber(dev, name, values, names, n))
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

        // Call the weather interface's checkWeatherUpdate to trigger updateWeather()
        // and then update critical parameters.
        WI::checkWeatherUpdate();

        SetTimer(getCurrentPollingPeriod());
    }
};

// This is typically how an INDI driver is instantiated
// static MyWeatherStation myWeatherStation;
```
