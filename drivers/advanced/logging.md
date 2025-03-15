---
sort: 6
---
# Logging in INDI Drivers

INDI provides a [Debugging and Logging API](http://www.indilib.org/api/structINDI_1_1Logger.html) where the driver can log different types of messages to the client and/or log file along with time in seconds since the start of the driver.

Furthermore, the driver can log messages locally to stderr using printf-style `IDLog(const char *msg, ...)`. For example:

```cpp
IDLog("Hello Driver!");
int number = 10;
IDLog("My Number is %d\n", number);
```

Note that `IDLog` does not append a new line, you have to insert it explicitly. If you don't add a new line, your message will not show in `stderr` until another message is logged that does include a new line.

When using the Debugging & Logging API, the driver can log messages using `DEBUG` and `DEBUGF` macros. By default, the logger defines 4 levels of debugging/logging levels:

* Errors: Use macro `DEBUG(INDI::Logger::DBG_ERROR, "My Error Message")`
* Warnings: Use macro `DEBUG(INDI::Logger::DBG_WARNING, "My Warning Message")`
* Session: Session messages are the regular status messages from the driver. Use macro `DEBUG(INDI::Logger::DBG_SESSION, "My Message")`
* Driver Debug: Driver debug messages are detailed driver debug output used for diagnostic purposes. Use macro `DEBUG(INDI::Logger::DBG_DEBUG, "My Driver Debug Message")`

Alternatively, The `DEBUGF` macro is used if you have printf-style message. e.g. `DEBUGF(INDI::Logger::DBG_SESSION, "Hello %s!", "There")`

There are shorter versions of these logging macros as well:

* `LOG_ERROR("error message");`
* `LOG_WARN("warning message");`
* `LOG_INFO("session message");`
* `LOG_DEBUG("debug message");`
* `LOG_EXTRA1("extra1 message");`
* `LOG_EXTRA2("extra2 message");`
* `LOG_EXTRA3("extra3 message");`
* `LOGF_ERROR("error %s", "message");`
* `LOGF_WARN("warning %s", "message");`
* `LOGF_INFO("session %s", "message");`
* `LOGF_DEBUG("debug %s", "message");`
* `LOGF_EXTRA1("extra1 %s", "message");`
* `LOGF_EXTRA2("extra2 %s", "message");`
* `LOGF_EXTRA3("extra3 %s", "message");`

Note that these macros only work when your current scope has a `getDeviceName` function or method.

To enable Debug and Logging support in your driver. Simple call `addDebugControl()` from within the `initProperties()` function. The end user is presented with a debug control as illustrated below:

![indi_debug](../images/indi_debug.png)

If enabled, then the Errors, Warnings, and Messages driver output are logged to the client by default. Debug Levels refer to the debug levels logged to the client. On the other hand, Logging Levels property indicates which levels should be logged to a file, if the user enables file logging. The driver simply calls `DEBUG` and `DEBUGF` macros. It is up to the end user to decide whether these messages are logged to the client, or a file, or both, or neither.

In addition to the default four built-in logging levels, you can add additional debug/logging levels up to a maximum of eight. To add a new debug level, call `addDebugLevel(..)` and pass the desired names of the debug and logging level required. The function returns a bitmask you can use for any calls to `DEBUG` and `DEBUGF` macros. For example, to add a new debug level for `SCOPE`, where `SCOPE` contains detailed telescope data:

```cpp
int DBG_SCOPE = addDebugLevel("SCOPE", "SCOPE");
double ra = 12, de = 90;
DEBUGF(DBG_SCOPE, "Mount RA: %g DE: %g", ra, de);
```

The log file is stored in `/tmp` and its name is the driver name plus a time-stamp of the creation date (e.g. `/tmp/indi_simulator_ccd_2016-04-04T06:17:21.log`).
