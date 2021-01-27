#include <cstring>

#include "libindi/indicom.h"
#include "libindi/connectionplugins/connectionserial.h"

#include "config.h"
#include "indi_dummy_dustcap.h"

// We declare an auto pointer to DummyDustcap.
static std::unique_ptr<DummyDustcap> mydriver(new DummyDustcap());

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

DummyDustcap::DummyDustcap()
{
    setVersion(CDRIVER_VERSION_MAJOR, CDRIVER_VERSION_MINOR);
}

const char *DummyDustcap::getDefaultName()
{
    return "Dummy Dustcap";
}

bool DummyDustcap::initProperties()
{
    // initialize the parent's properties first
    INDI::DefaultDevice::initProperties();

    // initialize the parent's properties first
    initDustCapProperties(getDeviceName(), MAIN_CONTROL_TAB);

    // TODO: Add any custom properties you need here.

    // Add debug/simulation/etc controls to the driver.
    addAuxControls();

    setDriverInterface(DUSTCAP_INTERFACE | AUX_INTERFACE);

    serialConnection = new Connection::Serial(this);
    serialConnection->registerHandshake([&]() { return Handshake(); });
    serialConnection->setDefaultBaudRate(Connection::Serial::B_57600);
    serialConnection->setDefaultPort("/dev/ttyACM0");
    registerConnection(serialConnection);

    return true;
}

void DummyDustcap::ISGetProperties(const char *dev)
{
    INDI::DefaultDevice::ISGetProperties(dev);
}

bool DummyDustcap::updateProperties()
{
    INDI::DefaultDevice::updateProperties();

    if (isConnected())
    {
        // The DustCapInterface doesn't define this for us, so we need to do it.
        defineProperty(&ParkCapSP);

        // TODO: Call define* for any custom properties only visible when connected.
    }
    else
    {
        // The DustCapInterface doesn't delete this for us, so we need to do it.
        deleteProperty(ParkCapSP.name);

        // TODO: Call deleteProperty for any custom properties only visible when connected.
    }

    return true;
}

bool DummyDustcap::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    // Make sure it is for us.
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        // TODO: Check to see if this is for any of my custom Number properties.
    }

    // Nobody has claimed this, so let the parent handle it
    return INDI::DefaultDevice::ISNewNumber(dev, name, values, names, n);
}

bool DummyDustcap::ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    // Make sure it is for us.
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        // TODO: Check to see if this is for any of my custom Switch properties.
    }

    if (processDustCapSwitch(dev, name, states, names, n))
    {
        return true;
    }

    // Nobody has claimed this, so let the parent handle it
    return INDI::DefaultDevice::ISNewSwitch(dev, name, states, names, n);
}

bool DummyDustcap::ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    // Make sure it is for us.
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        // TODO: Check to see if this is for any of my custom Text properties.
    }

    // Nobody has claimed this, so let the parent handle it
    return INDI::DefaultDevice::ISNewText(dev, name, texts, names, n);
}

bool DummyDustcap::ISSnoopDevice(XMLEle *root)
{
    // TODO: Check to see if this is for any of my custom Snoops. Fo shizzle.

    return INDI::DefaultDevice::ISSnoopDevice(root);
}

bool DummyDustcap::saveConfigItems(FILE *fp)
{
    // TODO: Call IUSaveConfig* for any custom properties I want to save.

    return INDI::DefaultDevice::saveConfigItems(fp);
}

bool DummyDustcap::Handshake()
{
    if (isSimulation())
    {
        LOGF_INFO("Connected successfuly to simulated %s.", getDeviceName());
        return true;
    }

    PortFD = serialConnection->getPortFD();

    return true;
}

void DummyDustcap::TimerHit()
{
    if (!isConnected())
        return;

    // TODO: Poll your device if necessary. Otherwise delete this method and it's
    // declaration in the header file.

    LOG_INFO("timer hit");

    // If you don't call SetTimer, we'll never get called again, until we disconnect
    // and reconnect.
    SetTimer(POLLMS);
}

IPState DummyDustcap::ParkCap()
{
    // TODO: Implement your own code to close the dust cap.
    return IPS_OK;
}

IPState DummyDustcap::UnParkCap()
{
    // TODO: Implement your own code to open the dust cap.

    return IPS_OK;
}
