---
sort: 4
---

# Implementing the Telescope Interface

This guide provides a comprehensive overview of implementing the telescope interface in INDI drivers. It covers the basic structure of a telescope driver, how to implement the required methods, and how to handle device-specific functionality.

## Introduction to the Telescope Interface

The telescope interface in INDI is designed for telescope mounts and other pointing devices. It provides a standardized way for clients to control telescope mounts, including slewing to coordinates, tracking celestial objects, and controlling various mount features.

The telescope interface is implemented by inheriting from the `INDI::Telescope` base class, which provides a set of standard properties and methods for controlling telescope mounts. By implementing this interface, your driver can be used with any INDI client that supports telescope devices.

## Prerequisites

Before implementing the telescope interface, you should have:

- Basic knowledge of C++ programming
- Understanding of the INDI protocol and architecture
- Familiarity with the device's communication protocol
- Development environment set up (compiler, build tools, etc.)
- INDI library installed

## Telescope Interface Structure

The telescope interface consists of several key components:

- **Base Class**: `INDI::Telescope` is the base class for all telescope drivers.
- **Standard Properties**: A set of standard properties for controlling telescope mounts.
- **Virtual Methods**: A set of virtual methods that must be implemented by the driver.
- **Helper Methods**: A set of helper methods for common telescope operations.

### Base Class

The `INDI::Telescope` base class inherits from `INDI::DefaultDevice` and provides additional functionality specific to telescope mounts. It defines standard properties for coordinate control, motion control, tracking, and more.

### Standard Properties

The telescope interface defines several standard properties:

- **EQUATORIAL_EOD_COORD**: Controls the equatorial coordinates (right ascension and declination).
- **TARGET_EOD_COORD**: Sets the target equatorial coordinates.
- **HORIZONTAL_COORD**: Controls the horizontal coordinates (altitude and azimuth).
- **TELESCOPE_ABORT_MOTION**: Aborts the current motion.
- **TELESCOPE_MOTION_NS**: Controls the north/south motion.
- **TELESCOPE_MOTION_WE**: Controls the west/east motion.
- **TELESCOPE_PARK**: Parks the telescope.
- **TELESCOPE_PARK_POSITION**: Sets the park position.
- **TELESCOPE_PARK_OPTION**: Sets the park options.
- **TELESCOPE_TRACK_MODE**: Controls the tracking mode.
- **TELESCOPE_TRACK_STATE**: Controls the tracking state.
- **TELESCOPE_TRACK_RATE**: Controls the tracking rate.
- **TELESCOPE_INFO**: Provides information about the telescope.
- **TELESCOPE_PIER_SIDE**: Indicates the pier side.

### Virtual Methods

The telescope interface defines several virtual methods that must be implemented by the driver:

- **MoveNS**: Moves the telescope in the north/south direction.
- **MoveWE**: Moves the telescope in the west/east direction.
- **Abort**: Aborts the current motion.
- **Park**: Parks the telescope.
- **UnPark**: Unparks the telescope.
- **SetTrackMode**: Sets the tracking mode.
- **SetTrackEnabled**: Enables or disables tracking.
- **SetTrackRate**: Sets the tracking rate.
- **Goto**: Slews the telescope to the specified coordinates.
- **Sync**: Synchronizes the telescope's internal coordinates with the specified coordinates.
- **UpdateLocation**: Updates the telescope's location.
- **UpdateTime**: Updates the telescope's time.

### Helper Methods

The telescope interface provides several helper methods for common telescope operations:

- **SetTelescopeCapability**: Sets the telescope capabilities.
- **SetParkDataType**: Sets the park data type.
- **SetDefaultParkPosition**: Sets the default park position.
- **InitPark**: Initializes the park data.
- **SetParkPosition**: Sets the park position.
- **GetParkPosition**: Gets the park position.
- **IsLocked**: Checks if the telescope is locked.
- **SetTrackEnabled**: Sets the tracking state.
- **GetTrackEnabled**: Gets the tracking state.
- **SetTrackMode**: Sets the tracking mode.
- **GetTrackMode**: Gets the tracking mode.
- **SetTrackRate**: Sets the tracking rate.
- **GetTrackRate**: Gets the tracking rate.
- **SetPierSide**: Sets the pier side.
- **GetPierSide**: Gets the pier side.

## Implementing a Basic Telescope Driver

