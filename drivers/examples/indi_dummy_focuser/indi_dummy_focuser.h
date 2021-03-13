#pragma once

#include "libindi/indifocuser.h"

class DummyFocuser : public INDI::Focuser
{
public:
    DummyFocuser();
    virtual ~DummyFocuser() = default;

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

    virtual IPState MoveFocuser(FocusDirection dir, int speed, uint16_t duration);
    virtual IPState MoveAbsFocuser(uint32_t targetTicks);
    virtual IPState MoveRelFocuser(FocusDirection dir, uint32_t ticks);
    virtual bool AbortFocuser();
};
