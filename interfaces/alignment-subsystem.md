---
title: INDI Alignment Subsystem
parent: Telescope Interface
grand_parent: Device Interfaces
nav_order: 1
---

# INDI Alignment Subsystem

## Introduction

The INDI Alignment Subsystem is a powerful framework that significantly improves telescope GOTO accuracy by using a database of sync points and mathematical transformations. It compensates for:

- Mount alignment errors (polar misalignment, cone errors, etc.)
- Atmospheric refraction effects
- Mechanical flexure and backlash
- Gravitational deformation based on telescope orientation
- Systematic errors in Alt-Az mounts

This subsystem is particularly valuable for:
- **Alt-Az mounts**: Transforms celestial coordinates to altitude-azimuth while accounting for field rotation
- **Imperfectly polar-aligned equatorial mounts**: Corrects for polar alignment errors
- **Any mount requiring precision pointing**: Creates a sky model based on actual observations

## How It Works

The alignment subsystem consists of three main components:

### 1. Sync Point Database

The database stores alignment points, each containing:
- **Celestial coordinates** (RA/Dec): Where the object actually is in the sky
- **Telescope direction vector**: Where the mount's encoders were pointing
- **Julian date**: When the sync was performed (important for time-dependent corrections)
- **Optional private data**: Driver-specific information

During an observing session, the database is held in memory and can be saved to/loaded from disk.

### 2. Math Plugin System

Math plugins perform the coordinate transformations using the sync point database. Two plugins are included:

#### Built-in Math Plugin (Default)
- Uses Toshimi Taki's matrix method for transformation
- Converts celestial RA/Dec to horizontal Alt/Az coordinates at sync time
- Builds transformation matrices based on the number of sync points:
  - **1 sync point**: Creates a simple alignment using mount type hint (ZENITH, NORTH_POLE, or SOUTH_POLE)
  - **2 sync points**: Builds a plane from two points plus a calculated third
  - **3 sync points**: Creates a single transformation matrix
  - **4+ sync points**: Constructs convex hulls with triangular facets for localized corrections

#### SVD Math Plugin
- Uses Markley's Singular Value Decomposition algorithm
- More robust and accurate, used in professional installations
- Handles the same sync point scenarios as the built-in plugin
- Highly resistant to numerical errors

### 3. Coordinate Transformation Functions

Two key transformation functions:
- **`TransformCelestialToTelescope()`**: Converts RA/Dec → Telescope direction vector (for GOTO)
- **`TransformTelescopeToCelestial()`**: Converts Telescope direction vector → RA/Dec (for position reporting)

## Integration Guide

### Step 1: Inherit from AlignmentSubsystemForDrivers

Add the alignment subsystem as a parent class to your driver:

```cpp
#include "alignment/AlignmentSubsystemForDrivers.h"

class MyMount : public INDI::Telescope, 
                public INDI::AlignmentSubsystem::AlignmentSubsystemForDrivers
{
    // ... your driver implementation
};
```

**Real-world example from skywatcherAPIMount.h:**
```cpp
class SkywatcherAPIMount : public INDI::Telescope,
                          public SkywatcherAPI,
                          public INDI::GuiderInterface,
                          public INDI::AlignmentSubsystem::AlignmentSubsystemForDrivers
```

### Step 2: Initialize Alignment Properties

In your `initProperties()` method, initialize the alignment subsystem:

```cpp
bool MyMount::initProperties()
{
    // Initialize parent properties first
    INDI::Telescope::initProperties();
    
    // ... your custom properties ...
    
    // Initialize alignment properties
    InitAlignmentProperties(this);
    
    return true;
}
```

**From skywatcherAPIMount.cpp:**
```cpp
bool SkywatcherAPIMount::initProperties()
{
    INDI::Telescope::initProperties();
    
    // ... other initialization ...
    
    // Add alignment properties
    InitAlignmentProperties(this);
    
    // Force the alignment system to always be on
    getSwitch("ALIGNMENT_SUBSYSTEM_ACTIVE")[0].setState(ISS_ON);
    
    return true;
}
```

### Step 3: Hook Property Handlers

Connect the alignment subsystem to your property handlers:

```cpp
bool MyMount::ISNewNumber(const char *dev, const char *name, double values[], 
                          char *names[], int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        // Process alignment properties
        ProcessAlignmentNumberProperties(this, name, values, names, n);
        
        // ... your custom number properties ...
    }
    
    return INDI::Telescope::ISNewNumber(dev, name, values, names, n);
}

bool MyMount::ISNewSwitch(const char *dev, const char *name, ISState *states, 
                          char *names[], int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        // Process alignment properties
        ProcessAlignmentSwitchProperties(this, name, states, names, n);
        
        // ... your custom switch properties ...
    }
    
    return INDI::Telescope::ISNewSwitch(dev, name, states, names, n);
}

bool MyMount::ISNewBLOB(const char *dev, const char *name, int sizes[], 
                        int blobsizes[], char *blobs[], char *formats[], 
                        char *names[], int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        ProcessAlignmentBLOBProperties(this, name, sizes, blobsizes, blobs, 
                                       formats, names, n);
    }
    
    return INDI::Telescope::ISNewBLOB(dev, name, sizes, blobsizes, blobs, 
                                      formats, names, n);
}

bool MyMount::ISNewText(const char *dev, const char *name, char *texts[], 
                        char *names[], int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        ProcessAlignmentTextProperties(this, name, texts, names, n);
    }
    
    return INDI::Telescope::ISNewText(dev, name, texts, names, n);
}
```

### Step 4: Update Location Handler

Override `updateLocation()` to notify the alignment subsystem:

```cpp
bool MyMount::updateLocation(double latitude, double longitude, double elevation)
{
    // Update the alignment subsystem with new location
    UpdateLocation(latitude, longitude, elevation);
    
    // ... your location-specific code ...
    
    return true;
}
```

### Step 5: Implement Sync to Store Alignment Points

The `Sync()` method adds sync points to the alignment database:

```cpp
bool MyMount::Sync(double ra, double dec)
{
    // Get current mount encoder positions
    if (!GetEncoder(AXIS1) || !GetEncoder(AXIS2))
        return false;
    
    // For Alt-Az mounts, get current Alt/Az position
    INDI::IHorizontalCoordinates AltAz;
    AltAz.azimuth = MicrostepsToDegrees(AXIS1, 
                        CurrentEncoders[AXIS1] - ZeroPositionEncoders[AXIS1]);
    AltAz.altitude = MicrostepsToDegrees(AXIS2, 
                        CurrentEncoders[AXIS2] - ZeroPositionEncoders[AXIS2]);
    
    // Create alignment database entry
    AlignmentDatabaseEntry NewEntry;
    NewEntry.ObservationJulianDate = ln_get_julian_from_sys();
    NewEntry.RightAscension = ra;
    NewEntry.Declination = dec;
    NewEntry.TelescopeDirection = TelescopeDirectionVectorFromAltitudeAzimuth(AltAz);
    NewEntry.PrivateDataSize = 0;
    
    // Check for duplicate sync points
    if (!CheckForDuplicateSyncPoint(NewEntry))
    {
        // Add to database
        GetAlignmentDatabase().push_back(NewEntry);
        
        // Notify client of database size change
        UpdateSize();
        
        // Reinitialize math plugin with new sync point
        Initialise(this);
        
        LOGF_INFO("Sync: Added alignment point at RA: %.2f DEC: %.2f", ra, dec);
        return true;
    }
    
    return false;
}
```

**From skywatcherAPIMount.cpp with special handling for parked position:**
```cpp
bool SkywatcherAPIMount::Sync(double ra, double dec)
{
    if (!GetEncoder(AXIS1) || !GetEncoder(AXIS2))
        return false;
    
    // Special handling when mount is parked
    if (isParked())
    {
        INDI::IHorizontalCoordinates AltAz { 0, 0 };
        TelescopeDirectionVector TDV;
        
        if (TransformCelestialToTelescope(ra, dec, 0.0, TDV))
        {
            AltitudeAzimuthFromTelescopeDirectionVector(TDV, AltAz);
            ZeroPositionEncoders[AXIS1] = PolarisPositionEncoders[AXIS1] - 
                                          DegreesToMicrosteps(AXIS1, AltAz.azimuth);
            ZeroPositionEncoders[AXIS2] = PolarisPositionEncoders[AXIS2] - 
                                          DegreesToMicrosteps(AXIS2, AltAz.altitude);
            LOGF_INFO("Sync in park position (Alt: %.2f Az: %.2f)", 
                      AltAz.altitude, AltAz.azimuth);
            GetAlignmentDatabase().clear();
            return true;
        }
    }
    
    // Normal sync point handling
    INDI::IHorizontalCoordinates AltAz { 0, 0 };
    AltAz.azimuth = range360(MicrostepsToDegrees(AXIS1,
                    CurrentEncoders[AXIS1] - ZeroPositionEncoders[AXIS1]));
    AltAz.altitude = MicrostepsToDegrees(AXIS2,
                    CurrentEncoders[AXIS2] - ZeroPositionEncoders[AXIS2]);
    
    AlignmentDatabaseEntry NewEntry;
    NewEntry.ObservationJulianDate = ln_get_julian_from_sys();
    NewEntry.RightAscension = ra;
    NewEntry.Declination = dec;
    NewEntry.TelescopeDirection = TelescopeDirectionVectorFromAltitudeAzimuth(AltAz);
    NewEntry.PrivateDataSize = 0;
    
    if (!CheckForDuplicateSyncPoint(NewEntry))
    {
        GetAlignmentDatabase().push_back(NewEntry);
        UpdateSize();
        Initialise(this);
        return true;
    }
    
    return false;
}
```

