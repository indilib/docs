---
sort: 7
---

# Troubleshooting INDI Drivers

This guide provides a comprehensive approach to troubleshooting INDI drivers. It covers common issues, debugging techniques, and solutions to help you diagnose and fix problems with your INDI drivers.

## Common Issues and Solutions

### Connection Problems

#### Device Not Found

**Symptoms:**

- The driver cannot find the device
- Error messages like "Device not found" or "No such device"

**Possible Causes:**

- The device is not connected
- The device is connected to a different port
- The device is not powered on
- The device has permission issues

**Solutions:**

1. Check that the device is physically connected and powered on
2. Verify the device permissions:
   ```bash
   ls -l /dev/ttyUSB*
   ```
3. Add your user to the appropriate group (usually `dialout`):
   ```bash
   sudo usermod -a -G dialout $USER
   ```
4. Create a udev rule for your device:
   ```bash
   sudo nano /etc/udev/rules.d/99-indi.rules
   ```
   Add a rule like:
   ```
   SUBSYSTEM=="tty", ATTRS{idVendor}=="1234", ATTRS{idProduct}=="5678", MODE="0666", GROUP="dialout"
   ```
5. Reload udev rules:
   ```bash
   sudo udevadm control --reload-rules
   sudo udevadm trigger
   ```

#### Connection Timeout

**Symptoms:**

- The driver times out when trying to connect to the device
- Error messages like "Connection timed out" or "No response from device"

**Possible Causes:**

- The device is not responding
- The connection settings are incorrect
- The device is busy or in an error state

**Solutions:**

1. Check the connection settings (baud rate, parity, etc.)
2. Reset the device by power cycling it
3. Increase the connection timeout:
   ```cpp
   // In your driver's Connect method
   serialConnection->setDefaultTimeout(10); // 10 seconds
   ```
4. Check if the device is being used by another program:
   ```bash
   lsof | grep ttyUSB
   ```

### Communication Issues

#### Command Failures

**Symptoms:**

- Commands sent to the device fail
- Error messages like "Command failed" or "Invalid response"

**Possible Causes:**

- The command format is incorrect
- The device is in an unexpected state
- The response parsing is incorrect

**Solutions:**

1. Verify the command format against the device documentation
2. Add debug logging to see the exact commands and responses:
   ```cpp
   LOGF_DEBUG("Sending command: %s", cmd);
   // Send command
   LOGF_DEBUG("Received response: %s", res);
   ```
3. Reset the device to a known state before sending commands
4. Implement retry logic for transient failures:

   ```cpp
   bool MyDriver::sendCommandWithRetry(const char *cmd, char *res, int reslen, int maxRetries)
   {
       for (int retry = 0; retry < maxRetries; retry++)
       {
           if (sendCommand(cmd, res, reslen))
               return true;

           LOGF_DEBUG("Command failed, retrying (%d/%d)...", retry + 1, maxRetries);
           usleep(100000); // 100ms delay between retries
       }

       LOG_ERROR("Command failed after maximum retries");
       return false;
   }
   ```

#### Data Corruption

**Symptoms:**

- Garbled or incomplete responses
- Checksum failures
- Unexpected device behavior

**Possible Causes:**

- Electrical interference
- Incorrect baud rate or serial settings
- Buffer overflows

**Solutions:**

1. Check and adjust the serial connection settings
2. Use shielded cables and reduce cable length
3. Implement checksums or CRC for data validation:

   ```cpp
   bool MyDriver::validateResponse(const char *res, int reslen)
   {
       // Simple checksum validation
       uint8_t checksum = 0;
       for (int i = 0; i < reslen - 2; i++)
           checksum ^= res[i];

       uint8_t receivedChecksum = strtol(&res[reslen - 2], nullptr, 16);
       return checksum == receivedChecksum;
   }
   ```

