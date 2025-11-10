---
title: Modbus Communication
parent: Hardware Communications
nav_order: 9
permalink: /drivers/communications/modbus/
---

# Modbus Protocol Communication

This guide covers implementing Modbus protocol communication in INDI drivers using the nanomodbus library. Modbus is a widely-used industrial communication protocol for connecting PLCs, sensors, actuators, relays, and other automation equipment.

## Table of Contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Overview

### What is Modbus?

Modbus is a serial communication protocol originally developed for industrial automation. It's now available in several variants:
- **Modbus RTU**: Serial communication (RS-232/RS-485)
- **Modbus TCP**: Ethernet-based communication
- **Modbus ASCII**: Human-readable serial format

### When to Use Modbus

Use Modbus communication when your device:
- Is an industrial control device (PLC, relay board, sensor)
- Explicitly supports Modbus protocol
- Uses standard Modbus registers (coils, discrete inputs, holding registers, input registers)
- Requires reliable request/response communication
- Needs to read/write multiple data points efficiently

### Library Used

**nanomodbus**: A compact, portable Modbus library included with INDI
- Supports both Modbus RTU and Modbus TCP
- Small memory footprint
- Platform-agnostic
- Simple, straightforward API
- Built into INDI library (no external dependencies)

---

## Modbus Basics

### Data Model

Modbus organizes data into four tables:

| Data Type | Access | Address Range | Function Codes | Description |
|-----------|--------|---------------|----------------|-------------|
| **Coils** | Read/Write | 0-65535 | 01, 05, 15 | Digital outputs (ON/OFF) |
| **Discrete Inputs** | Read-only | 0-65535 | 02 | Digital inputs (ON/OFF) |
| **Holding Registers** | Read/Write | 0-65535 | 03, 06, 16 | 16-bit registers (configuration, setpoints) |
| **Input Registers** | Read-only | 0-65535 | 04 | 16-bit registers (sensor readings) |

### Common Function Codes

- **01**: Read Coils
- **02**: Read Discrete Inputs
- **03**: Read Holding Registers
- **04**: Read Input Registers
- **05**: Write Single Coil
- **06**: Write Single Register
- **15**: Write Multiple Coils
- **16**: Write Multiple Registers

---

## Basic Setup

### Include Headers

```cpp
#include "connectionplugins/connectiontcp.h"  // For Modbus TCP
// or
#include "connectionplugins/connectionserial.h"  // For Modbus RTU

#include "libs/modbus/platform.h"  // nanomodbus platform functions
```

### Driver Class Setup

```cpp
class MyModbusDriver : public INDI::DefaultDevice
{
public:
    MyModbusDriver();
    virtual bool initProperties() override;
    virtual bool Handshake() override;
    
private:
    // Modbus client instance
    nmbs_t nmbs;
    
    // Connection (TCP or Serial)
    Connection::TCP *tcpConnection { nullptr };
    int PortFD { -1 };
};
```

---

## Modbus TCP Communication

### Initializing TCP Connection

```cpp
bool MyModbusDriver::initProperties()
{
    INDI::DefaultDevice::initProperties();

    // Set up TCP connection for Modbus
    tcpConnection = new Connection::TCP(this);
    tcpConnection->setDefaultHost("192.168.1.100");
    tcpConnection->setDefaultPort(502);  // Standard Modbus TCP port
    tcpConnection->registerHandshake([&]()
    {
        return Handshake();
    });

    registerConnection(tcpConnection);

    return true;
}
```

### Creating Modbus Client

