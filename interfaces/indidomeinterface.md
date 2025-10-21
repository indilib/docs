---
title: Dome Interface
nav_order: 12
parent: Device Interfaces
---

# Implementing the Dome Interface

This guide provides a comprehensive overview of implementing the Dome Interface in INDI drivers. It covers the extensive functionality for controlling astronomical domes, including absolute and relative movement, parking, shutter control, slaving to a mount, and various status reporting mechanisms.

## Introduction to the Dome Interface

The INDI Dome Interface (`INDI::Dome`) provides a robust framework for controlling observatory domes. It supports both relative and absolute position domes, and even open-loop control for domes without position feedback, using simple direction commands (Clockwise and Counter-Clockwise).

A key feature is **slaving**, which synchronizes the dome's azimuth position with that of an active mount. This involves snooping mount coordinates and initiating dome motion when the mount slews or its tracking position exceeds a configurable `AutoSync` threshold. Custom parking positions are also supported for absolute/relative domes, and for roll-off observatories, the parking state reflects the roof's open/closed status.

Developers need to subclass `INDI::Dome` to implement any driver for Domes within INDI.

**IMPORTANT**: Before using any of the dome functions, you must define the capabilities of the dome by calling `SetDomeCapability()` function.

## Prerequisites

Before implementing the Dome Interface, you should have:

- Basic knowledge of C++ programming
- Understanding of the INDI protocol and architecture
- Familiarity with the device's communication protocol
- Development environment set up (compiler, build tools, etc.)
- INDI library installed

## Dome Interface Structure

The Dome Interface is a comprehensive class that provides general functionality for a Dome device. It includes mechanisms for motion control, status reporting, configuration, and advanced features like slaving and geometric calculations.

### Base Class

The `INDI::Dome` class inherits from `INDI::DefaultDevice` and provides a wide array of functionalities specific to dome control.

### Enums and Typedefs

The interface defines several enums and typedefs to manage dome operations and states:

#### `DomeMeasurements`

Measurements necessary for dome-slit synchronization. All values are in meters. The displacements are measured from the true dome center, and the dome is assumed spherical.

```cpp
typedef enum
{
    DM_DOME_RADIUS,        /*!< Dome RADIUS */
    DM_SHUTTER_WIDTH,      /*!< Shutter width */
    DM_NORTH_DISPLACEMENT, /*!< Displacement to north of the mount center */
    DM_EAST_DISPLACEMENT,  /*!< Displacement to east of the mount center */
    DM_UP_DISPLACEMENT,    /*!< Up Displacement of the mount center */
    DM_OTA_OFFSET          /*!< Distance from the optical axis to the mount center*/
} DomeMeasurements;
```
- `DM_DOME_RADIUS`: The radius of the dome.
- `DM_SHUTTER_WIDTH`: The width of the dome's shutter aperture.
- `DM_NORTH_DISPLACEMENT`: Displacement of the mount center to the north from the true dome center.
- `DM_EAST_DISPLACEMENT`: Displacement of the mount center to the east from the true dome center.
- `DM_UP_DISPLACEMENT`: Upward displacement of the mount center from the true dome center.
- `DM_OTA_OFFSET`: Distance from the optical axis of the telescope to the mount center.

#### `DomeDirection`

Defines the direction of dome motion.

```cpp
enum DomeDirection
{
    DOME_CW,  /*!< Clockwise direction */
    DOME_CCW  /*!< Counter-clockwise direction */
};
```

#### `DomeMotionCommand`

Defines commands for dome motion.

```cpp
enum DomeMotionCommand
{
    MOTION_START, /*!< Start motion */
    MOTION_STOP   /*!< Stop motion */
};
```

#### `DomeParkData`

Defines the type of parking data supported by the dome.

```cpp
enum DomeParkData
{
    PARK_NONE,       /*!< 2-state parking (Open or Closed only)  */
    PARK_AZ,         /*!< Parking via azimuth angle control */
    PARK_AZ_ENCODER, /*!< Parking via azimuth encoder control */
};
```