4. Add flow control to your serial connection:
   ```cpp
   // In your driver's Connect method
   struct termios tty;
   memset(&tty, 0, sizeof(tty));
   tcgetattr(PortFD, &tty);
   tty.c_cflag |= CRTSCTS; // Enable hardware flow control
   tcsetattr(PortFD, TCSANOW, &tty);
   ```

### Property Issues

#### Properties Not Updating

**Symptoms:**

- Property values don't update in the client
- Changes made in the driver are not reflected in the client

**Possible Causes:**

- Missing property update calls
- Incorrect property state
- Race conditions

**Solutions:**

1. Ensure you call the appropriate update function after changing a property:

   ```cpp
   // For number properties
   SettingsNP.setState(IPS_OK);
   SettingsNP.apply();

   // For switch properties
   ModeSP.setState(IPS_OK);
   ModeSP.apply();

   // For text properties
   ConfigTP.setState(IPS_OK);
   ConfigTP.apply();

   // For light properties
   StatusLP.setState(IPS_OK);
   StatusLP.apply();
   ```

2. Check for race conditions in your driver's timer callback
3. Ensure property updates are sent from the main thread

#### Property State Inconsistencies

**Symptoms:**

- Properties show incorrect states (OK, BUSY, ALERT, IDLE)
- Properties don't reflect the actual device state

**Possible Causes:**

- Incorrect state management
- Missing state updates
- Asynchronous operations not properly handled

**Solutions:**

1. Update property states consistently:

   ```cpp
   // When starting an operation
   LOG_INFO("Changing settings...");
   SettingsNP.setState(IPS_BUSY);
   SettingsNP.apply();

   // When operation succeeds
   LOG_INFO("Settings changed successfully");
   SettingsNP.setState(IPS_OK);
   SettingsNP.apply();

   // When operation fails
   LOG_ERROR("Failed to change settings");
   SettingsNP.setState(IPS_ALERT);
   SettingsNP.apply();
   ```

2. Use the timer callback to update property states for asynchronous operations
3. Implement a state machine for complex device states

### Performance Issues

#### Slow Response

**Symptoms:**

- The driver responds slowly to user actions
- UI feels sluggish
- Operations take longer than expected

**Possible Causes:**

- Inefficient code
- Blocking I/O operations
- Too frequent polling
- Resource leaks

**Solutions:**

1. Use non-blocking I/O or separate threads for long operations
2. Optimize polling frequency:
   ```cpp
   // In your driver's constructor
   setDefaultPollingPeriod(500); // 500ms polling
   ```
3. Profile your code to identify bottlenecks
4. Implement caching for frequently accessed data
5. Use buffered I/O for better performance

#### Memory Leaks

**Symptoms:**

- Increasing memory usage over time
- Driver crashes after running for a long time
- System becomes unresponsive

**Possible Causes:**

- Unfreed memory allocations
- Resource leaks (file descriptors, etc.)
- Circular references

**Solutions:**

1. Use memory management tools like Valgrind:
   ```bash
   valgrind --leak-check=full indiserver -v indi_mydriver
   ```
2. Use smart pointers and RAII for resource management
3. Check for and fix resource leaks:

   ```cpp
   // Before
   char *buffer = new char[1024];
   // Use buffer
   // Forgot to delete buffer

   // After
   std::unique_ptr<char[]> buffer(new char[1024]);
   // Use buffer.get()
   // Automatically deleted when out of scope
   ```

4. Implement proper cleanup in your driver's destructor

## Debugging Techniques

### Using INDI Debug Logs

INDI provides a built-in logging system that can be very helpful for debugging. You can enable debug logs in your driver and in the INDI server.

#### Enabling Debug Logs in Your Driver

```cpp
// In your driver's initProperties method
addDebugControl();

// In your driver's code
LOG_DEBUG("This is a debug message");
LOGF_DEBUG("Formatted debug message: %s", value);
```

#### Running INDI Server with Debug Logs

