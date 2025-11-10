---
title: I2C Connection
parent: Hardware Communications
grand_parent: Driver Development
nav_order: 5
permalink: /drivers/communications/i2c/
---

# I2C Connection Plugin

The I2C Connection plugin provides access to I2C bus devices on Linux systems, commonly used with Raspberry Pi and embedded devices.

## When to Use I2C

- Temperature and humidity sensors
- ADCs (Analog-to-Digital Converters)
- DACs (Digital-to-Analog Converters)
- OLED/LCD displays
- Real-time clocks (RTC)
- EEPROM memory
- GPIO expanders

## Header File

```cpp
#include <connectionplugins/connectioni2c.h>
```

## Basic Setup

```cpp
class MyDriver : public INDI::DefaultDevice
{
public:
    MyDriver()
    {
        setConnectionMode(CONNECTION_I2C);
        
        i2cConnection = new Connection::I2C(this);
        i2cConnection->setDefaultBusPath("/dev/i2c-1");
        i2cConnection->setDefaultAddress(0x28);  // 7-bit address
        i2cConnection->registerHandshake([&]() { return Handshake(); });
        
        registerConnection(i2cConnection);
    }

private:
    Connection::I2C *i2cConnection = nullptr;
};
```

## Communication Example

```cpp
bool MyDriver::readSensor(uint8_t *data, size_t len)
{
    int fd = i2cConnection->getPortFD();
    if (fd < 0)
    {
        LOG_ERROR("I2C device not open");
        return false;
    }

    ssize_t nbytes = read(fd, data, len);
    if (nbytes < 0)
    {
        LOGF_ERROR("I2C read error: %s", strerror(errno));
        return false;
    }

    return nbytes == static_cast<ssize_t>(len);
}

bool MyDriver::writeSensor(const uint8_t *data, size_t len)
{
    int fd = i2cConnection->getPortFD();
    if (fd < 0)
    {
        LOG_ERROR("I2C device not open");
        return false;
    }

    ssize_t nbytes = write(fd, data, len);
    if (nbytes < 0)
    {
        LOGF_ERROR("I2C write error: %s", strerror(errno));
        return false;
    }

    return nbytes == static_cast<ssize_t>(len);
}
```

## Best Practices

- **Address Format**: Use 7-bit I2C addresses (0x00-0x7F)
- **Bus Selection**: Common buses are `/dev/i2c-0`, `/dev/i2c-1`, etc.
- **Permissions**: Add user to `i2c` group for access
- **Device Detection**: Scan with `i2cdetect` before implementing
- **Pull-up Resistors**: Ensure proper pull-up resistors (typically 4.7kΩ)
- **Clock Speed**: Most devices support 100kHz standard mode

## Troubleshooting

### Cannot Open I2C Bus
```bash
# Enable I2C on Raspberry Pi
sudo raspi-config
# Interface Options -> I2C -> Enable

# Add user to i2c group
sudo usermod -a -G i2c $USER
# Log out and log back in
```

### Device Not Responding
```bash
# Scan I2C bus
i2cdetect -y 1

# Check connections and pull-up resistors
```

## Related Guides

- [Connection Plugins Overview](connection-plugins/)
- [GPIO Communication](gpio/) - For Raspberry Pi projects
- [System Setup](system-setup/)
