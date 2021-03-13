#pragma once

#include "libindi/defaultdevice.h"

namespace Connection
{
    class Serial;
}

class MyCustomDriver : public INDI::DefaultDevice
{
public:
    MyCustomDriver();
    virtual ~MyCustomDriver() = default;

    virtual const char *getDefaultName() override;

    virtual bool initProperties() override;
    virtual bool updateProperties() override;

    virtual void ISGetProperties(const char *dev) override;
    virtual bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[],
                             int n) override;
    virtual bool ISNewText(const char *dev, const char *name, char *texts[], char *names[],
                           int n) override;

    virtual void TimerHit() override;

protected:
    virtual bool saveConfigItems(FILE *fp) override;

private:
    // Use the inherent autoincrementing of an enum to generate our indexes.
    // This makes keeping track of multiple values on a property MUCH easier
    // than remembering indexes throughout your code.
    // The last value _N is used as the total count.
    enum
    {
        SAY_HELLO_DEFAULT,
        SAY_HELLO_CUSTOM,
        SAY_HELLO_N,
    };
    ISwitch SayHelloS[SAY_HELLO_N]{};
    ISwitchVectorProperty SayHelloSP;

    IText WhatToSayT[1]{};
    ITextVectorProperty WhatToSayTP;

    INumber SayCountN[1]{};
    INumberVectorProperty SayCountNP;

private: // serial connection
    bool Handshake();
    bool sendCommand(const char *cmd);
    int PortFD{-1};

    Connection::Serial *serialConnection{nullptr};
};