```bash
indiserver -vvv indi_mydriver
```

The `-vvv` flag enables verbose logging. You can use fewer `v`s for less verbose logging.

#### Analyzing Debug Logs

Debug logs can help you identify:

- Command sequences
- Timing issues
- Error conditions
- State transitions

Look for patterns in the logs, such as:

- Repeated error messages
- Unexpected state changes
- Missing responses
- Timing correlations between events

### Using Ekos Debugger

Ekos, the astrophotography tool in KStars, includes a powerful debugger that can help diagnose issues with INDI drivers. The Ekos Debugger provides a graphical interface for monitoring INDI messages, property changes, and device states.

#### Enabling the Ekos Debugger

1. Open KStars and go to Ekos
2. Click on the "Debug" button in the Ekos toolbar
3. In the Debug window, select the devices you want to monitor
4. Choose the debug level (Messages, Properties, or Both)
5. Click "Start" to begin debugging

#### Using the Ekos Debugger

The Ekos Debugger provides several useful features:

1. **Message Monitoring**: View all INDI messages between the client and server
2. **Property Monitoring**: Track property changes and states
3. **Filtering**: Filter messages by device, property, or message type
4. **Logging**: Save debug logs to a file for later analysis
5. **Real-time Monitoring**: Watch property changes as they happen

#### Analyzing Ekos Debug Logs

The Ekos Debugger logs can help you identify:

1. **Communication Flow**: See the sequence of messages between client and server
2. **Property Changes**: Track how properties change over time
3. **Error Patterns**: Identify recurring errors or warnings
4. **Timing Issues**: Detect delays or timeouts in device responses
5. **Client-Server Interactions**: Understand how the client interacts with your driver

Example of using Ekos Debugger to diagnose a connection issue:

1. Start the Ekos Debugger and select your device
2. Attempt to connect to the device
3. Watch for connection messages and property changes
4. Look for error messages or unexpected property states
5. Check the timing of messages to identify delays or timeouts

### Using External Tools

#### Serial Port Monitoring

For serial devices, you can use tools like `socat` and `minicom` to monitor and test communication.

**Creating a Virtual Serial Port Pair:**

```bash
socat -d -d PTY,raw,echo=0,link=/tmp/virtualcom0 PTY,raw,echo=0,link=/tmp/virtualcom1
```

This creates two linked virtual serial ports. You can connect your driver to one and use the other for testing.

**Monitoring Serial Communication:**

```bash
socat -v /dev/ttyUSB0,raw,echo=0,b9600 STDOUT
```

This displays all data from the serial port.

**Interactive Serial Testing:**

```bash
minicom -D /dev/ttyUSB0
```

This allows you to send commands and see responses interactively.

#### Network Monitoring

For network-based devices, you can use tools like `tcpdump` and `wireshark` to monitor communication.

**Capturing Network Traffic:**

```bash
sudo tcpdump -i any host 192.168.1.100 -w capture.pcap
```

This captures all traffic to and from the specified host.

**Analyzing Network Traffic:**

```bash
wireshark capture.pcap
```

This opens the captured traffic in Wireshark for detailed analysis.

### Debugging with GDB

The GNU Debugger (GDB) is a powerful tool for debugging INDI drivers, especially for diagnosing crashes and logic issues.

#### Running INDI Server with GDB

```bash
gdb --args indiserver -v indi_mydriver
```

#### Basic GDB Commands

- `run` - Start the program
- `break MyDriver::Connect` - Set a breakpoint at the Connect method
- `continue` - Continue execution after a breakpoint
- `next` - Execute the next line of code
- `step` - Step into a function call
- `print variable` - Print the value of a variable
- `backtrace` - Show the call stack
- `quit` - Exit GDB

#### Debugging a Crash

If your driver crashes, you can use GDB to identify the cause:

1. **Run the driver under GDB**:

   ```bash
   gdb --args indiserver -v indi_mydriver
   ```

