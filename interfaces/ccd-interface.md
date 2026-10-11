---
title: Camera Interface
nav_order: 1
parent: Device Interfaces
---

# Implementing the Camera Interface

This guide provides a comprehensive overview of implementing the camera interface in INDI drivers. It covers the structure of a camera driver, the capabilities a camera can advertise, the methods you need to implement, and how to deliver images to clients.

## Introduction to the Camera Interface

The camera interface is implemented by inheriting from the `INDI::CCD` base class (defined in `indiccd.h`). The class name remains CCD for historical reasons, but it supports all types of astronomical cameras, including CMOS sensors.

`INDI::CCD` handles most of the work that is common to all cameras:

- Defining the standard exposure, frame, binning, temperature, and upload properties
- Encoding images as FITS or XISF (or sending native formats as-is) and uploading them to clients, saving them locally, or both
- Writing FITS headers, including telescope position, focal length, filter, and sky quality snooped from other devices
- Live video streaming and recording through `INDI::StreamManager`
- Guiding through an ST4 port, via `INDI::GuiderInterface`

Your driver only needs to talk to the camera hardware: start and abort exposures, read the image into a buffer, and report temperature.

## Prerequisites

Before implementing the camera interface, you should have:

- Basic knowledge of C++ programming
- Understanding of the INDI protocol and architecture
- Familiarity with the camera's SDK or communication protocol
- Development environment set up (compiler, build tools, etc.)
- INDI library installed

## Camera Interface Structure

The camera interface consists of several key components:

- **Base Class**: `INDI::CCD` inherits from `INDI::DefaultDevice` and `INDI::GuiderInterface`.
- **Chips**: Each camera has a primary chip (`PrimaryCCD`) and an optional guide chip (`GuideCCD`), both of type `INDI::CCDChip`. A chip holds the sensor geometry, the current frame and binning, the exposure properties, and the image frame buffer.
- **Capabilities**: A bitmask that tells the base class which features your camera supports.
- **Virtual Methods**: Methods that your driver overrides to control the hardware.

### CCDCapability Enum

Call `SetCCDCapability()` in your driver's constructor or `initProperties()` to declare what your camera can do. The capabilities can also be updated after the camera is connected (for example, after querying the SDK), but they must be set before `Connect()` returns.

```cpp
enum
{
    CCD_CAN_BIN        = 1 << 0,  /*!< Does the CCD support binning?  */
    CCD_CAN_SUBFRAME   = 1 << 1,  /*!< Does the CCD support setting ROI?  */
    CCD_CAN_ABORT      = 1 << 2,  /*!< Can the CCD exposure be aborted?  */
    CCD_HAS_GUIDE_HEAD = 1 << 3,  /*!< Does the CCD have a guide head?  */
    CCD_HAS_ST4_PORT   = 1 << 4,  /*!< Does the CCD have an ST4 port?  */
    CCD_HAS_SHUTTER    = 1 << 5,  /*!< Does the CCD have a mechanical shutter?  */
    CCD_HAS_COOLER     = 1 << 6,  /*!< Does the CCD have a cooler and temperature control?  */
    CCD_HAS_BAYER      = 1 << 7,  /*!< Does the CCD send color data in bayer format?  */
    CCD_HAS_STREAMING  = 1 << 8,  /*!< Does the CCD support live video streaming?  */
    CCD_HAS_DSP        = 1 << 10  /*!< Does the CCD support image processing?  */
} CCDCapability;
```

