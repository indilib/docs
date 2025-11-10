---
title: Web Protocols (HTTP/REST/JSON)
parent: Hardware Communications
nav_order: 8
permalink: /drivers/communications/web-protocols/
---

# Web Protocols: HTTP, REST, and JSON

This guide covers implementing HTTP clients and servers in INDI drivers for communicating with modern web-based devices, REST APIs, and cloud services. Unlike the TCP connection plugin which handles raw binary/text protocols, web protocols provide high-level HTTP communication with built-in JSON serialization.

## Table of Contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Overview

### When to Use Web Protocols

Use HTTP/REST/JSON when your device:
- Provides a REST API or web service interface
- Communicates using HTTP requests and JSON responses
- Is a cloud-based service or networked device
- Uses modern IoT protocols (HTTP-based)
- Requires structured data exchange (JSON, XML)

### When to Use TCP Plugin Instead

Use the [TCP Connection Plugin](tcp/) for:
- Raw text-based protocols (command/response)
- Binary protocols over TCP
- Custom protocol implementations
- Legacy equipment with proprietary protocols

### Libraries Used

**cpp-httplib**: A header-only C++ HTTP client and server library
- Simple, lightweight, and easy to integrate
- Supports HTTP/1.1 client and server
- Thread-safe operations
- Built-in timeout and error handling

**nlohmann/json**: Modern JSON library for C++
- Intuitive syntax for JSON manipulation
- Type-safe conversions
- Excellent error handling
- Header-only or system library options

---

## HTTP Client Operations

### Basic Setup

HTTP clients are standalone and don't require the TCP connection plugin:

```cpp
#include <httplib.h>

#ifdef _USE_SYSTEM_JSONLIB
#include <nlohmann/json.hpp>
#else
#include <indijson.hpp>
#endif

using json = nlohmann::json;
```

### Creating HTTP Client

Create a client instance with the target host and port:

```cpp
// Simple client
httplib::Client cli("192.168.1.100", 80);

// With HTTPS
httplib::SSLClient cli("api.example.com", 443);

// Using connection info from TCP plugin (if available)
httplib::Client cli(tcpConnection->host(), tcpConnection->port());
```

### Making GET Requests

#### Simple GET Request

```cpp
auto result = cli.Get("/api/status");
if (!result)
{
    LOG_ERROR("Failed to connect to device");
    return false;
}

if (result->status == 200)
{
    LOG_INFO("Success!");
}
```

#### GET with Query Parameters

Build query strings for API endpoints:

```cpp
std::string endpoint = "/api/xdevices.json";
endpoint += "?key=" + std::string(APIKeyTP[0].getText());
endpoint += "&Get=D";

auto result = cli.Get(endpoint);
if (!result)
{
    LOG_ERROR("Failed to get data");
    return false;
}
```

#### GET with Headers

Add custom headers for authentication or content types:

```cpp
httplib::Headers headers = {
    {"Authorization", "Bearer " + std::string(tokenTP[0].getText())},
    {"Accept", "application/json"}
};

auto result = cli.Get("/api/data", headers);
```

### Making PUT/POST Requests

#### PUT Request

```cpp
std::string endpoint = "/api/xdevices.json";
endpoint += "?key=" + std::string(APIKeyTP[0].getText());
endpoint += "&SetR=" + std::to_string(relayNumber);

auto result = cli.Put(endpoint, "", "text/plain");
if (!result || result->status != 200)
{
    LOGF_ERROR("Failed to set relay %d", relayNumber);
    return false;
}
```

#### POST with JSON Body

```cpp
json payload;
payload["device"] = "telescope";
payload["command"] = "goto";
payload["ra"] = 10.5;
payload["dec"] = 45.2;

auto result = cli.Post("/api/command",
                       payload.dump(),
                       "application/json");
```

### Error Handling

Always check for connection errors and HTTP status codes:

