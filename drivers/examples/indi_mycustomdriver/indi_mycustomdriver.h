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
    INDI::PropertySwitch SayHelloSP  {SAY_HELLO_N};
    INDI::PropertyText   WhatToSayTP {1};
    INDI::PropertyNumber SayCountNP  {1};

private: // serial connection
    bool Handshake();
    bool sendCommand(const char *cmd);
    int PortFD{-1};

    Connection::Serial *serialConnection{nullptr};
};
