#include <cstring>
#include <termios.h>

#include "libindi/indicom.h"
#include "libindi/connectionplugins/connectionserial.h"

#include "config.h"
#include "indi_mycustomdriver.h"

// We declare an auto pointer to MyCustomDriver.
static std::unique_ptr<MyCustomDriver> mydriver(new MyCustomDriver());

// libindidriver will try to link to these functions, so they MUST exist.
// Here we pass off handling of them to our driver's class.

void ISGetProperties(const char *dev)
{
    mydriver->ISGetProperties(dev);
}

void ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    mydriver->ISNewSwitch(dev, name, states, names, n);
}

void ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    mydriver->ISNewText(dev, name, texts, names, n);
}

void ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    mydriver->ISNewNumber(dev, name, values, names, n);
}

void ISNewBLOB(const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[],
               char *formats[], char *names[], int n)
{
    mydriver->ISNewBLOB(dev, name, sizes, blobsizes, blobs, formats, names, n);
}

void ISSnoopDevice(XMLEle *root)
{
    mydriver->ISSnoopDevice(root);
}

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

    IUFillSwitch(
        &SayHelloS[SAY_HELLO_DEFAULT], // A reference to the switch VALUE
        "SAY_HELLO_DEFAULT",           // The name of the VALUE
        "Say Hello",                   // The label of the VALUE
        ISS_OFF                        // The switch state
    );
    IUFillSwitch(
        &SayHelloS[SAY_HELLO_CUSTOM], // A reference to the switch VALUE
        "SAY_HELLO_CUSTOM",           // The name of the VALUE
        "Say Custom",                 // The label of the VALUE
        ISS_OFF                       // The switch state
    );

    IUFillSwitchVector(
        &SayHelloSP,      // A reference to the switch PROPERTY
        SayHelloS,        // The list of switch values on this PROPERTY
        SAY_HELLO_N,      // How many switch values are there?
        getDeviceName(),  // The name of the device
        "SAY_HELLO",      // The name of the PROPERTY
        "Hello Commands", // The label of the PROPERTY
        MAIN_CONTROL_TAB, // What tab should we be on?
        IP_RW,            // Let's make it read/write.
        ISR_ATMOST1,      // At most 1 can be on
        60,               // With a timeout of 60 seconds
        IPS_IDLE          // and an initial state of idle.
    );

    // now we register the property with the DefaultDevice
    // without this, the property won't show up on the control panel
    // but let's do that in updateProperties when we are connected now
    // defineProperty(&SayHelloSP);

    IUFillText(&WhatToSayT[0], "WHAT_TO_SAY", "What to say?", "Hello, world!");
    IUFillTextVector(&WhatToSayTP, WhatToSayT, 1, getDeviceName(), "WHAT_TO_SAY", "Got something to say?", MAIN_CONTROL_TAB, IP_RW, 60, IPS_IDLE);
    // defineProperty(&WhatToSayTP); // we moved this to updateProperties below

    // and now let's add a counter of how many times the user clicks the button
    IUFillNumber(&SayCountN[0], // First number VALUE in the property (and the only one)
                 "SAY_COUNT",   // name of the VALUE
                 "Count",       // label of the VALUE
                 "%0.f",        // format specifier to show the value to the user; this should be a format specifier for a double
                 0,             // minimum value; used by the client to render the UI
                 0,             // maximum value; used by the client to render the UI
                 0,             // step value; used by the client to render the UI
                 0);            // current value

    IUFillNumberVector(&SayCountNP,      // reference to the number PROPERTY
                       SayCountN,        // Array of number values
                       1,                // count of number values in the array
                       getDeviceName(),  // device name
                       "SAY_COUNT",      // PROPERTY name
                       "Say Count",      // PROPERTY label
                       MAIN_CONTROL_TAB, // What tab should we be on?
                       IP_RO,            // Make this read-only
                       0,                // With no timeout
                       IPS_IDLE);        // and an initial state of idle

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
    DefaultDevice::ISGetProperties(dev);
    loadConfig(true, WhatToSayTP.name);
}

bool MyCustomDriver::updateProperties()
{
    INDI::DefaultDevice::updateProperties();

    if (isConnected())
    {
        // Add the properties to the driver when we connect.
        defineProperty(&SayHelloSP);
        defineProperty(&WhatToSayTP);
        defineProperty(&SayCountNP);
    }
    else
    {
        // And remove them when we disconnect.
        deleteProperty(SayHelloSP.name);
        deleteProperty(WhatToSayTP.name);
        deleteProperty(SayCountNP.name);
    }

    return true;
}

bool MyCustomDriver::ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[],
                                 int n)
{
    // All of the ISNew* methods should return true if the property update was handled,
    // otherwise false. The return value is NOT an indication of success, but rather
    // that the property belonged to the device.
    // So we can either write our methods to check base classes first, like this...

    if (INDI::DefaultDevice::ISNewSwitch(dev, name, states, names, n))
    {
        // Looks like DefaultDevice handled this, so we don't need to worry with it.
        return true;
    }

    // Make sure it is for us.
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        if (strcmp(name, SayHelloSP.name) == 0)
        {
            // Accept what we received.
            IUUpdateSwitch(&SayHelloSP, states, names, n);

            // Find out what switch was clicked.
            int index = IUFindOnSwitchIndex(&SayHelloSP);
            switch (index)
            {
            case SAY_HELLO_DEFAULT:
                LOG_INFO("Hello, world!");
                break;
            case SAY_HELLO_CUSTOM:
                LOG_INFO(WhatToSayT[0].text);
                break;
            }

            // Increment our "Say Count" counter.
            // Here we update the value on the property.
            SayCountN[0].value = int(SayCountN[0].value) + 1;
            // And then send a message to the clients to let them know it is updated.
            IDSetNumber(&SayCountNP, nullptr);

            // Turn all switches back off.
            IUResetSwitch(&SayHelloSP);

            // Set the property state back to idle
            SayHelloSP.s = IPS_IDLE;

            // And actually inform INDI of those two operations
            IDSetSwitch(&SayHelloSP, nullptr);

            return true;
        }
    }

    return false;
}

bool MyCustomDriver::ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    // Make sure it is for us.
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        if (strcmp(name, WhatToSayTP.name) == 0)
        {
            WhatToSayTP.s = IPS_IDLE;
            // This is a helper method to just update the values
            // on the property.
            IUUpdateText(&WhatToSayTP, texts, names, n);

            // And tell the client they were updated.
            IDSetText(&WhatToSayTP, nullptr);

            // This is a really important value, so make sure we save it every time
            // the user sets it. Don't wait for the user to click the save
            // button in options...
            // You probably don't want to do this for all your properties, but
            // you might for some.
            saveConfig(true, WhatToSayTP.name);

            return true;
        }
    }

    // ...or pass it down if we don't handle it.
    // Nobody has claimed this, so let the parent handle it.
    return INDI::DefaultDevice::ISNewText(dev, name, texts, names, n);
}

bool MyCustomDriver::saveConfigItems(FILE *fp)
{
    INDI::DefaultDevice::saveConfigItems(fp);
    IUSaveConfigText(fp, &WhatToSayTP);

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