```cpp
auto result = cli.Get("/api/status");

// Check if request succeeded
if (!result)
{
    LOG_ERROR("Network error or connection failed");
    return false;
}

// Check HTTP status code
if (result->status != 200)
{
    LOGF_ERROR("HTTP error: %d", result->status);
    return false;
}

// Process response
std::string body = result->body;
```

### Complete Client Example

From `ipx800v4.cpp` - Controlling an IPX800 relay board:

```cpp
bool IPX800::UpdateDigitalInputs()
{
    httplib::Client cli(tcpConnection->host(), tcpConnection->port());

    std::string endpoint = "/api/xdevices.json";
    endpoint += "?key=" + std::string(APIKeyTP[0].getText());
    endpoint += "&Get=D";
    
    auto result = cli.Get(endpoint);
    if (!result)
    {
        LOG_ERROR("Failed to get digital inputs");
        return false;
    }

    try
    {
        auto j = json::parse(result->body);

        // Parse digital inputs array
        auto inputs = j.get<std::vector<int>>();
        for (size_t i = 0; i < DIGITAL_INPUTS && i < inputs.size(); i++)
        {
            auto state = inputs[i] ? ISS_ON : ISS_OFF;
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
    catch (json::parse_error &e)
    {
        LOGF_ERROR("JSON parse error: %s", e.what());
    }

    return false;
}

bool IPX800::CommandOutput(uint32_t index, OutputState command)
{
    httplib::Client cli(tcpConnection->host(), tcpConnection->port());

    // IPX800 uses 1-based indexing
    std::string endpoint = "/api/xdevices.json";
    endpoint += "?key=" + std::string(APIKeyTP[0].getText());
    endpoint += "&" + std::string(command == OutputState::On ? "SetR=" : "ClearR=") 
                + std::to_string(index + 1);

    auto result = cli.Get(endpoint);
    if (!result)
    {
        LOGF_ERROR("Failed to set output %d", index + 1);
        return false;
    }

    return result->status == 200;
}
```

---

## HTTP Server Operations

### Basic Server Setup

Create an HTTP server to expose device functionality as a REST API:

```cpp
#include <httplib.h>
#include <thread>
#include <memory>

class MyDriver : public INDI::DefaultDevice
{
private:
    std::unique_ptr<httplib::Server> m_Server;
    std::thread m_ServerThread;
    bool m_ServerRunning = false;
};
```

### Creating and Configuring Server

```cpp
bool MyDriver::startServer()
{
    if (m_ServerRunning)
        return true;

    // Create server instance
    m_Server = std::make_unique<httplib::Server>();

    // Configure routes
    setupRoutes();

    // Start server in background thread
    m_ServerThread = std::thread(&MyDriver::serverThreadFunc, this);

    m_ServerRunning = true;
    return true;
}

void MyDriver::serverThreadFunc()
{
    std::string host = ServerSettingsTP[0].getText();
    int port = std::stoi(ServerSettingsTP[1].getText());

    LOGF_INFO("HTTP server listening on %s:%d", host.c_str(), port);

    // Blocking call - runs until stopped
    m_Server->listen(host.c_str(), port);

    LOG_INFO("HTTP server stopped");
}
```

### Defining Routes and Handlers

#### GET Endpoint

```cpp
void MyDriver::setupRoutes()
{
    // Simple GET endpoint
    m_Server->Get("/api/status", [this](const httplib::Request &req, httplib::Response &res)
    {
        json response;
        response["connected"] = isConnected();
        response["device"] = getDeviceName();
        
        res.set_content(response.dump(), "application/json");
    });

    // GET with path parameters
    m_Server->Get("/api/devices/(.*)", [this](const httplib::Request &req, httplib::Response &res)
    {
        handleDeviceRequest(req, res);
    });
}
```

#### PUT/POST Endpoints