### Step 6: Use Transformations in Goto

Transform celestial coordinates to telescope coordinates before slewing:

```cpp
bool MyMount::Goto(double ra, double dec)
{
    INDI::IHorizontalCoordinates AltAz { 0, 0 };
    TelescopeDirectionVector TDV;
    
    // Try alignment subsystem transformation first
    if (TransformCelestialToTelescope(ra, dec, 0.0, TDV))
    {
        // Alignment subsystem successfully transformed coordinates
        AltitudeAzimuthFromTelescopeDirectionVector(TDV, AltAz);
        
        LOGF_DEBUG("Goto: Alignment transformed RA %.2f DEC %.2f to AZ %.2f ALT %.2f",
                   ra, dec, AltAz.azimuth, AltAz.altitude);
    }
    else
    {
        // Fallback: Use basic coordinate conversion
        INDI::IEquatorialCoordinates EquatorialCoords { ra, dec };
        INDI::EquatorialToHorizontal(&EquatorialCoords, &m_Location, 
                                     ln_get_julian_from_sys(), &AltAz);
        
        // Apply approximate mount alignment if known
        TDV = TelescopeDirectionVectorFromAltitudeAzimuth(AltAz);
        switch (GetApproximateMountAlignment())
        {
            case NORTH_CELESTIAL_POLE:
                TDV.RotateAroundY(m_Location.latitude - 90.0);
                break;
            case SOUTH_CELESTIAL_POLE:
                TDV.RotateAroundY(m_Location.latitude + 90.0);
                break;
            case ZENITH:
            default:
                break;
        }
        AltitudeAzimuthFromTelescopeDirectionVector(TDV, AltAz);
        
        LOGF_DEBUG("Goto: Basic conversion RA %.2f DEC %.2f to AZ %.2f ALT %.2f",
                   ra, dec, AltAz.azimuth, AltAz.altitude);
    }
    
    // Convert to motor steps and slew
    long AzimuthSteps = DegreesToMicrosteps(AXIS1, AltAz.azimuth);
    long AltitudeSteps = DegreesToMicrosteps(AXIS2, AltAz.altitude);
    
    SlewTo(AXIS1, AzimuthSteps);
    SlewTo(AXIS2, AltitudeSteps);
    
    TrackState = SCOPE_SLEWING;
    return true;
}
```

### Step 7: Use Transformations in ReadScopeStatus

Convert telescope encoder positions back to celestial coordinates:

```cpp
bool MyMount::ReadScopeStatus()
{
    // Read current encoder positions
    if (!GetEncoder(AXIS1) || !GetEncoder(AXIS2))
        return false;
    
    // Convert encoders to Alt/Az
    INDI::IHorizontalCoordinates AltAz { 0, 0 };
    AltAz.azimuth = MicrostepsToDegrees(AXIS1, 
                        CurrentEncoders[AXIS1] - ZeroPositionEncoders[AXIS1]);
    AltAz.altitude = MicrostepsToDegrees(AXIS2, 
                        CurrentEncoders[AXIS2] - ZeroPositionEncoders[AXIS2]);
    
    // Create telescope direction vector
    TelescopeDirectionVector TDV = TelescopeDirectionVectorFromAltitudeAzimuth(AltAz);
    
    double RightAscension, Declination;
    
    // Try alignment subsystem transformation
    if (!TransformTelescopeToCelestial(TDV, RightAscension, Declination))
    {
        // Fallback: Basic conversion
        TelescopeDirectionVector RotatedTDV(TDV);
        
        switch (GetApproximateMountAlignment())
        {
            case NORTH_CELESTIAL_POLE:
                RotatedTDV.RotateAroundY(90.0 - m_Location.latitude);
                break;
            case SOUTH_CELESTIAL_POLE:
                RotatedTDV.RotateAroundY(-90.0 - m_Location.latitude);
                break;
            case ZENITH:
            default:
                break;
        }
        
        AltitudeAzimuthFromTelescopeDirectionVector(RotatedTDV, AltAz);
        
        INDI::IEquatorialCoordinates EquatorialCoords;
        INDI::HorizontalToEquatorial(&AltAz, &m_Location, 
                                     ln_get_julian_from_sys(), &EquatorialCoords);
        RightAscension = EquatorialCoords.rightascension;
        Declination = EquatorialCoords.declination;
    }
    
    // Update clients with current position
    NewRaDec(RightAscension, Declination);
    
    return true;
}
```