| Capability           | Effect                                                                 | Methods to override                                        |
| -------------------- | ---------------------------------------------------------------------- | ---------------------------------------------------------- |
| _(always)_           | Exposure, frame type, info, compression, upload, and format properties | `StartExposure()`                                          |
| `CCD_CAN_ABORT`      | Defines `CCD_ABORT_EXPOSURE`                                           | `AbortExposure()`                                          |
| `CCD_CAN_SUBFRAME`   | Makes `CCD_FRAME` writable, and defines `CCD_FRAME_RESET`              | `UpdateCCDFrame()`                                         |
| `CCD_CAN_BIN`        | Defines `CCD_BINNING` and `CCD_FRAME_RESET`                            | `UpdateCCDBin()`                                           |
| `CCD_HAS_COOLER`     | Defines `CCD_TEMPERATURE` and `CCD_TEMP_RAMP`                          | `SetTemperature()`                                         |
| `CCD_HAS_SHUTTER`    | Allows dark and bias frames to be taken with the shutter closed        | `UpdateCCDFrameType()` (optional)                          |
| `CCD_HAS_BAYER`      | Defines `CCD_CFA` and adds Bayer keywords to the FITS header           | —                                                          |
| `CCD_HAS_ST4_PORT`   | Defines `TELESCOPE_TIMED_GUIDE_NS` and `TELESCOPE_TIMED_GUIDE_WE`      | `GuideNorth()`, `GuideSouth()`, `GuideEast()`, `GuideWest()` |
| `CCD_HAS_GUIDE_HEAD` | Defines the `GUIDER_*` properties for the guide chip                   | `StartGuideExposure()`, `AbortGuideExposure()`, and the `UpdateGuider*()` methods |
| `CCD_HAS_STREAMING`  | Creates `Streamer` and its streaming and recording properties          | `StartStreaming()`, `StopStreaming()`                      |
| `CCD_HAS_DSP`        | Creates the DSP manager for image processing                           | —                                                          |

You do not need to set `CCD_HAS_GUIDE_HEAD` yourself: calling `SetGuiderParams()` sets it.

## Standard Properties

