#include <cstring>

#include "libindi/indicom.h"
#include "libindi/connectionplugins/connectionserial.h"

#include "config.h"
#include "indi_dummy_lightbox.h"

// We declare an auto pointer to DummyLightbox.
static std::unique_ptr<DummyLightbox> mydriver(new DummyLightbox());

DummyLightbox::DummyLightbox() : INDI::LightBoxInterface(this, true)
{
    setVersion(CDRIVER_VERSION_MAJOR, CDRIVER_VERSION_MINOR);
}

const char *DummyLightbox::getDefaultName()
{
    return "Dummy Lightbox";
}

bool DummyLightbox::initProperties()
{
    // initialize the parent's properties first
    INDI::DefaultDevice::initProperties();

    // initialize the parent's properties first
    initLightBoxProperties(getDeviceName(), MAIN_CONTROL_TAB);

    // TODO: Add any custom properties you need here.

    // Add debug/simulation/etc controls to the driver.
    addAuxControls();

    setDriverInterface(LIGHTBOX_INTERFACE | AUX_INTERFACE);

    serialConnection = new Connection::Serial(this);
    serialConnection->registerHandshake([&]() { return Handshake(); });
    serialConnection->setDefaultBaudRate(Connection::Serial::B_57600);
    serialConnection->setDefaultPort("/dev/ttyACM0");
    registerConnection(serialConnection);

    return true;
}

void DummyLightbox::ISGetProperties(const char *dev)
{
    INDI::DefaultDevice::ISGetProperties(dev);

    isGetLightBoxProperties(dev);
}

bool DummyLightbox::updateProperties()
{
    INDI::DefaultDevice::updateProperties();

    if (!updateLightBoxProperties())
    {
        return false;
    }

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

bool DummyLightbox::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    // Make sure it is for us.
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        // TODO: Check to see if this is for any of my custom Number properties.
    }

    if (processLightBoxNumber(dev, name, values, names, n))
    {
        return true;
    }

    // Nobody has claimed this, so let the parent handle it
    return INDI::DefaultDevice::ISNewNumber(dev, name, values, names, n);
}

bool DummyLightbox::ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    // Make sure it is for us.
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        // TODO: Check to see if this is for any of my custom Switch properties.
    }

    if (processLightBoxSwitch(dev, name, states, names, n))
    {
        return true;
    }

    // Nobody has claimed this, so let the parent handle it
    return INDI::DefaultDevice::ISNewSwitch(dev, name, states, names, n);
}

bool DummyLightbox::ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    // Make sure it is for us.
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        // TODO: Check to see if this is for any of my custom Text properties.
    }

    if (processLightBoxText(dev, name, texts, names, n))
    {
        return true;
    }

    // Nobody has claimed this, so let the parent handle it
    return INDI::DefaultDevice::ISNewText(dev, name, texts, names, n);
}

bool DummyLightbox::ISSnoopDevice(XMLEle *root)
{
    // TODO: Check to see if this is for any of my custom Snoops. Fo shizzle.

    snoopLightBox(root);

    return INDI::DefaultDevice::ISSnoopDevice(root);
}

bool DummyLightbox::saveConfigItems(FILE *fp)
{
    saveLightBoxConfigItems(fp);

    // TODO: Call IUSaveConfig* for any custom properties I want to save.

    return INDI::DefaultDevice::saveConfigItems(fp);
}

bool DummyLightbox::Handshake()
{
    if (isSimulation())
    {
        LOGF_INFO("Connected successfuly to simulated %s.", getDeviceName());
        return true;
    }

    PortFD = serialConnection->getPortFD();

    return true;
}

void DummyLightbox::TimerHit()
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

bool DummyLightbox::SetLightBoxBrightness(uint16_t value)
{
    // TODO: Implement your own code to set the brightness of the lightbox.
    // Be sure to return true if successful, or false otherwise.

    INDI_UNUSED(value);

    return false;
}

bool DummyLightbox::EnableLightBox(bool enable)
{
    // TODO: Implement your own code to turn on/off the lightbox.
    // Be sure to return true if successful, or false otherwise.

    INDI_UNUSED(enable);

    return false;
}
