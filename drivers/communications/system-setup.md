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

### PWM Setup

Create `/etc/udev/rules.d/99-pwm.rules`:

```bash
SUBSYSTEM=="pwm", ACTION=="add", RUN+="/bin/chgrp -R gpio /sys%p", RUN+="/bin/chmod -R g+w /sys%p"
```

### Enable PWM Overlay (Raspberry Pi)

Edit `/boot/config.txt`:

```bash
# Enable PWM on GPIO 12 and 13
dtoverlay=pwm-2chan
```

Reboot after making changes.

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