#### `ShutterOperation`

Defines commands for shutter operation.

```cpp
typedef enum
{
    SHUTTER_OPEN, /*!< Open Shutter */
    SHUTTER_CLOSE /*!< Close Shutter */
} ShutterOperation;
```

#### `MountLockingPolicy`

Defines how the dome interacts with the mount's parking status.

```cpp
enum MountLockingPolicy
{
    MOUNT_IGNORED,      /*!< Mount is ignored. Dome can park or unpark irrespective of mount parking status */
    MOUNT_LOCKS,        /*!< Mount Locks. Dome can park if mount is completely parked first. */
};
```

#### `DomeState`

Defines the possible states of the dome.

```cpp
typedef enum
{
    DOME_IDLE,      /*!< Dome is idle */
    DOME_MOVING,    /*!< Dome is in motion */
    DOME_SYNCED,    /*!< Dome is synced */
    DOME_PARKING,   /*!< Dome is parking */
    DOME_UNPARKING, /*!< Dome is unparking */
    DOME_PARKED,    /*!< Dome is parked */
    DOME_UNPARKED,  /*!< Dome is unparked */
    DOME_UNKNOWN,   /*!< Dome state is known */
    DOME_ERROR,     /*!< Dome has errors */
} DomeState;
```

#### `ShutterState`

Defines the possible states of the shutter.

```cpp
typedef enum
{
    SHUTTER_OPENED,     /*!< Shutter is open */
    SHUTTER_CLOSED,     /*!< Shutter is closed */
    SHUTTER_MOVING,     /*!< Shutter in motion (opening or closing) */
    SHUTTER_UNKNOWN,    /*!< Shutter status is unknown */
    SHUTTER_ERROR       /*!< Shutter status is unknown */
} ShutterState;
```

#### Dome Capabilities

These flags define the capabilities of the dome, which must be set using `SetDomeCapability()`.

```cpp
enum
{
    DOME_CAN_ABORT          = 1 << 0, /*!< Can the dome motion be aborted? */
    DOME_CAN_ABS_MOVE       = 1 << 1, /*!< Can the dome move to an absolute azimuth position? */
    DOME_CAN_REL_MOVE       = 1 << 2, /*!< Can the dome move to a relative position a number of degrees away from current position? Positive degrees is Clockwise direction. Negative Degrees is counter clock wise direction */
    DOME_CAN_PARK           = 1 << 3, /*!< Can the dome park and unpark itself? */
    DOME_CAN_SYNC           = 1 << 4, /*!< Can the dome sync to arbitrary position? */
    DOME_HAS_SHUTTER        = 1 << 5, /*!< Does the dome has a shutter than can be opened and closed electronically? */
    DOME_HAS_VARIABLE_SPEED = 1 << 6, /*!< Can the dome move in different configurable speeds? */
    DOME_HAS_BACKLASH       = 1 << 7  /*!< Can the dome compensate for backlash? */
};
```

#### `DomeConnection`

Defines the connection mode of the Dome.

```cpp
enum
{
    CONNECTION_NONE   = 1 << 0, /** Do not use any connection plugin */
    CONNECTION_SERIAL = 1 << 1, /** For regular serial and bluetooth connections */
    CONNECTION_TCP    = 1 << 2  /** For Wired and WiFI connections */
} DomeConnection;
```

### Key Methods (Public)

The `INDI::Dome` class provides the following public methods:

-   `Dome();`
    Constructor for the `INDI::Dome` class.

-   `virtual ~Dome();`
    Destructor for the `INDI::Dome` class.

-   `virtual bool initProperties() override;`
    Initializes the INDI properties for the dome. This should be called within the driver's `initProperties()` method.

-   `virtual void ISGetProperties(const char * dev) override;`
    Handles client requests to get properties.

