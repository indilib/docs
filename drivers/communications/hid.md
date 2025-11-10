---
title: HID Communication
parent: Hardware Communications
grand_parent: Driver Development
nav_order: 7
permalink: /drivers/communications/hid/
---

# HID Communication

Human Interface Device (HID) communication using hidapi for USB HID devices.

## When to Use HID Communication

- USB devices implementing HID protocol
- Filter wheels with USB HID interface
- Focusers with HID control
- Custom keypads and input devices
- Devices that appear as HID devices in the system

## Required Headers

```cpp
#include <hidapi/hidapi.h>
```

## Basic HID Setup

```cpp
class SXWheel : public INDI::FilterWheel
{
public:
    SXWheel()
    {
        FilterSlotNP[0].setMin(1);
        FilterSlotNP[0].setMax(-1);
        CurrentFilter = 1;
        handle = nullptr;
    }
    
    ~SXWheel()
    {
        if (handle)
            hid_close(handle);
        hid_exit();
    }
    
    bool Connect()
    {
        if (isSimulation())
        {
            handle = (hid_device *)1;  // Dummy handle
            return true;
        }
        
        // Open HID device by vendor and product ID
        handle = hid_open(0x1278, 0x0920, nullptr);
        if (handle)
        {
            SelectFilter(CurrentFilter);
            return true;
        }
        
        LOG_ERROR("Failed to open HID device");
        return false;
    }
    
    bool Disconnect()
    {
        if (handle && !isSimulation())
            hid_close(handle);
        handle = nullptr;
        return true;
    }

private:
    hid_device *handle;
    int CurrentFilter;
};
```

## HID Communication Example

```cpp
int SXWheel::sendWheelMessage(int a, int b)
{
    if (!handle)
    {
        LOG_ERROR("Filter wheel not connected");
        return -1;
    }
    
    // Prepare HID report
    unsigned char buf[2] = { 
        static_cast<unsigned char>(a), 
        static_cast<unsigned char>(b) 
    };
    
    // Write HID report
    int rc = hid_write(handle, buf, 2);
    LOGF_DEBUG("hid_write({ %d, %d }) -> %d", buf[0], buf[1], rc);
    if (rc != 2)
    {
        LOG_ERROR("Failed to write to wheel");
        return -1;
    }
    
    usleep(100);  // Short delay
    
    // Read HID report
    rc = hid_read(handle, buf, 2);
    LOGF_DEBUG("hid_read() -> { %d, %d } %d", buf[0], buf[1], rc);
    if (rc != 2)
    {
        LOG_ERROR("Failed to read from wheel");
        return -1;
    }
    
    CurrentFilter = buf[0];
    FilterSlotNP[0].setMax(buf[1]);
    
    return 0;
}
```

## Finding HID Device IDs

```bash
# List USB devices including HID
lsusb

# Get vendor/product IDs
lsusb -v -d 1278:0920
```

## Best Practices

- **Vendor/Product IDs**: Obtain correct IDs using `lsusb` command
- **Report Structure**: Understand device's HID report descriptor
- **Timeouts**: Use `hid_read_timeout()` for non-blocking reads
- **Initialization**: Call `hid_init()` at program start (usually in constructor)
- **Cleanup**: Always call `hid_close()` and `hid_exit()`

## Troubleshooting

### hid_open() Returns NULL

```bash
# Check device is connected
lsusb | grep -i "1278"

# Verify it's an HID device
ls -l /dev/hidraw*

# Check permissions
ls -l /dev/hidraw0
```

### Permission Denied

Create udev rule `/etc/udev/rules.d/99-hidraw.rules`:
```bash
KERNEL=="hidraw*", ATTRS{idVendor}=="1278", MODE="0666"
```

Reload rules:
```bash
sudo udevadm control --reload-rules
sudo udevadm trigger
```

## Example Driver

See `indi-sx/sxwheel.cpp` for Starlight Xpress filter wheel implementation.

## Related Guides

- [USB Communication](usb/) - For non-HID USB devices
- [System Setup](system-setup/) - Detailed permission configuration
- [Troubleshooting](troubleshooting/)