2. **Start the program**:

   ```
   (gdb) run
   ```

3. **Wait for the crash to occur**

4. **Examine the backtrace**:

   ```
   (gdb) backtrace
   ```

   This shows the call stack at the time of the crash.

5. **Examine variables**:

   ```
   (gdb) frame 2  # Switch to frame 2 in the backtrace
   (gdb) print variable  # Print the value of a variable
   ```

6. **Check the crash location**:
   ```
   (gdb) list  # Show the source code around the crash
   ```

#### Advanced GDB Techniques

1. **Conditional Breakpoints**:

   ```
   (gdb) break MyDriver::sendCommand if strlen(cmd) > 10
   ```

   This sets a breakpoint that only triggers when the condition is met.

2. **Watchpoints**:

   ```
   (gdb) watch *pointer  # Break when the value at pointer changes
   ```

   This breaks execution when a variable changes.

3. **Catching Exceptions**:

   ```
   (gdb) catch throw
   ```

   This breaks execution when an exception is thrown.

4. **Examining Memory**:

   ```
   (gdb) x/10x pointer  # Examine 10 hex values starting at pointer
   ```

   This displays memory contents.

5. **Attaching to a Running Process**:
   ```bash
   ps aux | grep indi  # Find the process ID
   gdb -p <process_id>  # Attach to the process
   ```
   This attaches GDB to an already running process.

#### Creating a Core Dump

If you can't run GDB directly, you can enable core dumps and analyze them later:

1. **Enable core dumps**:

   ```bash
   ulimit -c unlimited
   ```

2. **Run the program until it crashes**:

   ```bash
   indiserver -v indi_mydriver
   ```

3. **Analyze the core dump**:
   ```bash
   gdb /usr/bin/indiserver core
   ```

#### Using GDB with Ekos

You can also debug INDI drivers when they're running through Ekos:

1. **Find the indiserver process**:

   ```bash
   ps aux | grep indiserver
   ```

2. **Attach GDB to the process**:

   ```bash
   sudo gdb -p <process_id>
   ```

3. **Set breakpoints and debug as needed**:

   ```
   (gdb) break MyDriver::Connect
   (gdb) continue
   ```

4. **Detach when finished**:
   ```
   (gdb) detach
   (gdb) quit
   ```

#### Common Crash Causes and GDB Diagnosis

1. **Null Pointer Dereference**:

   ```
   Program received signal SIGSEGV, Segmentation fault.
   0x00007ffff7b4a3e5 in MyDriver::sendCommand(char const*, char*, int) at mydriver.cpp:245
   245     strcpy(buffer, cmd);
   (gdb) print buffer
   $1 = 0x0
   ```

   Diagnosis: `buffer` is a null pointer.

2. **Buffer Overflow**:

   ```
   Program received signal SIGSEGV, Segmentation fault.
   0x00007ffff7b4a3e5 in memcpy () from /lib/x86_64-linux-gnu/libc.so.6
   (gdb) backtrace
   #0  0x00007ffff7b4a3e5 in memcpy () from /lib/x86_64-linux-gnu/libc.so.6
   #1  0x00007ffff7b4a3e5 in MyDriver::readResponse(char*, int) at mydriver.cpp:300
   ```

   Diagnosis: Likely a buffer overflow in `readResponse`.

3. **Use After Free**:

   ```
   Program received signal SIGSEGV, Segmentation fault.
   0x00007ffff7b4a3e5 in MyDriver::processData() at mydriver.cpp:400
   400     int value = *dataPtr;
   (gdb) print dataPtr
   $1 = 0x7fffffffd800 <already freed memory>
   ```

   Diagnosis: `dataPtr` points to memory that has been freed.