```cpp
bool MyModbusDriver::Handshake()
{
    PortFD = tcpConnection->getPortFD();

    // Configure platform for TCP
    nmbs_platform_conf platform_conf;
    platform_conf.transport = NMBS_TRANSPORT_TCP;
    platform_conf.read = read_fd_linux;
    platform_conf.write = write_fd_linux;
    platform_conf.arg = &PortFD;

    // Create the modbus client
    nmbs_error err = nmbs_client_create(&nmbs, &platform_conf);
    if (err != NMBS_ERROR_NONE)
    {
        LOGF_ERROR("Error creating modbus client: %d", err);
        return false;
    }

    // Set response timeout (milliseconds)
    nmbs_set_read_timeout(&nmbs, 1000);

    // Verify connection with a test read
    uint16_t test_register;
    err = nmbs_read_holding_registers(&nmbs, 0, 1, &test_register);
    if (err != NMBS_ERROR_NONE)
    {
        LOGF_ERROR("Failed to communicate with device: %s", nmbs_strerror(err));
        return false;
    }

    LOG_INFO("Modbus connection established");
    return true;
}
```

---

## Modbus RTU Communication

### Initializing Serial Connection

```cpp
bool MyModbusDriver::initProperties()
{
    INDI::DefaultDevice::initProperties();

    // Set up serial connection for Modbus RTU
    serialConnection = new Connection::Serial(this);
    serialConnection->setDefaultBaudRate(Connection::Serial::B_9600);
    serialConnection->registerHandshake([&]()
    {
        return Handshake();
    });

    registerConnection(serialConnection);

    return true;
}
```

### Creating Modbus RTU Client

```cpp
bool MyModbusDriver::Handshake()
{
    PortFD = serialConnection->getPortFD();

    // Configure platform for RTU
    nmbs_platform_conf platform_conf;
    platform_conf.transport = NMBS_TRANSPORT_RTU;
    platform_conf.read = read_fd_linux;
    platform_conf.write = write_fd_linux;
    platform_conf.arg = &PortFD;

    // Create the modbus client
    nmbs_error err = nmbs_client_create(&nmbs, &platform_conf);
    if (err != NMBS_ERROR_NONE)
    {
        LOGF_ERROR("Error creating modbus client: %d", err);
        return false;
    }

    // Set read timeout and byte timeout for RTU
    nmbs_set_read_timeout(&nmbs, 1000);
    nmbs_set_byte_timeout(&nmbs, 100);

    // Set destination address (slave ID)
    nmbs_set_destination_rtu_address(&nmbs, 1);

    LOG_INFO("Modbus RTU connection established");
    return true;
}
```

---

## Reading Data

### Reading Coils (Digital Outputs)

Coils are 1-bit read/write values, typically used for relay outputs or digital control signals.

```cpp
bool MyModbusDriver::ReadRelayStates()
{
    // Buffer to hold coil states (bits packed into bytes)
    nmbs_bitfield coils = {0};

    // Read 8 coils starting at address 0
    nmbs_error err = nmbs_read_coils(&nmbs, 0, 8, coils);
    
    if (err != NMBS_ERROR_NONE)
    {
        LOGF_ERROR("Error reading coils: %s", nmbs_strerror(err));
        return false;
    }

    // Extract individual coil states
    for (int i = 0; i < 8; i++)
    {
        bool state = nmbs_bitfield_read(coils, i);
        LOGF_DEBUG("Coil %d: %s", i, state ? "ON" : "OFF");
        
        // Update INDI properties
        RelayStatesSP[i].reset();
        RelayStatesSP[i][state ? 0 : 1].setState(ISS_ON);
        RelayStatesSP[i].setState(IPS_OK);
        RelayStatesSP[i].apply();
    }

    return true;
}
```

### Reading Discrete Inputs (Digital Inputs)

Discrete inputs are 1-bit read-only values, typically used for switches, sensors, or status signals.

```cpp
bool MyModbusDriver::ReadDigitalInputs()
{
    nmbs_bitfield inputs_buffer = {0};

    // Read 8 discrete inputs starting at address 0
    nmbs_error err = nmbs_read_discrete_inputs(&nmbs, 0, 8, inputs_buffer);

    if (err != NMBS_ERROR_NONE)
    {
        LOGF_ERROR("Error reading discrete inputs: %s", nmbs_strerror(err));
        return false;
    }

    // Process input states
    for (int i = 0; i < 8; i++)
    {
        bool state = nmbs_bitfield_read(inputs_buffer, i);
        
        // Update INDI properties
        if (DigitalInputsSP[i].findOnSwitchIndex() != state)
        {
            DigitalInputsSP[i].reset();
            DigitalInputsSP[i][state].setState(ISS_ON);
            DigitalInputsSP[i].setState(IPS_OK);
            DigitalInputsSP[i].apply();
        }
    }

    return true;
}
```