Let's create a simple INDI driver for a hypothetical telescope mount called "MyMount". This mount has a simple serial interface and supports basic commands for slewing, tracking, and parking.

### Step 1: Create the Header File

Create a file named `mymountdriver.h` with the following content:

```cpp
#pragma once

#include <inditelescope.h>

class MyMountDriver : public INDI::Telescope
{
public:
    MyMountDriver();
    virtual ~MyMountDriver() = default;

    // DefaultDevice overrides
    virtual const char *getDefaultName() override;
    virtual bool initProperties() override;
    virtual bool updateProperties() override;
    virtual bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) override;
    virtual bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n) override;

    // Telescope specific overrides
    virtual bool Abort() override;
    virtual bool Park() override;
    virtual bool UnPark() override;
    virtual bool SetTrackMode(uint8_t mode) override;
    virtual bool SetTrackEnabled(bool enabled) override;
    virtual bool SetTrackRate(double raRate, double deRate) override;
    virtual bool Goto(double ra, double dec) override;
    virtual bool Sync(double ra, double dec) override;
    virtual bool MoveNS(INDI_DIR_NS dir, TelescopeMotionCommand command) override;
    virtual bool MoveWE(INDI_DIR_WE dir, TelescopeMotionCommand command) override;
    virtual bool updateLocation(double latitude, double longitude, double elevation) override;
    virtual bool updateTime(ln_date *utc, double utc_offset) override;

protected:
    // Connection overrides
    virtual bool Connect() override;
    virtual bool Disconnect() override;

    // Periodic updates
    virtual void TimerHit() override;

    // Helpers
    bool sendCommand(const char *cmd, char *res = nullptr, int reslen = 0);
    bool readResponse(char *res, int reslen);
    bool getCoords(double *ra, double *dec);
    bool setCoords(double ra, double dec);
    bool isSlewing();
    bool isParked();
    bool isTracking();

private:
    // Device handle
    int PortFD = -1;

    // Current state
    bool Slewing = false;
    bool Tracking = false;
    double CurrentRA = 0;
    double CurrentDEC = 0;
    double TargetRA = 0;
    double TargetDEC = 0;
};
```

### Step 2: Create the Implementation File

Create a file named `mymountdriver.cpp` with the following content:

```cpp
#include "mymountdriver.h"

#include <memory>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <connectionplugins/connectionserial.h>
#include <libnova/julian_day.h>
#include <libnova/sidereal_time.h>

// We declare an auto pointer to MyMountDriver
static std::unique_ptr<MyMountDriver> mymount(new MyMountDriver());

MyMountDriver::MyMountDriver()
{
    // Set the driver version
    setVersion(1, 0);

    // Set the telescope capabilities
    SetTelescopeCapability(
        TELESCOPE_CAN_GOTO |
        TELESCOPE_CAN_SYNC |
        TELESCOPE_CAN_PARK |
        TELESCOPE_CAN_ABORT |
        TELESCOPE_HAS_TIME |
        TELESCOPE_HAS_LOCATION |
        TELESCOPE_HAS_TRACK_MODE |
        TELESCOPE_CAN_CONTROL_TRACK |
        TELESCOPE_HAS_TRACK_RATE,
        4);  // 4 slew rates
}

const char *MyMountDriver::getDefaultName()
{
    return "My Mount";
}

bool MyMountDriver::initProperties()
{
    // Initialize the parent's properties
    INDI::Telescope::initProperties();

    // Add debug, simulation, and configuration controls
    addAuxControls();

    // Initialize park data
    InitPark();

    return true;
}

bool MyMountDriver::updateProperties()
{
    // Call the parent's updateProperties
    INDI::Telescope::updateProperties();

    if (isConnected())
    {
        // Define properties when connected
        // These are already defined by the parent class
    }
    else
    {
        // Delete properties when disconnected
        // These are already deleted by the parent class
    }

    return true;
}

bool MyMountDriver::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    // Check if the message is for this device
    if (!strcmp(dev, getDeviceName()))
    {
        // Handle custom number properties here
    }

    // If the message is not for this device or property, call the parent's ISNewNumber
    return INDI::Telescope::ISNewNumber(dev, name, values, names, n);
}

bool MyMountDriver::ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    // Check if the message is for this device
    if (!strcmp(dev, getDeviceName()))
    {
        // Handle custom switch properties here
    }

    // If the message is not for this device or property, call the parent's ISNewSwitch
    return INDI::Telescope::ISNewSwitch(dev, name, states, names, n);
}

bool MyMountDriver::Connect()
{
    // Call the parent's Connect method
    bool result = INDI::Telescope::Connect();

    if (result)
    {
        // Get the file descriptor for the serial port
        PortFD = serialConnection->getPortFD();

        // Send a test command to verify the connection
        if (!sendCommand("PING\r\n"))
        {
            LOG_ERROR("Failed to communicate with the mount");
            return false;
        }

        // Get the current coordinates
        if (!getCoords(&CurrentRA, &CurrentDEC))
        {
            LOG_ERROR("Failed to get coordinates from the mount");
            return false;
        }

        // Set the initial target coordinates to the current coordinates
        TargetRA = CurrentRA;
        TargetDEC = CurrentDEC;

        // Check if the mount is parked
        SetParked(isParked());

        // Check if the mount is tracking
        Tracking = isTracking();
        TrackState = Tracking ? SCOPE_TRACKING : SCOPE_IDLE;

        // Start the timer
        SetTimer(POLLMS);

        LOG_INFO("Mount connected successfully");
    }

    return result;
}

bool MyMountDriver::Disconnect()
{
    // Close the serial port
    if (PortFD > 0)
    {
        close(PortFD);
        PortFD = -1;
    }

    // Call the parent's Disconnect method
    return INDI::Telescope::Disconnect();
}

bool MyMountDriver::Abort()
{
    // Send the abort command to the mount
    if (!sendCommand("ABORT\r\n"))
    {
        LOG_ERROR("Failed to abort mount motion");
        return false;
    }

    // Update the state
    Slewing = false;
    TrackState = SCOPE_IDLE;

    LOG_INFO("Mount motion aborted");
    return true;
}

bool MyMountDriver::Park()
{
    // Check if we're already parked
    if (isParked())
    {
        LOG_INFO("Mount is already parked");
        return true;
    }

    // Send the park command to the mount
    if (!sendCommand("PARK\r\n"))
    {
        LOG_ERROR("Failed to park the mount");
        return false;
    }

    // Update the state
    Slewing = true;
    TrackState = SCOPE_PARKING;

    LOG_INFO("Parking mount...");
    return true;
}

bool MyMountDriver::UnPark()
{
    // Check if we're already unparked
    if (!isParked())
    {
        LOG_INFO("Mount is already unparked");
        return true;
    }

    // Send the unpark command to the mount
    if (!sendCommand("UNPARK\r\n"))
    {
        LOG_ERROR("Failed to unpark the mount");
        return false;
    }

    // Update the state
    SetParked(false);
    TrackState = SCOPE_IDLE;

    LOG_INFO("Mount unparked");
    return true;
}

bool MyMountDriver::SetTrackMode(uint8_t mode)
{
    // Send the track mode command to the mount
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "TRACK_MODE %d\r\n", mode);
    if (!sendCommand(cmd))
    {
        LOG_ERROR("Failed to set track mode");
        return false;
    }

    // Update the state
    LOG_INFO("Track mode set");
    return true;
}

bool MyMountDriver::SetTrackEnabled(bool enabled)
{
    // Send the track state command to the mount
    const char *cmd = enabled ? "TRACK_ON\r\n" : "TRACK_OFF\r\n";
    if (!sendCommand(cmd))
    {
        LOG_ERROR("Failed to set track state");
        return false;
    }

    // Update the state
    Tracking = enabled;
    TrackState = enabled ? SCOPE_TRACKING : SCOPE_IDLE;

    LOG_INFO("Track state set");
    return true;
}

bool MyMountDriver::SetTrackRate(double raRate, double deRate)
{
    // Send the track rate command to the mount
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "TRACK_RATE %.6f %.6f\r\n", raRate, deRate);
    if (!sendCommand(cmd))
    {
        LOG_ERROR("Failed to set track rate");
        return false;
    }

    // Update the state
    LOG_INFO("Track rate set");
    return true;
}

bool MyMountDriver::Goto(double ra, double dec)
{
    // Check if we're already slewing
    if (Slewing)
    {
        LOG_ERROR("Mount is already slewing");
        return false;
    }

    // Send the goto command to the mount
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "GOTO %.6f %.6f\r\n", ra, dec);
    if (!sendCommand(cmd))
    {
        LOG_ERROR("Failed to slew the mount");
        return false;
    }

    // Update the state
    TargetRA = ra;
    TargetDEC = dec;
    Slewing = true;
    TrackState = SCOPE_SLEWING;

    LOGF_INFO("Slewing to RA: %.6f - DEC: %.6f", ra, dec);
    return true;
}

bool MyMountDriver::Sync(double ra, double dec)
{
    // Send the sync command to the mount
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "SYNC %.6f %.6f\r\n", ra, dec);
    if (!sendCommand(cmd))
    {
        LOG_ERROR("Failed to sync the mount");
        return false;
    }

    // Update the state
    CurrentRA = ra;
    CurrentDEC = dec;
    TargetRA = ra;
    TargetDEC = dec;

    LOGF_INFO("Synced to RA: %.6f - DEC: %.6f", ra, dec);
    return true;
}

bool MyMountDriver::MoveNS(INDI_DIR_NS dir, TelescopeMotionCommand command)
{
    // Send the move command to the mount
    char cmd[32];
    if (command == MOTION_START)
    {
        snprintf(cmd, sizeof(cmd), "MOVE_NS %d\r\n", (dir == DIRECTION_NORTH) ? 1 : 0);
    }
    else
    {
        snprintf(cmd, sizeof(cmd), "STOP_NS\r\n");
    }

    if (!sendCommand(cmd))
    {
        LOG_ERROR("Failed to move the mount");
        return false;
    }

    // Update the state
    LOGF_INFO("Moving %s", (dir == DIRECTION_NORTH) ? "North" : "South");
    return true;
}

bool MyMountDriver::MoveWE(INDI_DIR_WE dir, TelescopeMotionCommand command)
{
    // Send the move command to the mount
    char cmd[32];
    if (command == MOTION_START)
    {
        snprintf(cmd, sizeof(cmd), "MOVE_WE %d\r\n", (dir == DIRECTION_WEST) ? 1 : 0);
    }
    else
    {
        snprintf(cmd, sizeof(cmd), "STOP_WE\r\n");
    }

    if (!sendCommand(cmd))
    {
        LOG_ERROR("Failed to move the mount");
        return false;
    }

    // Update the state
    LOGF_INFO("Moving %s", (dir == DIRECTION_WEST) ? "West" : "East");
    return true;
}

bool MyMountDriver::updateLocation(double latitude, double longitude, double elevation)
{
    // Send the location command to the mount
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "LOCATION %.6f %.6f %.2f\r\n", latitude, longitude, elevation);
    if (!sendCommand(cmd))
    {
        LOG_ERROR("Failed to update location");
        return false;
    }

    // Update the state
    LOGF_INFO("Location updated: Lat %.6f, Lon %.6f, Elev %.2f", latitude, longitude, elevation);
    return true;
}

bool MyMountDriver::updateTime(ln_date *utc, double utc_offset)
{
    // Convert the date to a Julian day
    double jd = ln_get_julian_day(utc);

    // Send the time command to the mount
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "TIME %.6f %.2f\r\n", jd, utc_offset);
    if (!sendCommand(cmd))
    {
        LOG_ERROR("Failed to update time");
        return false;
    }

    // Update the state
    LOGF_INFO("Time updated: JD %.6f, Offset %.2f", jd, utc_offset);
    return true;
}

void MyMountDriver::TimerHit()
{
    // Check if we're connected
    if (!isConnected())
        return;

    // Get the current coordinates
    double newRA, newDEC;
    if (getCoords(&newRA, &newDEC))
    {
        // Update the current coordinates
        CurrentRA = newRA;
        CurrentDEC = newDEC;

        // Update the telescope state
        NewRaDec(CurrentRA, CurrentDEC);
    }

    // Check if we're slewing
    if (Slewing)
    {
        // Check if the slew is complete
        if (!isSlewing())
        {
            // Slew is complete
            Slewing = false;
            if (TrackState == SCOPE_SLEWING)
                TrackState = SCOPE_TRACKING;
            else if (TrackState == SCOPE_PARKING)
                SetParked(true);

            LOG_INFO("Slew complete");
        }
    }

    // Check if we're tracking
    if (TrackState == SCOPE_TRACKING)
    {
        // Check if tracking is still enabled
        if (!isTracking())
        {
            // Tracking is disabled
            Tracking = false;
            TrackState = SCOPE_IDLE;

            LOG_INFO("Tracking stopped");
        }
    }

    // Set the timer for the next update
    SetTimer(POLLMS);
}

bool MyMountDriver::sendCommand(const char *cmd, char *res, int reslen)
{
    // Check if the port is open
    if (PortFD < 0)
    {
        LOG_ERROR("Serial port not open");
        return false;
    }

    // Write the command
    int nbytes_written = write(PortFD, cmd, strlen(cmd));
    if (nbytes_written < 0)
    {
        LOGF_ERROR("Error writing to mount: %s", strerror(errno));
        return false;
    }

    // If no response is expected, return success
    if (res == nullptr || reslen <= 0)
        return true;

    // Read the response
    if (!readResponse(res, reslen))
    {
        LOG_ERROR("Error reading response from mount");
        return false;
    }

    return true;
}

bool MyMountDriver::readResponse(char *res, int reslen)
{
    // Check if the port is open
    if (PortFD < 0)
    {
        LOG_ERROR("Serial port not open");
        return false;
    }

    // Read the response
    int nbytes_read = read(PortFD, res, reslen - 1);
    if (nbytes_read < 0)
    {
        LOGF_ERROR("Error reading from mount: %s", strerror(errno));
        return false;
    }

    // Null-terminate the response
    res[nbytes_read] = '\0';

    return true;
}

bool MyMountDriver::getCoords(double *ra, double *dec)
{
    // Send the get coordinates command to the mount
    char res[32];
    if (!sendCommand("GET_COORDS\r\n", res, sizeof(res)))
    {
        LOG_ERROR("Failed to get coordinates from the mount");
        return false;
    }

    // Parse the response
    if (sscanf(res, "COORDS %lf %lf", ra, dec) != 2)
    {
        LOGF_ERROR("Failed to parse coordinates: %s", res);
        return false;
    }

    return true;
}

bool MyMountDriver::setCoords(double ra, double dec)
{
    // Send the set coordinates command to the mount
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "SET_COORDS %.6f %.6f\r\n", ra, dec);
    if (!sendCommand(cmd))
    {
        LOG_ERROR("Failed to set coordinates");
        return false;
    }

    return true;
}

bool MyMountDriver::isSlewing()
{
    // Send the get status command to the mount
    char res[32];
    if (!sendCommand("GET_STATUS\r\n", res, sizeof(res)))
    {
        LOG_ERROR("Failed to get status from the mount");
        return false;
    }

    // Parse the response
    int slewing = 0;
    if (sscanf(res, "STATUS %d", &slewing) != 1)
    {
        LOGF_ERROR("Failed to parse status: %s", res);
        return false;
    }

    return slewing != 0;
}

bool MyMountDriver::isParked()
{
    // Send the get park status command to the mount
    char res[32];
    if (!sendCommand("GET_PARK\r\n", res, sizeof(res)))
    {
        LOG_ERROR("Failed to get park status from the mount");
        return false;
    }

    // Parse the response
    int parked = 0;
    if (sscanf(res, "PARK %d", &parked) != 1)
    {
        LOGF_ERROR("Failed to parse park status: %s", res);
        return false;
    }

    return parked != 0;
}

bool MyMountDriver::isTracking()
{
    // Send the get tracking status command to the mount
    char res[32];
    if (!sendCommand("GET_TRACK\r\n", res, sizeof(res)))
    {
        LOG_ERROR("Failed to get tracking status from the mount");
        return false;
    }

    // Parse the response
    int tracking = 0;
    if (sscanf(res, "TRACK %d", &tracking) != 1)
    {
        LOGF_ERROR("Failed to parse tracking status: %s", res);
        return false;
    }

    return tracking != 0;
}
```

