---
title: IMU Interface
nav_order: 16
parent: Device Interfaces
---

# Implementing the IMU Interface

This guide explains how to implement a driver for an Inertial Measurement Unit (IMU): a sensor that combines an accelerometer, a gyroscope, and often a magnetometer. Mounted on a telescope, an IMU can report where the telescope is pointing, which is useful for push-to mounts, for parking checks, and for monitoring vibration.

## Introduction to the IMU Interface

IMU support consists of two classes:

- `INDI::IMUInterface` (`indiimuinterface.h`) defines the IMU capabilities and the standard sensor properties.
- `INDI::IMU` (`indiimu.h`) combines `INDI::DefaultDevice` and `INDI::IMUInterface`. It adds serial and I2C [connection plugins](../drivers/communications/connection-plugins.md), and turns the IMU's orientation into telescope coordinates.

For IMU devices, inherit from `INDI::IMU`. Your driver reads the sensor and passes the readings to the base class. The base class then:

- Publishes acceleration, angular velocity, magnetic field, and orientation to clients
- Runs **software sensor fusion** (Madgwick or Mahony) for sensors that only provide raw data, to calculate orientation
- Converts orientation into equatorial (HA/DEC) or horizontal (AZ/ALT) coordinates, using the site location and the direction the telescope points relative to the sensor
- Lets users correct the sensor's mounting orientation and sync it to a known position

## Prerequisites

Before implementing the IMU interface, you should have:

- Basic knowledge of C++ programming
- Understanding of the INDI protocol and architecture
- Familiarity with the sensor's datasheet or library
- Development environment set up (compiler, build tools, etc.)
- INDI library installed

## IMU Interface Structure

### IMUCapability Enum

Call `SetCapability()` in your driver's constructor to declare which data your sensor provides:

```cpp
enum IMUCapability
{
    IMU_HAS_ORIENTATION   = 1 << 0, /*!< Has orientation data (Roll, Pitch, Yaw) */
    IMU_HAS_ACCELERATION  = 1 << 1, /*!< Has acceleration data */
    IMU_HAS_GYROSCOPE     = 1 << 2, /*!< Has gyroscope data */
    IMU_HAS_MAGNETOMETER  = 1 << 3, /*!< Has magnetometer data */
    IMU_HAS_CALIBRATION   = 1 << 4, /*!< Supports calibration */
    IMU_HAS_TEMPERATURE   = 1 << 5, /*!< Has temperature sensor */
    IMU_HAS_STABILITY_MON = 1 << 6, /*!< Supports stability monitoring */
    IMU_HAS_SENSOR_FUSION = 1 << 7  /*!< Base-class software sensor fusion is active (Madgwick/Mahony) */
};
```

There are two kinds of IMU sensors:

- **Sensors with on-chip fusion**, such as the BNO08x family, calculate orientation themselves. Set `IMU_HAS_ORIENTATION` and pass the orientation to `SetOrientationData()`.
- **Raw-data sensors**, such as the ICM-20948, only provide acceleration, angular velocity, and magnetic field. Do **not** set `IMU_HAS_ORIENTATION`. `INDI::IMU` then sets `IMU_HAS_SENSOR_FUSION` itself, and calculates orientation from the raw data.

You do not need to set `IMU_HAS_SENSOR_FUSION` yourself.

### Connection

`INDI::IMU` offers serial and I2C connections. Call `setSupportedConnections()` in your constructor with `INDI::IMU::CONNECTION_SERIAL`, `INDI::IMU::CONNECTION_I2C`, both, or `INDI::IMU::CONNECTION_NONE`. After connecting, the base class sets `PortFD` (the serial port or the I2C bus file descriptor) and calls your `Handshake()`.

`INDI::IMU` does not set the driver interface flag, so call `setDriverInterface(IMU_INTERFACE)` in your constructor.

## Standard Properties

### Sensor Data