-   `virtual bool updateProperties() override;`
    Defines or deletes dome properties based on the connection status of the device.

-   `virtual bool ISNewNumber(const char * dev, const char * name, double values[], char * names[], int n) override;`
    Processes incoming client requests for number properties.

-   `virtual bool ISNewSwitch(const char * dev, const char * name, ISState * states, char * names[], int n) override;`
    Processes incoming client requests for switch properties.

-   `virtual bool ISNewText(const char * dev, const char * name, char * texts[], char * names[], int n) override;`
    Processes incoming client requests for text properties.

-   `virtual bool ISSnoopDevice(XMLEle * root) override;`
    Handles snooping of properties from other devices (e.g., mount coordinates for slaving).

-   `static void buttonHelper(const char * button_n, ISState state, void * context);`
    A helper function for handling button presses.

-   `void setDomeConnection(const uint8_t &value);`
    Sets the dome connection mode (e.g., serial, TCP). Should be called in the child class constructor.

-   `uint8_t getDomeConnection() const;`
    Returns the current dome connection mode.

-   `uint32_t GetDomeCapability() const;`
    Returns the capabilities of the dome as a bitmask.

-   `void SetDomeCapability(uint32_t cap);`
    Sets the capabilities of the dome. This **must** be called before using other dome functions.

-   `bool CanAbort();`
    Returns `true` if the dome supports aborting motion.

-   `bool CanAbsMove();`
    Returns `true` if the dome supports moving to an absolute azimuth position.

-   `bool CanRelMove();`
    Returns `true` if the dome supports moving to a relative position.

-   `bool CanPark();`
    Returns `true` if the dome supports parking and unparking.

-   `bool CanSync();`
    Returns `true` if the dome supports syncing to an arbitrary position.

-   `bool HasShutter();`
    Returns `true` if the dome has an electronically controllable shutter.

-   `bool HasVariableSpeed();`
    Returns `true` if the dome supports different configurable speeds.

-   `bool HasBacklash();`
    Returns `true` if the dome supports backlash compensation.

-   `bool isLocked();`
    Checks if the dome is currently locked (e.g., due to mount parking policy).

-   `DomeState getDomeState() const;`
    Returns the current state of the dome.

-   `void setDomeState(const DomeState &value);`
    Sets the current state of the dome.

-   `ShutterState getShutterState() const;`
    Returns the current state of the shutter.

-   `void setShutterState(const ShutterState &value);`
    Sets the current state of the shutter.

-   `IPState getMountState() const;`
    Returns the current state of the mount (snooped).

### Key Methods (Protected Virtual)

These methods are intended to be overridden by the child driver class to implement specific hardware control.

-   `virtual bool SetSpeed(double rpm);`
    Sets the dome's rotation speed. This does not initiate motion.
    -   `rpm`: Desired speed in RPM.
    -   Returns `true` if successful, `false` otherwise.

-   `virtual IPState Move(DomeDirection dir, DomeMotionCommand operation);`
    Moves the dome in a specified direction (clockwise or counter-clockwise).
    -   `dir`: Direction of motion (`DOME_CW` or `DOME_CCW`).
    -   `operation`: Command (`MOTION_START` or `MOTION_STOP`).
    -   Returns `IPS_OK` if complete, `IPS_BUSY` if in progress, `IPS_ALERT` on error.

-   `virtual IPState MoveAbs(double az);`
    Moves the dome to an absolute azimuth position.
    -   `az`: Target azimuth in degrees.
    -   Returns `IPS_OK` if complete, `IPS_BUSY` if in progress, `IPS_ALERT` on error.

-   `virtual IPState MoveRel(double azDiff);`
    Moves the dome by a relative azimuth difference.
    -   `azDiff`: Relative azimuth angle in degrees (positive for CW, negative for CCW).
    -   Returns `IPS_OK` if complete, `IPS_BUSY` if in progress, `IPS_ALERT` on error.

