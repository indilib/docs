---
title: USB Communication
parent: Hardware Communications
grand_parent: Driver Development
nav_order: 6
permalink: /drivers/communications/usb/
---

# USB Communication

Direct USB communication using libusb for devices that use bulk transfer endpoints.

## When to Use USB Communication

- Custom USB devices with bulk transfer endpoints
- Devices that don't use standard protocols (serial, HID)
- DSLR camera shutter control
- Proprietary USB hardware
- Direct USB control without intermediate drivers

## Required Headers

```cpp
#include <libusb.h>
#include <indiusbdevice.h>
```

Your driver should inherit from `INDI::USBDevice`:

```cpp
class MyDriver : public INDI::USBDevice
{
    // Driver implementation
};
```

## Basic USB Device Setup

```cpp
class DSUSBDriver : public INDI::USBDevice
{
public:
    DSUSBDriver(const char *device)
    {
        snprintf(this->device, sizeof(this->device), "%s", device);
        
        // Find device by vendor and product ID
        dev = FindDevice(0x134A, 0x9021, 0);
        if (!dev)
        {
            // Try alternate product ID
            dev = FindDevice(0x134A, 0x9026, 0);
        }
        
        if (dev)
        {
            int rc = Open();
            connected = (rc != -1);
            if (connected)
            {
                LOG_INFO("USB device connected");
                readState();
            }
        }
    }
    
private:
    char device[256];
    bool connected = false;
};
```

## USB Communication Example

```cpp
bool DSUSBDriver::sendCommand(uint8_t command)
{
    DEBUGFDEVICE(device, INDI::Logger::DBG_DEBUG, "Sending command: 0x%02X", command);
    
    // Write bulk transfer
    int rc = WriteBulk(&command, 1, 1000);  // 1 byte, 1000ms timeout
    if (rc < 0)
    {
        LOGF_ERROR("USB write failed: %d", rc);
        return false;
    }
    
    return true;
}

bool DSUSBDriver::readState()
{
    uint8_t infoByte;
    
    // Read bulk transfer
    int rc = ReadBulk(&infoByte, 1, 1000);  // 1 byte, 1000ms timeout
    if (rc != 1)
    {
        LOGF_ERROR("USB read failed: %d", rc);
        return false;
    }
    
    DEBUGFDEVICE(device, INDI::Logger::DBG_DEBUG, "Info byte: 0x%02X", infoByte);
    return true;
}
```

## Finding USB Device IDs

```bash
# List USB devices
lsusb

# Get detailed information
lsusb -v -d 134a:9021
```

## Best Practices

- **Device Discovery**: Use `FindDevice(vendor_id, product_id, index)` for enumeration
- **Timeouts**: Always specify reasonable timeouts for bulk transfers (typically 1000ms)
- **Error Handling**: Check return values from all USB operations
- **Cleanup**: Ensure proper device closure in destructor
- **Permissions**: May require udev rules for non-root access

## Troubleshooting

### Device Not Found
```bash
# Check if device is connected
lsusb | grep -i "134a"

# Check permissions
ls -l /dev/bus/usb/*/
```

### Permission Denied
Create udev rule `/etc/udev/rules.d/99-mydevice.rules`:
```bash
SUBSYSTEM=="usb", ATTRS{idVendor}=="134a", ATTRS{idProduct}=="9021", MODE="0666"
```

Then reload:
```bash
sudo udevadm control --reload-rules
sudo udevadm trigger
```

## Example Driver

See `indi-gphoto/dsusbdriver.cpp` for DSLR shutter control implementation.

## Related Guides

- [HID Communication](hid/) - For HID devices
- [System Setup](system-setup/) - udev rules and permissions
- [Troubleshooting](troubleshooting/)