### Reading Holding Registers (Read/Write 16-bit)

Holding registers store 16-bit values for configuration, setpoints, or control values.

```cpp
bool MyModbusDriver::ReadConfiguration()
{
    // Buffer for register values
    uint16_t registers[4];

    // Read 4 holding registers starting at address 0x1000
    nmbs_error err = nmbs_read_holding_registers(&nmbs, 0x1000, 4, registers);

    if (err != NMBS_ERROR_NONE)
    {
        LOGF_ERROR("Error reading holding registers: %s", nmbs_strerror(err));
        return false;
    }

    // Process register values
    int setpoint = registers[0];
    int timeout = registers[1];
    int mode = registers[2];
    int status = registers[3];

    LOGF_INFO("Setpoint: %d, Timeout: %d, Mode: %d, Status: %d",
              setpoint, timeout, mode, status);

    return true;
}
```

### Reading Input Registers (Read-only 16-bit)

Input registers are read-only 16-bit values, typically used for sensor readings.

```cpp
bool MyModbusDriver::ReadSensors()
{
    uint16_t sensor_data[3];

    // Read 3 input registers starting at address 0
    nmbs_error err = nmbs_read_input_registers(&nmbs, 0, 3, sensor_data);

    if (err != NMBS_ERROR_NONE)
    {
        LOGF_ERROR("Error reading input registers: %s", nmbs_strerror(err));
        return false;
    }

    // Convert to actual values (depends on device scaling)
    double temperature = sensor_data[0] / 10.0;  // e.g., 235 = 23.5°C
    double humidity = sensor_data[1] / 10.0;
    double pressure = sensor_data[2];

    LOGF_INFO("Temperature: %.1f°C, Humidity: %.1f%%, Pressure: %d hPa",
              temperature, humidity, pressure);

    return true;
}
```

---

## Writing Data

### Writing Single Coil

```cpp
bool MyModbusDriver::SetRelay(uint8_t relay_index, bool state)
{
    // Value: 0xFF00 for ON, 0x0000 for OFF
    uint16_t value = state ? 0xFF00 : 0x0000;

    nmbs_error err = nmbs_write_single_coil(&nmbs, relay_index, value);

    if (err != NMBS_ERROR_NONE)
    {
        LOGF_ERROR("Error writing coil %d: %s", relay_index, nmbs_strerror(err));
        return false;
    }

    LOGF_INFO("Relay %d set to %s", relay_index, state ? "ON" : "OFF");
    return true;
}
```

### Writing Multiple Coils

```cpp
bool MyModbusDriver::SetMultipleRelays(uint8_t start_address, uint8_t count, bool *states)
{
    // Pack boolean states into bitfield
    nmbs_bitfield coils = {0};
    for (int i = 0; i < count; i++)
    {
        nmbs_bitfield_write(coils, i, states[i]);
    }

    nmbs_error err = nmbs_write_multiple_coils(&nmbs, start_address, count, coils);

    if (err != NMBS_ERROR_NONE)
    {
        LOGF_ERROR("Error writing multiple coils: %s", nmbs_strerror(err));
        return false;
    }

    return true;
}
```

### Writing Single Register

```cpp
bool MyModbusDriver::SetSetpoint(uint16_t value)
{
    nmbs_error err = nmbs_write_single_register(&nmbs, 0x1000, value);

    if (err != NMBS_ERROR_NONE)
    {
        LOGF_ERROR("Error writing register: %s", nmbs_strerror(err));
        return false;
    }

    LOGF_INFO("Setpoint set to %d", value);
    return true;
}
```

### Writing Multiple Registers

