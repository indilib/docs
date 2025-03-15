---
sort: 6
---

# INDI Driver Development Best Practices

This guide provides a comprehensive set of best practices for developing INDI drivers. Following these guidelines will help you create robust, maintainable, and user-friendly drivers that work well with the INDI ecosystem.

## Code Organization

### Project Structure

Organize your driver project with a clear and consistent structure:

- **Header Files**: Place all header files in the root directory or a dedicated `include` directory.
- **Source Files**: Place all source files in the root directory or a dedicated `src` directory.
- **Build Files**: Place CMake files in the root directory.
- **Documentation**: Place documentation files in a dedicated `doc` directory.
- **Examples**: Place example code and configuration files in a dedicated `examples` directory.
- **Tests**: Place test code in a dedicated `tests` directory.

Example project structure:

```
indi-mydriver/
├── CMakeLists.txt
├── config.h.cmake
├── indi_mydriver.xml.cmake
├── README.md
├── include/
│   ├── mydriver.h
│   └── mydriver_utils.h
├── src/
│   ├── mydriver.cpp
│   ├── mydriver_utils.cpp
│   └── main.cpp
├── doc/
│   ├── manual.md
│   └── api.md
├── examples/
│   ├── config.xml
│   └── script.sh
└── tests/
    ├── test_mydriver.cpp
    └── CMakeLists.txt
```

### Class Design

Design your driver classes with clear responsibilities and interfaces:

- **Single Responsibility Principle**: Each class should have a single responsibility.
- **Interface Segregation**: Define clear interfaces for different aspects of your driver.
- **Dependency Injection**: Use dependency injection to make your code more testable.
- **Encapsulation**: Hide implementation details behind well-defined interfaces.

Example class design:

```cpp
// MyDriver.h
class MyDriver : public INDI::DefaultDevice
{
public:
    MyDriver();
    virtual ~MyDriver() = default;

    // DefaultDevice overrides
    virtual const char *getDefaultName() override;
    virtual bool initProperties() override;
    virtual bool updateProperties() override;
    virtual bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) override;
    virtual bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n) override;
    virtual bool ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n) override;
    virtual bool ISNewBLOB(const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[], char *names[], int n) override;
    virtual bool ISSnoopDevice(XMLEle *root) override;

    // Connection overrides
    virtual bool Connect() override;
    virtual bool Disconnect() override;

    // Timer callback
    virtual void TimerHit() override;

private:
    // Helper methods
    bool sendCommand(const char *cmd, char *res = nullptr, int reslen = 0);
    bool readResponse(char *res, int reslen);
    bool getStatus();
    bool setConfig(const char *config);

    // Properties
    INDI::PropertyText ConfigTP {1};

    INDI::PropertySwitch ModeSP {3};

    INDI::PropertyNumber SettingsNP {4};

    INDI::PropertyLight StatusLP {2};

    // State variables
    bool Connected = false;
    bool Configured = false;
    int Mode = 0;
    double Settings[4] = {0, 0, 0, 0};
    int Status = 0;

    // Connection
    Connection::Serial *serialConnection = nullptr;
    int PortFD = -1;
};
```

## Error Handling

### Robust Error Checking

Always check for errors and handle them appropriately:

- **Return Values**: Check return values from all function calls.
- **Error Codes**: Use error codes to indicate different types of errors.
- **Exceptions**: Use exceptions for exceptional conditions, but handle them properly.
- **Logging**: Log errors with appropriate severity levels.

Example error handling:

```cpp
bool MyDriver::sendCommand(const char *cmd, char *res, int reslen)
{
    // Check if the port is open
    if (PortFD < 0)
    {
        LOG_ERROR("Serial port not open");
        return false;
    }

    // Write the command
    int nbytes_written = write(PortFD, cmd, strlen(cmd));
    if (nbytes_written < 0)
    {
        LOGF_ERROR("Error writing to device: %s", strerror(errno));
        return false;
    }

    // If no response is expected, return success
    if (res == nullptr || reslen <= 0)
        return true;

    // Read the response
    if (!readResponse(res, reslen))
    {
        LOG_ERROR("Error reading response from device");
        return false;
    }

    return true;
}
```