## Using the Alignment Subsystem from KStars

The easiest way to use a telescope driver with alignment support is through KStars:

### Initial Setup

1. **Connect your mount** and start the INDI driver
2. **In KStars**, go to Tools → Devices → Device Manager
3. **Connect to your mount** through the INDI Control Panel
4. **Set location**: Ensure accurate site coordinates are configured
5. **Check Alignment tab**: Verify alignment properties are visible

### Creating Sync Points

1. **Find your first star**: Use KStars to locate a bright star
2. **Center the star**: Manually center the star in your eyepiece/camera
3. **Sync**: Right-click the star in KStars → [Your Mount] → Sync
4. **Add more points**: Repeat for additional stars across the sky

**Best practices for sync points:**
- Start with 3-4 widely separated stars
- Cover different areas of the sky (east, west, south, zenith)
- For best results, distribute points evenly across your observing region
- More points = better accuracy (especially with 4+ points using convex hull method)

### Testing Alignment

1. **Select a target**: Choose a star near your sync points
2. **GOTO**: Right-click → [Your Mount] → Slew
3. **Verify accuracy**: Check how close the GOTO landed
4. **Iterate**: Add more sync points in areas with poor accuracy

### Saving/Loading Alignment

The alignment database can be saved and loaded through the INDI Control Panel:
- **Save**: Stores sync points to disk for future sessions
- **Load**: Restores previously saved sync points
- **Clear**: Removes all sync points (start fresh)

## Advanced Features

### Approximate Mount Alignment

For mounts without any sync points, set an approximate alignment:

```cpp
SetApproximateMountAlignment(ZENITH);           // Alt-Az mount
SetApproximateMountAlignment(NORTH_CELESTIAL_POLE);  // Equatorial mount (northern hemisphere)
SetApproximateMountAlignment(SOUTH_CELESTIAL_POLE);  // Equatorial mount (southern hemisphere)
```

This provides basic transformation before any sync points are added.

### Selecting Math Plugins

Users can switch between math plugins through INDI properties:
- **Built-in**: Fast, good for most applications
- **SVD**: More accurate, recommended for precision work

### Private Data in Sync Points

Drivers can store custom data with each sync point:

```cpp
AlignmentDatabaseEntry NewEntry;
// ... set standard fields ...

// Add custom data
struct MyCustomData {
    double temperature;
    int encoderOffset;
};
MyCustomData customData = {20.5, 100};
NewEntry.PrivateDataSize = sizeof(MyCustomData);
memcpy(NewEntry.PrivateData, &customData, sizeof(MyCustomData));
```

## Complete Example: Alt-Az Mount

Here's a complete minimal example based on the SkywatcherAPIMount driver:

```cpp
#pragma once

#include <inditelescope.h>
#include "alignment/AlignmentSubsystemForDrivers.h"

class MyAltAzMount : public INDI::Telescope,
                     public INDI::AlignmentSubsystem::AlignmentSubsystemForDrivers
{
public:
    MyAltAzMount()
    {
        SetTelescopeCapability(
            TELESCOPE_CAN_GOTO |
            TELESCOPE_CAN_SYNC |
            TELESCOPE_CAN_ABORT |
            TELESCOPE_HAS_LOCATION |
            TELESCOPE_HAS_TIME,
            4);
    }
    
    virtual bool initProperties() override
    {
        INDI::Telescope::initProperties();
        InitAlignmentProperties(this);
        SetApproximateMountAlignment(ZENITH);
        return true;
    }
    
    virtual bool ISNewNumber(const char *dev, const char *name, 
                            double values[], char *names[], int n) override
    {
        if (dev && !strcmp(dev, getDeviceName()))
            ProcessAlignmentNumberProperties(this, name, values, names, n);
        return INDI::Telescope::ISNewNumber(dev, name, values, names, n);
    }
    
    virtual bool ISNewSwitch(const char *dev, const char *name, 
                            ISState *states, char *names[], int n) override
    {
        if (dev && !strcmp(dev, getDeviceName()))
            ProcessAlignmentSwitchProperties(this, name, states, names, n);
        return INDI::Telescope::ISNewSwitch(dev, name, states, names, n);
    }
    
    virtual bool updateLocation(double lat, double lon, double elev) override
    {
        UpdateLocation(lat, lon, elev);
        return true;
    }
    
    virtual bool Sync(double ra, double dec) override
    {
        // Get current Alt/Az from encoders
        INDI::IHorizontalCoordinates altaz = getCurrentAltAz();
        
        AlignmentDatabaseEntry entry;
        entry.ObservationJulianDate = ln_get_julian_from_sys();
        entry.RightAscension = ra;
        entry.Declination = dec;
        entry.TelescopeDirection = TelescopeDirectionVectorFromAltitudeAzimuth(altaz);
        entry.PrivateDataSize = 0;
        
        if (!CheckForDuplicateSyncPoint(entry))
        {
            GetAlignmentDatabase().push_back(entry);
            UpdateSize();
            Initialise(this);
            return true;
        }
        return false;
    }
    
    virtual bool Goto(double ra, double dec) override
    {
        TelescopeDirectionVector tdv;
        INDI::IHorizontalCoordinates altaz;
        
        if (TransformCelestialToTelescope(ra, dec, 0.0, tdv))
        {
            AltitudeAzimuthFromTelescopeDirectionVector(tdv, altaz);
        }
        else
        {
            // Fallback to basic conversion
            INDI::IEquatorialCoordinates eq{ra, dec};
            INDI::EquatorialToHorizontal(&eq, &m_Location, 
                                        ln_get_julian_from_sys(), &altaz);
        }
        
        // Slew to calculated Alt/Az
        return slewToAltAz(altaz);
    }
    
    virtual bool ReadScopeStatus() override
    {
        INDI::IHorizontalCoordinates altaz = getCurrentAltAz();
        TelescopeDirectionVector tdv = 
            TelescopeDirectionVectorFromAltitudeAzimuth(altaz);
        
        double ra, dec;
        if (!TransformTelescopeToCelestial(tdv, ra, dec))
        {
            // Fallback
            INDI::IEquatorialCoordinates eq;
            INDI::HorizontalToEquatorial(&altaz, &m_Location,
                                        ln_get_julian_from_sys(), &eq);
            ra = eq.rightascension;
            dec = eq.declination;
        }
        
        NewRaDec(ra, dec);
        return true;
    }
    
private:
    INDI::IHorizontalCoordinates getCurrentAltAz();
    bool slewToAltAz(const INDI::IHorizontalCoordinates& altaz);
};
```

## Troubleshooting

### Poor GOTO Accuracy

- **Add more sync points**: 3-4 minimum, more for better coverage
- **Distribute points**: Cover the entire observable sky
- **Check location**: Verify site coordinates are accurate
- **Try SVD plugin**: Switch from built-in to SVD math plugin

### Sync Points Not Persisting

- **Save configuration**: Use the INDI save config feature
- **Check file permissions**: Ensure driver can write to config directory
- **Verify database**: Check the Alignment tab shows your sync points

### Transformations Failing

- **Set approximate alignment**: Initialize with ZENITH, NORTH_POLE, or SOUTH_POLE
- **Check location and time**: Both must be set before transformations work
- **Verify math plugin**: Ensure a plugin is loaded and initialized

## References

- [Toshimi Taki's Matrix Method](http://www.geocities.jp/toshimi_taki/matrix/matrix_method_rev_e.pdf)
- [Markley's SVD Algorithm](http://www.control.auc.dk/~tb/best/aug23-Bak-svdalg.pdf)
- [SkywatcherAPIMount Driver](https://github.com/indilib/indi/blob/master/drivers/telescope/skywatcherAPIMount.cpp) - Reference implementation
- [INDI Alignment Subsystem White Paper](https://github.com/indilib/indi/blob/master/libs/alignment/alignment_white_paper.md)

## Summary

The INDI Alignment Subsystem transforms telescope drivers from basic pointing devices into precision instruments. By:

1. **Inheriting** from `AlignmentSubsystemForDrivers`
2. **Storing sync points** in the `Sync()` method
3. **Using transformations** in `Goto()` and `ReadScopeStatus()`
4. **Hooking property handlers** to manage alignment properties

You enable your mount to achieve professional-grade pointing accuracy across the entire sky. The subsystem handles all the complex mathematics, coordinate transformations, and database management—you just need to integrate it into your driver's coordinate flow.
