---
title: System Setup
parent: Hardware Communications
grand_parent: Driver Development
nav_order: 10
permalink: /drivers/communications/system-setup/
---

# System Setup Guide

This guide covers system configuration, permissions, and setup requirements for different communication methods.

## Serial Communication Setup

### Add User to dialout Group

```bash
sudo usermod -a -G dialout $USER
```

Log out and log back in for changes to take effect.

### Verify Serial Devices

```bash
# List serial devices
ls -l /dev/ttyUSB* /dev/ttyACM*

# Check permissions
ls -l /dev/ttyUSB0
```

## USB Communication Setup

### Create udev Rules

Create `/etc/udev/rules.d/99-indi-usb.rules`:

```bash
# DSLR Shutter Control
SUBSYSTEM=="usb", ATTRS{idVendor}=="134a", ATTRS{idProduct}=="9021", MODE="0666"
SUBSYSTEM=="usb", ATTRS{idVendor}=="134a", ATTRS{idProduct}=="9026", MODE="0666"

# Add your devices here
# SUBSYSTEM=="usb", ATTRS{idVendor}=="XXXX", ATTRS{idProduct}=="YYYY", MODE="0666"
```

### Reload udev Rules

```bash
sudo udevadm control --reload-rules
sudo udevadm trigger
```

## HID Communication Setup

### Create HID udev Rules

Create `/etc/udev/rules.d/99-indi-hid.rules`:

```bash
# SX Filter Wheel
KERNEL=="hidraw*", ATTRS{idVendor}=="1278", ATTRS{idProduct}=="0920", MODE="0666"

# Generic HID devices for INDI
KERNEL=="hidraw*", ATTRS{idVendor}=="1278", MODE="0666"
```

### Reload Rules

```bash
sudo udevadm control --reload-rules
sudo udevadm trigger
```

## GPIO Communication Setup (Raspberry Pi)

### Add User to gpio Group

```bash
sudo usermod -a -G gpio $USER
```

### Enable GPIO

GPIO is typically enabled by default on Raspberry Pi. Verify with:

```bash
ls -l /dev/gpiochip*
```

### Configuring GPIO Pins at Boot

Edit `/boot/config.txt` or `/boot/firmware/config.txt` to configure GPIO pins at boot time with specific directions and pull resistor settings.

#### GPIO Pin Configuration Syntax

```bash
# Format: gpio=<pin>[,<pin>...]=<mode>[,<option>...]
```

**Modes:**
- `ip` - Input
- `op` - Output
- `a0-a5` - Alternate functions 0-5

**Options for Inputs:**
- `pu` - Pull-up resistor enabled
- `pd` - Pull-down resistor enabled  
- `np` - No pull resistor (floating)

**Options for Outputs:**
- `dh` - Drive high (initially high)
- `dl` - Drive low (initially low)

#### Example: StellarMate Pro GPIO Configuration

```bash
# Configure input pins with no pull resistors
# GPIOs 6, 18, 19, 24 as inputs (for switches, sensors)
gpio=6,18,19,24=ip,np

# Configure output pins, initially low
# GPIOs 7, 17, 21, 23, 25, 26, 27 as outputs (for relays, LEDs)
gpio=7,17,21,23,25,26,27=op,dl
```

#### Common GPIO Configurations

**Input Pins with Pull-up (for buttons/switches):**
```bash
gpio=23,24=ip,pu
```

**Output Pins Initially High:**
```bash
gpio=17,27=op,dh
```

**Mixed Configuration:**
```bash
# Inputs with pull-ups
gpio=5,6=ip,pu
# Outputs initially low
gpio=17,27=op,dl
# Inputs with no pulls
gpio=22,23=ip,np
```

### PWM Configuration

PWM (Pulse Width Modulation) allows generating variable-duty-cycle signals for motor control, LED brightness, servo control, etc.

#### PWM Setup - udev Rules

Create `/etc/udev/rules.d/99-pwm.rules`:

```bash
SUBSYSTEM=="pwm", ACTION=="add", RUN+="/bin/chgrp -R gpio /sys%p", RUN+="/bin/chmod -R g+w /sys%p"
```

#### PWM Overlay Configuration

Edit `/boot/config.txt` or `/boot/firmware/config.txt`:

**Single Channel PWM:**
```bash
# Enable PWM on GPIO 12 (hardware PWM0)
dtoverlay=pwm,pin=12,func=4
```

**Dual Channel PWM:**
```bash
# Enable PWM on GPIO 12 (PWM0) and GPIO 13 (PWM1)
dtoverlay=pwm-2chan,pin=12,func=4,pin2=13,func2=4
```