### Graceful Degradation

Design your driver to degrade gracefully when errors occur:

- **Partial Functionality**: If some features are unavailable, continue with the available ones.
- **Retry Mechanisms**: Implement retry mechanisms for transient errors.
- **Fallback Options**: Provide fallback options when the preferred approach fails.
- **User Feedback**: Inform the user about errors and their implications.

Example graceful degradation:

```cpp
bool MyDriver::Connect()
{
    // Call the parent's Connect method
    bool result = INDI::DefaultDevice::Connect();

    if (result)
    {
        // Get the file descriptor for the serial port
        PortFD = serialConnection->getPortFD();

        // Try to get the device status
        if (!getStatus())
        {
            // If we can't get the status, we can still continue with limited functionality
            LOG_WARN("Could not get device status, some features may be unavailable");
            Status = 0;  // Assume a default status
        }

        // Try to configure the device
        if (!setConfig(ConfigT[0].text))
        {
            // If we can't configure the device, we can still continue with default settings
            LOG_WARN("Could not configure device, using default settings");
            Configured = false;
        }
        else
        {
            Configured = true;
        }

        // Set the connection status
        Connected = true;

        // Start the timer
        SetTimer(POLLMS);

        LOG_INFO("Device connected successfully");
    }

    return result;
}
```

## Resource Management

### Memory Management

Manage memory carefully to avoid leaks and corruption:

- **RAII**: Use Resource Acquisition Is Initialization (RAII) to manage resources.
- **Smart Pointers**: Use smart pointers instead of raw pointers.
- **Memory Allocation**: Be careful with memory allocation and deallocation.
- **Buffer Overflows**: Prevent buffer overflows by checking bounds.

Example memory management:

```cpp
// Use smart pointers for dynamic memory
std::unique_ptr<uint8_t[]> buffer(new uint8_t[size]);

// Use RAII for file handling
{
    std::ifstream file(filename, std::ios::binary);
    if (file)
    {
        // File will be automatically closed when the scope ends
        file.read(reinterpret_cast<char *>(buffer.get()), size);
    }
}

// Prevent buffer overflows
void MyDriver::readResponse(char *res, int reslen)
{
    // Ensure we don't overflow the buffer
    int nbytes_read = read(PortFD, res, reslen - 1);
    if (nbytes_read >= 0)
    {
        // Null-terminate the string
        res[nbytes_read] = '\0';
    }
}
```

### File Descriptors

Manage file descriptors carefully to avoid leaks:

- **Open/Close**: Always close file descriptors that you open.
- **Error Checking**: Check for errors when opening and closing file descriptors.
- **Resource Limits**: Be aware of resource limits on the number of open file descriptors.

Example file descriptor management:

```cpp
bool MyDriver::Connect()
{
    // Open the serial port
    PortFD = open(serialConnection->port(), O_RDWR | O_NOCTTY);
    if (PortFD < 0)
    {
        LOGF_ERROR("Error opening serial port %s: %s", serialConnection->port(), strerror(errno));
        return false;
    }

    // Configure the serial port
    // ...

    return true;
}

bool MyDriver::Disconnect()
{
    // Close the serial port
    if (PortFD >= 0)
    {
        close(PortFD);
        PortFD = -1;
    }

    return true;
}
```

### Threads

Use threads carefully to avoid race conditions and deadlocks:

- **Thread Safety**: Ensure that shared resources are accessed in a thread-safe manner.
- **Synchronization**: Use appropriate synchronization mechanisms (mutexes, condition variables, etc.).
- **Thread Pools**: Consider using thread pools for better resource management.
- **Thread Lifecycle**: Manage thread lifecycle carefully (creation, execution, termination).

Example thread management:

```cpp
class MyDriver : public INDI::DefaultDevice
{
public:
    // ...

private:
    // Thread-related members
    std::thread workerThread;
    std::mutex mutex;
    std::condition_variable cv;
    bool terminateThread = false;
    bool dataReady = false;
    std::vector<uint8_t> sharedData;

    // Thread function
    void workerFunction();
};

void MyDriver::workerFunction()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [this] { return dataReady || terminateThread; });

        if (terminateThread)
            break;

        // Process the shared data
        processData(sharedData);

        // Reset the flag
        dataReady = false;
    }
}

bool MyDriver::Connect()
{
    // ...

    // Start the worker thread
    terminateThread = false;
    workerThread = std::thread(&MyDriver::workerFunction, this);

    // ...
}

bool MyDriver::Disconnect()
{
    // ...

    // Terminate the worker thread
    {
        std::lock_guard<std::mutex> lock(mutex);
        terminateThread = true;
    }
    cv.notify_one();
    if (workerThread.joinable())
        workerThread.join();

    // ...
}

void MyDriver::newData(const std::vector<uint8_t> &data)
{
    {
        std::lock_guard<std::mutex> lock(mutex);
        sharedData = data;
        dataReady = true;
    }
    cv.notify_one();
}
```

## Performance Optimization

### Efficient Algorithms

Use efficient algorithms and data structures:

- **Time Complexity**: Choose algorithms with appropriate time complexity.
- **Space Complexity**: Be mindful of memory usage.
- **Data Structures**: Choose appropriate data structures for your use case.
- **Algorithmic Optimizations**: Apply algorithmic optimizations where appropriate.

Example efficient algorithm:

```cpp
// Efficient string parsing using string_view (C++17)
std::vector<std::string_view> split(std::string_view str, char delimiter)
{
    std::vector<std::string_view> result;
    size_t start = 0;
    size_t end = str.find(delimiter);

    while (end != std::string_view::npos)
    {
        result.push_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delimiter, start);
    }

    result.push_back(str.substr(start));
    return result;
}
```

### Minimizing I/O

Minimize I/O operations to improve performance:

- **Buffering**: Use buffering to reduce the number of I/O operations.
- **Batching**: Batch I/O operations where possible.
- **Asynchronous I/O**: Consider using asynchronous I/O for better performance.
- **Memory-Mapped I/O**: Consider using memory-mapped I/O for large files.

Example I/O optimization:

```cpp
// Use a buffer to reduce the number of I/O operations
class BufferedReader
{
public:
    BufferedReader(int fd, size_t bufferSize = 4096)
        : fd(fd), bufferSize(bufferSize), buffer(new char[bufferSize]), bufferPos(0), bufferEnd(0)
    {
    }

    ~BufferedReader()
    {
        delete[] buffer;
    }

    ssize_t read(void *data, size_t size)
    {
        char *dest = static_cast<char *>(data);
        size_t remaining = size;
        size_t totalRead = 0;

        while (remaining > 0)
        {
            // If the buffer is empty, refill it
            if (bufferPos >= bufferEnd)
            {
                bufferPos = 0;
                bufferEnd = ::read(fd, buffer, bufferSize);
                if (bufferEnd <= 0)
                    break;  // EOF or error
            }

            // Copy data from the buffer
            size_t toCopy = std::min(remaining, static_cast<size_t>(bufferEnd - bufferPos));
            memcpy(dest + totalRead, buffer + bufferPos, toCopy);
            bufferPos += toCopy;
            totalRead += toCopy;
            remaining -= toCopy;
        }

        return totalRead;
    }

private:
    int fd;
    size_t bufferSize;
    char *buffer;
    ssize_t bufferPos;
    ssize_t bufferEnd;
};
```

### Profiling and Optimization

Profile your code to identify and optimize bottlenecks:

- **Profiling Tools**: Use profiling tools to identify bottlenecks.
- **Benchmarking**: Benchmark critical sections of your code.
- **Optimization Techniques**: Apply appropriate optimization techniques.
- **Premature Optimization**: Avoid premature optimization.

Example profiling and optimization:

```cpp
// Simple benchmarking function
template <typename Func>
double benchmark(Func func, int iterations = 1000)
{
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i)
        func();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    return elapsed.count() / iterations;
}

// Usage
double time = benchmark([&]() {
    // Code to benchmark
    processImage(image);
});
LOGF_DEBUG("Image processing took %.6f seconds", time);
```

## Testing and Debugging

### Unit Testing

Write unit tests to verify the correctness of your code:

- **Test Framework**: Use a test framework like Google Test or Catch2.
- **Test Coverage**: Aim for high test coverage.
- **Test Isolation**: Isolate tests from each other.
- **Test Fixtures**: Use test fixtures for common setup and teardown.

Example unit test:

```cpp
// Using Google Test
#include <gtest/gtest.h>
#include "mydriver.h"

class MyDriverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Set up test environment
        driver = new MyDriver();
        driver->initProperties();
    }

    void TearDown() override
    {
        // Clean up test environment
        delete driver;
    }

    MyDriver *driver;
};

TEST_F(MyDriverTest, InitProperties)
{
    // Test that properties are initialized correctly
    ASSERT_TRUE(driver->getProperty("CONFIG"));
    ASSERT_TRUE(driver->getProperty("MODE"));
    ASSERT_TRUE(driver->getProperty("SETTINGS"));
    ASSERT_TRUE(driver->getProperty("STATUS"));
}

TEST_F(MyDriverTest, Connect)
{
    // Test connection (with mocked serial port)
    ASSERT_TRUE(driver->Connect());
    ASSERT_TRUE(driver->isConnected());
}

TEST_F(MyDriverTest, SendCommand)
{
    // Test sending a command (with mocked serial port)
    driver->Connect();
    char response[32];
    ASSERT_TRUE(driver->sendCommand("TEST", response, sizeof(response)));
    ASSERT_STREQ(response, "OK");
}
```

### Integration Testing

Write integration tests to verify that your driver works correctly with the INDI system:

- **Test Environment**: Set up a test environment that mimics the real environment.
- **Test Scenarios**: Test common usage scenarios.
- **Error Conditions**: Test error conditions and edge cases.
- **Performance Testing**: Test performance under various conditions.

Example integration test:

```cpp
// Using a custom test framework
#include "test_framework.h"
#include <indiapi.h>
#include <defaultdevice.h>
#include <indidriverinterface.h>

class INDIDriverTest : public TestCase
{
public:
    void setUp() override
    {
        // Set up the INDI driver interface
        driverInterface = new INDI::DriverInterface();
        driverInterface->addDriver("indi_mydriver");
    }

    void tearDown() override
    {
        // Clean up
        delete driverInterface;
    }

    void testDriverInitialization()
    {
        // Test that the driver initializes correctly
        ASSERT_TRUE(driverInterface->isDriverLoaded("My Driver"));
    }

    void testDriverConnection()
    {
        // Test that the driver can connect to a device
        INDI::BaseDevice *device = driverInterface->getDevice("My Driver");
        ASSERT_NOT_NULL(device);

        // Connect to the device
        ISwitchVectorProperty *connectionSP = device->getSwitch("CONNECTION");
        ASSERT_NOT_NULL(connectionSP);

        ISwitch *connectSwitch = IUFindSwitch(connectionSP, "CONNECT");
        ASSERT_NOT_NULL(connectSwitch);

        connectSwitch->s = ISS_ON;
        driverInterface->sendNewSwitch(connectionSP);

        // Wait for the connection to complete
        ASSERT_TRUE(waitForPropertyState(device, "CONNECTION", IPS_OK, 5000));
    }

private:
    INDI::DriverInterface *driverInterface;
};

// Register the tests
REGISTER_TEST(INDIDriverTest, testDriverInitialization);
REGISTER_TEST(INDIDriverTest, testDriverConnection);
```

### Debugging Techniques

Use effective debugging techniques to identify and fix issues:

- **Logging**: Use logging to track the execution flow and state.
- **Debuggers**: Use debuggers to step through code and inspect state.
- **Assertions**: Use assertions to catch programming errors.
- **Tracing**: Use tracing to follow the execution path.

Example debugging techniques:

```cpp
// Logging
LOGF_DEBUG("Processing image: %s", filename);
LOGF_DEBUG("Image dimensions: %dx%d", width, height);
LOGF_DEBUG("Processing took %.3f seconds", elapsed);

// Assertions
assert(buffer != nullptr && "Buffer should not be null");
assert(size > 0 && "Size should be positive");
assert(index < count && "Index out of bounds");

// Tracing
void MyDriver::processCommand(const char *cmd)
{
    LOGF_DEBUG("Entering processCommand: %s", cmd);

    // Process the command
    // ...

    LOG_DEBUG("Exiting processCommand");
}
```