### Step 3: Create the Main File

Create a file named `main.cpp` with the following content:

```cpp
#include "mymountdriver.h"

int main(int argc, char *argv[])
{
    // Create and initialize the driver
    std::unique_ptr<MyMountDriver> mymount(new MyMountDriver());

    // Set the driver version
    mymount->setVersion(1, 0);

    // Start the driver
    mymount->ISGetProperties(nullptr);

    // Run the driver
    return mymount->run();
}
```

### Step 4: Create the CMakeLists.txt File

Create a file named `CMakeLists.txt` with the following content:

```cmake
cmake_minimum_required(VERSION 3.0)
project(indi-mymount CXX C)

include(GNUInstallDirs)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")

find_package(INDI REQUIRED)
find_package(Nova REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${INDI_INCLUDE_DIR})
include_directories(${NOVA_INCLUDE_DIR})

include(CMakeCommon)

add_executable(indi_mymount mymountdriver.cpp main.cpp)

target_link_libraries(indi_mymount ${INDI_LIBRARIES} ${NOVA_LIBRARIES})

install(TARGETS indi_mymount RUNTIME DESTINATION bin)
```

### Step 5: Create the XML File

Create a file named `indi_mymount.xml` with the following content:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<driversList>
   <devGroup group="Telescopes">
      <device label="My Mount" manufacturer="INDI">
         <driver name="My Mount">indi_mymount</driver>
         <version>1.0</version>
      </device>
   </devGroup>