-   `virtual bool Sync(double az);`
    Synchronizes the dome's current azimuth to a given azimuth position.
    -   `az`: Target azimuth in degrees.
    -   Returns `true` if successful, `false` otherwise.

-   `virtual bool Abort();`
    Aborts all dome motion.
    -   Returns `true` if successful, `false` otherwise.

-   `virtual IPState Park();`
    Commands the dome to its park position.
    -   Returns `IPS_OK` if complete, `IPS_BUSY` if in progress, `IPS_ALERT` on error.

-   `virtual IPState UnPark();`
    Commands the dome to unpark. This action is dome-specific and may include opening the shutter and moving to a home position.
    -   Returns `IPS_OK` if complete, `IPS_BUSY` if in progress, `IPS_ALERT` on error.

-   `virtual bool SetBacklash(int32_t steps);`
    Sets the dome backlash compensation value.
    -   `steps`: Value in absolute steps to compensate.
    -   Returns `true` if successful, `false` otherwise.

-   `virtual bool SetBacklashEnabled(bool enabled);`
    Enables or disables dome backlash compensation.
    -   `enabled`: `true` to enable, `false` to disable.
    -   Returns `true` if successful, `false` otherwise.

-   `virtual IPState ControlShutter(ShutterOperation operation);`
    Opens or closes the dome shutter.
    -   `operation`: `SHUTTER_OPEN` or `SHUTTER_CLOSE`.
    -   Returns `IPS_OK` if complete, `IPS_BUSY` if in progress, `IPS_ALERT` on error.

-   `const char * GetShutterStatusString(ShutterState status);`
    Returns a string representation of the shutter status.

-   `void SetParkDataType(DomeParkData type);`
    Sets the type of parking data (e.g., azimuth angle, encoder values).

-   `bool InitPark();`
    Loads parking data from `~/.indi/ParkData.xml`. Should be called after successful connection.
    -   Returns `true` if successful, `false` otherwise.

-   `bool isParked();`
    Checks if the dome is currently parked.
    -   Returns `true` if parked, `false` otherwise.

-   `void SetParked(bool isparked);`
    Changes the dome parking status and updates the park data file.

-   `double GetAxis1Park();`
    Returns the current azimuth parking position.

-   `double GetAxis1ParkDefault();`
    Returns the default azimuth parking position.

-   `void SetAxis1Park(double value);`
    Sets the current azimuth parking position.

-   `void SetAxis1ParkDefault(double steps);`
    Sets the default azimuth parking position.

-   `virtual bool SetCurrentPark();`
    Sets the current coordinates/encoders value as the desired parking position. (No action unless subclassed).

-   `virtual bool SetDefaultPark();`
    Sets default coordinates/encoders value as the desired parking position. (No action unless subclassed).

-   `const char * LoadParkData();`
    Loads parking data from XML.

-   `bool WriteParkData();`
    Writes parking data to XML.

-   `bool GetTargetAz(double &Az, double &Alt, double &minAz, double &maxAz);`
    Calculates the required dome azimuth to center the shutter aperture with the telescope.

-   `bool Intersection(point3D p1, point3D p2, double r, double &mu1, double &mu2);`
    Calculates the intersection of a ray and a sphere (used in geometry calculations).

-   `bool OpticalCenter(point3D MountCenter, double dOpticalAxis, double Lat, double Ah, point3D &OP);`
    Calculates the distance from the optical axis to the dome center.

-   `bool OpticalVector(double Az, double Alt, point3D &OV);`
    Calculates a second point for determining the optical axis.

-   `bool CheckHorizon(double HA, double dec, double lat);`
    Checks if the telescope points above the horizon.

-   `virtual bool saveConfigItems(FILE * fp) override;`
    Saves device port and dome presets in the configuration file.

-   `void UpdateMountCoords();`
    Updates the horizontal coordinates (Az & Alt) of the mount from snooped RA, DEC, and observer's location.

