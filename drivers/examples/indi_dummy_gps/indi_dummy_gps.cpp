#include <cstring>

#include "libindi/indicom.h"
#include "libindi/connectionplugins/connectionserial.h"

#include "config.h"
#include "indi_dummy_gps.h"

// We declare an auto pointer to DummyGPS.
static std::unique_ptr<DummyGPS> mydriver(new DummyGPS());

DummyGPS::DummyGPS()
{
    setVersion(CDRIVER_VERSION_MAJOR, CDRIVER_VERSION_MINOR);
}

const char *DummyGPS::getDefaultName()
{
    return "Dummy GPS";
}

bool DummyGPS::initProperties()
{
    // initialize the parent's properties first
    INDI::GPS::initProperties();

    // TODO: Add any custom properties you need here.

    addAuxControls();

    serialConnection = new Connection::Serial(this);
    serialConnection->registerHandshake([&]() { return Handshake(); });
    serialConnection->setDefaultBaudRate(Connection::Serial::B_57600);
    serialConnection->setDefaultPort("/dev/ttyACM0");
    registerConnection(serialConnection);

    return true;
}

void DummyGPS::ISGetProperties(const char *dev)
{
    INDI::GPS::ISGetProperties(dev);

    // TODO: Call define* for any custom properties.
}

bool DummyGPS::updateProperties()
{
    INDI::GPS::updateProperties();

    if (isConnected())
    {
        // TODO: Call define* for any custom properties only visible when connected.
    }
    else
    {
        // TODO: Call deleteProperty for any custom properties only visible when connected.
    }

    return true;
}

bool DummyGPS::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    // Make sure it is for us.
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        // TODO: Check to see if this is for any of my custom Number properties.
    }

    // Nobody has claimed this, so let the parent handle it
    return INDI::GPS::ISNewNumber(dev, name, values, names, n);
}

bool DummyGPS::ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    // Make sure it is for us.
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        // TODO: Check to see if this is for any of my custom Switch properties.
    }

    // Nobody has claimed this, so let the parent handle it
    return INDI::GPS::ISNewSwitch(dev, name, states, names, n);
}

bool DummyGPS::ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    // Make sure it is for us.
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        // TODO: Check to see if this is for any of my custom Text properties.
    }

    // Nobody has claimed this, so let the parent handle it
    return INDI::GPS::ISNewText(dev, name, texts, names, n);
}

bool DummyGPS::ISSnoopDevice(XMLEle *root)
{
    // TODO: Check to see if this is for any of my custom Snoops. Fo shizzle.

    return INDI::GPS::ISSnoopDevice(root);
}

bool DummyGPS::saveConfigItems(FILE *fp)
{
    INDI::GPS::saveConfigItems(fp);

    // TODO: Call IUSaveConfig* for any custom properties I want to save.

    return true;
}

bool DummyGPS::Handshake()
{
    if (isSimulation())
    {
        LOGF_INFO("Connected successfuly to simulated %s.", getDeviceName());
        return true;
    }

    PortFD = serialConnection->getPortFD();

    return true;
}

IPState DummyGPS::updateGPS()
{
    // TODO: Update the GPS and Time
    static char ts[32] = {0};
    struct tm *utc, *local;

    time_t raw_time;
    time(&raw_time);

    utc = gmtime(&raw_time);
    strftime(ts, sizeof(ts), "%Y-%m-%dT%H:%M:%S", utc);
    IUSaveText(&TimeT[0], ts);

    local = localtime(&raw_time);
    snprintf(ts, sizeof(ts), "%4.2f", (local->tm_gmtoff / 3600.0));
    IUSaveText(&TimeT[1], ts);

    LocationN[LOCATION_LATITUDE].value = 0.0;  // -90 to 90 deg
    LocationN[LOCATION_LONGITUDE].value = 0.0; // 0 to 360 deg
    LocationN[LOCATION_ELEVATION].value = 0.0; // -200 to 10000 m

    // Base class calls IDSetNumber and IDSetText for us

    return IPS_OK;
}