## Documentation

### Code Documentation

Document your code thoroughly:

- **Function Documentation**: Document the purpose, parameters, return values, and exceptions of functions.
- **Class Documentation**: Document the purpose, responsibilities, and usage of classes.
- **File Documentation**: Document the purpose and contents of files.
- **Implementation Notes**: Document non-obvious implementation details.

Example code documentation:

```cpp
/**
 * @brief Send a command to the device and optionally read a response.
 *
 * This function sends a command to the device over the serial port and
 * optionally reads a response. It handles error checking and logging.
 *
 * @param cmd The command to send.
 * @param res Buffer to store the response (can be nullptr if no response is expected).
 * @param reslen Size of the response buffer.
 * @return true if the command was sent successfully and a valid response was received (if expected).
 * @return false if an error occurred.
 */
bool MyDriver::sendCommand(const char *cmd, char *res, int reslen)
{
    // Implementation
    // ...
}
```

### User Documentation

Provide comprehensive user documentation:

- **Installation Guide**: Document how to install and configure the driver.
- **User Manual**: Document how to use the driver.
- **API Reference**: Document the driver's API for developers.
- **Examples**: Provide examples of common usage scenarios.

Example user documentation:

````markdown
# My Driver User Manual

## Installation

### Prerequisites

- INDI Library (version 1.8.0 or later)
- CMake (version 3.10 or later)
- GCC (version 7.0 or later)

### Building from Source

```bash
mkdir build
cd build
cmake ..
make
sudo make install
```
````

## Configuration

The driver supports the following configuration options:

- **Port**: Serial port to use (e.g., `/dev/ttyUSB0`).
- **Baud Rate**: Serial port baud rate (default: 9600).
- **Config File**: Path to the device configuration file.

## Usage

### Basic Usage

1. Start the INDI server with the driver:

```bash
indiserver indi_mydriver
```

2. Connect to the INDI server using your favorite INDI client.

3. Connect to the device by setting the port and clicking the "Connect" button.

4. Configure the device by setting the configuration options.

5. Use the device features through the INDI client interface.

### Advanced Usage

...

````

## Compatibility and Portability

### Cross-Platform Compatibility

Make your driver compatible with different platforms:

- **Platform-Specific Code**: Isolate platform-specific code.
- **Conditional Compilation**: Use conditional compilation for platform-specific features.
- **Portable Libraries**: Use portable libraries for platform-independent functionality.
- **Feature Detection**: Use feature detection instead of platform detection.

Example cross-platform compatibility:

```cpp
// Platform-specific includes
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#endif

// Platform-specific code
bool MyDriver::sleep(int milliseconds)
{
#ifdef _WIN32
    Sleep(milliseconds);
    return true;
#else
    usleep(milliseconds * 1000);
    return true;
#endif
}

// Feature detection
#if defined(__cpp_lib_filesystem)
#include <filesystem>
namespace fs = std::filesystem;
#elif defined(__cpp_lib_experimental_filesystem)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#error "Filesystem library not available"
#endif
````

### Backward Compatibility

Maintain backward compatibility with older versions:

- **API Stability**: Avoid breaking changes to the API.
- **Deprecation Process**: Use a deprecation process for obsolete features.
- **Version Checking**: Check version compatibility at runtime.
- **Compatibility Layers**: Provide compatibility layers for older versions.

Example backward compatibility:

```cpp
// Deprecation process
[[deprecated("Use newFunction() instead")]]
void oldFunction()
{
    // Forward to the new function
    newFunction();
}

