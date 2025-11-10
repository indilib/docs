---
title: GPIO Communication
parent: Hardware Communications
grand_parent: Driver Development
nav_order: 7
permalink: /drivers/communications/gpio/
---

# GPIO Communication

GPIO (General Purpose Input/Output) communication using libgpiod for Linux systems, particularly Raspberry Pi and other single-board computers.

## When to Use GPIO

- Raspberry Pi or Linux SBC digital I/O
- Relay control for devices (covers, heaters, dew heaters)
- Reading sensors (switches, buttons, limit switches)
- PWM control for motors, fans, or LEDs
- Custom hardware interfacing
- Direct hardware control without USB/Serial overhead

## Important: libgpiod Version

**Use libgpiod v2 for all new projects.** The v2 API provides significant improvements in stability, performance, and features. If you're working with existing code using libgpiod v1, plan to migrate to v2 as the older version is deprecated.

Key differences:
- **v2**: Modern C++ API with RAII, better error handling, and improved performance
- **v1**: Legacy API, deprecated, to be phased out

All examples in this documentation use libgpiod v2.

## INDI Interface Classes

INDI provides specialized interface classes for GPIO devices:

- **`INDI::InputInterface`**: For digital sensors and input devices (buttons, switches, sensors)
- **`INDI::OutputInterface`**: For digital outputs and control devices (relays, LEDs, actuators)

These interfaces provide standardized properties and methods for handling GPIO operations, making your driver consistent with other INDI GPIO drivers.

## Required Headers

```cpp
#include <gpiod.hpp>
```

## Basic GPIO Setup

```cpp
class INDIGPIO : public INDI::DefaultDevice,
                 public INDI::InputInterface,
                 public INDI::OutputInterface
{
public:
    INDIGPIO()
    {
        setVersion(VERSION_MAJOR, VERSION_MINOR);
    }
    
    bool Connect() override
    {
        try
        {
            // Open GPIO chip (usually gpiochip0 on Raspberry Pi)
            m_GPIO.reset(new gpiod::chip("gpiochip0"));
            
            // Setup PWM if available
            setupPWMProperties();
            
            // Enumerate available GPIO lines
            detectGPIOLines();
            
            return true;
        }
        catch (const std::exception &e)
        {
            LOGF_ERROR("Failed to connect: %s", e.what());
            return false;
        }
    }
    
private:
    std::unique_ptr<gpiod::chip> m_GPIO;
    std::vector<int> m_InputOffsets;
    std::vector<int> m_OutputOffsets;
};
```

## GPIO Input Example

Reading digital inputs (sensors, switches, buttons):

```cpp
bool INDIGPIO::updateDigitalInputs()
{
    try
    {
        for (size_t i = 0; i < m_InputOffsets.size(); i++)
        {
            auto oldState = DigitalInputsSP[i].findOnSwitchIndex();
            
            // Request line for input
            auto request = m_GPIO->prepare_request()
                .set_consumer("indi-gpio")
                .add_line_settings(
                    m_InputOffsets[i],
                    gpiod::line_settings().set_direction(
                        gpiod::line::direction::INPUT))
                .do_request();
            
            // Read value
            auto newState = request.get_value(m_InputOffsets[i]) == 
                           gpiod::line::value::ACTIVE ? 1 : 0;
            
            // Update property if changed
            if (oldState != newState)
            {
                DigitalInputsSP[i].reset();
                DigitalInputsSP[i][newState].setState(ISS_ON);
                DigitalInputsSP[i].setState(IPS_OK);
                DigitalInputsSP[i].apply();
            }
        }
        return true;
    }
    catch (const std::exception &e)
    {
        LOGF_ERROR("Failed to update inputs: %s", e.what());
        return false;
    }
}
```

## GPIO Output Example

Controlling digital outputs (relays, LEDs, actuators):

```cpp
bool INDIGPIO::commandOutput(uint32_t index, OutputState command)
{
    if (index >= m_OutputOffsets.size())
    {
        LOGF_ERROR("Invalid output index %d", index);
        return false;
    }
    
    try
    {
        auto offset = m_OutputOffsets[index];
        
        // Request line for output
        auto request = m_GPIO->prepare_request()
            .set_consumer("indi-gpio")
            .add_line_settings(
                offset,
                gpiod::line_settings().set_direction(
                    gpiod::line::direction::OUTPUT))
            .do_request();
        
        // Set value
        gpiod::line::value val = (command == OutputState::Off) ? 
                                 gpiod::line::value::INACTIVE : 
                                 gpiod::line::value::ACTIVE;
        request.set_value(offset, val);
        
        return true;
    }
    catch (const std::exception &e)
    {
        LOGF_ERROR("Failed to set output: %s", e.what());
        return false;
    }
}
```

## PWM Control Example

For motor control, fans, or LED dimming:

```cpp
bool INDIGPIO::setPWMDutyCycle(size_t index, int dutyCycle)
{
    if (index >= m_PWMPins.size() || dutyCycle < 0 || dutyCycle > 100)
        return false;
    
    auto &pin = m_PWMPins[index];
    
    // Calculate duty cycle in nanoseconds
    uint64_t period_ns = 1000000000ULL / pin.frequency;
    uint64_t duty_ns = (period_ns * dutyCycle) / 100;
    
    // Write to sysfs PWM interface
    std::string dutyPath = "/sys/class/pwm/" + pin.pwmChip + 
                          "/pwm" + std::to_string(pin.channel) + 
                          "/duty_cycle";
    
    std::ofstream dutyFile(dutyPath);
    if (!dutyFile.is_open())
    {
        LOGF_ERROR("Failed to open duty cycle file: %s", dutyPath.c_str());
        return false;
    }
    
    dutyFile << duty_ns << std::endl;
    dutyFile.close();
    
    pin.dutyCycle = dutyCycle;
    return true;
}

bool INDIGPIO::enablePWM(size_t index, bool enabled)
{
    if (index >= m_PWMPins.size())
        return false;
    
    auto &pin = m_PWMPins[index];
    
    std::string enablePath = "/sys/class/pwm/" + pin.pwmChip + 
                            "/pwm" + std::to_string(pin.channel) + 
                            "/enable";
    
    std::ofstream enableFile(enablePath);
    if (!enableFile.is_open())
    {
        LOGF_ERROR("Failed to open enable file: %s", enablePath.c_str());
        return false;
    }
    
    enableFile << (enabled ? "1" : "0") << std::endl;
    enableFile.close();
    
    pin.active = enabled;
    return true;
}
```

## Raspberry Pi Pin Reference

Common GPIO pins on Raspberry Pi:

| GPIO Number | Physical Pin | Common Use |
|-------------|--------------|------------|
| GPIO2 | Pin 3 | I2C SDA |
| GPIO3 | Pin 5 | I2C SCL |
| GPIO4 | Pin 7 | General Purpose |
| GPIO17 | Pin 11 | General Purpose |
| GPIO27 | Pin 13 | General Purpose |
| GPIO22 | Pin 15 | General Purpose |
| GPIO12 | Pin 32 | PWM0 |
| GPIO13 | Pin 33 | PWM1 |

## Best Practices

- **Chip Selection**: Use `gpiochip0` for Raspberry Pi, may vary on other boards
- **Line Direction**: Always set direction (INPUT or OUTPUT) before use
- **PWM Access**: PWM requires sysfs interface (`/sys/class/pwm/`)
- **Permissions**: Add user to `gpio` group or create udev rules
- **Resource Cleanup**: Release GPIO lines when done
- **Error Handling**: Wrap GPIO operations in try-catch blocks
- **Pull Resistors**: Configure pull-up/pull-down for inputs as needed

## Common Use Cases

### Relay Control (Flat Panel, Heater, etc.)

```cpp
// Turn on relay
commandOutput(RELAY_INDEX, OutputState::On);

// Turn off relay
commandOutput(RELAY_INDEX, OutputState::Off);
```

### Limit Switch Monitoring

```cpp
// Check if cover is fully open
void TimerHit()
{
    updateDigitalInputs();
    if (DigitalInputsSP[LIMIT_SWITCH_INDEX][1].getState() == ISS_ON)
    {
        // Limit switch triggered, cover fully open
        CoverSP.setState(IPS_OK);
        CoverSP.apply();
    }
}
```

### Fan Speed Control via PWM

```cpp
// Set fan to 50% speed
setPWMDutyCycle(FAN_PWM_INDEX, 50);
enablePWM(FAN_PWM_INDEX, true);

// Stop fan
enablePWM(FAN_PWM_INDEX, false);
```

## Troubleshooting

### Cannot Open GPIO Chip

```bash
# Verify chip exists
ls /dev/gpiochip*

# Add user to gpio group
sudo usermod -a -G gpio $USER
# Log out and log back in

# Check libgpiod is installed
dpkg -l | grep libgpiod
```

### PWM Not Working

```bash
# Check PWM is enabled in device tree
ls /sys/class/pwm/

# Verify permissions
ls -l /sys/class/pwm/pwmchip0/

# Check PWM overlay is enabled (Raspberry Pi)
grep dtoverlay /boot/config.txt
```

### Permission Issues

```bash
# Create udev rule for PWM
sudo nano /etc/udev/rules.d/99-pwm.rules
# Add:
SUBSYSTEM=="pwm", ACTION=="add", RUN+="/bin/chgrp -R gpio /sys%p", RUN+="/bin/chmod -R g+w /sys%p"

# Reload rules
sudo udevadm control --reload-rules
sudo udevadm trigger
```

## Example Driver

See the complete GPIO driver implementation:
- `indi-gpio/indi_gpio.cpp` - Full-featured Raspberry Pi GPIO driver

## Related Guides

- [Best Practices](best-practices/)
- [System Setup](system-setup/) - Detailed permission configuration
- [Troubleshooting](troubleshooting/)
- [INDI API Documentation](https://www.indilib.org/api/)