```cpp
void MyDriver::setupRoutes()
{
    // PUT endpoint for updates
    m_Server->Put("/api/v1/telescope/(.+)/action", 
        [this](const httplib::Request &req, httplib::Response &res)
    {
        handleTelescopeAction(req, res);
    });

    // POST endpoint for commands
    m_Server->Post("/api/command", 
        [this](const httplib::Request &req, httplib::Response &res)
    {
        try
        {
            auto j = json::parse(req.body);
            std::string command = j["command"];
            
            // Process command
            json response;
            response["status"] = "ok";
            response["result"] = processCommand(command);
            
            res.set_content(response.dump(), "application/json");
        }
        catch (json::exception &e)
        {
            res.status = 400;
            json error;
            error["error"] = e.what();
            res.set_content(error.dump(), "application/json");
        }
    });
}
```

### Accessing Request Data

Extract information from HTTP requests:

```cpp
void MyDriver::handleRequest(const httplib::Request &req, httplib::Response &res)
{
    // Query parameters
    if (req.has_param("device"))
    {
        std::string device = req.get_param_value("device");
    }

    // Headers
    if (req.has_header("Authorization"))
    {
        std::string auth = req.get_header_value("Authorization");
    }

    // Path segments (from regex captures)
    // For route "/api/devices/(.*)"
    std::string deviceId = req.matches[1];

    // Request body
    std::string body = req.body;
}
```

### Stopping the Server

Properly shut down the server and thread:

```cpp
bool MyDriver::stopServer()
{
    if (!m_ServerRunning)
        return true;

    // Stop server (will unblock listen())
    if (m_Server)
    {
        m_Server->stop();

        // Wait for thread to finish
        if (m_ServerThread.joinable())
            m_ServerThread.join();

        m_Server.reset();
    }

    m_ServerRunning = false;
    return true;
}
```

### Complete Server Example

From `indi_alpaca_server.cpp` - INDI Alpaca Bridge:

```cpp
bool INDIAlpacaServer::startAlpacaServer()
{
    if (m_ServerRunning)
        return true;

    // Create HTTP server
    m_Server = std::make_unique<httplib::Server>();

    // Set up routes for Alpaca protocol
    m_Server->Get("/management/(.*)", 
        [this](const httplib::Request &req, httplib::Response &res)
    {
        m_DeviceManager->handleAlpacaRequest(req, res);
    });

    m_Server->Get("/api/v1/(.*)", 
        [this](const httplib::Request &req, httplib::Response &res)
    {
        m_DeviceManager->handleAlpacaRequest(req, res);
    });

    m_Server->Put("/api/v1/(.*)", 
        [this](const httplib::Request &req, httplib::Response &res)
    {
        m_DeviceManager->handleAlpacaRequest(req, res);
    });

    // Setup API
    m_Server->Get("/setup/v1/(.*)", 
        [this](const httplib::Request &req, httplib::Response &res)
    {
        m_DeviceManager->handleSetupRequest(req, res);
    });

    // Start server thread
    m_ServerThread = std::thread(&INDIAlpacaServer::serverThreadFunc, this);

    // Wait for server to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    m_ServerRunning = true;
    LOGF_INFO("Alpaca server started on port %s", ServerSettingsTP[1].getText());
    
    return true;
}

void INDIAlpacaServer::serverThreadFunc()
{
    std::string host = ServerSettingsTP[0].getText();
    int port = std::stoi(ServerSettingsTP[1].getText());

    LOGF_INFO("Alpaca server listening on %s:%d", host.c_str(), port);

    // Blocking call
    m_Server->listen(host.c_str(), port);

    LOG_INFO("Alpaca server thread stopped");
}
```

---

## JSON Data Handling

### Library Configuration

INDI supports both system and bundled JSON libraries:

```cpp
#ifdef _USE_SYSTEM_JSONLIB
#include <nlohmann/json.hpp>
#else
#include <indijson.hpp>
#endif

using json = nlohmann::json;
```

### Parsing JSON

#### Basic Parsing

```cpp
auto result = cli.Get("/api/data");
if (result && result->status == 200)
{
    try
    {
        auto j = json::parse(result->body);
        
        // Access simple values
        std::string product = j["product"];
        int version = j["version"];
        double temperature = j["temperature"];
    }
    catch (json::parse_error &e)
    {
        LOGF_ERROR("JSON parse error: %s", e.what());
    }
}
```

