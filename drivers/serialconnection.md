---
sort: 4
---
## Serial Connections

Most of the hard work for dealing with serial connections is handled for you.

Let's add this to our header file:

```cpp
namespace Connection
{
    class Serial;
}
```

And this to our class header definition.

```cpp
private: // serial connection
    bool Handshake();
    bool sendCommand(const char *cmd);
    int PortFD{-1};

    Connection::Serial *serialConnection{nullptr};
```

Then we need to add this include:

```cpp
#include <libindi/connectionplugins/connectionserial.h>
```

to our cpp file.

Then add some new lines to `initProperties`:

```cpp
    // Add debug/simulation/etc controls to the driver.
    addAuxControls();

    serialConnection = new Connection::Serial(this);
    serialConnection->registerHandshake([&]() { return Handshake(); });
    serialConnection->setDefaultBaudRate(Connection::Serial::B_57600);
    serialConnection->setDefaultPort("/dev/ttyACM0");
    registerConnection(serialConnection);
```

This is pretty straight forward, but we are just registering a new serial connection
(there's one available for TCP as well). `DefaultDevice` will use it to connect when
the user clicks the `Connect` button, then call `Handshake` when it is connected.

Speaking of `Handshake`...

```cpp
bool MyCustomDriver::Handshake()
{
    if (isSimulation())
    {
        LOGF_INFO("Connected successfuly to simulated %s.", getDeviceName());
        return true;
    }

    PortFD = serialConnection->getPortFD();

    return true;
}
```

Here we get a reference to the file descriptor that we can use in the `tty_*` functions exposed
in `libindi/indicom.h`.

I've also included an example `sendCmd` method, but this will be really up to you and the protocol of
your device.

```cpp
bool MyCustomDriver::sendCommand(const char *cmd)
{
    int nbytes_read = 0, nbytes_written = 0, tty_rc = 0;
    char res[8] = {0};
    LOGF_DEBUG("CMD <%s>", cmd);

    if (!isSimulation())
    {
        tcflush(PortFD, TCIOFLUSH);
        if ((tty_rc = tty_write_string(PortFD, cmd, &nbytes_written)) != TTY_OK)
        {
            char errorMessage[MAXRBUF];
            tty_error_msg(tty_rc, errorMessage, MAXRBUF);
            LOGF_ERROR("Serial write error: %s", errorMessage);
            return false;
        }
    }

    if (isSimulation())
    {
        strncpy(res, "OK#", 8);
        nbytes_read = 3;
    }
    else
    {
        if ((tty_rc = tty_read_section(PortFD, res, '#', 1, &nbytes_read)) != TTY_OK)
        {
            char errorMessage[MAXRBUF];
            tty_error_msg(tty_rc, errorMessage, MAXRBUF);
            LOGF_ERROR("Serial read error: %s", errorMessage);
            return false;
        }
    }

    res[nbytes_read - 1] = '\0';
    LOGF_DEBUG("RES <%s>", res);

    return true;
}
```

## Properties defined after connecting

If you have properties that you want defined only when you are connected, you'll
need to override another method, `updateProperties`.

We'll move our `defineProperty` calls out of `initProperties` and into here.

Remember, you can call `defineProperty` any time, not just in `initProperties` or
`updateProperties`. You could query the capabilities of your device first,
or call it in response to user interaction.

```cpp
bool MyCustomDriver::updateProperties()
{
    INDI::DefaultDevice::updateProperties();

    if (isConnected())
    {
        // Add the properties to the driver when we connect.
        defineProperty(&SayHelloSP);
        defineProperty(&WhatToSayTP);
    }
    else
    {
        // And remove them when we disconnect.
        deleteProperty(SayHelloSP.name);
        deleteProperty(WhatToSayTP.name);
    }

    return true;
}
```

So there we have it. We are connected to our actual device (or the simulator). Now
how do we do things like check status over and over again? Read on in [loops](loops.md).