| Property             | Capability             | Elements                                   | Units                         |
| -------------------- | ---------------------- | ------------------------------------------ | ----------------------------- |
| `ORIENTATION`        | Orientation or fusion  | `ROLL`, `PITCH`, `YAW`, `QUATERNION_W`     | Degrees                       |
| `ACCELERATION`       | `IMU_HAS_ACCELERATION` | `ACCEL_X`, `ACCEL_Y`, `ACCEL_Z`            | m/s²                          |
| `GYROSCOPE`          | `IMU_HAS_GYROSCOPE`    | `GYRO_X`, `GYRO_Y`, `GYRO_Z`               | Degrees or radians per second |
| `MAGNETOMETER`       | `IMU_HAS_MAGNETOMETER` | `MAG_X`, `MAG_Y`, `MAG_Z`                  | µT                            |
| `TEMPERATURE`        | `IMU_HAS_TEMPERATURE`  | `TEMPERATURE`                              | °C                            |
| `CALIBRATION_STATUS` | `IMU_HAS_CALIBRATION`  | `CAL_SYS`, `CAL_GYRO`, `CAL_ACCEL`, `CAL_MAG` | Light per sensor           |
| `CALIBRATION_CONTROL`| `IMU_HAS_CALIBRATION`  | `CAL_START`, `CAL_SAVE`, `CAL_LOAD`, `CAL_RESET` | —                       |
| `STABILITY_MONITORING` | `IMU_HAS_STABILITY_MON` | `VIBRATION_LEVEL`, `STABILITY_MONITORING_STABILITY_THRESHOLD` | —     |

### Configuration

| Property          | Elements                                | Description                                               |
| ----------------- | --------------------------------------- | --------------------------------------------------------- |
| `POWER_MODE`      | `NORMAL`, `LOW_POWER`, `SUSPEND`        | Sensor power mode                                         |
| `OPERATION_MODE`  | `IMU`, `COMPASS`, `M4G`, `NDOF`         | Sensor operation (fusion) mode                            |
| `DISTANCE_UNITS`  | `METRIC`, `IMPERIAL`                    | Distance units                                            |
| `ANGULAR_UNITS`   | `DEGREES`, `RADIANS`                    | Units of the gyroscope data you report                    |
| `UPDATE_RATE`     | `RATE`                                  | Sensor update rate in Hz                                  |
| `DEVICE_INFO`     | `CHIP_ID`, `FIRMWARE_VERSION`, `SENSOR_STATUS` | Information about the sensor (read only)           |
| `SENSOR_FUSION`   | `FUSION_ENABLE`, `FUSION_DISABLE`       | Enable software fusion (raw-data sensors only)            |
| `FUSION_TYPE`     | Madgwick, Mahony                        | Fusion algorithm (raw-data sensors only)                  |
| `FUSION_PARAMS`   | β (Madgwick), or Kp and Ki (Mahony)     | Fusion algorithm tuning (raw-data sensors only)           |

### Telescope Pointing

| Property                  | Description                                                                                   |
| ------------------------- | --------------------------------------------------------------------------------------------- |
| `COORDINATES`             | Where the telescope points: HA/DEC or AZ/ALT, depending on `COORDS_TYPE` (read only)          |
| `COORDS_TYPE`             | `EQUATORIAL` (HA/DEC) or `ALTAZ` (AZ/ALT)                                                      |
| `IMU_FRAME`               | The sensor's axis convention: `ENU` (East-North-Up), `NWU` (North-West-Up), or `SWU` (South-West-Up) |
| `TELESCOPE_VECTOR`        | The direction the telescope points, in the sensor's frame                                     |
| `ORIENTATION_ADJUSTMENTS` | Multipliers (`ROLL_M`, `PITCH_M`, `YAW_M`) and offsets (`ROLL_O`, `PITCH_O`, `YAW_O`) to correct how the sensor is mounted. A multiplier of −1 inverts an axis. |
| `SYNC_AXIS`               | Sync the reported coordinates to a known position                                             |
| `GEOGRAPHIC_COORD`        | Site location, used to convert orientation to coordinates                                     |
| `MAGNETIC_DECLINATION`    | Difference between magnetic and true north at the site, in degrees                            |

## Key Methods

### Methods to Call with Sensor Data

Call these from your driver whenever you read new data. They update the properties and send them to clients:

- `bool SetOrientationData(double i, double j, double k, double w)`:
  Passes the orientation as a **quaternion** (i, j, k, w). The base class applies the orientation adjustments, publishes roll, pitch, and yaw in degrees, and updates `COORDINATES`. Only call it for sensors with on-chip fusion.

- `bool SetAccelerationData(double x, double y, double z)`:
  Acceleration in m/s².

- `bool SetGyroscopeData(double x, double y, double z)`:
  Angular velocity, in the units selected in `ANGULAR_UNITS` (degrees per second by default). For raw-data sensors, each call also runs one step of sensor fusion, so call it after `SetAccelerationData()` and `SetMagnetometerData()`.

