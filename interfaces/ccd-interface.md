---
sort: 3
---

# Implementing the Camera Interface

This guide provides a comprehensive overview of implementing the camera interface in INDI drivers. It covers the basic structure of a camera driver, how to implement the required methods, and how to handle device-specific functionality.

## Introduction to the Camera Interface

The camera interface in INDI is designed for astronomical cameras and other imaging devices. It provides a standardized way for clients to control imaging devices, including setting exposure parameters, downloading images, and controlling various camera features.

The camera interface is implemented by inheriting from the `INDI::CCD` base class (the class name remains CCD for historical reasons, but it supports all types of cameras including CMOS sensors). This class provides a set of standard properties and methods for controlling imaging devices. By implementing this interface, your driver can be used with any INDI client that supports camera devices.

## Prerequisites

Before implementing the camera interface, you should have:

- Basic knowledge of C++ programming
- Understanding of the INDI protocol and architecture
- Familiarity with the device's communication protocol
- Development environment set up (compiler, build tools, etc.)
- INDI library installed

## Camera Interface Structure

The camera interface consists of several key components:

- **Base Class**: `INDI::CCD` is the base class for all camera drivers.
- **Standard Properties**: A set of standard properties for controlling camera devices.
- **Virtual Methods**: A set of virtual methods that must be implemented by the driver.
- **Helper Methods**: A set of helper methods for common camera operations.

### Base Class

The `INDI::CCD` base class inherits from `INDI::DefaultDevice` and provides additional functionality specific to camera devices. It defines standard properties for exposure control, frame settings, temperature control, and more.

### Standard Properties

The camera interface defines several standard properties:

- **CCD_EXPOSURE**: Controls the exposure duration.
- **CCD_ABORT_EXPOSURE**: Aborts the current exposure.
- **CCD_FRAME**: Controls the frame settings (binning, region of interest).
- **CCD_TEMPERATURE**: Controls and monitors the camera temperature.
- **CCD_COOLER**: Controls the camera cooler.
- **CCD_COOLER_POWER**: Monitors the camera cooler power.
- **CCD_GAIN**: Controls the camera gain.
- **CCD_OFFSET**: Controls the camera offset.
- **CCD_FRAME_TYPE**: Controls the frame type (light, dark, bias, flat).
- **CCD_FRAME_RESET**: Resets the frame to full size.
- **CCD_INFO**: Provides information about the camera (pixel size, bit depth, etc.).
- **CCD_COMPRESSION**: Controls image compression.
- **CCD_RAPID_GUIDE**: Controls rapid guiding.
- **CCD_FAST_EXPOSURE**: Controls fast exposure mode.

### Virtual Methods

The camera interface defines several virtual methods that must be implemented by the driver:

- **StartExposure**: Starts an exposure.
- **AbortExposure**: Aborts the current exposure.
- **UpdateCCDFrame**: Updates the camera frame settings.
- **UpdateCCDBin**: Updates the camera binning settings.
- **UpdateCCDFrameType**: Updates the camera frame type.
- **UpdateCCDCompression**: Updates the camera compression settings.
- **UpdateRapidGuide**: Updates the rapid guide settings.
- **UpdateCCDTemperature**: Updates the camera temperature.

### Helper Methods

The camera interface provides several helper methods for common camera operations:

- **SetCCDParams**: Sets the camera parameters (resolution, pixel size, bit depth, etc.).
- **SetGuiderParams**: Sets the guider parameters (resolution, pixel size, bit depth, etc.).
- **SetCCDCapability**: Sets the camera capabilities (can bin, can subframe, has cooler, etc.).
- **SetCCDTemperature**: Sets the camera temperature.
- **SetCCDTemp**: Sets the camera temperature (alias for SetCCDTemperature).
- **SetGuiderExposure**: Sets the guider exposure.
- **ExposureComplete**: Signals that an exposure is complete.
- **GuideComplete**: Signals that a guide exposure is complete.
- **GrabImage**: Grabs an image from the camera.
- **GrabImageHelper**: Helper function for GrabImage.

## Implementing a Basic Camera Driver

Let's create a simple INDI driver for a hypothetical camera called "MyCamera". This camera has a simple USB interface and supports basic commands for setting exposure, binning, and temperature.

### Step 1: Create the Header File

Create a file named `mycameradriver.h` with the following content:

```cpp
#pragma once

#include <indiccd.h>

class MyCameraDriver : public INDI::CCD
{
public:
    MyCameraDriver();
    virtual ~MyCameraDriver() = default;

    // DefaultDevice overrides
    virtual const char *getDefaultName() override;
    virtual bool initProperties() override;
    virtual bool updateProperties() override;
    virtual bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) override;
    virtual bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n) override;

    // Camera specific overrides
    virtual bool StartExposure(float duration) override;
    virtual bool AbortExposure() override;
    virtual bool UpdateCCDFrame(int x, int y, int w, int h) override;
    virtual bool UpdateCCDBin(int binx, int biny) override;
    virtual bool UpdateCCDFrameType(CCDChip::CCD_FRAME type) override;
    virtual bool UpdateCCDCompression(int compression) override;
    virtual bool UpdateRapidGuide(const char *name, bool enabled) override;
    virtual bool UpdateCCDTemperature() override;

protected:
    // Connection overrides
    virtual bool Connect() override;
    virtual bool Disconnect() override;

    // Helpers
    void setupParams();
    void grabImage();
    static void timerCallback(void *userpointer);

private:
    // Device handle
    int handle = -1;

    // Exposure
    double ExposureRequest = 0;
    struct timeval ExpStart;
    bool InExposure = false;
    int timerID = -1;

    // Temperature
    double TemperatureRequest = 20.0;
    bool TemperatureUpdateRunning = false;
};
```

### Step 2: Create the Implementation File

Create a file named `mycameradriver.cpp` with the following content:

```cpp
#include "mycameradriver.h"

#include <memory>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>

// We declare an auto pointer to MyCameraDriver
static std::unique_ptr<MyCameraDriver> mycamera(new MyCameraDriver());

MyCameraDriver::MyCameraDriver()
{
    // Set the driver version
    setVersion(1, 0);

    // Set the camera capabilities
    SetCCDCapability(CCD_CAN_BIN | CCD_CAN_SUBFRAME | CCD_HAS_COOLER | CCD_HAS_SHUTTER);
}

const char *MyCameraDriver::getDefaultName()
{
    return "My Camera";
}

bool MyCameraDriver::initProperties()
{
    // Initialize the parent's properties
    INDI::CCD::initProperties();

    // Add debug, simulation, and configuration controls
    addAuxControls();

    return true;
}

bool MyCameraDriver::updateProperties()
{
    // Call the parent's updateProperties
    INDI::CCD::updateProperties();

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

bool MyCameraDriver::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    // Check if the message is for this device
    if (!strcmp(dev, getDeviceName()))
    {
        // Handle custom number properties here
    }

    // If the message is not for this device or property, call the parent's ISNewNumber
    return INDI::CCD::ISNewNumber(dev, name, values, names, n);
}

bool MyCameraDriver::ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    // Check if the message is for this device
    if (!strcmp(dev, getDeviceName()))
    {
        // Handle custom switch properties here
    }

    // If the message is not for this device or property, call the parent's ISNewSwitch
    return INDI::CCD::ISNewSwitch(dev, name, states, names, n);
}

bool MyCameraDriver::Connect()
{
    // Call the parent's Connect method
    bool result = INDI::CCD::Connect();

    if (result)
    {
        // Open the device
        handle = open("/dev/mycamera", O_RDWR);
        if (handle < 0)
        {
            LOG_ERROR("Failed to open device");
            return false;
        }

        // Set up the camera parameters
        setupParams();

        LOG_INFO("Device connected successfully");
    }

    return result;
}

bool MyCameraDriver::Disconnect()
{
    // Close the device
    if (handle >= 0)
    {
        close(handle);
        handle = -1;
    }

    // Call the parent's Disconnect method
    return INDI::CCD::Disconnect();
}

void MyCameraDriver::setupParams()
{
    // Set the camera parameters
    // These are the parameters of the camera sensor
    SetCCDParams(1280, 1024, 16, 5.2, 5.2);

    // Set the minimum exposure time
    PrimaryCCD.setMinMaxStep("CCD_EXPOSURE", "CCD_EXPOSURE_VALUE", 0.001, 3600, 0.001);

    // Set the image size
    uint32_t nbuf = PrimaryCCD.getXRes() * PrimaryCCD.getYRes() * PrimaryCCD.getBPP() / 8;
    PrimaryCCD.setFrameBufferSize(nbuf);
}

bool MyCameraDriver::StartExposure(float duration)
{
    // Check if we're already in an exposure
    if (InExposure)
    {
        LOG_ERROR("Camera is already exposing");
        return false;
    }

    // Check if the duration is valid
    if (duration < PrimaryCCD.getMinX())
    {
        LOGF_ERROR("Exposure duration %f is less than minimum %f", duration, PrimaryCCD.getMinX());
        return false;
    }

    // Set the exposure duration
    ExposureRequest = duration;

    // Start the exposure
    gettimeofday(&ExpStart, nullptr);
    InExposure = true;

    // Set the timer for the exposure
    timerID = SetTimer(duration * 1000);

    // Update the exposure status
    PrimaryCCD.setExposureDuration(duration);
    PrimaryCCD.setExposureLeft(duration);

    // Log the exposure start
    LOGF_INFO("Starting exposure of %f seconds", duration);

    return true;
}

bool MyCameraDriver::AbortExposure()
{
    // Check if we're in an exposure
    if (!InExposure)
    {
        LOG_WARNING("No exposure in progress");
        return true;
    }

    // Abort the exposure
    InExposure = false;

    // Remove the timer
    if (timerID > 0)
    {
        RemoveTimer(timerID);
        timerID = -1;
    }

    // Log the exposure abort
    LOG_INFO("Exposure aborted");

    return true;
}

bool MyCameraDriver::UpdateCCDFrame(int x, int y, int w, int h)
{
    // Check if the frame is valid
    if (x < 0 || y < 0 || w <= 0 || h <= 0 || x + w > PrimaryCCD.getXRes() || y + h > PrimaryCCD.getYRes())
    {
        LOGF_ERROR("Invalid frame: x=%d, y=%d, w=%d, h=%d", x, y, w, h);
        return false;
    }

    // Set the frame
    PrimaryCCD.setFrame(x, y, w, h);

    // Calculate the new frame buffer size
    uint32_t nbuf = w * h * PrimaryCCD.getBPP() / 8;
    PrimaryCCD.setFrameBufferSize(nbuf);

    // Log the frame update
    LOGF_INFO("Frame updated: x=%d, y=%d, w=%d, h=%d", x, y, w, h);

    return true;
}

bool MyCameraDriver::UpdateCCDBin(int binx, int biny)
{
    // Check if the binning is valid
    if (binx < 1 || biny < 1 || binx > 4 || biny > 4)
    {
        LOGF_ERROR("Invalid binning: %dx%d", binx, biny);
        return false;
    }

    // Set the binning
    PrimaryCCD.setBin(binx, biny);

    // Calculate the new frame buffer size
    uint32_t nbuf = PrimaryCCD.getSubW() * PrimaryCCD.getSubH() * PrimaryCCD.getBPP() / 8;
    PrimaryCCD.setFrameBufferSize(nbuf);

    // Log the binning update
    LOGF_INFO("Binning updated: %dx%d", binx, biny);

    return true;
}

bool MyCameraDriver::UpdateCCDFrameType(CCDChip::CCD_FRAME type)
{
    // Set the frame type
    PrimaryCCD.setFrameType(type);

    // Log the frame type update
    LOGF_INFO("Frame type updated: %d", type);

    return true;
}

bool MyCameraDriver::UpdateCCDCompression(int compression)
{
    // Set the compression
    PrimaryCCD.setCompression(compression);

    // Log the compression update
    LOGF_INFO("Compression updated: %d", compression);

    return true;
}

bool MyCameraDriver::UpdateRapidGuide(const char *name, bool enabled)
{
    // Check if rapid guide is supported
    if (strcmp(name, "TELESCOPE_TIMED_GUIDE_WE") && strcmp(name, "TELESCOPE_TIMED_GUIDE_NS") && strcmp(name, "SEND_GUIDE_SECONDARY"))
    {
        LOG_ERROR("Rapid guide not supported");
        return false;
    }

    // Set the rapid guide
    RapidGuideEnabled = enabled;

    // Log the rapid guide update
    LOGF_INFO("Rapid guide updated: %s", enabled ? "enabled" : "disabled");

    return true;
}

bool MyCameraDriver::UpdateCCDTemperature()
{
    // Check if we're already updating the temperature
    if (TemperatureUpdateRunning)
        return true;

    // Set the temperature update flag
    TemperatureUpdateRunning = true;

    // Get the current temperature
    double temperature = 0;
    // In a real driver, you would get the temperature from the device
    // For this example, we'll simulate a temperature that approaches the requested temperature
    temperature = TemperatureNP[0].getValue() + (TemperatureRequest - TemperatureNP[0].getValue()) * 0.1;

    // Set the temperature
    TemperatureNP[0].setValue(temperature);

    // Update the temperature status
    if (fabs(temperature - TemperatureRequest) < 0.1)
    {
        TemperatureNP.setState(IPS_OK);
        TemperatureUpdateRunning = false;
    }
    else
    {
        TemperatureNP.setState(IPS_BUSY);
        // Set a timer to update the temperature again
        SetTimer(1000);
    }

    // Send the temperature update to the client
    TemperatureNP.apply();

    return true;
}

void MyCameraDriver::timerCallback(void *userpointer)
{
    // Cast the userpointer to a MyCameraDriver pointer
    MyCameraDriver *driver = static_cast<MyCameraDriver *>(userpointer);

    // Check if we're in an exposure
    if (driver->InExposure)
    {
        // Check if the exposure is complete
        struct timeval now;
        gettimeofday(&now, nullptr);
        double elapsed = now.tv_sec - driver->ExpStart.tv_sec + (now.tv_usec - driver->ExpStart.tv_usec) / 1e6;
        if (elapsed >= driver->ExposureRequest)
        {
            // Exposure is complete
            driver->InExposure = false;
            driver->timerID = -1;

            // Grab the image
            driver->grabImage();
        }
        else
        {
            // Exposure is still in progress
            driver->PrimaryCCD.setExposureLeft(driver->ExposureRequest - elapsed);
            driver->timerID = driver->SetTimer(100);
        }
    }
    else if (driver->TemperatureUpdateRunning)
    {
        // Update the temperature
        driver->UpdateCCDTemperature();
    }
}

void MyCameraDriver::grabImage()
{
    // Allocate memory for the image
    uint8_t *image = PrimaryCCD.getFrameBuffer();
    uint32_t width = PrimaryCCD.getSubW() / PrimaryCCD.getBinX();
    uint32_t height = PrimaryCCD.getSubH() / PrimaryCCD.getBinY();
    uint32_t bpp = PrimaryCCD.getBPP();
    uint32_t nbuf = width * height * bpp / 8;

    // In a real driver, you would get the image from the device
    // For this example, we'll generate a simple gradient
    for (uint32_t y = 0; y < height; y++)
    {
        for (uint32_t x = 0; x < width; x++)
        {
            uint32_t index = (y * width + x) * bpp / 8;
            uint16_t value = (x + y) % 65535;
            if (bpp == 8)
            {
                image[index] = value % 255;
            }
            else
            {
                image[index] = value & 0xFF;
                image[index + 1] = (value >> 8) & 0xFF;
            }
        }
    }

    // Set the image data
    PrimaryCCD.setFrameBuffer(image);
    PrimaryCCD.setFrameBufferSize(nbuf);
    PrimaryCCD.setResolution(width, height);
    PrimaryCCD.setNAxis(2);
    PrimaryCCD.setBPP(bpp);

    // Signal that the exposure is complete
    ExposureComplete(&PrimaryCCD);
}
```

