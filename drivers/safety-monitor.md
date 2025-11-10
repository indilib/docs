---
title: Safety Monitor
nav_order: 12
parent: Drivers
permalink: /drivers/safety-monitor/
---

# INDI Safety Monitor Driver

## Overview

The Safety Monitor driver aggregates safety status from multiple INDI devices and reports the worst-case condition. This allows observatory automation systems (like Ekos) to make informed decisions about dome operations, mount parking, and other safety-critical actions.

## Key Features

- **Multi-Device Monitoring**: Monitor safety status from multiple devices simultaneously
- **Event-Driven**: Real-time updates via INDI client callbacks (no polling)
- **Flexible Connection**: Support for local and remote devices
- **Dynamic Configuration**: Add or remove devices without driver restart
- **Override Capability**: Emergency override switch for manual control
- **Human-Readable Logging**: Status changes reported as "Ok", "Busy", "Alert" instead of numbers

## SAFETY_STATUS Standard Property

The Safety Monitor relies on the `SAFETY_STATUS` standard property, which any INDI driver can implement:

- **Property Name**: `SAFETY_STATUS`
- **Type**: Light Property
- **Element**: `SAFETY`
- **States**:
  - `IPS_OK` - Safe for operations
  - `IPS_BUSY` - Warning condition
  - `IPS_ALERT` - Unsafe/critical condition
  - `IPS_IDLE` - Unknown/disconnected

## Compatible Devices

Any INDI driver implementing the `SAFETY_STATUS` property can be monitored. This includes:

### Weather Drivers
All weather drivers automatically expose `SAFETY_STATUS` (mirrored from `WEATHER_STATUS`):
- OpenWeatherMap
- Weather Meta
- Weather Simulator
- And all other weather drivers

### Power Devices
- UPS Driver (battery charge monitoring)
- Power supply drivers

### Custom Devices
Any driver can implement `SAFETY_STATUS` for application-specific safety monitoring

## Configuration

### Connection String Format

Device connections are specified as a comma-separated list in the `CONNECTION_STRINGS` property:

```
DeviceName, DeviceName@host, DeviceName@host:port
```

**Format Details:**
- **Device name only**: Uses localhost:7624 (default INDI port)
- **Device@host**: Uses specified host, port 7624
- **Device@host:port**: Fully specified connection
- **Multiple devices**: Separate with commas
- **Spaces in names**: Supported (no quotes needed)

### Examples

**Local devices only:**
```
Open Weather Map, UPS
```

**Mix of local and remote:**
```
My Weather Station, UPS@localhost:7624, Remote Power@192.168.1.10
```

**All remote with custom ports:**
```
Weather@192.168.1.5:7624, UPS@192.168.1.5:7625, Power Supply@192.168.1.10:7624
```

## Operation

### Safety Status Property

The driver creates a dynamic `SAFETY_STATUS` property with one element per monitored device:

```
SAFETY_STATUS
├─ Open Weather Map: Ok
├─ UPS: Busy  
└─ Power Supply: Ok
```

The **overall property state** (not individual elements) represents the worst case:
- If all devices are `Ok`, overall state is `Ok`
- If one device is `Busy`, overall state is `Busy`
- If any device is `Alert`, overall state is `Alert`

### Override Switch

The `SAFETY_OVERRIDE` switch forces the overall status to `Ok` regardless of actual device states:

- **Use Case**: Emergency situations requiring manual control
- **Warning**: When enabled, the driver logs a warning and sets the switch to `Busy` state
- **Recommendation**: Disable as soon as the emergency condition is resolved

### Dynamic Reconfiguration

Changes to `CONNECTION_STRINGS` take effect immediately:
1. All existing client connections are closed
2. New connections are established based on updated string
3. `SAFETY_STATUS` property is rebuilt with new device list
4. No driver restart required

## Integration with Observatory Automation

### Ekos Integration

Ekos Observatory module monitors the `SAFETY_STATUS` property to make go/no-go decisions:

- **Safe (`IPS_OK`)**: Observatory can operate normally
- **Warning (`IPS_BUSY`)**: Issue warnings, may continue with caution
- **Unsafe (`IPS_ALERT`)**: Emergency shutdown:
  - Stop mount tracking
  - Park mount
  - Close dome
  - Stop all automated sequences

### Client Usage

INDI clients should monitor the **overall property state**, not individual element states:

```python
# Pseudocode
safety_property = device.get_property("SAFETY_STATUS")

if safety_property.state == IPS_OK:
    # Safe to operate
elif safety_property.state == IPS_BUSY:
    # Warning condition
elif safety_property.state == IPS_ALERT:
    # Unsafe - take protective action
```

## Example Scenario

**Observatory Setup:**
- Weather station monitoring cloud coverage and wind
- UPS monitoring power supply
- Remote power controller monitoring mains power

**Configuration:**
```
CONNECTION_STRINGS: Weather Station, UPS, Power Controller@192.168.1.10
```

**Scenario Timeline:**

1. **Normal Operation** (All Ok)
   - Weather: Clear skies, calm wind → `Ok`
   - UPS: Battery at 100%, on mains → `Ok`  
   - Power: Mains stable → `Ok`
   - **Overall Status**: `Ok` ✓

2. **Warning Condition** (One device Busy)
   - Weather: Increasing clouds → `Busy`
   - UPS: Battery at 100%, on mains → `Ok`
   - Power: Mains stable → `Ok`
   - **Overall Status**: `Busy` ⚠️
   - **Action**: Ekos issues warning, continues with caution

3. **Critical Condition** (One device Alert)
   - Weather: Heavy clouds, unsafe → `Alert`
   - UPS: Battery at 30%, on battery → `Busy`
   - Power: Mains failed → `Alert`
   - **Overall Status**: `Alert` 🛑
   - **Action**: Ekos initiates emergency shutdown

## Implementing SAFETY_STATUS in Your Driver

To make your driver compatible with Safety Monitor:

### 1. Add the Property

```cpp
// In your driver header
INDI::PropertyLight SafetyStatusLP {1};

// In initProperties()
SafetyStatusLP[0].fill("SAFETY", "Safety", IPS_IDLE);
SafetyStatusLP.fill(getDeviceName(), "SAFETY_STATUS", "Status", 
                    MAIN_CONTROL_TAB, IPS_IDLE);
```

### 2. Update the Status

```cpp
// When your safety condition changes
IPState safetyState = IPS_OK;  // or IPS_BUSY, IPS_ALERT

if (criticalCondition)
    safetyState = IPS_ALERT;
else if (warningCondition)
    safetyState = IPS_BUSY;
else
    safetyState = IPS_OK;

SafetyStatusLP.setState(safetyState);
SafetyStatusLP.apply();
```

### 3. Define/Delete with Connection

```cpp
// In updateProperties()
if (isConnected())
    defineProperty(SafetyStatusLP);
else
    deleteProperty(SafetyStatusLP);
```

## Troubleshooting

### Device Not Appearing in Status

**Symptoms**: Device listed in connection string but not appearing in SAFETY_STATUS

**Possible Causes:**
1. Device name misspelled (case-sensitive)
2. Device not connected on specified host
3. Device doesn't implement SAFETY_STATUS property
4. Network connectivity issues

**Solutions:**
- Verify device name matches exactly (check `device.getDeviceName()`)
- Ensure device is connected before Safety Monitor connects
- Check device logs for SAFETY_STATUS property definition
- Test network connectivity to remote hosts

### Status Always Shows "Idle"

**Symptoms**: SAFETY_STATUS elements stuck at `Idle`

**Possible Causes:**
1. Device offline
2. Device hasn't sent SAFETY_STATUS property yet  
3. Property not being updated by device

**Solutions:**
- Check device connection status
- Wait for device initialization
- Enable debug logging to see property updates

### Override Not Working

**Symptoms**: Override switch doesn't force status to Ok

**Possible Causes:**
1. Switch not properly saved
2. Using individual element states instead of overall state

**Solutions:**
- Ensure override switch is `On` (check switch state)
- Monitor overall property state, not individual elements
- Check driver logs for override confirmation

## Best Practices

1. **Start Simple**: Begin with one or two devices, add more as needed
2. **Test Thoroughly**: Verify safety shutdown sequences before deployment  
3. **Monitor Logs**: Enable debug logging during initial setup
4. **Use Override Sparingly**: Only for genuine emergencies
5. **Regular Checks**: Periodically verify all monitored devices are online
6. **Document Configuration**: Keep notes on which devices are monitored and why

## See Also

- [INDI Standard Properties](../standard-properties/) - Complete list of standard properties
- [Weather Interface](../weather-interface/) - Weather driver implementation details
- [Ekos Observatory](https://docs.kde.org/stable5/en/kstars/kstars/ekos-observatory.html) - Observatory automation
