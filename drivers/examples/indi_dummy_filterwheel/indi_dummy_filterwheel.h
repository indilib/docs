#pragma once

#include "libindi/indifilterwheel.h"

namespace Connection
{
    class Serial;
}

class DummyFilterWheel : public INDI::FilterWheel
{
public:
    DummyFilterWheel();
    virtual ~DummyFilterWheel() = default;

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
    virtual int QueryFilter() override;
    virtual bool SelectFilter(int) override;
    virtual bool SetFilterNames() override;
    virtual bool GetFilterNames() override;
};