- `bool SetMagnetometerData(double x, double y, double z)`:
  Magnetic field in µT.

- `bool SetTemperature(double temperature)`, `SetCalibrationStatus(int sys, int gyro, int accel, int mag)`, `SetDeviceInfo(...)`, `SetStabilityMonitoring(...)`:
  Report the sensor temperature in °C, the calibration level of each sensor (0 to 3), the chip information, and the vibration level.

> **Note:** In older INDI releases, `INDI::IMU` overrode these four methods with versions that did nothing. If your driver must support those releases, call the `INDI::IMUInterface` versions explicitly, as in `INDI::IMUInterface::SetTemperature(t)`.

### Virtual Methods to Override

Override these if your sensor supports the feature. The `INDI::IMU` defaults do nothing and return `false`, so the request is reported as failed:

- `virtual bool Handshake()`: Initialize the sensor and verify that it responds.
- `virtual bool StartCalibration()`, `SaveCalibrationData()`, `LoadCalibrationData()`, `ResetCalibration()`: Calibration commands, for `IMU_HAS_CALIBRATION`.
- `virtual bool SetPowerMode(const std::string &mode)`, `SetOperationMode(const std::string &mode)`: Change the sensor's power or operation mode.
- `virtual bool SetUpdateRate(double rate)`: Change the sensor's update rate in Hz.
- `virtual bool SetAngularUnits(bool degrees)`, `SetDistanceUnits(bool metric)`: Called when the user changes units.

## Example Implementation

Here's a simplified driver for a raw-data I2C sensor. The sensor library is represented by a hypothetical `MySensor` class; replace it with your sensor's library.

```cpp
#include <libindi/indiimu.h>

#include <memory>

class MyIMU : public INDI::IMU
{
    public:
        MyIMU()
        {
            setVersion(1, 0);

            // Raw sensor data only. The base class calculates orientation with sensor fusion.
            SetCapability(IMU_HAS_ACCELERATION | IMU_HAS_GYROSCOPE | IMU_HAS_MAGNETOMETER | IMU_HAS_TEMPERATURE);

            setSupportedConnections(INDI::IMU::CONNECTION_I2C);
            setDriverInterface(IMU_INTERFACE);
        }

        const char *getDefaultName() override
        {
            return "My IMU";
        }

        bool initProperties() override
        {
            INDI::IMU::initProperties();
            addAuxControls();
            return true;
        }

    protected:
        bool Handshake() override
        {
            // PortFD is the I2C bus file descriptor, set by the base class.
            if (!m_Sensor.begin(PortFD))
            {
                LOG_ERROR("Sensor is not responding. Check the I2C bus and address.");
                return false;
            }

            SetDeviceInfo("MY-SENSOR", m_Sensor.firmwareVersion(), "Operational");
            return true;
        }

        void TimerHit() override
        {
            if (!isConnected())
                return;

            if (m_Sensor.read())
            {
                SetAccelerationData(m_Sensor.accelX(), m_Sensor.accelY(), m_Sensor.accelZ());
                SetMagnetometerData(m_Sensor.magX(), m_Sensor.magY(), m_Sensor.magZ());
                // Call last: each gyroscope update runs one step of sensor fusion.
                SetGyroscopeData(m_Sensor.gyroX(), m_Sensor.gyroY(), m_Sensor.gyroZ());
                SetTemperature(m_Sensor.temperature());
            }

            SetTimer(getCurrentPollingPeriod());
        }

    private:
        MySensor m_Sensor;
};

static std::unique_ptr<MyIMU> myIMU(new MyIMU());
```

The polling period sets how often the sensor is read. Sensor fusion integrates the gyroscope readings over time, so read the sensor often and at a steady rate: set a short default polling period with `setDefaultPollingPeriod()` in `initProperties()`.

For a sensor with on-chip fusion, set `IMU_HAS_ORIENTATION` and call `SetOrientationData()` with the quaternion from the sensor:

```cpp
SetOrientationData(quat.i, quat.j, quat.k, quat.real);
```

## Real-World Examples

The [INDI 3rd party](https://github.com/indilib/indi-3rdparty) repository contains IMU drivers:

- **ICM-20948** (`indi-icm-imu`): A raw-data sensor using the base class's sensor fusion, with gyroscope, accelerometer, and magnetometer calibration.
- **BNO08x** (`indi-bno-imu`): A sensor with on-chip fusion that reports orientation as a quaternion.