**Alternative PWM Pins:**
```bash
# PWM0 can use: GPIO 12, 18, 40, 52
# PWM1 can use: GPIO 13, 19, 41, 45, 53

# Example: PWM on GPIO 18 and 19
dtoverlay=pwm-2chan,pin=18,func=2,pin2=19,func2=2
```

#### PWM Parameters Explained

- `pin` / `pin2` - GPIO pin number to use for PWM
- `func` / `func2` - Alternate function number (typically 4 for standard PWM pins, 2 for alternate pins)

#### StellarMate Pro PWM Example

```bash
# Enable dual-channel PWM on GPIO 12 and 13
# Used for CH5, CH6 outputs (focus motor control, dew heaters, etc.)
dtoverlay=pwm-2chan,pin=12,func=4,pin2=13,func2=4
```

#### Testing PWM

After reboot, test PWM access:

```bash
# Check if PWM chip is available
ls -l /sys/class/pwm/pwmchip*

# Export PWM channel
echo 0 > /sys/class/pwm/pwmchip0/export

# Configure period and duty cycle
echo 20000000 > /sys/class/pwm/pwmchip0/pwm0/period      # 20ms (50Hz)
echo 1500000 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle   # 1.5ms pulse
echo 1 > /sys/class/pwm/pwmchip0/pwm0/enable
```

### SPI Configuration

SPI (Serial Peripheral Interface) is a high-speed synchronous serial communication protocol commonly used for sensors, displays, and ADCs.

#### Enable SPI Interface

Edit `/boot/config.txt` or `/boot/firmware/config.txt`:

```bash
# Enable SPI interface
dtparam=spi=on
```

#### SPI with Chip Select Configuration

**Important:** Chip select (CS) pins must be explicitly configured for hardware SPI to work correctly.

**Single Chip Select (CS0 only):**
```bash
# Enable SPI0 with one chip select on GPIO 8
dtoverlay=spi0-1cs,cs0_pin=8
```

**Dual Chip Select (CS0 and CS1):**
```bash
# Enable SPI0 with two chip selects on GPIO 8 and GPIO 7
dtoverlay=spi0-2cs,cs0_pin=8,cs1_pin=7
```

**StellarMate Pro SPI Example:**
```bash
# Enable SPI interface
dtparam=spi=on

# Configure single chip select on GPIO 8
dtoverlay=spi0-1cs,cs0_pin=8
```

#### SPI Pin Mappings

**SPI0 (Primary):**
- MOSI: GPIO 10
- MISO: GPIO 9
- SCLK: GPIO 11
- CS0: GPIO 8 (default, configurable)
- CS1: GPIO 7 (default, configurable)

**SPI1 (Secondary):**
- MOSI: GPIO 20
- MISO: GPIO 19
- SCLK: GPIO 21
- CS0: GPIO 18
- CS1: GPIO 17
- CS2: GPIO 16

#### Custom CS Pins

You can configure chip select on alternative GPIO pins:

```bash
# Use GPIO 25 for CS0 instead of GPIO 8
dtoverlay=spi0-1cs,cs0_pin=25

# Use GPIO 25 and 26 for CS0 and CS1
dtoverlay=spi0-2cs,cs0_pin=25,cs1_pin=26
```

#### Testing SPI

After reboot, verify SPI devices:

```bash
# List SPI devices
ls -l /dev/spidev*

# Should show:
# /dev/spidev0.0  (SPI0, CS0)
# /dev/spidev0.1  (SPI0, CS1) - if 2cs overlay is used

# Check permissions
ls -l /dev/spidev0.0
```

#### SPI User Permissions

Add user to SPI group:

```bash
sudo usermod -a -G spi $USER
```

Create udev rule `/etc/udev/rules.d/99-spi.rules`:

```bash
SUBSYSTEM=="spidev", GROUP="spi", MODE="0660"
```

Reload udev rules:

```bash
sudo udevadm control --reload-rules
sudo udevadm trigger
```

### Complete Raspberry Pi Configuration Example

Here's a complete `/boot/config.txt` section for a typical astronomy setup (based on StellarMate Pro):

```bash
######################################
# Hardware Interface Configuration   #
######################################

# GPIO Configuration
# Inputs: Limit switches, sensors
gpio=6,18,19,24=ip,np

# Outputs: Relays, LEDs (initially off)
gpio=7,17,21,23,25,26,27=op,dl

# I2C Interface
dtparam=i2c_arm=on,i2c_arm_baudrate=400000
dtparam=i2c_vc=on,i2c_vc_baudrate=100000

# SPI Interface with chip select
dtparam=spi=on
dtoverlay=spi0-1cs,cs0_pin=8

# PWM Outputs (for focus motors, dew heaters)
dtoverlay=pwm-2chan,pin=12,func=4,pin2=13,func2=4

# Additional UART for GPS
dtoverlay=uart2,init_uart_baud=115200

# PPS (Precision Time Protocol) signal on GPIO 19
dtoverlay=pps-gpio,gpiopin=19
```