</driversList>
```

### Step 6: Build the Driver

To build the driver, create a `build` directory and run CMake:

```bash
mkdir build
cd build
cmake ..
make
```

### Step 7: Install the Driver

To install the driver, run:

```bash
sudo make install
```

This will install the driver executable to `/usr/bin` and the XML file to `/usr/share/indi`.

### Step 8: Test the Driver

To test the driver, start the INDI server with your driver:

```bash
indiserver -v indi_mymount
```

Then, connect to the INDI server using an INDI client, such as the INDI Control Panel:

```bash
indi_control_panel
```

## Advanced Topics

### Pier Side

The telescope interface includes support for pier side, which indicates which side of the pier the telescope is on. This is important for German equatorial mounts (GEMs), which can point to the same position in the sky from two different physical orientations.

To implement pier side support, you need to set the pier side capability and implement the pier side-specific methods.

To set the pier side capability, use the `SetTelescopeCapability` method:

```cpp
MyMountDriver::MyMountDriver()
{
    // Set the driver version
    setVersion(1, 0);

    // Set the telescope capabilities
    SetTelescopeCapability(
        TELESCOPE_CAN_GOTO |
        TELESCOPE_CAN_SYNC |
        TELESCOPE_CAN_PARK |
        TELESCOPE_CAN_ABORT |
        TELESCOPE_HAS_TIME |
        TELESCOPE_HAS_LOCATION |
        TELESCOPE_HAS_TRACK_MODE |
        TELESCOPE_CAN_CONTROL_TRACK |
        TELESCOPE_HAS_TRACK_RATE |
        TELESCOPE_HAS_PIER_SIDE,
        4);  // 4 slew rates
}
```

To implement the pier side-specific methods, override the `GetPierSide` method:

```cpp
IPState MyMountDriver::GetPierSide()
{
    // Send the get pier side command to the mount
    char res[32];
    if (!sendCommand("GET_PIER_SIDE\r\n", res, sizeof(res)))
    {
        LOG_ERROR("Failed to get pier side from the mount");
        return IPS_ALERT;
    }

    // Parse the response
    int pierSide = 0;
    if (sscanf(res, "PIER_SIDE %d", &pierSide) != 1)
    {
        LOGF_ERROR("Failed to parse pier side: %s", res);
        return IPS_ALERT;
    }

    // Set the pier side
    setPierSide(pierSide == 0 ? PIER_EAST : PIER_WEST);

    return IPS_OK;
}
```

### Parking

The telescope interface includes support for parking, which allows the telescope to be moved to a safe position when not in use. To implement parking support, you need to set the park capability and implement the park-specific methods.

To set the park capability, use the `SetTelescopeCapability` method:

```cpp
MyMountDriver::MyMountDriver()
{
    // Set the driver version
    setVersion(1, 0);

    // Set the telescope capabilities
    SetTelescopeCapability(
        TELESCOPE_CAN_GOTO |
        TELESCOPE_CAN_SYNC |
        TELESCOPE_CAN_PARK |
        TELESCOPE_CAN_ABORT |
        TELESCOPE_HAS_TIME |
        TELESCOPE_HAS_LOCATION |
        TELESCOPE_HAS_TRACK_MODE |
        TELESCOPE_CAN_CONTROL_TRACK |
        TELESCOPE_HAS_TRACK_RATE,
        4);  // 4 slew rates
}
```

To implement the park-specific methods, override the `Park` and `UnPark` methods:

```cpp
bool MyMountDriver::Park()
{
    // Check if we're already parked
    if (isParked())
    {
        LOG_INFO("Mount is already parked");
        return true;
    }

    // Send the park command to the mount
    if (!sendCommand("PARK\r\n"))
    {
        LOG_ERROR("Failed to park the mount");
        return false;
    }

    // Update the state
    Slewing = true;
    TrackState = SCOPE_PARKING;

    LOG_INFO("Parking mount...");
    return true;
}