#### Nested Structures

```cpp
try
{
    auto j = json::parse(result->body);
    
    // Access nested objects
    if (j.contains("device"))
    {
        auto device = j["device"];
        std::string name = device["name"];
        std::string type = device["type"];
    }
    
    // Access arrays
    if (j.contains("sensors"))
    {
        auto sensors = j["sensors"];
        for (const auto &sensor : sensors)
        {
            std::string id = sensor["id"];
            double value = sensor["value"];
        }
    }
}
catch (json::exception &e)
{
    LOGF_ERROR("JSON error: %s (id: %d)", e.what(), e.id);
}
```

#### Type-Safe Extraction

```cpp
// Direct conversion to vector
auto inputs = j.get<std::vector<int>>();

// With type checking
if (j.is_array())
{
    auto inputs = j.get<std::vector<int>>();
}

// Check before access
if (j.contains("temperature") && j["temperature"].is_number())
{
    double temp = j["temperature"].get<double>();
}
```

### Creating JSON

#### Building Objects

```cpp
json j;
j["device"] = "telescope";
j["connected"] = true;
j["ra"] = 10.5;
j["dec"] = 45.2;

// Nested objects
j["location"]["latitude"] = 30.0;
j["location"]["longitude"] = -120.0;

// Convert to string
std::string payload = j.dump();

// Pretty print with indentation
std::string pretty = j.dump(4);
```

#### Building Arrays

```cpp
json j;
j["sensors"] = json::array();

for (int i = 0; i < numSensors; i++)
{
    json sensor;
    sensor["id"] = i;
    sensor["value"] = getSensorValue(i);
    sensor["unit"] = "celsius";
    
    j["sensors"].push_back(sensor);
}
```

### Error Handling

Always wrap JSON operations in try-catch blocks:

```cpp
try
{
    auto j = json::parse(response);
    
    // Safe access with checks
    if (j.contains("status") && j["status"].is_string())
    {
        std::string status = j["status"];
    }
    else
    {
        LOG_ERROR("Missing or invalid status field");
    }
}
catch (json::parse_error &e)
{
    LOGF_ERROR("JSON parse error: %s at byte %zu", e.what(), e.byte);
}
catch (json::type_error &e)
{
    LOGF_ERROR("JSON type error: %s", e.what());
}
catch (json::exception &e)
{
    LOGF_ERROR("JSON error: %s (id: %d)", e.what(), e.id);
}
```

### Complete JSON Example

From `alto.cpp` - Processing device status with JSON:

```cpp
void ALTO::TimerHit()
{
    // Check position updates
    if (PositionNP.getState() == IPS_BUSY)
    {
        uint8_t newPosition = PositionNP[0].value;
        try
        {
            m_ALTO->getPosition(newPosition);
        }
        catch (json::exception &e)
        {
            LOGF_ERROR("%s %d", e.what(), e.id);
        }

        if (newPosition == m_TargetPosition)
        {
            PositionNP[0].setValue(m_TargetPosition);
            PositionNP.setState(IPS_OK);
            PositionNP.apply();
        }
        else if (newPosition != PositionNP[0].getValue())
        {
            PositionNP[0].setValue(newPosition);
            PositionNP.apply();
        }
    }

    // Check motor status
    if (ParkCapSP.getState() == IPS_BUSY)
    {
        json status;
        try
        {
            m_ALTO->getStatus(status);
            std::string mst = status["MST"];
            if (mst == "stop")
            {
                ParkCapSP.setState(IPS_OK);
                ParkCapSP.apply();
            }
        }
        catch (json::exception &e)
        {
            LOGF_ERROR("%s %d", e.what(), e.id);
        }
    }

    SetTimer(getCurrentPollingPeriod());
}
```

---

## CMake Configuration

### Required Dependencies

Add cpp-httplib and nlohmann/json to your CMakeLists.txt:

```cmake
# Find httplib
find_package(httplib REQUIRED)

# Option for JSON library
option(USE_SYSTEM_JSONLIB "Use system JSON library" ON)

if(USE_SYSTEM_JSONLIB)
    find_package(nlohmann_json REQUIRED)
    add_definitions(-D_USE_SYSTEM_JSONLIB)
else()
    # Use bundled indijson.hpp
endif()
```

### Linking Libraries

```cmake
target_link_libraries(indi_mydriver
    ${INDI_LIBRARIES}
    httplib::httplib
)

if(USE_SYSTEM_JSONLIB)
    target_link_libraries(indi_mydriver nlohmann_json::nlohmann_json)
endif()
```

### Complete CMake Example

```cmake
cmake_minimum_required(VERSION 3.16)
PROJECT(indi_webdevice CXX)

find_package(INDI REQUIRED)
find_package(httplib REQUIRED)

option(USE_SYSTEM_JSONLIB "Use system JSON library" ON)

if(USE_SYSTEM_JSONLIB)
    find_package(nlohmann_json REQUIRED)
    add_definitions(-D_USE_SYSTEM_JSONLIB)
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${INDI_INCLUDE_DIR})

add_executable(indi_webdevice
    webdevice.cpp
    webdevice.h
)

target_link_libraries(indi_webdevice
    ${INDI_LIBRARIES}
    httplib::httplib
)

if(USE_SYSTEM_JSONLIB)
    target_link_libraries(indi_webdevice nlohmann_json::nlohmann_json)
endif()

install(TARGETS indi_webdevice RUNTIME DESTINATION bin)
```

---

## Best Practices

### 1. Error Handling

Always check both network and application errors:

```cpp
auto result = cli.Get("/api/data");

// Check network/connection error
if (!result)
{
    LOG_ERROR("Network error - check connection");
    return false;
}

// Check HTTP status
if (result->status != 200)
{
    LOGF_ERROR("HTTP %d error", result->status);
    return false;
}

// Check JSON parsing
try
{
    auto j = json::parse(result->body);
    // Process data
}
catch (json::exception &e)
{
    LOGF_ERROR("JSON error: %s", e.what());
    return false;
}
```

### 2. Thread Safety

HTTP servers run in separate threads - ensure thread-safe access to shared data:

```cpp
class MyDriver : public INDI::DefaultDevice
{
private:
    std::mutex m_DataMutex;
    std::map<std::string, double> m_DeviceData;

    void handleRequest(const httplib::Request &req, httplib::Response &res)
    {
        // Lock before accessing shared data
        std::lock_guard<std::mutex> lock(m_DataMutex);
        
        json response;
        for (const auto &[key, value] : m_DeviceData)
        {
            response[key] = value;
        }
        
        res.set_content(response.dump(), "application/json");
    }
};
```

### 3. Resource Management

Properly manage server lifecycle:

```cpp
// In destructor
MyDriver::~MyDriver()
{
    if (m_ServerRunning)
        stopServer();
}

// On disconnect
bool MyDriver::Disconnect()
{
    if (m_ServerRunning)
        stopServer();
        
    return INDI::DefaultDevice::Disconnect();
}
```

### 4. Timeout Configuration

Set appropriate timeouts for network operations:

```cpp
httplib::Client cli("192.168.1.100", 80);

// Set connection timeout (seconds)
cli.set_connection_timeout(5, 0);

// Set read timeout (seconds)
cli.set_read_timeout(10, 0);

// Set write timeout (seconds)
cli.set_write_timeout(10, 0);
```

### 5. API Authentication

Store credentials securely:

```cpp
// In driver properties
APIKeyTP[0].fill("API_KEY", "API Key", "");
APIKeyTP.fill(getDeviceName(), "API_KEY", "Authentication", 
              MAIN_CONTROL_TAB, IP_RW, 60, IPS_IDLE);

// Save encrypted/protected
APIKeyTP.load();

// Use in requests
std::string endpoint = "/api/data?key=" + std::string(APIKeyTP[0].getText());
```

### 6. Connection Reuse