```cpp
bool MyModbusDriver::WriteConfiguration(uint16_t *values, uint8_t count)
{
    nmbs_error err = nmbs_write_multiple_registers(&nmbs, 0x1000, count, values);

    if (err != NMBS_ERROR_NONE)
    {
        LOGF_ERROR("Error writing multiple registers: %s", nmbs_strerror(err));
        return false;
    }

    LOGF_INFO("Written %d configuration registers", count);
    return true;
}
```

---

## Complete Working Example

From `waveshare_modbus_relay.cpp` - Waveshare Modbus relay board driver:

### Initialization

```cpp
bool WaveshareRelay::initProperties()
{
    INDI::DefaultDevice::initProperties();

    // Initialize interfaces
    INDI::OutputInterface::initProperties("Relays", 8, "Output");
    INDI::InputInterface::initProperties("Digital Inputs", 8, 0, "Input");

    setDriverInterface(AUX_INTERFACE | OUTPUT_INTERFACE);

    // Set up TCP connection
    tcpConnection = new Connection::TCP(this);
    tcpConnection->setDefaultHost("192.168.1.1");
    tcpConnection->setDefaultPort(502);
    tcpConnection->registerHandshake([&]() { return Handshake(); });
    registerConnection(tcpConnection);

    return true;
}
```

### Handshake and Client Creation

```cpp
bool WaveshareRelay::Handshake()
{
    PortFD = tcpConnection->getPortFD();

    // Configure platform for TCP
    nmbs_platform_conf platform_conf;
    platform_conf.transport = NMBS_TRANSPORT_TCP;
    platform_conf.read = read_fd_linux;
    platform_conf.write = write_fd_linux;
    platform_conf.arg = &PortFD;

    // Create modbus client
    nmbs_error err = nmbs_client_create(&nmbs, &platform_conf);
    if (err != NMBS_ERROR_NONE)
    {
        LOGF_ERROR("Error creating modbus client: %d", err);
        return false;
    }

    nmbs_set_read_timeout(&nmbs, 1000);

    // Check for input support
    nmbs_bitfield inputs_buffer = {0};
    m_HaveInput = nmbs_read_discrete_inputs(&nmbs, 0, 8, inputs_buffer) == NMBS_ERROR_NONE;
    
    if (m_HaveInput)
    {
        setDriverInterface(AUX_INTERFACE | OUTPUT_INTERFACE | INPUT_INTERFACE);
        syncDriverInfo();
    }

    // Read firmware version
    uint16_t output;
    err = nmbs_read_holding_registers(&nmbs, 0x8000, 1, &output);
    if (err == NMBS_ERROR_NONE)
    {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << output / 100.0;
        FirmwareVersionTP[0].setText(ss.str().c_str());
        FirmwareVersionTP.setState(IPS_OK);
        return true;
    }

    return false;
}
```

### Reading Outputs (Coils)

```cpp
bool WaveshareRelay::UpdateDigitalOutputs()
{
    nmbs_bitfield coils = {0};
    auto err = nmbs_read_coils(&nmbs, 0, 8, coils);
    
    if (err != NMBS_ERROR_NONE)
    {
        LOGF_ERROR("Error reading coils: %s", nmbs_strerror(err));
        return false;
    }

    for (size_t i = 0; i < DigitalOutputsSP.size(); i++)
    {
        auto oldState = DigitalOutputsSP[i].findOnSwitchIndex();
        auto newState = nmbs_bitfield_read(coils, i);
        
        if (oldState != newState)
        {
            DigitalOutputsSP[i].reset();
            DigitalOutputsSP[i][newState].setState(ISS_ON);
            DigitalOutputsSP[i].setState(IPS_OK);
            DigitalOutputsSP[i].apply();
        }
    }
    
    return true;
}
```

### Writing Output (Coil)

```cpp
bool WaveshareRelay::CommandOutput(uint32_t index, OutputState command)
{
    uint16_t value = (command == OutputState::On) ? 0xFF00 : 0;

    auto err = nmbs_write_single_coil(&nmbs, index, value);
    if (err != NMBS_ERROR_NONE)
    {
        LOGF_ERROR("Error writing coil %d: %s", index, nmbs_strerror(err));
        return false;
    }

    return true;
}
```

