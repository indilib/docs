---
title: Hotplug Support
nav_order: 5
parent: Advanced
---

# Hotplug Support

Many USB devices, such as cameras, focusers, and filter wheels, can be plugged in and unplugged while INDI is running. A driver with hotplug support creates a device as soon as it is plugged in, and removes it when it is unplugged, without restarting the INDI server. One driver process can also manage several identical devices at once, for example two focusers of the same model.

Hotplug support is provided by two classes in `libindi`:

- `INDI::HotPlugCapableDevice` (`hotplugcapabledevice.h`): An interface that your driver implements to discover, create, and destroy devices.
- `INDI::HotPlugManager` (`hotplugmanager.h`): A singleton that watches for USB changes and calls your handler when devices appear or disappear.

## How It Works

1. When the driver starts, it registers a handler with `HotPlugManager` and starts it.
2. `HotPlugManager` periodically asks the handler which devices are connected (`discoverConnectedDeviceIdentifiers()`) and which devices it already manages (`getManagedDevices()`).
3. For each new identifier, it calls `createDevice()`, then calls `ISGetProperties()` on the new device so clients see it.
4. For each managed device that is no longer connected, it calls `destroyDevice()`.

Each device is a separate `INDI::DefaultDevice` instance with its own name. `libindidriver` routes client requests to the right instance by device name, so you do not need any extra code for that.

### When Devices Are Checked

- **Linux with udev**: The manager checks for devices at the given interval for a few seconds after startup (5 seconds by default). After that, it only checks when udev reports that a USB device was added or removed. Bursts of udev events are debounced into a single check.
- **Other systems (macOS, systems without udev)**: The manager checks at the given interval for 60 seconds by default, then stops. Devices plugged in after that are not detected until the driver restarts.

You can change these durations with `setInitialPollingDuration(seconds)` (udev systems) and `setNonUdevPollingDuration(seconds)` (other systems, where `0` means poll forever). Both accept `-1` to restore the default.

## Implementing a Hotplug Handler

### The HotPlugCapableDevice Interface

```cpp
class HotPlugCapableDevice
{
    public:
        // Return a unique identifier for each connected device of this driver's type.
        virtual std::vector<std::string> discoverConnectedDeviceIdentifiers() = 0;

        // Create a device instance for the given identifier.
        virtual std::shared_ptr<DefaultDevice> createDevice(const std::string &identifier) = 0;

        // Clean up and remove a device that was unplugged.
        virtual void destroyDevice(std::shared_ptr<DefaultDevice> device) = 0;

        // Return the devices this handler currently manages, keyed by identifier.
        virtual const std::map<std::string, std::shared_ptr<DefaultDevice>> &getManagedDevices() const = 0;
};
```

The identifier must be unique among connected devices and must stay the same while the device is connected, because the manager compares identifiers between checks. Use the SDK's device ID or serial number.

### Example

Here's a handler for a hypothetical focuser whose SDK provides `myfoc_count()`, `myfoc_get_id()`, and `myfoc_get_serial()`. `MyFocuser` is the driver's device class, derived from `INDI::Focuser`, and takes its SDK ID, device name, and serial number in its constructor.

```cpp
#include <libindi/hotplugcapabledevice.h>
#include <libindi/hotplugmanager.h>

#include <algorithm>
#include <map>
#include <memory>
#include <vector>

#include "my_focuser.h"

class MyFocuserHotPlugHandler : public INDI::HotPlugCapableDevice
{
    public:
        std::vector<std::string> discoverConnectedDeviceIdentifiers() override
        {
            std::vector<std::string> ids;
            for (int i = 0; i < myfoc_count(); i++)
                ids.push_back(std::to_string(myfoc_get_id(i)));
            return ids;
        }

        std::shared_ptr<INDI::DefaultDevice> createDevice(const std::string &identifier) override
        {
            int id = std::stoi(identifier);

            // Give each device a unique name: "My Focuser", "My Focuser 1", ...
            std::string name = "My Focuser";
            for (int index = 1; nameInUse(name); index++)
                name = "My Focuser " + std::to_string(index);

            auto device = std::make_shared<MyFocuser>(id, name.c_str(), myfoc_get_serial(id));
            m_Devices[identifier] = device;
            return device;
        }

        void destroyDevice(std::shared_ptr<INDI::DefaultDevice> device) override
        {
            // Remove the device's properties from clients.
            device->deleteProperty(nullptr);

            for (auto it = m_Devices.begin(); it != m_Devices.end(); ++it)
            {
                if (it->second == device)
                {
                    m_Devices.erase(it);
                    break;
                }
            }
        }

        const std::map<std::string, std::shared_ptr<INDI::DefaultDevice>> &getManagedDevices() const override
        {
            return m_Devices;
        }

    private:
        bool nameInUse(const std::string &name) const
        {
            return std::any_of(m_Devices.begin(), m_Devices.end(), [&](const auto &entry)
            {
                return name == entry.second->getDeviceName();
            });
        }

        std::map<std::string, std::shared_ptr<INDI::DefaultDevice>> m_Devices;
};
```

### Registering the Handler

Register the handler and start the manager when the driver starts. A static object is a convenient place to do this, and replaces the usual static driver instance:

```cpp
static class Loader
{
    public:
        Loader()
        {
            m_Handler = std::make_shared<MyFocuserHotPlugHandler>();
            INDI::HotPlugManager::getInstance().registerHandler(m_Handler);

            // Check for devices every second.
            INDI::HotPlugManager::getInstance().start(1000);
        }

    private:
        std::shared_ptr<MyFocuserHotPlugHandler> m_Handler;
} loader;
```

`start(intervalMs, oneShot)` takes the check interval in milliseconds. If `oneShot` is `true`, the manager only checks once, at startup.

### The Device Class

Because one driver can now create several devices, the device class should:

- Take its device name in the constructor and call `setDeviceName()`, instead of relying on `getDefaultName()` alone.
- Keep all its state in member variables, never in static or global variables.
- Use the SDK ID it was created with for every SDK call.

```cpp
MyFocuser::MyFocuser(int id, const char *name, const std::string &serialNumber)
    : m_ID(id), m_SerialNumber(serialNumber)
{
    setVersion(1, 0);
    setDeviceName(name);

    // The SDK handles the USB connection, so no connection plugin is needed.
    setSupportedConnections(CONNECTION_NONE);
    FI::SetCapability(FOCUSER_CAN_ABS_MOVE | FOCUSER_CAN_REL_MOVE | FOCUSER_CAN_ABORT);
}
```

To let users give each device a name that stays the same no matter which USB port it is plugged into, combine hotplug with [device nicknames](device-nicknames.md).

## Real-World Examples

The ZWO ASI drivers in [INDI 3rd party](https://github.com/indilib/indi-3rdparty/tree/master/indi-asi) use hotplug support for cameras, focusers (`asi_focuser_hotplug_handler.cpp`), and filter wheels, together with device nicknames.