`INDI::CCD` and `INDI::CCDChip` define the following properties. See [Standard Properties](../drivers/standard-properties.md#ccd-properties) for their elements.

| Property                                       | Description                                                                                |
| ---------------------------------------------- | ------------------------------------------------------------------------------------------ |
| `CCD_EXPOSURE`                                 | Start an exposure of the given duration in seconds. Shows the time left while exposing.    |
| `CCD_ABORT_EXPOSURE`                           | Abort the current exposure.                                                                |
| `CCD_FRAME`                                    | Region of interest (X, Y, width, height) in unbinned pixels. Read only unless the camera can subframe. |
| `CCD_FRAME_RESET`                              | Reset the frame to the full sensor and the binning to 1x1.                                 |
| `CCD_BINNING`                                  | Horizontal and vertical binning.                                                           |
| `CCD_FRAME_TYPE`                               | Light, bias, dark, or flat.                                                                |
| `CCD_INFO`                                     | Sensor resolution, pixel size, and bits per pixel (read only).                             |
| `CCD_TEMPERATURE`                              | Current sensor temperature. Setting it requests a new target temperature.                  |
| `CCD_TEMP_RAMP`                                | Maximum temperature change per minute, and the threshold for reaching the target.           |
| `CCD_CAPTURE_FORMAT`                           | Native capture formats added by the driver with `addCaptureFormat()`.                      |
| `CCD_TRANSFER_FORMAT`                          | How images are encoded for upload: FITS, native, or XISF.                                  |
| `CCD_COMPRESSION`                              | Compress images before upload.                                                             |
| `CCD1`                                         | The BLOB used to send the primary chip image to clients (`CCD2` for the guide chip).       |
| `UPLOAD_MODE`, `UPLOAD_SETTINGS`               | Send images to the client, save them locally, or both, and where to save them.             |
| `CCD_FAST_TOGGLE`, `CCD_FAST_COUNT`            | Fast exposure mode: start the next exposure as soon as the previous one completes.         |
| `ACTIVE_DEVICES`                               | Telescope, rotator, focuser, filter wheel, and sky quality meter to snoop for FITS headers. |
| `SCOPE_INFO`                                   | Focal length and aperture, used for FITS headers and plate scale.                          |
| `FITS_HEADER`                                  | Lets clients add custom FITS keywords.                                                     |
| `GUIDER_EXPOSURE`, `GUIDER_FRAME`, ...         | The equivalent properties for the guide chip, if the camera has one.                      |

Common camera features that are **not** defined by the base class, such as gain, offset, and cooler control, are defined by each driver. Use the conventional names so clients can find them:

| Property           | Type   | Elements                  | Description                       |
| ------------------ | ------ | ------------------------- | --------------------------------- |
| `CCD_GAIN`         | Number | `GAIN`                    | Sensor gain                       |
| `CCD_OFFSET`       | Number | `OFFSET`                  | Sensor offset                     |
| `CCD_COOLER`       | Switch | `COOLER_ON`, `COOLER_OFF` | Turn the cooler on or off         |
| `CCD_COOLER_POWER` | Number | `CCD_COOLER_VALUE`        | Cooler power in percent (read only) |

## Key Methods

### Virtual Methods to Override

- `virtual bool StartExposure(float duration)`:
  Starts an exposure of `duration` seconds on the primary chip. **Required.** Return `true` if the exposure started, or `false` on error. Do not block until the exposure ends; report completion from `TimerHit()` or an SDK callback, as described in [The Exposure Lifecycle](#the-exposure-lifecycle).

- `virtual bool AbortExposure()`:
  Aborts the current exposure. Required for `CCD_CAN_ABORT`.

- `virtual bool UpdateCCDFrame(int x, int y, int w, int h)`:
  Called when the client changes the region of interest. Coordinates are in unbinned pixels, and (0,0) is the top-left pixel. Configure the hardware, then call `INDI::CCD::UpdateCCDFrame()` (or `PrimaryCCD.setFrame()`) to store the new frame, and resize the frame buffer. Return `false` if the frame is not supported.

- `virtual bool UpdateCCDBin(int hor, int ver)`:
  Called when the client changes binning. Configure the hardware, then call `INDI::CCD::UpdateCCDBin()` (or `PrimaryCCD.setBin()`) to store the new binning, and resize the frame buffer. Return `false` if the binning is not supported.

- `virtual bool UpdateCCDFrameType(INDI::CCDChip::CCD_FRAME fType)`:
  Called when the client changes the frame type (`LIGHT_FRAME`, `BIAS_FRAME`, `DARK_FRAME`, or `FLAT_FRAME`). Optional: you can instead check `PrimaryCCD.getFrameType()` when the exposure starts, for example to keep the shutter closed for dark and bias frames.

- `virtual int SetTemperature(double temperature)`:
  Sets the target sensor temperature in °C. Required for `CCD_HAS_COOLER`. Return:
  - `1` if the camera is already at the requested temperature
  - `0` if the camera is moving toward it. The base class sets `CCD_TEMPERATURE` to busy and sets it back to OK once the temperature you report is within the ramp threshold.
  - `-1` on error

- `virtual bool SetCaptureFormat(uint8_t index)`:
  Called when the client selects a capture format from `CCD_CAPTURE_FORMAT`. `index` is the order in which you added the formats with `addCaptureFormat()`.

- `virtual bool StartStreaming()` / `virtual bool StopStreaming()`:
  Start and stop live video. Required for `CCD_HAS_STREAMING`.

- `virtual IPState GuideNorth(uint32_t ms)`, `GuideSouth()`, `GuideEast()`, `GuideWest()`:
  Send an ST4 guide pulse of `ms` milliseconds. Required for `CCD_HAS_ST4_PORT`. See the [Guider Interface](guider.md) for return values.

- `virtual bool StartGuideExposure(float duration)`, `AbortGuideExposure()`, `UpdateGuiderFrame()`, `UpdateGuiderBin()`, `UpdateGuiderFrameType()`:
  The equivalent methods for the guide chip.

- `virtual void addFITSKeywords(INDI::CCDChip *targetChip, std::vector<INDI::FITSRecord> &fitsKeywords)`:
  Adds FITS header keywords. Override it to add camera-specific keywords, and call the base implementation first.

- `virtual void UploadComplete(INDI::CCDChip *targetChip)`:
  Called after an image has been uploaded or saved. Override it if you need to know when the upload finished.

### Helper Methods

- `void SetCCDCapability(uint32_t cap)`: Sets the camera capabilities.
- `void SetCCDParams(int x, int y, int bpp, float xf, float yf)`: Sets the primary chip resolution, bits per pixel, and pixel size in microns. It also resets the frame to the full sensor and binning to 1x1.
- `void SetGuiderParams(int x, int y, int bpp, float xf, float yf)`: Sets the guide chip parameters, and sets `CCD_HAS_GUIDE_HEAD`.
- `void addCaptureFormat(const CaptureFormat &format)`: Adds a native capture format, such as `{"INDI_MONO", "Mono", 16, true}` (name, label, bits per pixel, default).
- `bool ExposureComplete(INDI::CCDChip *targetChip)`: Call this when the image is in the chip's frame buffer. The base class then encodes, uploads, and saves the image in a background thread, and sets `CCD_EXPOSURE` back to OK.

### CCDChip Methods

These are the `INDI::CCDChip` methods you will use most often, on `PrimaryCCD` or `GuideCCD`:

| Method                                      | Description                                                                 |
| ------------------------------------------- | --------------------------------------------------------------------------- |
| `getXRes()`, `getYRes()`                    | Full sensor resolution.                                                     |
| `getSubX()`, `getSubY()`, `getSubW()`, `getSubH()` | Current frame, in unbinned pixels.                                   |
| `getBinX()`, `getBinY()`                    | Current binning.                                                            |
| `getBPP()`                                  | Bits per pixel.                                                             |
| `getFrameType()`                            | Current frame type.                                                         |
| `getFrameBuffer()`                          | Pointer to the image buffer.                                                |
| `setFrameBufferSize(uint32_t nbuf)`         | Allocates the image buffer.                                                 |
| `setExposureDuration(double duration)`      | Records the exposure duration and start time for the FITS header. Call it when the exposure starts. |
| `setExposureLeft(double duration)`          | Updates the time left in `CCD_EXPOSURE` and sends it to clients.             |
| `setExposureFailed()`                       | Sets `CCD_EXPOSURE` to the alert state.                                     |
| `binFrame()`                                | Bins the frame buffer in software, for cameras that do not bin in hardware. |
| `setNAxis(int value)`                       | Sets 2 for mono images, or 3 for RGB images.                                |
| `setMinMaxStep(property, element, min, max, step, sendToClient)` | Changes the limits of a chip property, such as the exposure range. |

## The Exposure Lifecycle

When a client sets `CCD_EXPOSURE`:

1. The base class validates the duration against the limits of `CCD_EXPOSURE` (for bias frames, it uses the minimum exposure), aborts any exposure in progress, and calls `StartExposure()`.
2. In `StartExposure()`, start the exposure on the camera, call `PrimaryCCD.setExposureDuration(duration)`, and return `true`. The base class sets `CCD_EXPOSURE` to busy.
3. While exposing, update the time left with `PrimaryCCD.setExposureLeft()`, typically from `TimerHit()`.
4. When the camera has finished, read the image into `PrimaryCCD.getFrameBuffer()` while holding the `ccdBufferLock` mutex, release the mutex, and call `ExposureComplete(&PrimaryCCD)`.
5. The base class encodes and uploads the image, then sets `CCD_EXPOSURE` back to OK.

If the exposure fails, call `PrimaryCCD.setExposureFailed()` instead of `ExposureComplete()`.

> **Important:** `INDI::CCD` uploads images in a background thread. Always lock `ccdBufferLock` while writing to the frame buffer, so that the upload thread never reads a half-written image.

### Sizing the Frame Buffer

The base class uploads an image of `getSubW() / getBinX()` by `getSubH() / getBinY()` pixels. The frame buffer must hold at least that many pixels at the chip's bits per pixel. Recalculate it whenever the frame or binning changes:

```cpp
uint32_t nbuf = (PrimaryCCD.getSubW() / PrimaryCCD.getBinX()) *
                (PrimaryCCD.getSubH() / PrimaryCCD.getBinY()) *
                PrimaryCCD.getBPP() / 8;
PrimaryCCD.setFrameBufferSize(nbuf);
```

For RGB images, multiply by 3 and call `PrimaryCCD.setNAxis(3)`.

## Example Implementation

Here's a simplified example of a cooled camera driver. The camera SDK is represented by hypothetical `mycam_*` functions; replace them with your camera's SDK or protocol.

Most camera SDKs manage their own USB connection, so camera drivers usually override `Connect()` and `Disconnect()` instead of using the serial or TCP [connection plugins](../drivers/communications/connection-plugins.md).

### Header (`my_camera.h`)

```cpp
#pragma once

#include <libindi/indiccd.h>
#include <libindi/indielapsedtimer.h>

class MyCamera : public INDI::CCD
{
    public:
        MyCamera();

        const char *getDefaultName() override;
        bool initProperties() override;
        bool updateProperties() override;
        bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) override;

    protected:
        bool Connect() override;
        bool Disconnect() override;
        void TimerHit() override;

        bool StartExposure(float duration) override;
        bool AbortExposure() override;
        bool UpdateCCDFrame(int x, int y, int w, int h) override;
        bool UpdateCCDBin(int hor, int ver) override;
        int SetTemperature(double temperature) override;

        void addFITSKeywords(INDI::CCDChip *targetChip, std::vector<INDI::FITSRecord> &fitsKeywords) override;
        bool saveConfigItems(FILE *fp) override;

    private:
        bool setupParams();
        void updateFrameBufferSize();
        bool grabImage();

        INDI::PropertyNumber GainNP {1};

        int m_Handle {-1};
        bool m_InExposure {false};
        float m_ExposureRequest {0};
        INDI::ElapsedTimer m_ExposureTimer;
};
```

### Implementation (`my_camera.cpp`)

```cpp
#include "my_camera.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <memory>
#include <mutex>

static std::unique_ptr<MyCamera> myCamera(new MyCamera());

MyCamera::MyCamera()
{
    setVersion(1, 0);
}

const char *MyCamera::getDefaultName()
{
    return "My Camera";
}

bool MyCamera::initProperties()
{
    INDI::CCD::initProperties();

    SetCCDCapability(CCD_CAN_ABORT | CCD_CAN_BIN | CCD_CAN_SUBFRAME | CCD_HAS_COOLER | CCD_HAS_SHUTTER);

    // Every camera must add at least one capture format.
    addCaptureFormat({"INDI_MONO", "Mono", 16, true});

    // Gain is not defined by the base class, so we define it here.
    GainNP[0].fill("GAIN", "Gain", "%.f", 0, 100, 1, 0);
    GainNP.fill(getDeviceName(), "CCD_GAIN", "Gain", MAIN_CONTROL_TAB, IP_RW, 60, IPS_IDLE);

    // The camera supports exposures from 1 ms to 1 hour.
    PrimaryCCD.setMinMaxStep("CCD_EXPOSURE", "CCD_EXPOSURE_VALUE", 0.001, 3600, 1, false);

    addAuxControls();

    return true;
}

bool MyCamera::updateProperties()
{
    INDI::CCD::updateProperties();

    if (isConnected())
    {
        setupParams();
        defineProperty(GainNP);
    }
    else
    {
        deleteProperty(GainNP);
    }

    return true;
}

bool MyCamera::Connect()
{
    m_Handle = mycam_open();
    if (m_Handle < 0)
    {
        LOG_ERROR("Failed to open the camera. Check that it is connected and powered.");
        return false;
    }

    // Start polling for exposure progress and temperature.
    SetTimer(getCurrentPollingPeriod());
    return true;
}

bool MyCamera::Disconnect()
{
    mycam_close(m_Handle);
    m_Handle = -1;
    return true;
}

bool MyCamera::setupParams()
{
    // Sensor: 4144 x 2822 pixels, 16 bits, 4.63 x 4.63 micron pixels.
    SetCCDParams(4144, 2822, 16, 4.63, 4.63);
    updateFrameBufferSize();
    return true;
}

void MyCamera::updateFrameBufferSize()
{
    uint32_t nbuf = (PrimaryCCD.getSubW() / PrimaryCCD.getBinX()) *
                    (PrimaryCCD.getSubH() / PrimaryCCD.getBinY()) *
                    PrimaryCCD.getBPP() / 8;
    PrimaryCCD.setFrameBufferSize(nbuf);
}

bool MyCamera::StartExposure(float duration)
{
    // Keep the shutter closed for dark and bias frames.
    bool isDark = PrimaryCCD.getFrameType() == INDI::CCDChip::DARK_FRAME ||
                  PrimaryCCD.getFrameType() == INDI::CCDChip::BIAS_FRAME;

    if (mycam_start_exposure(m_Handle, duration, isDark) != 0)
    {
        LOG_ERROR("Failed to start exposure.");
        return false;
    }

    m_ExposureRequest = duration;
    PrimaryCCD.setExposureDuration(duration);
    m_ExposureTimer.start();
    m_InExposure = true;

    LOGF_INFO("Taking a %.3f seconds frame...", duration);
    return true;
}

bool MyCamera::AbortExposure()
{
    mycam_abort_exposure(m_Handle);
    m_InExposure = false;
    return true;
}

bool MyCamera::UpdateCCDFrame(int x, int y, int w, int h)
{
    if (x + w > PrimaryCCD.getXRes() || y + h > PrimaryCCD.getYRes())
    {
        LOGF_ERROR("Frame (%d,%d) %dx%d is outside the sensor.", x, y, w, h);
        return false;
    }

    // The SDK expects the region of interest in binned pixels.
    if (mycam_set_roi(m_Handle, x / PrimaryCCD.getBinX(), y / PrimaryCCD.getBinY(),
                      w / PrimaryCCD.getBinX(), h / PrimaryCCD.getBinY()) != 0)
        return false;

    // Store the new frame in the chip, then resize the frame buffer.
    INDI::CCD::UpdateCCDFrame(x, y, w, h);
    updateFrameBufferSize();
    return true;
}

bool MyCamera::UpdateCCDBin(int hor, int ver)
{
    if (hor != ver || hor > 4)
    {
        LOGF_ERROR("%dx%d binning is not supported.", hor, ver);
        return false;
    }

    if (mycam_set_binning(m_Handle, hor) != 0)
        return false;

    // Store the new binning, then apply the current frame with the new binning.
    INDI::CCD::UpdateCCDBin(hor, ver);
    return UpdateCCDFrame(PrimaryCCD.getSubX(), PrimaryCCD.getSubY(),
                          PrimaryCCD.getSubW(), PrimaryCCD.getSubH());
}

int MyCamera::SetTemperature(double temperature)
{
    // Already at the requested temperature.
    if (std::fabs(temperature - TemperatureNP[0].getValue()) < 0.1)
        return 1;

    if (mycam_set_temperature(m_Handle, temperature) != 0)
        return -1;

    // The base class reports completion once TemperatureNP reaches the target.
    return 0;
}

void MyCamera::TimerHit()
{
    if (!isConnected())
        return;

    uint32_t nextTimer = getCurrentPollingPeriod();

    if (m_InExposure)
    {
        double timeLeft = std::max(0.0, m_ExposureRequest - m_ExposureTimer.elapsed() / 1000.0);
        PrimaryCCD.setExposureLeft(timeLeft);

        if (mycam_is_image_ready(m_Handle))
        {
            m_InExposure = false;
            grabImage();
        }
        else if (timeLeft < 1.0)
        {
            // Poll more often near the end of the exposure.
            nextTimer = 100;
        }
    }

    // Report the current temperature. The base class compares it with the target.
    double temperature = 0;
    if (HasCooler() && mycam_get_temperature(m_Handle, &temperature) == 0 &&
            std::fabs(temperature - TemperatureNP[0].getValue()) >= 0.05)
    {
        TemperatureNP[0].setValue(temperature);
        TemperatureNP.apply();
    }

    SetTimer(nextTimer);
}

bool MyCamera::grabImage()
{
    {
        // Lock the buffer while writing to it, so the upload thread never reads a partial image.
        std::unique_lock<std::mutex> guard(ccdBufferLock);
        if (mycam_read_image(m_Handle, PrimaryCCD.getFrameBuffer(), PrimaryCCD.getFrameBufferSize()) != 0)
        {
            guard.unlock();
            LOG_ERROR("Failed to download image.");
            PrimaryCCD.setExposureFailed();
            return false;
        }
    }

    LOG_INFO("Download complete.");

    // Encode, upload, and save the image.
    ExposureComplete(&PrimaryCCD);
    return true;
}

bool MyCamera::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    if (dev != nullptr && !strcmp(dev, getDeviceName()))
    {
        if (GainNP.isNameMatch(name))
        {
            if (mycam_set_gain(m_Handle, static_cast<int>(values[0])) != 0)
            {
                GainNP.setState(IPS_ALERT);
                GainNP.apply();
                return true;
            }

            GainNP.update(values, names, n);
            GainNP.setState(IPS_OK);
            GainNP.apply();
            saveConfig(GainNP);
            return true;
        }
    }

    // Let the base class handle the standard properties.
    return INDI::CCD::ISNewNumber(dev, name, values, names, n);
}

void MyCamera::addFITSKeywords(INDI::CCDChip *targetChip, std::vector<INDI::FITSRecord> &fitsKeywords)
{
    // Add the standard keywords first.
    INDI::CCD::addFITSKeywords(targetChip, fitsKeywords);

    fitsKeywords.push_back({"GAIN", GainNP[0].getValue(), 0, "Gain"});
}

bool MyCamera::saveConfigItems(FILE *fp)
{
    INDI::CCD::saveConfigItems(fp);
    GainNP.save(fp);
    return true;
}
```

There is no `main()` function: drivers are linked against `libindidriver`, which provides it and dispatches client requests to your driver through the static instance. See [Creating Custom INDI Drivers](../drivers/basics/custom-driver.md) for the CMake setup and driver XML file.

## Advanced Topics

### Software Binning

If the camera cannot bin in hardware, read the full-resolution frame into the buffer and call `PrimaryCCD.binFrame()` (or `binBayerFrame()` for color sensors) before `ExposureComplete()`. In that case, size the frame buffer for the unbinned frame.

### Guide Chip

For cameras with a separate guide sensor, call `SetGuiderParams()` once the camera is connected and allocate `GuideCCD`'s frame buffer. Then implement `StartGuideExposure()` and `AbortGuideExposure()`, and call `ExposureComplete(&GuideCCD)` when a guide frame is ready.

### ST4 Guide Port

For cameras with an ST4 port, set `CCD_HAS_ST4_PORT` and implement the four `Guide*()` methods. The base class defines the guide properties and calls your methods. See the [Guider Interface](guider.md) for details.

### Streaming

For cameras that support live video, set `CCD_HAS_STREAMING`. The base class creates `Streamer`, an `INDI::StreamManager` that handles streaming and recording. Implement `StartStreaming()` and `StopStreaming()`, and for each video frame:

```cpp
std::unique_lock<std::mutex> guard(ccdBufferLock);
// Copy the frame into PrimaryCCD.getFrameBuffer()...
Streamer->newFrame(PrimaryCCD.getFrameBuffer(), frameSize);
```

Before streaming starts, call `Streamer->setPixelFormat()` and `Streamer->setSize()` to describe the frames.

### Temperature Ramp

When `CCD_TEMP_RAMP` has a non-zero slope, the base class limits each temperature change to that many degrees per minute, and calls `SetTemperature()` repeatedly with intermediate targets. Your `SetTemperature()` implementation does not need to handle ramping.

## Best Practices

- **Never block in `StartExposure()`**: Start the exposure and return. Report progress and completion from `TimerHit()` or the SDK's callback.
- **Lock the frame buffer**: Hold `ccdBufferLock` while writing image data.
- **Keep the frame buffer size in sync** with the frame, binning, and capture format.
- **Report temperature regularly** so the base class can tell when the target is reached.
- **Use the conventional names** for gain, offset, and cooler properties.
- **Support simulation**: Check `isSimulation()` so the driver can be tested without hardware.

## Real-World Examples

The [INDI library](https://github.com/indilib/indi/tree/master/drivers/ccd) and [INDI 3rd party](https://github.com/indilib/indi-3rdparty) repositories contain many camera drivers:

- **CCD Simulator** (`drivers/ccd/ccd_simulator.cpp`): Implements nearly every feature of the interface, including guide chip, streaming, cooling, and Bayer.
- **Guide Simulator** (`drivers/ccd/guide_simulator.cpp`): A simpler simulator for guide cameras.
- **ZWO ASI** (`indi-asi` in INDI 3rd party): A full-featured driver for a production camera SDK.