4. **Stack Overflow**:
   ```
   Program received signal SIGSEGV, Segmentation fault.
   0x00007ffff7b4a3e5 in MyDriver::recursiveFunction(int) at mydriver.cpp:500
   500     return recursiveFunction(n-1) + 1;
   (gdb) backtrace
   [Extremely long backtrace with the same function repeated]
   ```
   Diagnosis: Infinite recursion causing stack overflow.

## Advanced Troubleshooting

### Driver Initialization Issues

#### Property Initialization Failures

**Symptoms:**

- Properties are not created or are incorrectly initialized
- Error messages during driver initialization

**Possible Causes:**

- Incorrect property definitions
- Memory allocation failures
- Initialization order issues

**Solutions:**

1. Check property definitions for correctness
2. Ensure all arrays are properly sized
3. Follow the correct initialization order:

   ```cpp
   bool MyDriver::initProperties()
   {
       // Always call the parent's initProperties first
       INDI::DefaultDevice::initProperties();

       // Initialize your properties
       // Define enum for property indices to avoid magic numbers
       enum
       {
           SETTING_1,
           SETTING_COUNT
       };

       // Initialize the number property using the fill method
       SettingsNP[SETTING_1].fill("SETTING_1", "Setting 1", "%.2f", 0, 100, 1, 50);
       SettingsNP.fill(getDeviceName(), "SETTINGS", "Settings", MAIN_CONTROL_TAB, IP_RW, 60, IPS_IDLE);

       // Add debug, simulation, and configuration controls
       addDebugControl();
       addSimulationControl();
       addConfigurationControl();

       return true;
   }

   bool MyDriver::updateProperties()
   {
       // Always call the parent's updateProperties first
       INDI::DefaultDevice::updateProperties();

       if (isConnected())
       {
           // Define properties when connected
           defineProperty(&SettingsNP);
       }
       else
       {
           // Delete properties when disconnected
           deleteProperty(SettingsNP.name);
       }

       return true;
   }
   ```

#### Driver Loading Failures

**Symptoms:**

- The driver fails to load
- Error messages like "Cannot load driver" or "Symbol not found"

**Possible Causes:**

- Missing dependencies
- Incorrect installation
- Binary incompatibility

**Solutions:**

1. Check for missing dependencies:
   ```bash
   ldd /usr/bin/indi_mydriver
   ```
2. Verify the installation path:
   ```bash
   ls -l /usr/bin/indi_mydriver
   ls -l /usr/share/indi/indi_mydriver.xml
   ```
3. Rebuild the driver with the correct compiler flags
4. Check for symbol conflicts:
   ```bash
   nm -D /usr/bin/indi_mydriver | grep "T _Z"
   ```

### Thread-Related Issues

#### Deadlocks

**Symptoms:**

- The driver hangs or becomes unresponsive
- Operations never complete

**Possible Causes:**

- Mutex deadlocks
- Waiting on resources that are never released
- Circular dependencies

**Solutions:**

1. Use thread-safe programming practices
2. Implement timeout mechanisms for all blocking operations
3. Use lock hierarchies to prevent deadlocks
4. Use tools like Helgrind to detect potential deadlocks:
   ```bash
   valgrind --tool=helgrind indiserver -v indi_mydriver
   ```

#### Race Conditions

**Symptoms:**

- Intermittent failures
- Different behavior on different runs
- Timing-dependent issues

**Possible Causes:**

- Shared resources accessed without proper synchronization
- Order of operations not properly controlled
- Assumptions about timing

**Solutions:**

1. Use proper synchronization primitives (mutexes, condition variables)
2. Make critical sections as small as possible
3. Avoid shared mutable state when possible
4. Use atomic operations for simple shared variables
5. Implement proper thread synchronization:

   ```cpp
   std::mutex mutex;

   void MyDriver::updateValue(int newValue)
   {
       std::lock_guard<std::mutex> lock(mutex);
       value = newValue;
   }

   int MyDriver::getValue()
   {
       std::lock_guard<std::mutex> lock(mutex);
       return value;
   }
   ```