bool MyMountDriver::UnPark()
{
    // Check if we're already unparked
    if (!isParked())
    {
        LOG_INFO("Mount is already unparked");
        return true;
    }

    // Send the unpark command to the mount
    if (!sendCommand("UNPARK\r\n"))
    {
        LOG_ERROR("Failed to unpark the mount");
        return false;
    }

    // Update the state
    SetParked(false);
    TrackState = SCOPE_IDLE;

    LOG_INFO("Mount unparked");
    return true;
}
```

### Tracking Modes

The telescope interface includes support for different tracking modes, such as sidereal, lunar, and solar. To implement tracking mode support, you need to set the track mode capability and implement the track mode-specific methods.

To set the track mode capability, use the `SetTelescopeCapability` method:

```cpp
MyMountDriver::MyMountDriver()
{
    // Set the driver version
    setVersion(1, 0);

    // Set the telescope capabilities
    SetTelescopeCapability(
        TELESCOPE_CAN_GOTO |
        TELESCOPE_CAN_SYNC |
        TELESCOPE_CAN_PARK |
        TELESCOPE_CAN_ABORT |
        TELESCOPE_HAS_TIME |
        TELESCOPE_HAS_LOCATION |
        TELESCOPE_HAS_TRACK_MODE |
        TELESCOPE_CAN_CONTROL_TRACK |
        TELESCOPE_HAS_TRACK_RATE,
        4);  // 4 slew rates
}
```

To implement the track mode-specific methods, override the `SetTrackMode` method:

```cpp
bool MyMountDriver::SetTrackMode(uint8_t mode)
{
    // Send the track mode command to the mount
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "TRACK_MODE %d\r\n", mode);
    if (!sendCommand(cmd))
    {
        LOG_ERROR("Failed to set track mode");
        return false;
    }

    // Update the state
    LOG_INFO("Track mode set");
    return true;
}
```

### Simulation Mode

The telescope interface includes support for simulation mode, which can be used to test the driver without connecting to the actual hardware. To implement simulation mode, check the `isSimulation()` flag and provide simulated responses.

```cpp
bool MyMountDriver::Connect()
{
    // Call the parent's Connect method
    bool result = INDI::Telescope::Connect();

    if (result)
    {
        // Check if we're in simulation mode
        if (isSimulation())
        {
            LOG_INFO("Simulation mode enabled");
            PortFD = 1;
        }
        else
        {
            // Get the file descriptor for the serial port
            PortFD = serialConnection->getPortFD();

            // Send a test command to verify the connection
            if (!sendCommand("PING\r\n"))
            {
                LOG_ERROR("Failed to communicate with the mount");
                return false;
            }
        }

        // Get the current coordinates
        if (!getCoords(&CurrentRA, &CurrentDEC))
        {
            LOG_ERROR("Failed to get coordinates from the mount");
            return false;
        }

        // Set the initial target coordinates to the current coordinates
        TargetRA = CurrentRA;
        TargetDEC = CurrentDEC;

        // Check if the mount is parked
        SetParked(isParked());

        // Check if the mount is tracking
        Tracking = isTracking();
        TrackState = Tracking ? SCOPE_TRACKING : SCOPE_IDLE;

        // Start the timer
        SetTimer(POLLMS);

        LOG_INFO("Mount connected successfully");
    }

    return result;
}