#### After Configuration Changes

Always reboot after modifying `/boot/config.txt`:

```bash
sudo reboot
```

Verify your changes:

```bash
# Check GPIO configuration
gpioinfo gpiochip0

# Check I2C
i2cdetect -y 1

# Check SPI
ls -l /dev/spidev*

# Check PWM
ls -l /sys/class/pwm/
```

## I2C Communication Setup

### Add User to i2c Group

```bash
sudo usermod -a -G i2c $USER
```

### Enable I2C on Raspberry Pi

```bash
sudo raspi-config
```

Navigate to: **Interface Options** → **I2C** → **Enable**

### Verify I2C

```bash
# List I2C devices
ls -l /dev/i2c-*

# Scan I2C bus (install i2c-tools if needed)
sudo apt-get install i2c-tools
i2cdetect -y 1
```

## Network Communication Setup

### Firewall Configuration

For TCP/UDP connections, ensure firewall allows the required ports:

```bash
# UFW (Ubuntu)
sudo ufw allow 9999/tcp
sudo ufw allow 9999/udp

# firewalld (Fedora/CentOS)
sudo firewall-cmd --permanent --add-port=9999/tcp
sudo firewall-cmd --permanent --add-port=9999/udp
sudo firewall-cmd --reload
```

## Testing Permissions

### Test Serial Access

```bash
# Without sudo, this should work:
screen /dev/ttyUSB0 9600
# Press Ctrl+A, then K to exit
```

### Test USB Access

```bash
# List USB devices (should work without sudo)
lsusb -v
```

### Test GPIO Access

```bash
# Should work without sudo
gpiodetect
gpioinfo gpiochip0
```

### Test I2C Access

```bash
# Should work without sudo
i2cdetect -y 1
```

## Troubleshooting Permissions

### Check Group Membership

```bash
groups $USER
```

Should show: `dialout`, `gpio`, `i2c`, etc.

### Force Group Refresh

If groups don't show after adding:

```bash
# Log out and log back in, OR
newgrp dialout
newgrp gpio
newgrp i2c
```

### Verify udev Rules

```bash
# List udev rules
ls -l /etc/udev/rules.d/

# Test rule syntax
udevadm test /sys/class/hidraw/hidraw0
```

## Platform-Specific Notes

### Raspberry Pi OS

- GPIO and I2C typically require `raspi-config` to enable
- PWM requires device tree overlay configuration
- Default user `pi` often has necessary permissions

### Ubuntu/Debian

- Groups: `dialout`, `gpio`, `i2c`
- May need to install `i2c-tools`, `libgpiod-dev`

### Fedora/CentOS

- Groups: `dialout`, `gpio`, `i2c`
- SELinux may require additional configuration

### Arch Linux

- Groups: `uucp` (instead of dialout), `gpio`, `i2c`
- Use `pacman` to install required packages

## Installation of Required Libraries

### Debian/Ubuntu

```bash
sudo apt-get install libusb-1.0-0-dev
sudo apt-get install libhidapi-dev
sudo apt-get install libgpiod-dev
sudo apt-get install i2c-tools libi2c-dev
```

### Fedora/CentOS

```bash
sudo dnf install libusb-devel
sudo dnf install hidapi-devel
sudo dnf install libgpiod-devel
sudo dnf install i2c-tools i2c-tools-devel
```

### Arch Linux

```bash
sudo pacman -S libusb
sudo pacman -S hidapi
sudo pacman -S libgpiod
sudo pacman -S i2c-tools
```

## Security Considerations

### Production Systems

For production systems, consider more restrictive permissions:

```bash
# Instead of MODE="0666", use group-based access
SUBSYSTEM=="usb", ATTRS{idVendor}=="134a", GROUP="indi", MODE="0660"
```

Create an `indi` group:

```bash
sudo groupadd indi
sudo usermod -a -G indi $USER
```

### Root Access

Avoid running INDI drivers as root. Use proper udev rules and group memberships instead.

## Related Guides

- [Serial Connection](serial/)
- [USB Communication](usb/)
- [HID Communication](hid/)
- [GPIO Communication](gpio/)
- [I2C Communication](i2c/)
- [Troubleshooting](troubleshooting/)