// Version checking
bool MyDriver::initProperties()
{
    // Check INDI library version
    if (INDI::getVersionMajor() < 1 || (INDI::getVersionMajor() == 1 && INDI::getVersionMinor() < 8))
    {
        LOG_ERROR("This driver requires INDI library version 1.8.0 or later");
        return false;
    }

    // Initialize properties
    // ...

    return true;
}
```

## Security

### Input Validation

Validate all input to prevent security vulnerabilities:

- **Boundary Checking**: Check array bounds to prevent buffer overflows.
- **Type Checking**: Validate input types to prevent type confusion.
- **Format String Validation**: Validate format strings to prevent format string vulnerabilities.
- **Command Injection**: Prevent command injection by validating and sanitizing input.

Example input validation:

```cpp
bool MyDriver::setConfig(const char *config)
{
    // Check for null pointer
    if (config == nullptr)
    {
        LOG_ERROR("Config is null");
        return false;
    }

    // Check for empty string
    if (*config == '\0')
    {
        LOG_ERROR("Config is empty");
        return false;
    }

    // Check for maximum length
    if (strlen(config) > MAX_CONFIG_LENGTH)
    {
        LOGF_ERROR("Config is too long (maximum length is %d)", MAX_CONFIG_LENGTH);
        return false;
    }

    // Check for valid characters
    for (const char *p = config; *p != '\0'; ++p)
    {
        if (!isalnum(*p) && *p != '_' && *p != '-' && *p != '.')
        {
            LOGF_ERROR("Config contains invalid character: %c", *p);
            return false;
        }
    }

    // Config is valid, use it
    // ...

    return true;
}
```

### Secure Communication

Implement secure communication with devices:

- **Encryption**: Use encryption for sensitive data.
- **Authentication**: Implement authentication mechanisms.
- **Secure Protocols**: Use secure protocols for communication.
- **Certificate Validation**: Validate certificates for secure connections.

Example secure communication:

```cpp
bool MyDriver::connectSecure()
{
    // Initialize OpenSSL
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    // Create SSL context
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    if (ctx == nullptr)
    {
        LOG_ERROR("Failed to create SSL context");
        return false;
    }

    // Load certificates
    if (SSL_CTX_load_verify_locations(ctx, "ca.pem", nullptr) != 1)
    {
        LOG_ERROR("Failed to load CA certificate");
        SSL_CTX_free(ctx);
        return false;
    }

    // Create SSL connection
    SSL *ssl = SSL_new(ctx);
    if (ssl == nullptr)
    {
        LOG_ERROR("Failed to create SSL connection");
        SSL_CTX_free(ctx);
        return false;
    }

    // Connect to the device
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        LOG_ERROR("Failed to create socket");
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        return false;
    }

    // Set up the address
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(host);

    // Connect to the server
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0)
    {
        LOG_ERROR("Failed to connect to server");
        close(fd);
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        return false;
    }

    // Associate the SSL connection with the socket
    SSL_set_fd(ssl, fd);

    // Perform the SSL handshake
    if (SSL_connect(ssl) != 1)
    {
        LOG_ERROR("SSL handshake failed");
        close(fd);
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        return false;
    }

    // Verify the certificate
    X509 *cert = SSL_get_peer_certificate(ssl);
    if (cert == nullptr)
    {
        LOG_ERROR("No certificate presented by the server");
        SSL_shutdown(ssl);
        close(fd);
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        return false;
    }

    // Certificate verification
    long verifyResult = SSL_get_verify_result(ssl);
    if (verifyResult != X509_V_OK)
    {
        LOGF_ERROR("Certificate verification failed: %s", X509_verify_cert_error_string(verifyResult));
        X509_free(cert);
        SSL_shutdown(ssl);
        close(fd);
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        return false;
    }

    // Certificate is valid
    X509_free(cert);

    // Store the SSL connection and context for later use
    this->ssl = ssl;
    this->ctx = ctx;
    this->fd = fd;

    return true;
}
```

## User Experience

### User Interface

Design a user-friendly interface:

- **Intuitive Controls**: Make controls intuitive and easy to use.
- **Consistent Layout**: Use a consistent layout for properties.
- **Meaningful Labels**: Use meaningful labels for properties and controls.
- **Grouping**: Group related properties together.

Example user interface:

```cpp
// Define enums for property indices to avoid magic numbers
// This is a good practice to make the code more readable and maintainable
enum
{
    CONFIG_FILE,
    CONFIG_COUNT
};

enum
{
    MODE_NORMAL,
    MODE_FAST,
    MODE_PRECISE,
    MODE_COUNT
};

enum
{
    SETTING_1,
    SETTING_2,
    SETTING_3,
    SETTING_4,
    SETTING_COUNT
};