### Reading Inputs (Discrete Inputs)

```cpp
bool WaveshareRelay::UpdateDigitalInputs()
{
    nmbs_bitfield inputs_buffer = {0};

    nmbs_error err = nmbs_read_discrete_inputs(&nmbs, 0, 8, inputs_buffer);

    if (err != NMBS_ERROR_NONE)
    {
        LOGF_ERROR("Error reading discrete inputs: %s", nmbs_strerror(err));
        return false;
    }

    for (int i = 0; i < 8; i++)
    {
        auto oldStateIndex = DigitalInputsSP[i].findOnSwitchIndex();
        auto newState = nmbs_bitfield_read(inputs_buffer, i);

        if (oldStateIndex != newState)
        {
            DigitalInputsSP[i].reset();
            DigitalInputsSP[i][newState].setState(ISS_ON);
            DigitalInputsSP[i].setState(IPS_OK);
            DigitalInputsSP[i].apply();
        }
    }

    return true;
}
```

---

## Error Handling

### Checking for Errors

Always check return values and use `nmbs_strerror()` for meaningful error messages:

```cpp
nmbs_error err = nmbs_read_coils(&nmbs, 0, 8, coils);

if (err != NMBS_ERROR_NONE)
{
    LOGF_ERROR("Modbus error: %s", nmbs_strerror(err));
    
    // Check if it's a Modbus exception
    if (nmbs_error_is_exception(err))
    {
        LOGF_ERROR("Modbus exception code: %d", err);
    }
    
    return false;
}
```

### Common Error Codes

```cpp
// Timeout errors
if (err == NMBS_ERROR_TIMEOUT)
{
    LOG_ERROR("Communication timeout - check connection");
}

// Invalid parameters
if (err == NMBS_ERROR_INVALID_ARGUMENT)
{
    LOG_ERROR("Invalid address or quantity");
}

// Invalid response
if (err == NMBS_ERROR_INVALID_RESPONSE)
{
    LOG_ERROR("Device sent invalid response");
}
```

---

## Best Practices

### 1. Set Appropriate Timeouts

```cpp
// TCP: typically 1-3 seconds
nmbs_set_read_timeout(&nmbs, 1000);

// RTU: add byte timeout for serial
nmbs_set_read_timeout(&nmbs, 1000);
nmbs_set_byte_timeout(&nmbs, 100);
```

### 2. Handle Device-Specific Addressing

Many Modbus devices use non-standard address offsets:

```cpp
// Some devices start coils at address 1, not 0
const int COIL_OFFSET = 1;

// Adjust addresses when reading/writing
nmbs_write_single_coil(&nmbs, relay_index + COIL_OFFSET, value);
```

### 3. Batch Operations When Possible

Reading/writing multiple registers in one operation is more efficient:

```cpp
// Less efficient: Multiple single reads
for (int i = 0; i < 10; i++)
{
    uint16_t value;
    nmbs_read_holding_registers(&nmbs, i, 1, &value);
}

// More efficient: Single batch read
uint16_t values[10];
nmbs_read_holding_registers(&nmbs, 0, 10, values);
```

### 4. Validate Register Ranges

```cpp
bool MyDriver::ValidateAddress(uint16_t address, uint16_t quantity)
{
    // Check device-specific limits
    const uint16_t MAX_COILS = 64;
    
    if (address + quantity > MAX_COILS)
    {
        LOGF_ERROR("Address range exceeds device limits: %d-%d",
                   address, address + quantity - 1);
        return false;
    }
    
    return true;
}
```

### 5. Implement Retry Logic

```cpp
bool MyDriver::ReadWithRetry(uint16_t address, uint16_t count, uint16_t *buffer)
{
    const int MAX_RETRIES = 3;
    
    for (int retry = 0; retry < MAX_RETRIES; retry++)
    {
        nmbs_error err = nmbs_read_holding_registers(&nmbs, address, count, buffer);
        
        if (err == NMBS_ERROR_NONE)
            return true;
            
        if (retry < MAX_RETRIES - 1)
        {
            LOGF_WARN("Read failed, retrying (%d/%d)...", retry + 1, MAX_RETRIES);
            usleep(100000);  // 100ms delay
        }
    }
    
    LOG_ERROR("Read failed after maximum retries");
    return false;
}
```