bool MyMountDriver::sendCommand(const char *cmd, char *res, int reslen)
{
    // Check if we're in simulation mode
    if (isSimulation())
    {
        // Simulate a response
        if (res && reslen > 0)
        {
            if (!strcmp(cmd, "GET_COORDS\r\n"))
                snprintf(res, reslen, "COORDS %.6f %.6f\r\n", CurrentRA, CurrentDEC);
            else if (!strcmp(cmd, "GET_STATUS\r\n"))
                snprintf(res, reslen, "STATUS %d\r\n", Slewing ? 1 : 0);
            else if (!strcmp(cmd, "GET_PARK\r\n"))
                snprintf(res, reslen, "PARK %d\r\n", isParked() ? 1 : 0);
            else if (!strcmp(cmd, "GET_TRACK\r\n"))
                snprintf(res, reslen, "TRACK %d\r\n", Tracking ? 1 : 0);
            else if (!strcmp(cmd, "PING\r\n"))
                snprintf(res, reslen, "PONG\r\n");
            else
                snprintf(res, reslen, "ERROR\r\n");
        }
        return true;
    }

    // Check if the port is open
    if (PortFD < 0)
    {
        LOG_ERROR("Serial port not open");
        return false;
    }

    // Write the command
    int nbytes_written = write(PortFD, cmd, strlen(cmd));
    if (nbytes_written < 0)
    {
        LOGF_ERROR("Error writing to mount: %s", strerror(errno));
        return false;
    }

    // If no response is expected, return success
    if (res == nullptr || reslen <= 0)
        return true;

    // Read the response
    if (!readResponse(res, reslen))
    {
        LOG_ERROR("Error reading response from mount");
        return false;
    }

    return true;
}
```

### Custom Properties

The telescope interface allows you to add custom properties to the driver. This can be useful for adding mount-specific functionality that is not covered by the standard properties.

To add a custom property, define the property in the header file and initialize it in the `initProperties` method:

```cpp
// In the header file
private:
    // Custom properties
    INDI::PropertyNumber CustomRateNP {2};