### Device-Specific Issues

#### Firmware Compatibility

**Symptoms:**

- Unexpected device behavior
- Commands that should work don't
- Features not working as expected

**Possible Causes:**

- Incompatible firmware version
- Firmware bugs
- Undocumented firmware changes

**Solutions:**

1. Check the device's firmware version
2. Update to the latest firmware if possible
3. Add firmware version detection to your driver:

   ```cpp
   bool MyDriver::checkFirmwareVersion()
   {
       char response[32];
       if (!sendCommand("VERSION", response, sizeof(response)))
           return false;

       int major, minor;
       if (sscanf(response, "VERSION %d.%d", &major, &minor) != 2)
           return false;

       LOGF_INFO("Device firmware version: %d.%d", major, minor);

       // Check for minimum required version
       if (major < 2 || (major == 2 && minor < 5))
       {
           LOG_WARN("Firmware version 2.5 or later recommended for full functionality");
       }

       return true;
   }
   ```

4. Implement workarounds for known firmware issues

#### Hardware Limitations

**Symptoms:**

- Performance issues
- Functionality limitations
- Reliability problems

**Possible Causes:**

- Device hardware limitations
- Environmental factors (temperature, power, etc.)
- Physical constraints

**Solutions:**

1. Document known hardware limitations
2. Implement workarounds where possible
3. Add checks for hardware capabilities:

   ```cpp
   bool MyDriver::checkCapabilities()
   {
       char response[32];
       if (!sendCommand("CAPABILITIES", response, sizeof(response)))
           return false;

       // Parse capabilities
       bool hasFeatureA = strstr(response, "FEATURE_A") != nullptr;
       bool hasFeatureB = strstr(response, "FEATURE_B") != nullptr;

       // Update driver capabilities
       CapabilityNP[0].setValue(hasFeatureA ? 1 : 0);
       CapabilityNP[1].setValue(hasFeatureB ? 1 : 0);
       CapabilityNP.setState(IPS_OK);
       CapabilityNP.apply();

       return true;
   }
   ```

4. Provide user guidance for optimal hardware usage

## Creating a Troubleshooting Checklist

When troubleshooting INDI drivers, it's helpful to follow a systematic approach. Here's a checklist you can use:

1. **Verify the basics**

   - Is the device connected and powered on?
   - Are the permissions correct?
   - Is the device being used by another program?

2. **Check the logs**

   - Run the INDI server with verbose logging
   - Look for error messages and warnings
   - Check the sequence of events

3. **Verify the connection**

   - Are the connection settings correct?
   - Can you connect to the device using other tools?
   - Is the device responding to commands?

4. **Test with simplification**

   - Try a minimal driver configuration
   - Test basic functionality first
   - Isolate the problem by disabling features

5. **Check for common issues**

   - Memory leaks
   - Thread synchronization
   - Resource management
   - Error handling

6. **Use debugging tools**

   - GDB for crashes and logic issues
   - Valgrind for memory issues
   - Serial/network monitoring tools for communication issues
   - Ekos Debugger for client-server interaction issues

7. **Review the code**

   - Check for logical errors
   - Verify against the device documentation
   - Look for edge cases and error conditions

8. **Seek help**
   - Check the INDI forums and GitHub issues
   - Ask for help from other developers
   - Share your logs and debugging information

## Conclusion

Troubleshooting INDI drivers can be challenging, but with a systematic approach and the right tools, most issues can be resolved. Remember to:

- Start with the basics and work your way up
- Use the available debugging tools and techniques
- Document your findings for future reference
- Share your solutions with the INDI community

By following the guidelines in this document, you should be able to diagnose and fix most issues with your INDI drivers.

For more information, refer to the [INDI Library Documentation](https://www.indilib.org/api/index.html) and the [INDI Driver Development Guide](https://www.indilib.org/develop/developer-manual/100-driver-development.html).