### 6. Use Polling for Continuous Monitoring

```cpp
void MyDriver::TimerHit()
{
    if (!isConnected())
        return;

    // Update all data points
    UpdateDigitalInputs();
    UpdateDigitalOutputs();
    UpdateSensorReadings();

    // Schedule next update
    SetTimer(getCurrentPollingPeriod());
}
```

### 7. Handle Byte Order (Endianness)

Some devices may require byte swapping:

```cpp
// Swap bytes if needed (depends on device)
uint16_t SwapBytes(uint16_t value)
{
    return ((value & 0xFF00) >> 8) | ((value & 0x00FF) << 8);
}

// For 32-bit values from two 16-bit registers
uint32_t CombineRegisters(uint16_t high, uint16_t low)
{
    return ((uint32_t)high << 16) | low;
}
```

---

## CMake Configuration

### Required Setup

Modbus support is built into INDI, so no external dependencies are needed:

```cmake
cmake_minimum_required(VERSION 3.16)
PROJECT(indi_modbus_device CXX)

find_package(INDI REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${INDI_INCLUDE_DIR})

add_executable(indi_modbus_device
    modbus_device.cpp
    modbus_device.h
)

target_link_libraries(indi_modbus_device
    ${INDI_LIBRARIES}
)

install(TARGETS indi_modbus_device RUNTIME DESTINATION bin)
```

---

## Debugging Tips

### Enable Modbus Logging

```cpp
// Log all Modbus communication (if supported by library version)
#ifdef NMBS_DEBUG
nmbs_set_debug_level(&nmbs, NMBS_DEBUG_ALL);
#endif
```

### Verify Device Communication

Test basic connectivity before complex operations:

```cpp
bool MyDriver::TestConnection()
{
    // Try reading a known register
    uint16_t test_value;
    nmbs_error err = nmbs_read_holding_registers(&nmbs, 0, 1, &test_value);
    
    if (err == NMBS_ERROR_NONE)
    {
        LOGF_INFO("Connection OK, test register value: 0x%04X", test_value);
        return true;
    }
    
    LOGF_ERROR("Connection test failed: %s", nmbs_strerror(err));
    return false;
}
```

### Monitor Communication

Use INDI logging to track Modbus operations:

```cpp
// Before operation
LOGF_DEBUG("Reading %d coils from address %d", count, address);

// After operation
if (err == NMBS_ERROR_NONE)
{
    LOGF_DEBUG("Successfully read coils");
}
else
{
    LOGF_DEBUG("Failed to read coils: %s", nmbs_strerror(err));
}
```

---

## Modbus TCP vs RTU Comparison

| Feature | Modbus TCP | Modbus RTU |
|---------|-----------|------------|
| **Transport** | Ethernet | RS-232/RS-485 |
| **Connection** | Connection::TCP | Connection::Serial |
| **Address** | IP:Port | Slave ID (1-247) |
| **CRC** | Not needed | Automatic |
| **Speed** | Fast (100Mbps+) | Slower (9600-115200 baud) |
| **Distance** | 100m+ (with switches) | Up to 1200m (RS-485) |
| **Setup** | NMBS_TRANSPORT_TCP | NMBS_TRANSPORT_RTU |
| **Byte Timeout** | Not used | Required |

---

## Additional Resources

- **nanomodbus Documentation**: Included in INDI source tree at `libs/modbus/`
- **Modbus Specification**: [modbus.org](https://modbus.org/)
- **INDI API Documentation**: [https://www.indilib.org/api/](https://www.indilib.org/api/)
- **Modbus Calculator**: Online tools for address conversion and register mapping

## Support

For questions and support:
- [INDI Forum](https://indilib.org/forum.html)
- [INDI GitHub Issues](https://github.com/indilib/indi/issues)
- [Driver Development Guide](/drivers/)