```

```cpp
// In the implementation file
bool MyMountDriver::initProperties()
{
    // Initialize the parent's properties
    INDI::Telescope::initProperties();

    // Initialize custom rate property
    CustomRateNP[0].fill("CUSTOM_RATE_RA", "RA Rate (arcsec/s)", "%6.2f", 0, 100, 1, 15.0);
    CustomRateNP[1].fill("CUSTOM_RATE_DEC", "DEC Rate (arcsec/s)", "%6.2f", 0, 100, 1, 15.0);
    CustomRateNP.fill(getDeviceName(), "CUSTOM_RATE", "Custom Rate", MOTION_TAB, IP_RW, 60, IPS_IDLE);

    // Add debug, simulation, and configuration controls
    addAuxControls();

    // Initialize park data
    InitPark();

    return true;
}

bool MyMountDriver::updateProperties()
{
    // Call the parent's updateProperties
    INDI::Telescope::updateProperties();

    if (isConnected())
    {
        // Define custom properties when connected
        defineProperty(&CustomRateNP);
    }
    else
    {
        // Delete custom properties when disconnected
        deleteProperty(CustomRateNP.name);
    }

    return true;
}

bool MyMountDriver::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    // Check if the message is for this device
    if (!strcmp(dev, getDeviceName()))
    {
        // Check if the message is for the custom rate property
        if (!strcmp(name, CustomRateNP.name))
        {
            // Update the property values
            CustomRateNP.update(values, names, n);

            // Send the custom rate command to the mount
            char cmd[32];
            snprintf(cmd, sizeof(cmd), "CUSTOM_RATE %.6f %.6f\r\n", CustomRateNP[0].getValue(), CustomRateNP[1].getValue());
            if (!sendCommand(cmd))
            {
                LOG_ERROR("Failed to set custom rate");
                CustomRateNP.setState(IPS_ALERT);
                CustomRateNP.apply();
                return false;
            }

            // Update the property state
            LOG_INFO("Custom rate set");
            CustomRateNP.setState(IPS_OK);
            CustomRateNP.apply();
            return true;
        }
    }

    // If the message is not for this device or property, call the parent's ISNewNumber
    return INDI::Telescope::ISNewNumber(dev, name, values, names, n);
}
```

## Best Practices

When implementing the telescope interface, follow these best practices:

- **Use the appropriate base class** for your device.
- **Implement simulation mode** to allow testing without hardware.
- **Provide informative error messages** to help users troubleshoot issues.
- **Handle connection and disconnection gracefully** to avoid resource leaks.
- **Update property states** to reflect the current state of the device.
- **Use appropriate property types** for different kinds of data.
- **Follow the INDI naming conventions** for properties and elements.
- **Document your driver** to help users understand how to use it.
- **Test your driver thoroughly** with different clients and configurations.

## Conclusion

Implementing the telescope interface in INDI drivers involves inheriting from the `INDI::Telescope` base class, implementing the required methods, and handling device-specific functionality. By following the steps and best practices outlined in this guide, you can create robust and feature-rich telescope drivers for your mounts.

For more information, refer to the [INDI Library Documentation](https://www.indilib.org/api/index.html) and the [INDI Driver Development Guide](https://www.indilib.org/develop/developer-manual/100-driver-development.html).
