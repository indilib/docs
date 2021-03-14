#include <cstring>

#include "libindi/indicom.h"

#include "config.h"
#include "indi_dummy_filterwheel.h"

// We declare an auto pointer to DummyFilterWheel.
static std::unique_ptr<DummyFilterWheel> mydriver(new DummyFilterWheel());

DummyFilterWheel::DummyFilterWheel()
{
    setVersion(CDRIVER_VERSION_MAJOR, CDRIVER_VERSION_MINOR);

    // Here we tell the base filterwheel class what types of connections we can support
    setFilterConnection(CONNECTION_SERIAL | CONNECTION_TCP);
}

const char *DummyFilterWheel::getDefaultName()
{
    return "Dummy FilterWheel";
}

bool DummyFilterWheel::initProperties()
{
    // initialize the parent's properties first
    INDI::FilterWheel::initProperties();

    // TODO: Add any custom properties you need here.

    CurrentFilter = 1;

    // TODO: If you know how many filters are on the wheel before connecting,
    // set FilterSlotN[0].min and FilterSlotN[0].max here.

    addAuxControls();

    return true;
}

void DummyFilterWheel::ISGetProperties(const char *dev)
{
    INDI::FilterWheel::ISGetProperties(dev);

    // TODO: Call define* for any custom properties.
}

bool DummyFilterWheel::updateProperties()
{
    INDI::FilterWheel::updateProperties();

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

bool DummyFilterWheel::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    // Make sure it is for us.
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        // TODO: Check to see if this is for any of my custom Number properties.
    }

    // Nobody has claimed this, so let the parent handle it
    return INDI::FilterWheel::ISNewNumber(dev, name, values, names, n);
}

bool DummyFilterWheel::ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    // Make sure it is for us.
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        // TODO: Check to see if this is for any of my custom Switch properties.
    }

    // Nobody has claimed this, so let the parent handle it
    return INDI::FilterWheel::ISNewSwitch(dev, name, states, names, n);
}

bool DummyFilterWheel::ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    // Make sure it is for us.
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        // TODO: Check to see if this is for any of my custom Text properties.
    }

    // Nobody has claimed this, so let the parent handle it
    return INDI::FilterWheel::ISNewText(dev, name, texts, names, n);
}

bool DummyFilterWheel::ISSnoopDevice(XMLEle *root)
{
    // TODO: Check to see if this is for any of my custom Snoops. Fo shizzle.

    return INDI::FilterWheel::ISSnoopDevice(root);
}

bool DummyFilterWheel::saveConfigItems(FILE *fp)
{
    INDI::FilterWheel::saveConfigItems(fp);

    // TODO: Call IUSaveConfig* for any custom properties I want to save.

    return true;
}

bool DummyFilterWheel::Handshake()
{
    if (isSimulation())
    {
        LOGF_INFO("Connected successfuly to simulated %s.", getDeviceName());
    }

    // NOTE: PortFD is set by the base class.

    // TODO: Any initial communciation needed with our filterwheel, we have an active
    // connection.

    // TODO: If you can query the hardware to get a count of filters, do it here

    FilterSlotN[0].min = 1;
    FilterSlotN[0].max = 8;

    IUUpdateMinMax(&FilterSlotNP);

    return true;
}

void DummyFilterWheel::TimerHit()
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

int DummyFilterWheel::QueryFilter()
{
    // TODO: Query the hardware (or a local variable) to return what index
    // the filter wheel is currently at.

    return CurrentFilter;
}

bool DummyFilterWheel::SelectFilter(int index)
{
    // NOTE: index starts at 1, not 0

    TargetFilter = index;

    // TODO: Tell the hardware to change to the given index.
    // Be sure to call SelectFilterDone when it has finished moving.

    CurrentFilter = TargetFilter;
    SelectFilterDone(index);
    return true;
}

bool DummyFilterWheel::SetFilterNames()
{
    // TODO: If you can set the filter names to save in hardware, do it here.
    // Filter names are in the FilterNameT class var.
    // Otherwise, just save them to the config file with this.
    return INDI::FilterInterface::SetFilterNames();
}

bool DummyFilterWheel::GetFilterNames()
{
    // TODO: If you can get the filter names from hardware, do it here.
    // Use the hardware to populate FilterNameT.
    // Otherwise, just use the default.
    return INDI::FilterInterface::GetFilterNames();
}