-   `virtual void UpdateAutoSync();`
    Calculates target dome azimuth from mount's target coordinates and commands dome motion if the difference exceeds the AutoSync threshold.

-   `virtual bool Handshake();`
    Performs a handshake with the device to check communication.

-   `virtual void ActiveDevicesUpdated() {};`
    Signal to concrete driver when Active Devices are updated.

-   `double Csc(double x);`
    Calculates cosecant.

-   `double Sec(double x);`
    Calculates secant.

### Member Variables

The `INDI::Dome` class includes numerous `INDI::Property` members and other significant variables for managing dome state and configuration:

-   `INDI::PropertyNumber DomeSpeedNP;`
    Property for setting and reporting dome speed.

-   `INDI::PropertySwitch DomeMotionSP;`
    Property for controlling continuous dome motion (CW/CCW, Start/Stop).

-   `INDI::PropertyNumber DomeAbsPosNP;`
    Property for setting and reporting absolute azimuth position.

-   `INDI::PropertyNumber DomeRelPosNP;`
    Property for setting and reporting relative azimuth movement.

-   `INDI::PropertySwitch AbortSP;`
    Property for aborting dome motion.

-   `INDI::PropertyNumber DomeParamNP;`
    Generic number property for dome parameters.

-   `INDI::PropertyNumber DomeSyncNP;`
    Property for syncing the dome to a specific azimuth.

-   `INDI::PropertySwitch DomeShutterSP;`
    Property for controlling the shutter (Open/Close).

-   `INDI::PropertySwitch ParkSP;`
    Property for parking and unparking the dome.

-   `INDI::PropertyNumber ParkPositionNP;`
    Property for setting a custom park position.

-   `INDI::PropertySwitch ParkOptionSP;`
    Property for various parking options.

-   `INDI::PropertyText ActiveDeviceTP;`
    Property for specifying active devices (Mount, Input, Output) for slaving.

-   `INDI::PropertySwitch MountPolicySP;`
    Property for defining mount locking policy (ignored, locks).

-   `INDI::PropertySwitch ShutterParkPolicySP;`
    Property for defining shutter behavior during park/unpark (close on park, open on unpark).

-   `INDI::PropertyNumber PresetNP;`
    Property for dome presets.

-   `INDI::PropertySwitch PresetGotoSP;`
    Property for initiating movement to presets.

-   `INDI::PropertyNumber DomeMeasurementsNP;`
    Property for dome measurement parameters (radius, shutter width, displacements, etc.).

-   `INDI::PropertySwitch OTASideSP;`
    Property for specifying the side of the OTA (Optical Tube Assembly) relative to the mount.

-   `INDI::PropertySwitch DomeAutoSyncSP;`
    Property for enabling/disabling and configuring auto-synchronization.

-   `INDI::PropertySwitch DomeBacklashSP;`
    Property for enabling/disabling backlash compensation.

-   `INDI::PropertyNumber DomeBacklashNP;`
    Property for setting backlash compensation steps.

-   `uint32_t capability;`
    Stores the bitmask of dome capabilities.

-   `DomeParkData parkDataType;`
    Stores the type of parking data.

-   `DomeState m_DomeState;`
    Current state of the dome.

-   `ShutterState m_ShutterState;`
    Current state of the shutter.

-   `IPState m_MountState;`
    Current state of the snooped mount.

-   `IGeographicCoordinates observer;`
    Observer's geographic coordinates (snooped from mount).

-   `bool HaveLatLong;`
    Indicates if valid geographic coordinates are available.

-   `INDI::IHorizontalCoordinates mountHoriztonalCoords;`
    Mount's horizontal coordinates (snooped).

-   `INDI::IEquatorialCoordinates mountEquatorialCoords;`
    Mount's equatorial coordinates (snooped).

-   `bool HaveRaDec;`
    Indicates if valid RA/Dec coordinates are available.

-   `int PortFD;`
    File descriptor for serial/TCP connection.

