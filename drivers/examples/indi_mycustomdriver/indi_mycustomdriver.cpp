#include <cstring>
#include <termios.h>

#include "libindi/indicom.h"
#include "libindi/connectionplugins/connectionserial.h"

#include "config.h"
#include "indi_mycustomdriver.h"

// We declare an auto pointer to MyCustomDriver.
static std::unique_ptr<MyCustomDriver> mydriver(new MyCustomDriver());

MyCustomDriver::MyCustomDriver()
{
    setVersion(CDRIVER_VERSION_MAJOR, CDRIVER_VERSION_MINOR);
}

const char *MyCustomDriver::getDefaultName()
{
    return "My Custom Driver";
}

bool MyCustomDriver::initProperties()
{
    // initialize the parent's properties first
    INDI::DefaultDevice::initProperties();

    // A reference to the switch VALUE
    SayHelloSP[SAY_HELLO_DEFAULT].fill(
        "SAY_HELLO_DEFAULT",  // The name of the VALUE
        "Say Hello",          // The label of the VALUE
        ISS_OFF               // The switch state
    );

    // A reference to the switch VALUE
    SayHelloSP[SAY_HELLO_CUSTOM].fill(
        "SAY_HELLO_CUSTOM",   // The name of the VALUE
        "Say Custom",         // The label of the VALUE
        ISS_OFF               // The switch state
    );

    // A reference to the switch PROPERTY
    SayHelloSP.fill(
        getDeviceName(),  // The name of the device
        "SAY_HELLO",      // The name of the PROPERTY
        "Hello Commands", // The label of the PROPERTY
        MAIN_CONTROL_TAB, // What tab should we be on?
        IP_RW,            // Let's make it read/write.
        ISR_ATMOST1,      // At most 1 can be on
        60,               // With a timeout of 60 seconds
        IPS_IDLE          // and an initial state of idle.
    );

    SayHelloSP.onUpdate([this]
    {
        // Find out what switch was clicked.
        switch (SayHelloSP.findOnSwitchIndex())
        {
        case SAY_HELLO_DEFAULT:
            LOG_INFO("Hello, world!");
            break;
        case SAY_HELLO_CUSTOM:
            LOG_INFO(WhatToSayTP[0].getText());
            break;
        }

        // Increment our "Say Count" counter.
        // Here we update the value on the property.
        SayCountNP[0].setValue(SayCountNP[0].getValue() + 1);

        // And then send a message to the clients to let them know it is updated.
        SayCountNP.apply();

        // Turn all switches back off.
        SayHelloSP.reset();

        // Set the property state back to idle
        SayHelloSP.setState(IPS_IDLE);

        // And actually inform INDI of those two operations
        SayHelloSP.apply();
    });

    // now we register the property with the DefaultDevice
    // without this, the property won't show up on the control panel
    // but let's do that in updateProperties when we are connected now
    // defineProperty(&SayHelloSP);

    WhatToSayTP[0].fill("WHAT_TO_SAY", "What to say?", "Hello, custom world!");
    WhatToSayTP.fill(getDeviceName(), "WHAT_TO_SAY", "Got something to say?", MAIN_CONTROL_TAB, IP_RW, 60, IPS_IDLE);
    // defineProperty(&WhatToSayTP); // we moved this to updateProperties below

    // and now let's add a counter of how many times the user clicks the button
    // First number VALUE in the property (and the only one)
    SayCountNP[0].fill(
        "SAY_COUNT",  // name of the VALUE
        "Count",      // label of the VALUE
        "%0.f",       // format specifier to show the value to the user; this should be a format specifier for a double
        0,            // minimum value; used by the client to render the UI
        0,            // maximum value; used by the client to render the UI
        0,            // step value; used by the client to render the UI
        0             // current value
    );

    SayCountNP.fill(
        getDeviceName(),  // device name
        "SAY_COUNT",      // PROPERTY name
        "Say Count",      // PROPERTY label
        MAIN_CONTROL_TAB, // What tab should we be on?
        IP_RO,            // Make this read-only
        0,                // With no timeout
        IPS_IDLE          // and an initial state of idle
    );

    WhatToSayTP.onUpdate([this]
    {
        WhatToSayTP.setState(IPS_IDLE);

        // Tell the clien tthey were updated
        WhatToSayTP.apply();

        // This is a really important value, so make sure we save it every time
        // the user sets it. Don't wait for the user to click the save
        // button in options...
        // You probably don't want to do this for all your properties, but
        // you might for some.
        saveConfig(WhatToSayTP);
    });

    addAuxControls();

    serialConnection = new Connection::Serial(this);
    serialConnection->registerHandshake([&]() { return Handshake(); });
    serialConnection->setDefaultBaudRate(Connection::Serial::B_57600);
    serialConnection->setDefaultPort("/dev/ttyACM0");
    registerConnection(serialConnection);

    return true;
}

void MyCustomDriver::ISGetProperties(const char *dev)
{
    loadConfig(WhatToSayTP);
    DefaultDevice::ISGetProperties(dev);
}

bool MyCustomDriver::updateProperties()
{
    INDI::DefaultDevice::updateProperties();

    if (isConnected())
    {
        // Add the properties to the driver when we connect.
        defineProperty(SayHelloSP);
        defineProperty(WhatToSayTP);
        defineProperty(SayCountNP);
    }
    else
    {
        // And remove them when we disconnect.
        deleteProperty(SayHelloSP);
        deleteProperty(WhatToSayTP);
        deleteProperty(SayCountNP);
    }

    return true;
}

bool MyCustomDriver::saveConfigItems(FILE *fp)
{
    INDI::DefaultDevice::saveConfigItems(fp);
    WhatToSayTP.save(fp);
    return true;
}

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

void MyCustomDriver::TimerHit()
{
    if (!isConnected())
        return;

    LOG_INFO("timer hit");

    // If you don't call SetTimer, we'll never get called again, until we disconnect
    // and reconnect.
    SetTimer(getCurrentPollingPeriod());
}
