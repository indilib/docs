#pragma once

#include "libindi/defaultdevice.h"
#include "libindi/indidustcapinterface.h"

namespace Connection
{
    class Serial;
}

class DummyDustcap : public INDI::DefaultDevice, public INDI::DustCapInterface
{
public:
    DummyDustcap();
    virtual ~DummyDustcap() = default;

    virtual const char *getDefaultName() override;

    virtual bool initProperties() override;
    virtual bool updateProperties() override;

    virtual void ISGetProperties(const char *dev) override;
    virtual bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) override;
    virtual bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n) override;
    virtual bool ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n) override;
    virtual bool ISSnoopDevice(XMLEle *root) override;

    virtual void TimerHit() override;

protected:
    virtual bool saveConfigItems(FILE *fp) override;

    virtual IPState ParkCap() override;
    virtual IPState UnParkCap() override;

private: // serial connection
    bool Handshake();
    bool sendCommand(const char *cmd);
    int PortFD{-1};

    Connection::Serial *serialConnection{nullptr};
};
