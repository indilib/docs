#pragma once

#include "libindi/indidome.h"

namespace Connection
{
    class Serial;
}

class DummyDome : public INDI::Dome
{
public:
    DummyDome();
    virtual ~DummyDome() = default;

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

    virtual bool Handshake() override;

    // The need for these methods depends on what capabilities you set for your
    // dome in initProperties.
    virtual bool SetSpeed(double rpm) override;
    virtual IPState Move(DomeDirection dir, DomeMotionCommand operation) override;
    virtual IPState MoveAbs(double az) override;
    virtual IPState MoveRel(double azDiff) override;
    virtual bool Sync(double az) override;
    virtual bool Abort() override;
    virtual IPState Park() override;
    virtual IPState UnPark() override;
    virtual bool SetBacklash(int32_t steps) override;
    virtual bool SetBacklashEnabled(bool enabled) override;
    virtual IPState ControlShutter(ShutterOperation operation) override;
    virtual bool SetCurrentPark() override;
    virtual bool SetDefaultPark() override;
};
