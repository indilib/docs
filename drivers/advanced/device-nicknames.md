---
title: Device Nicknames
nav_order: 6
parent: Advanced
---

# Device Nicknames

When a user has several devices of the same model, for example a focuser on the main telescope and another on the guide scope, the driver names them "My Focuser" and "My Focuser 1" in the order they are found. That order can change between sessions, so the user cannot tell which device is which.

Nicknames solve this. A user gives each device a nickname, such as "MainScope", and the device is then named "My Focuser MainScope" in every session, no matter which USB port it is plugged into. Clients and saved equipment profiles can rely on this name.

Nicknames are supported by `INDI::DefaultDevice`, so they are available to every driver. A driver needs a **stable identifier** for each device, usually its serial number, to remember which nickname belongs to which device.

## How It Works

- Nicknames are stored in `~/.indi/Nicknames.xml`, keyed by the driver's default name and the device identifier:

  ```xml
  <INDINicknames>
    <nickname driver="My Focuser" identifier="SN123">MainScope</nickname>
    <nickname driver="My Focuser" identifier="SN456">GuideScope</nickname>
  </INDINicknames>
  ```

- When the device is created, the driver looks up the nickname for the device's identifier. If one is found, the device name becomes the default name followed by the nickname, for example "My Focuser MainScope".
- Users set the nickname through the `NICKNAME` text property on the device's General Info tab. Because a device cannot be renamed while the driver is running, a new nickname takes effect the next time the driver starts.

Identifiers are compared as plain strings, and each driver uses its own identifiers, so devices from different drivers can share a nickname.

## Adding Nickname Support to a Driver

1. **Apply the saved nickname in the constructor.** The device name can only be set before the driver is initialized. Call `setDeviceNicknameFromId()` with the device's identifier. It returns `true` if a nickname was found.
2. **Add the `NICKNAME` property** by calling `addNicknameControl()` in `initProperties()`.
3. **Save new nicknames.** When the user changes the nickname, the base class calls `nicknameSet()`. Override it and call `saveNicknameId()` to store the nickname for the device's identifier.

```cpp
MyFocuser::MyFocuser(int id, const char *name, const std::string &serialNumber)
    : m_ID(id), m_SerialNumber(serialNumber)
{
    setVersion(1, 0);

    // Use the saved nickname for this serial number. If there is none,
    // use the name we were given (e.g. "My Focuser 1").
    if (m_SerialNumber.empty() || !setDeviceNicknameFromId(m_SerialNumber.c_str()))
        setDeviceName(name);
}

bool MyFocuser::initProperties()
{
    INDI::Focuser::initProperties();

    // Lets the user set a nickname on the General Info tab.
    addNicknameControl();

    addAuxControls();
    return true;
}

void MyFocuser::nicknameSet(const char *nickname)
{
    // Store the nickname for this serial number. It takes effect on the next start.
    if (!m_SerialNumber.empty())
        saveNicknameId(nickname, m_SerialNumber.c_str());
}
```

The identifier must not depend on the USB port or the order in which devices are found. If the device has no serial number or other unique ID, do not offer nicknames.

## API Reference

| Method                                                   | Description                                                                 |
| -------------------------------------------------------- | --------------------------------------------------------------------------- |
| `void addNicknameControl()`                              | Adds the `NICKNAME` property. Call it in `initProperties()`.                |
| `bool setDeviceNicknameFromId(const char *identifier)`   | Looks up the nickname for `identifier` and, if found, sets it. Returns `true` if a nickname was found. Call it in the constructor. |
| `const char *lookupDeviceNicknameFromId(const char *identifier)` | Returns the nickname saved for `identifier`, or an empty string.    |
| `void setDeviceNickname(const char *nick)`               | Sets the nickname. Before initialization, it also sets the device name to the default name followed by the nickname. |
| `const char *getDeviceNickname()`                        | Returns the current nickname.                                               |
| `void saveNicknameId(const char *nickname, const char *identifier)` | Saves the nickname for `identifier` in `~/.indi/Nicknames.xml`.  |
| `virtual void nicknameSet(const char *nickname)`         | Called when the user sets a nickname. Override it to call `saveNicknameId()`. |

Nicknames are most useful together with [hotplug support](hotplug.md), which lets one driver manage several identical devices.

## Real-World Examples

The ZWO ASI camera, focuser, and filter wheel drivers in [INDI 3rd party](https://github.com/indilib/indi-3rdparty/tree/master/indi-asi) use the device serial number as the nickname identifier.