enum
{
    STATUS_READY,
    STATUS_BUSY,
    STATUS_COUNT
};

bool MyDriver::initProperties()
{
    // Call the parent's initProperties
    INDI::DefaultDevice::initProperties();

    // Add device-specific properties

    // Connection group (already defined by DefaultDevice)

    // Configuration group
    ConfigTP[CONFIG_FILE].fill("CONFIG_FILE", "Config File", "");
    ConfigTP.fill(getDeviceName(), "CONFIG", "Configuration", MAIN_CONTROL_TAB, IP_RW, 60, IPS_IDLE);

    // Mode group
    ModeSP[MODE_NORMAL].fill("MODE_NORMAL", "Normal", ISS_ON);
    ModeSP[MODE_FAST].fill("MODE_FAST", "Fast", ISS_OFF);
    ModeSP[MODE_PRECISE].fill("MODE_PRECISE", "Precise", ISS_OFF);
    ModeSP.fill(getDeviceName(), "MODE", "Mode", MAIN_CONTROL_TAB, IP_RW, ISR_1OFMANY, 60, IPS_IDLE);

    // Settings group
    SettingsNP[SETTING_1].fill("SETTING_1", "Setting 1", "%.2f", 0, 100, 1, 50);
    SettingsNP[SETTING_2].fill("SETTING_2", "Setting 2", "%.2f", 0, 100, 1, 50);
    SettingsNP[SETTING_3].fill("SETTING_3", "Setting 3", "%.2f", 0, 100, 1, 50);
    SettingsNP[SETTING_4].fill("SETTING_4", "Setting 4", "%.2f", 0, 100, 1, 50);
    SettingsNP.fill(getDeviceName(), "SETTINGS", "Settings", MAIN_CONTROL_TAB, IP_RW, 60, IPS_IDLE);

    // Status group
    StatusLP[STATUS_READY].fill("STATUS_READY", "Ready", IPS_IDLE);
    StatusLP[STATUS_BUSY].fill("STATUS_BUSY", "Busy", IPS_IDLE);
    StatusLP.fill(getDeviceName(), "STATUS", "Status", MAIN_CONTROL_TAB, IPS_IDLE);

    // Add debug, simulation, and configuration controls
    addAuxControls();

    return true;
}
```

### Responsive Feedback

Provide responsive feedback to user actions:

- **Status Updates**: Update property states to reflect the current status.
- **Progress Indicators**: Use progress indicators for long-running operations.
- **Error Messages**: Provide clear and helpful error messages.
- **Success Messages**: Confirm successful operations.

Example responsive feedback:

```cpp
bool MyDriver::setConfig(const char *config)
{
    // Update property state to indicate that we're busy
    LOG_INFO("Setting configuration...");
    ConfigTP.setState(IPS_BUSY);
    ConfigTP.apply();

    // Try to set the configuration
    bool success = false;
    try
    {
        // Attempt to set the configuration
        success = doSetConfig(config);
    }
    catch (const std::exception &e)
    {
        // Handle exceptions
        LOGF_ERROR("Error setting configuration: %s", e.what());
        ConfigTP.setState(IPS_ALERT);
        ConfigTP.apply();
        return false;
    }

    // Update property state based on the result
    if (success)
    {
        LOG_INFO("Configuration set successfully");
        ConfigTP.setState(IPS_OK);
        ConfigTP.apply();
        return true;
    }
    else
    {
        LOG_ERROR("Failed to set configuration");
        ConfigTP.setState(IPS_ALERT);
        ConfigTP.apply();
        return false;
    }
}
```

## Logging Best Practices

### Using Log Levels Appropriately

Use the appropriate log level for each message:

- **LOG_ERROR**: Use for errors that prevent the driver from functioning correctly.
- **LOG_WARN**: Use for warnings about potential issues that don't prevent the driver from functioning.
- **LOG_INFO**: Use only for important informational messages that users should see during normal operation.
- **LOG_DEBUG**: Use for detailed debugging information that is only useful during development or troubleshooting.

Example of appropriate log level usage:

```cpp
bool MyDriver::Connect()
{
    // Log an informational message about connection attempt
    LOG_INFO("Connecting to device...");

    // Open the serial port
    PortFD = open(serialConnection->port(), O_RDWR | O_NOCTTY);
    if (PortFD < 0)
    {
        // Log an error when connection fails
        LOGF_ERROR("Error opening serial port %s: %s", serialConnection->port(), strerror(errno));
        return false;
    }

    // Log debug information about port configuration
    LOGF_DEBUG("Configuring port with baud rate %d", serialConnection->baud());

    // Configure the serial port
    struct termios tty;
    memset(&tty, 0, sizeof(tty));
    if (tcgetattr(PortFD, &tty) != 0)
    {
        // Log a warning when port configuration has issues
        LOGF_WARN("Error getting port attributes: %s", strerror(errno));
        // Continue anyway, using default attributes
    }

    // Set port attributes
    // ...

    // Log success at INFO level since it's important for users to know
    LOG_INFO("Device connected successfully");
    return true;
}
```

### Contextual Information

Include relevant contextual information in log messages:

- **Function Context**: Include the function name or context in log messages.
- **Parameter Values**: Include relevant parameter values in log messages.
- **State Information**: Include relevant state information in log messages.
- **Error Details**: Include detailed error information in log messages.

Example of contextual logging:

```cpp
bool MyDriver::sendCommand(const char *cmd, char *res, int reslen)
{
    // Include the command in debug logs
    LOGF_DEBUG("Sending command: %s", cmd);

    // Check if the port is open
    if (PortFD < 0)
    {
        LOG_ERROR("Cannot send command: Serial port not open");
        return false;
    }

    // Write the command
    int nbytes_written = write(PortFD, cmd, strlen(cmd));
    if (nbytes_written < 0)
    {
        // Include error details in error logs
        LOGF_ERROR("Error writing command '%s' to device: %s", cmd, strerror(errno));
        return false;
    }

    // If no response is expected, return success
    if (res == nullptr || reslen <= 0)
    {
        LOG_DEBUG("Command sent successfully (no response expected)");
        return true;
    }

    // Read the response
    if (!readResponse(res, reslen))
    {
        LOG_ERROR("Error reading response from device");
        return false;
    }

    // Include the response in debug logs
    LOGF_DEBUG("Received response: %s", res);

    return true;
}
```

### Avoiding Log Spam

Avoid excessive logging that could overwhelm the log or distract users:

- **Repetitive Messages**: Avoid logging the same message repeatedly.
- **High-Frequency Events**: Don't log high-frequency events at INFO level.
- **Verbose Details**: Keep verbose details at DEBUG level.
- **Rate Limiting**: Consider rate limiting for frequent log messages.

Example of avoiding log spam:

```cpp
void MyDriver::TimerHit()
{
    // Static variables to track state changes and limit logging
    static bool lastConnected = false;
    static int lastStatus = -1;
    static time_t lastTemperatureLog = 0;
    time_t now = time(nullptr);

    // Check connection status
    bool connected = isConnected();
    if (connected != lastConnected)
    {
        // Log connection state changes at INFO level
        LOGF_INFO("Device %s", connected ? "connected" : "disconnected");
        lastConnected = connected;
    }

    if (!connected)
        return;

    // Update device status
    int status = getDeviceStatus();
    if (status != lastStatus)
    {
        // Log status changes at INFO level
        LOGF_INFO("Device status changed to: %s", statusToString(status));
        lastStatus = status;
    }

    // Update temperature (log at most once per minute)
    double temperature = getDeviceTemperature();
    if (difftime(now, lastTemperatureLog) >= 60)
    {
        // Log temperature at DEBUG level since it's frequent and not critical
        LOGF_DEBUG("Device temperature: %.1f C", temperature);
        lastTemperatureLog = now;
    }

    // Set the timer for the next update
    SetTimer(POLLMS);
}
```

## Conclusion

Following these best practices will help you create robust, maintainable, and user-friendly INDI drivers. Remember that these are guidelines, not strict rules, and you should adapt them to your specific needs and constraints.

For more information, refer to the [INDI Library Documentation](https://www.indilib.org/api/index.html) and the [INDI Driver Development Guide](https://www.indilib.org/develop/developer-manual/100-driver-development.html).