Reuse client instances when making multiple requests:

```cpp
void MyDriver::updateAllData()
{
    // Create client once
    httplib::Client cli(m_Host, m_Port);
    
    // Reuse for multiple requests
    updateTemperature(cli);
    updateHumidity(cli);
    updatePressure(cli);
}

bool MyDriver::updateTemperature(httplib::Client &cli)
{
    auto result = cli.Get("/api/temperature");
    // Process result
}
```

### 7. Validation

Validate JSON structure before accessing:

```cpp
bool validateResponse(const json &j)
{
    if (!j.contains("status"))
    {
        LOG_ERROR("Missing status field");
        return false;
    }
    
    if (!j["status"].is_string())
    {
        LOG_ERROR("Status field is not a string");
        return false;
    }
    
    if (!j.contains("data") || !j["data"].is_object())
    {
        LOG_ERROR("Missing or invalid data field");
        return false;
    }
    
    return true;
}
```

---

## Comparison: Web Protocols vs TCP Plugin

| Feature | Web Protocols | TCP Connection Plugin |
|---------|---------------|----------------------|
| **Protocol** | HTTP/HTTPS | Raw TCP |
| **Data Format** | JSON, XML, HTML | Binary or text |
| **Connection Management** | Built into httplib | Handled by plugin |
| **Use Case** | REST APIs, web services | Custom protocols |
| **Setup Complexity** | Simple | Moderate |
| **User Configuration** | Minimal | Connection settings UI |
| **Authentication** | API keys, OAuth, tokens | Protocol-specific |
| **Error Handling** | HTTP status codes | Protocol-specific |

### When to Choose What

**Choose Web Protocols when:**
- Device has a documented REST API
- Communication uses HTTP and JSON
- Device is cloud-based or web-enabled
- Modern IoT or smart device
- Authentication via API keys/tokens

**Choose TCP Plugin when:**
- Custom binary protocol
- Legacy text-based commands
- Direct TCP socket control needed
- Proprietary communication protocol
- Need connection management UI

---

## Future Protocols

The Web Protocols guide will be expanded to cover:

- **WebSocket** - Real-time bidirectional communication
- **MQTT** - Publish/subscribe messaging for IoT
- **GraphQL** - Modern API query language
- **gRPC** - High-performance RPC framework
- **Server-Sent Events** - Server push notifications

---

## Complete Working Examples

### HTTP Client Example (IPX800 Relay Controller)

File: `drivers/auxiliary/ipx800v4.cpp`

A complete HTTP client implementation for controlling an IPX800 V4 relay board via its REST API.

**Key Features:**
- REST API calls with query parameters
- API key authentication
- JSON response parsing
- Input/output device control
- Polling updates

### HTTP Server Example (INDI Alpaca Bridge)

File: `drivers/alpaca/indi_alpaca_server.cpp`

A complete HTTP server implementation that bridges INDI to ASCOM Alpaca protocol.

**Key Features:**
- Multi-route HTTP server
- Background thread execution
- Request routing and handling
- REST API implementation
- Device management

### JSON Handling Example (ALTO Cover)

File: `drivers/auxiliary/alto.cpp`

Complex JSON parsing for device status and control.

**Key Features:**
- Nested JSON parsing
- Error handling with try-catch
- Status monitoring
- Type-safe extraction
- Multiple data types

---

## Additional Resources

- **cpp-httplib Documentation**: [https://github.com/yhirose/cpp-httplib](https://github.com/yhirose/cpp-httplib)
- **nlohmann/json Documentation**: [https://json.nlohmann.me/](https://json.nlohmann.me/)
- **INDI API Documentation**: [https://www.indilib.org/api/](https://www.indilib.org/api/)
- **REST API Best Practices**: [https://restfulapi.net/](https://restfulapi.net/)

## Support

For questions and support:
- [INDI Forum](https://indilib.org/forum.html)
- [INDI GitHub Issues](https://github.com/indilib/indi/issues)
- [Driver Development Guide](/drivers/)