-   `Connection::Serial * serialConnection;`
    Pointer to serial connection object.

-   `Connection::TCP * tcpConnection;`
    Pointer to TCP connection object.

-   `bool IsParked;`
    Internal flag indicating if the dome is parked.

-   `bool IsMountParked;`
    Internal flag indicating if the mount is parked.

-   `bool IsLocked;`
    Internal flag indicating if the dome is locked.

-   `bool AutoSyncWarning;`
    Internal flag for auto-sync warnings.

-   `bool UseHourAngle;`
    Internal flag for using hour angle in calculations.

-   `const char * ParkDeviceName;`
    Name of the device used for parking.

-   `const std::string ParkDataFileName;`
    File name for parking data.

-   `INDI::Timer m_MountUpdateTimer;`
    Timer for mount updates.

-   `XMLEle * ParkdataXmlRoot, *ParkdeviceXml, *ParkstatusXml, *ParkpositionXml, *ParkpositionAxis1Xml;`
    XML elements for parsing parking data.

-   `double Axis1ParkPosition;`
    Current park position for Axis 1 (Azimuth).

-   `double Axis1DefaultParkPosition;`
    Default park position for Axis 1 (Azimuth).

-   `uint8_t domeConnection;`
    Stores the dome connection type.

### Example Implementation

Here's a simplified example of how a driver might implement the `INDI::Dome` interface, drawing inspiration from the `domepro2.cpp` driver. This example focuses on the core structure and omits complex serial communication and detailed error handling for clarity.

