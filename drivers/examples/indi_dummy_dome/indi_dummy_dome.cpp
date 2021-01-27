#include <cstring>

#include "libindi/indicom.h"

#include "config.h"
#include "indi_dummy_dome.h"

// We declare an auto pointer to DummyDome.
static std::unique_ptr<DummyDome> mydriver(new DummyDome());

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

DummyDome::DummyDome()
{
    setVersion(CDRIVER_VERSION_MAJOR, CDRIVER_VERSION_MINOR);

    // Here we tell the base dome class what types of connections we can support
    setDomeConnection(CONNECTION_SERIAL | CONNECTION_TCP);

    // And here we tell the base class about our dome's capabilities.
    SetDomeCapability(
        DOME_CAN_ABORT |
        DOME_CAN_ABS_MOVE |
        DOME_CAN_REL_MOVE |
        DOME_CAN_PARK |
        DOME_CAN_SYNC |
        DOME_HAS_SHUTTER |
        DOME_HAS_VARIABLE_SPEED |
        DOME_HAS_BACKLASH);
}

const char *DummyDome::getDefaultName()
{
    return "Dummy Dome";
}

bool DummyDome::initProperties()
{
    // initialize the parent's properties first
    INDI::Dome::initProperties();

    // TODO: Add any custom properties you need here.

    addAuxControls();

    return true;
}

void DummyDome::ISGetProperties(const char *dev)
{
    INDI::Dome::ISGetProperties(dev);

    // TODO: Call define* for any custom properties.
}

bool DummyDome::updateProperties()
{
    INDI::Dome::updateProperties();

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

bool DummyDome::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    // Make sure it is for us.
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        // TODO: Check to see if this is for any of my custom Number properties.
    }

    // Nobody has claimed this, so let the parent handle it
    return INDI::Dome::ISNewNumber(dev, name, values, names, n);
}

bool DummyDome::ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    // Make sure it is for us.
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        // TODO: Check to see if this is for any of my custom Switch properties.
    }

    // Nobody has claimed this, so let the parent handle it
    return INDI::Dome::ISNewSwitch(dev, name, states, names, n);
}

bool DummyDome::ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    // Make sure it is for us.
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        // TODO: Check to see if this is for any of my custom Text properties.
    }

    // Nobody has claimed this, so let the parent handle it
    return INDI::Dome::ISNewText(dev, name, texts, names, n);
}

bool DummyDome::ISSnoopDevice(XMLEle *root)
{
    // TODO: Check to see if this is for any of my custom Snoops. Fo shizzle.

    return INDI::Dome::ISSnoopDevice(root);
}

bool DummyDome::saveConfigItems(FILE *fp)
{
    INDI::Dome::saveConfigItems(fp);

    // TODO: Call IUSaveConfig* for any custom properties I want to save.

    return true;
}

bool DummyDome::Handshake()
{
    if (isSimulation())
    {
        LOGF_INFO("Connected successfuly to simulated %s.", getDeviceName());
        return true;
    }

    // NOTE: PortFD is set by the base class.

    // TODO: Any initial communciation needed with our dome, we have an active
    // connection.

    return true;
}

void DummyDome::TimerHit()
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

bool DummyDome::SetSpeed(double rpm)
{
    // TODO: Set the speed of the dome's rotation. Do not start moving, but if we
    // are already moving, go ahead and change the speed.
    LOGF_INFO("SetSpeed(%f)", rpm);
    return false;
}

IPState DummyDome::Move(DomeDirection dir, DomeMotionCommand operation)
{
    // TODO: Start moving...
    LOGF_INFO("Move(%d, %d)", dir, operation);
    return IPS_ALERT;
}

IPState DummyDome::MoveAbs(double az)
{
    // TODO: Move to an absolute azimuth
    LOGF_INFO("MoveAbs(%f)", az);
    return IPS_ALERT;
}

IPState DummyDome::MoveRel(double azDiff)
{
    // TODO: Move to an relative azimuth
    LOGF_INFO("MoveRel(%f)", azDiff);
    return IPS_ALERT;
}

bool DummyDome::Sync(double az)
{
    // TODO: Sync to the given azimuth
    LOGF_INFO("Sync(%f)", az);
    return false;
}

bool DummyDome::Abort()
{
    // TODO: Stop moving
    LOG_INFO("Abort()");
    return false;
}

IPState DummyDome::Park()
{
    // TODO: Park the dome
    LOG_INFO("Park()");
    return IPS_ALERT;
}

IPState DummyDome::UnPark()
{
    // TODO: UnPark the dome
    LOG_INFO("UnPark()");
    return IPS_ALERT;
}

bool DummyDome::SetBacklash(int32_t steps)
{
    // TODO: Set the backlash compensation
    LOGF_INFO("SetBacklash(%d)", steps);
    return false;
}

bool DummyDome::SetBacklashEnabled(bool enabled)
{
    // TODO: Set the backlash compensation
    LOGF_INFO("SetBacklashEnabled(%d)", enabled);
    return false;
}

IPState DummyDome::ControlShutter(ShutterOperation operation)
{
    // TODO: Open or close the shutter
    LOGF_INFO("ControlShutter(%d)", operation);
    return IPS_ALERT;
}

bool DummyDome::SetCurrentPark()
{
    // TODO: Set the current position to the park position.
    LOG_INFO("SetCurrentPark()");
    return false;
}

bool DummyDome::SetDefaultPark()
{
    // TODO: Set the default park position to the park position.
    LOG_INFO("SetDefaultPark()");
    return false;
}