### Step 3: Create the Main File

Create a file named `main.cpp` with the following content:

```cpp
#include "mycameradriver.h"

int main(int argc, char *argv[])
{
    // Create and initialize the driver
    std::unique_ptr<MyCameraDriver> mycamera(new MyCameraDriver());

    // Set the driver version
    mycamera->setVersion(1, 0);

    // Start the driver
    mycamera->ISGetProperties(nullptr);

    // Run the driver
    return mycamera->run();
}
```

### Step 4: Create the CMakeLists.txt File

Create a file named `CMakeLists.txt` with the following content:

```cmake
cmake_minimum_required(VERSION 3.0)
project(indi-mycamera CXX C)

include(GNUInstallDirs)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")

find_package(INDI REQUIRED)
find_package(Nova REQUIRED)
find_package(CFITSIO REQUIRED)
find_package(ZLIB REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${INDI_INCLUDE_DIR})
include_directories(${NOVA_INCLUDE_DIR})
include_directories(${CFITSIO_INCLUDE_DIR})

include(CMakeCommon)

add_executable(indi_mycamera mycameradriver.cpp main.cpp)

target_link_libraries(indi_mycamera ${INDI_LIBRARIES} ${NOVA_LIBRARIES} ${CFITSIO_LIBRARIES} ${ZLIB_LIBRARIES})

install(TARGETS indi_mycamera RUNTIME DESTINATION bin)
```

### Step 5: Create the XML File