```cpp
#include "indibase.h"
#include "indidome.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <cmath> // For std::abs

// Forward declaration of a dummy DefaultDevice for the example
// INDI::Dome already inherits from DefaultDevice, so no need to inherit it again.
class MyDomeDevice : public INDI::Dome
{
public:
    MyDomeDevice() : INDI::Dome()
    {
        // Set dome capabilities in the constructor
        SetDomeCapability(DOME_CAN_ABORT |
                          DOME_CAN_ABS_MOVE |
                          DOME_CAN_REL_MOVE |
                          DOME_CAN_PARK |
                          DOME_CAN_SYNC |
                          DOME_HAS_SHUTTER);

        // Set the driver interface
        setDriverInterface(DOME_INTERFACE);
    }

    virtual const char *getDefaultName() override
    {
        return "MyDome";
    }

    virtual bool initProperties() override
    {
        INDI::Dome::initProperties(); // Call base class initProperties

        // Example of defining custom properties if needed, beyond what INDI::Dome provides
        // For instance, a property for dome specific settings
        // MyCustomDomeSettingNP.fill(...);
        // defineProperty(MyCustomDomeSettingNP);

        // Initialize parking data
        SetParkDataType(PARK_AZ); // This dome parks by azimuth angle
        if (InitPark())
        {
            // If loading parking data is successful, we just set the default parking values.
            SetAxis1ParkDefault(0); // Default park azimuth is 0 degrees
        }
        else
        {
            // Otherwise, we set all parking data to default in case no parking data is found.
            SetAxis1Park(0);
            SetAxis1ParkDefault(0);
        }

        return true;
    }

    virtual bool updateProperties() override
    {
        INDI::Dome::updateProperties(); // Call base class updateProperties

        if (isConnected())
        {
            // Define any custom properties here if they depend on connection status
            // defineProperty(MyCustomDomeSettingNP);
            SetTimer(getCurrentPollingPeriod()); // Start timer for periodic updates
        }
        else
        {
            // Delete any custom properties here
            // deleteProperty(MyCustomDomeSettingNP);
        }
        return true;
    }

    // Implement the crucial virtual methods from INDI::Dome

    virtual IPState MoveAbs(double az) override
    {
        LOGF_INFO("Dome: Moving to absolute azimuth %.2f degrees.", az);
        // Simulate hardware action
        std::thread([this, az]() {
            std::this_thread::sleep_for(std::chrono::seconds(5)); // Simulate motion time
            m_currentAzimuth = az; // Update internal state
            LOGF_INFO("Dome: Reached absolute azimuth %.2f degrees.", az);
            DomeAbsPosNP[0].setValue(m_currentAzimuth);
            DomeAbsPosNP.setState(IPS_OK);
            IDSetNumber(&DomeAbsPosNP, nullptr); // Notify clients
            setDomeState(DOME_IDLE);
        }).detach();

        setDomeState(DOME_MOVING);
        DomeAbsPosNP.setState(IPS_BUSY);
        IDSetNumber(&DomeAbsPosNP, nullptr);
        return IPS_BUSY;
    }

    virtual IPState MoveRel(double azDiff) override
    {
        double targetAz = m_currentAzimuth + azDiff;
        // Handle wrap-around for azimuth (0-360 degrees)
        if (targetAz < 0) targetAz += 360;
        if (targetAz >= 360) targetAz -= 360;

        LOGF_INFO("Dome: Moving relative by %.2f degrees to target %.2f.", azDiff, targetAz);
        return MoveAbs(targetAz); // Delegate to MoveAbs
    }

    virtual bool Sync(double az) override
    {
        LOGF_INFO("Dome: Syncing current azimuth to %.2f degrees.", az);
        m_currentAzimuth = az; // Instantly sync in simulation
        DomeAbsPosNP[0].setValue(m_currentAzimuth);
        DomeAbsPosNP.setState(IPS_OK);
        IDSetNumber(&DomeAbsPosNP, nullptr);
        setDomeState(DOME_SYNCED);
        return true;
    }

    virtual IPState Abort() override
    {
        LOG_INFO("Dome: Aborting motion.");
        // In a real driver, send an abort command to hardware.
        // For simulation, just set state to ALERT and IDLE.
        DomeAbsPosNP.setState(IPS_ALERT);
        IDSetNumber(&DomeAbsPosNP, nullptr);
        setDomeState(DOME_IDLE);
        setShutterState(SHUTTER_UNKNOWN); // Shutter state might be unknown after abort
        return IPS_OK;
    }

    virtual IPState Park() override
    {
        LOG_INFO("Dome: Parking (moving to park position and closing shutter).");
        // Simulate parking sequence
        std::thread([this]() {
            // Move to park azimuth
            std::this_thread::sleep_for(std::chrono::seconds(5));
            m_currentAzimuth = GetAxis1Park(); // Get the configured park azimuth
            DomeAbsPosNP[0].setValue(m_currentAzimuth);
            DomeAbsPosNP.setState(IPS_OK);
            IDSetNumber(&DomeAbsPosNP, nullptr);
            LOGF_INFO("Dome: Reached park azimuth %.2f degrees.", m_currentAzimuth);

            // Close shutter
            std::this_thread::sleep_for(std::chrono::seconds(3));
            m_isShutterOpen = false;
            LOG_INFO("Dome: Shutter closed.");
            setShutterState(SHUTTER_CLOSED);

            SetParked(true); // Mark as parked
            setDomeState(DOME_PARKED);
            ParkSP.setState(IPS_OK);
            IDSetSwitch(&ParkSP, nullptr);
        }).detach();

        setDomeState(DOME_PARKING);
        ParkSP.setState(IPS_BUSY);
        IDSetSwitch(&ParkSP, nullptr);
        return IPS_BUSY;
    }

    virtual IPState UnPark() override
    {
        LOG_INFO("Dome: Unparking (opening shutter and moving to home position).");
        // Simulate unparking sequence
        std::thread([this]() {
            // Open shutter
            std::this_thread::sleep_for(std::chrono::seconds(3));
            m_isShutterOpen = true;
            LOG_INFO("Dome: Shutter opened.");
            setShutterState(SHUTTER_OPENED);

            // Move to a default unpark/home azimuth (e.g., 0 degrees)
            std::this_thread::sleep_for(std::chrono::seconds(5));
            m_currentAzimuth = 0; // Example home position
            DomeAbsPosNP[0].setValue(m_currentAzimuth);
            DomeAbsPosNP.setState(IPS_OK);
            IDSetNumber(&DomeAbsPosNP, nullptr);
            LOGF_INFO("Dome: Reached unpark/home azimuth %.2f degrees.", m_currentAzimuth);

            SetParked(false); // Mark as unparked
            setDomeState(DOME_UNPARKED);
            ParkSP.setState(IPS_OK);
            IDSetSwitch(&ParkSP, nullptr);
        }).detach();

        setDomeState(DOME_UNPARKING);
        ParkSP.setState(IPS_BUSY);
        IDSetSwitch(&ParkSP, nullptr);
        return IPS_BUSY;
    }

    virtual IPState ControlShutter(ShutterOperation operation) override
    {
        if (operation == SHUTTER_OPEN)
        {
            LOG_INFO("Dome: Opening shutter.");
            std::thread([this]() {
                std::this_thread::sleep_for(std::chrono::seconds(3));
                m_isShutterOpen = true;
                LOG_INFO("Dome: Shutter opened.");
                setShutterState(SHUTTER_OPENED);
                DomeShutterSP.setState(IPS_OK);
                IDSetSwitch(&DomeShutterSP, nullptr);
            }).detach();
            setShutterState(SHUTTER_MOVING);
            DomeShutterSP.setState(IPS_BUSY);
            IDSetSwitch(&DomeShutterSP, nullptr);
            return IPS_BUSY;
        }
        else if (operation == SHUTTER_CLOSE)
        {
            LOG_INFO("Dome: Closing shutter.");
            std::thread([this]() {
                std::this_thread::sleep_for(std::chrono::seconds(3));
                m_isShutterOpen = false;
                LOG_INFO("Dome: Shutter closed.");
                setShutterState(SHUTTER_CLOSED);
                DomeShutterSP.setState(IPS_OK);
                IDSetSwitch(&DomeShutterSP, nullptr);
            }).detach();
            setShutterState(SHUTTER_MOVING);
            DomeShutterSP.setState(IPS_BUSY);
            IDSetSwitch(&DomeShutterSP, nullptr);
            return IPS_BUSY;
        }
        return IPS_ALERT;
    }

    virtual bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n) override
    {
        // Forward to base class for Dome properties
        if (INDI::Dome::ISNewSwitch(dev, name, states, names, n))
            return true;

        // Handle any custom switch properties here
        return INDI::DefaultDevice::ISNewSwitch(dev, name, states, names, n);
    }

    virtual bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) override
    {
        // Forward to base class for Dome properties
        if (INDI::Dome::ISNewNumber(dev, name, values, names, n))
            return true;

        // Handle any custom number properties here
        return INDI::DefaultDevice::ISNewNumber(dev, name, values, names, n);
    }

    virtual void TimerHit() override
    {
        if (!isConnected())
        {
            SetTimer(getCurrentPollingPeriod());
            return;
        }

        // Update mount coordinates for slaving
        UpdateMountCoords();
        UpdateAutoSync(); // Check for auto-sync

        // Update dome and shutter states based on internal flags (for simulation)
        if (getDomeState() == DOME_MOVING && std::abs(DomeAbsPosNP[0].getValue() - m_currentAzimuth) < 0.1)
        {
            // If motion is "complete" in simulation, set to idle
            setDomeState(DOME_IDLE);
            DomeAbsPosNP.setState(IPS_OK);
            IDSetNumber(&DomeAbsPosNP, nullptr);
        }

        SetTimer(getCurrentPollingPeriod());
    }

private:
    double m_currentAzimuth {0.0};
    bool m_isShutterOpen {false};
};

// This is typically how an INDI driver is instantiated
// static MyDomeDevice myDomeDevice;
