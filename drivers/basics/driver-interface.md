---
title: Driver Interface
nav_order: 2
parent: Basics
---

## Driver Interface

There are several different device types recognized by the INDI ecosystem. Your driver can implement
any number of these.

```cpp
enum DRIVER_INTERFACE
{
    GENERAL_INTERFACE       = 0,         /**< Default interface for all INDI devices */
    TELESCOPE_INTERFACE     = (1 << 0),  /**< Telescope interface, must subclass INDI::Telescope */
    CCD_INTERFACE           = (1 << 1),  /**< CCD interface, must subclass INDI::CCD */
    GUIDER_INTERFACE        = (1 << 2),  /**< Guider interface, must subclass INDI::GuiderInterface */
    FOCUSER_INTERFACE       = (1 << 3),  /**< Focuser interface, must subclass INDI::FocuserInterface */
    FILTER_INTERFACE        = (1 << 4),  /**< Filter interface, must subclass INDI::FilterInterface */
    DOME_INTERFACE          = (1 << 5),  /**< Dome interface, must subclass INDI::Dome */
    GPS_INTERFACE           = (1 << 6),  /**< GPS interface, must subclass INDI::GPS */
    WEATHER_INTERFACE       = (1 << 7),  /**< Weather interface, must subclass INDI::Weather */
    AO_INTERFACE            = (1 << 8),  /**< Adaptive Optics Interface */
    DUSTCAP_INTERFACE       = (1 << 9),  /**< Dust Cap Interface */
    LIGHTBOX_INTERFACE      = (1 << 10), /**< Light Box Interface */
    DETECTOR_INTERFACE      = (1 << 11), /**< Detector interface, must subclass INDI::Detector */
    ROTATOR_INTERFACE       = (1 << 12), /**< Rotator interface, must subclass INDI::RotatorInterface */
    SPECTROGRAPH_INTERFACE  = (1 << 13), /**< Spectrograph interface */
    CORRELATOR_INTERFACE    = (1 << 14), /**< Correlators (interferometers) interface */
    AUX_INTERFACE           = (1 << 15), /**< Auxiliary interface */
    OUTPUT_INTERFACE        = (1 << 16), /**< Digital Output (e.g. Relay) interface */
    INPUT_INTERFACE         = (1 << 17), /**< Digital/Analog Input (e.g. GPIO) interface */
    POWER_INTERFACE         = (1 << 18), /**< Auxiliary interface */

    SENSOR_INTERFACE        = SPECTROGRAPH_INTERFACE | DETECTOR_INTERFACE | CORRELATOR_INTERFACE
};
```

You can let INDI know what you implement by calling `setDriverInterface` in `initProperties`.

Many of these interfaces have base classes you can and should inherit from if you implmement the interface.

- `TELESCOPE_INTERFACE`

  - `libindi/inditelescope.h`
  - `Telescope`

- `CCD_INTERFACE`

  - `libindi/indiccd.h`
  - `CCD`

- `GUIDER_INTERFACE`

  - `libindi/indiguiderinterface.h`
  - `GuiderInterface`

- `FOCUSER_INTERFACE`

  - `libindi/indifocuserinterface.h`
  - `FocuserInterface`

- `FILTER_INTERFACE`

  - `libindi/indifilterinterface.h`
  - `FilterInterface`

- `DOME_INTERFACE`

  - `libindi/indidome.h`
  - `Dome`

- `GPS_INTERFACE`

  - `libindi/indigps.h`
  - `GPS`

- `WEATHER_INTERFACE`

  - `libindi/indiweatherinterface.h`
  - `WeatherInterface`

- `DETECTOR_INTERFACE`

  - `libindi/indidetector.h`
  - `Detector`

- `ROTATOR_INTERFACE`

  - `libindi/indirotatorinterface.h`
  - `RotatorInterface`

- `OUTPUT_INTERFACE`

  - `libindi/indioutputinterface.h`
  - `OutputInterface`

- `INPUT_INTERFACE`

  - `libindi/indiinputinterface.h`
  - `InputInterface`

- `SPECTROGRAPH_INTERFACE`

  - `libindi/indispectrograph.h`
  - `Spectrograph`

- `CORRELATOR_INTERFACE`
  - `libindi/indicorrelator.h`
  - `Correlator`

Each of these base classes have methods you must override for your driver to work correctly.
They also take care of creating any
[standard properties](../standard-properties.md)
that your device type needs to implement.

For example, if you want to implement a guider, you'll need to override these methods
to do the actual heavy lifting.

```cpp
    virtual IPState GuideNorth(uint32_t ms) override;
    virtual IPState GuideSouth(uint32_t ms) override;
    virtual IPState GuideEast(uint32_t ms) override;
    virtual IPState GuideWest(uint32_t ms) override;
```

The driver incorporates the interfaces by means of C++ class inheritance. You can inherit from multiple INDI interfaces.

```cpp
class MyCustomDriver : public INDI::GuiderInterface
{
}
```

Many interfaces have a short alias to facilitate its usage. For example, the INDI::GuiderInterface alias is **GI**.

```cpp
MyCustomDriver::MyCustomDriver : GI(this)
{
}
```

You'll also need to initialize the guider properties in `initProperties`:

```cpp
bool MyCustomDriver::initProperties()
{
    // initialize the parent's properties first
    INDI::DefaultDevice::initProperties();

    // Send which tab you want to define the guider properties in
    GI::initProperties(MOTION_TAB);

    // Important to update driver interface to indicate that it supports GUIDER interface
    setDriverInterface(AUX_INTERFACE | GUIDER_INTERFACE);

    return true;
}
```

Define them in `updateProperties`:

```cpp
bool MyCustomDriver::updateProperties()
{
    INDI::DefaultDevice::updateProperties();
    GI::updateProperties();

    if (isConnected())
    {
        // Define MyCustomDriver properties.
    }
    else
    {
        // Delete MyCustomDriver properties.
    }

    return true;
}
```

And process new values from the client in `ISNewNumber`:

```cpp
bool MyCustomDriver::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    // Check if GuiderInterface accepts the properties. If it can, then no need to proceed further
    if (GI::ISNewNumber(dev, name, values, names, n))
      return true;

    return INDI::DefaultDevice::ISNewNumber(dev, name, values, names, n);
}
```

The `GuiderInterface` base class will take care of processing the number properties received from the client and turning them into calls to one of the four virtual methods above.

Note that some base classes will handle much of this for you. For example, the `Telescope` base class has its own `updateProperties` that you can call from yours.

`Telescope` also handles `TimerHit` for you, and instead you would override `ReadScopeStatus`.

Be sure to get a good feel for the base class you are going to implememnt before trying to code it. Start with looking at the functions we have already talked about in this tutorial that are implemented on the base class. Also look for any pure virtual methods on the base class (they will be marked as `virtual` and have ` = 0;` at the end of the declaration). These are the methods you will HAVE to override.