Create a file named `indi_mycamera.xml` with the following content:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<driversList>
   <devGroup group="CCDs">
      <device label="My Camera" manufacturer="INDI">
         <driver name="My Camera">indi_mycamera</driver>
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
indiserver -v indi_mycamera
```

Then, connect to the INDI server using an INDI client, such as the INDI Control Panel:

```bash
indi_control_panel
```

## Advanced Topics

### FITS Header

The camera interface allows you to add custom FITS header keywords to the image. This can be useful for adding metadata about the image, such as the telescope position, filter used, or other relevant information.

To add a FITS header keyword, use the `addFITSKeywords` method:

```cpp
void MyCameraDriver::addFITSKeywords(fitsfile *fptr, CCDChip *targetChip)
{
    // Call the parent's addFITSKeywords
    INDI::CCD::addFITSKeywords(fptr, targetChip);

    // Add custom keywords
    int status = 0;
    fits_update_key_s(fptr, TSTRING, "OBSERVER", "John Doe", "Observer name", &status);
    fits_update_key_s(fptr, TSTRING, "OBJECT", "M42", "Object name", &status);
    fits_update_key_s(fptr, TSTRING, "TELESCOP", "My Telescope", "Telescope name", &status);
}
```

### Guider Interface

The camera interface includes support for a guider chip, which can be used for autoguiding. To implement the guider interface, you need to set up the guider parameters and implement the guider-specific methods.

To set up the guider parameters, use the `SetGuiderParams` method:

```cpp
void MyCameraDriver::setupParams()
{
    // Set the camera parameters
    SetCCDParams(1280, 1024, 16, 5.2, 5.2);

    // Set the guider parameters
    SetGuiderParams(512, 512, 16, 5.2, 5.2);

    // ...
}
```

To implement the guider-specific methods, override the `StartGuideExposure` and `AbortGuideExposure` methods:

```cpp
bool MyCameraDriver::StartGuideExposure(float duration)
{
    // Start the guider exposure
    // ...
    return true;
}

bool MyCameraDriver::AbortGuideExposure()
{
    // Abort the guider exposure
    // ...
    return true;
}
```

### Streaming Interface

The camera interface includes support for streaming, which can be used for video capture or live viewing. To implement the streaming interface, you need to set the streaming capability and implement the streaming-specific methods.

To set the streaming capability, use the `SetCCDCapability` method:

```cpp
MyCameraDriver::MyCameraDriver()
{
    // Set the driver version
    setVersion(1, 0);

    // Set the camera capabilities
    SetCCDCapability(CCD_CAN_BIN | CCD_CAN_SUBFRAME | CCD_HAS_COOLER | CCD_HAS_SHUTTER | CCD_HAS_STREAMING);
}
```

To implement the streaming-specific methods, override the `StartStreaming` and `StopStreaming` methods:

```cpp
bool MyCameraDriver::StartStreaming()
{
    // Start the streaming
    // ...
    return true;
}

bool MyCameraDriver::StopStreaming()
{
    // Stop the streaming
    // ...
    return true;
}
```

### Temperature Control

The camera interface includes support for temperature control, which can be used to cool the camera sensor. To implement temperature control, you need to set the temperature capability and implement the temperature-specific methods.

To set the temperature capability, use the `SetCCDCapability` method:

```cpp
MyCameraDriver::MyCameraDriver()
{
    // Set the driver version
    setVersion(1, 0);

    // Set the camera capabilities
    SetCCDCapability(CCD_CAN_BIN | CCD_CAN_SUBFRAME | CCD_HAS_COOLER | CCD_HAS_SHUTTER);
}
```

To implement the temperature-specific methods, override the `SetTemperature` method:

```cpp
bool MyCameraDriver::SetTemperature(double temperature)
{
    // Set the temperature
    TemperatureRequest = temperature;
    TemperatureUpdateRunning = true;

    // Start the temperature update
    UpdateCCDTemperature();

    return true;
}
```

### Simulation Mode

The camera interface includes support for simulation mode, which can be used to test the driver without connecting to the actual hardware. To implement simulation mode, check the `isSimulation()` flag and provide simulated responses.

```cpp
bool MyCameraDriver::Connect()
{
    // Call the parent's Connect method
    bool result = INDI::CCD::Connect();

    if (result)
    {
        // Check if we're in simulation mode
        if (isSimulation())
        {
            LOG_INFO("Simulation mode enabled");
            handle = 1;
        }
        else
        {
            // Open the device
            handle = open("/dev/mycamera", O_RDWR);
            if (handle < 0)
            {
                LOG_ERROR("Failed to open device");
                return false;
            }
        }

        // Set up the camera parameters
        setupParams();

        LOG_INFO("Device connected successfully");
    }

    return result;
}
```

## Best Practices

When implementing the camera interface, follow these best practices:

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

Implementing the camera interface in INDI drivers involves inheriting from the `INDI::CCD` base class, implementing the required methods, and handling device-specific functionality. By following the steps and best practices outlined in this guide, you can create robust and feature-rich camera drivers for your devices.

For more information, refer to the [INDI Library Documentation](https://www.indilib.org/api/index.html) and the [INDI Driver Development Guide](https://www.indilib.org/develop/developer-manual/100-driver-development.html).
