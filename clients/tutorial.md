---
sort: 1
---
# Client Development Tutorial

In this short tutorial, our client (`tutorial_client`) needs connect to an INDI server which may be running several drivers. However, we only need to receive information regarding the "CCD Simulator" driver. Once received, we try to set the temperature to -20 C. In order to establish a client, we need to subclass `INDI::BaseClient` and extend it as necessary.

The source for this tutorial can be found [here](https://github.com/indilib/indi/tree/master/examples/tutorial_six).

## Building

### myclient.h

```cpp
class MyClient : public INDI::BaseClient
{
 public:
    MyClient();
    ~MyClient();
    void setTemperature();
protected:
    virtual void newDevice(INDI::BaseDevice *dp);
    virtual void removeDevice(INDI::BaseDevice *dp) {}
    virtual void newProperty(INDI::Property *property);
    virtual void removeProperty(INDI::Property *property) {}
    virtual void newBLOB(IBLOB *bp) {}
    virtual void newSwitch(ISwitchVectorProperty *svp) {}
    virtual void newNumber(INumberVectorProperty *nvp);
    virtual void newMessage(INDI::BaseDevice *dp, int messageID);
    virtual void newText(ITextVectorProperty *tvp) {}
    virtual void newLight(ILightVectorProperty *lvp) {}
    virtual void serverConnected() {}
    virtual void serverDisconnected(int exit_code) {}
private:
   INDI::BaseDevice * ccd_simulator;
};
```

Above we declared all the functions from `INDI::BaseMediator`. We will only implement a few of those for the purpose of the tutorial.

### main.cpp

```cpp
#define MYCCD "Simple CCD"
/* Our client auto pointer */
auto_ptr camera_client(0);

int main(int argc, char *argv[])
{
    if (camera_client.get() == 0)
        camera_client.reset(new MyClient());

    camera_client->setServer("localhost", 7624);
    camera_client->watchDevice(MYCCD);
    camera_client->connectServer();
    cout << "Press any key to terminate the client.\n";
    string term;
    cin >> term;
}
```

In `main()`, we initiate the client object camera_client, then we set the server parameters. Note that by default, `INDI::BaseClient` will connect to an INDI server running on localhost and on port 7624. Next, we ask the client to only watch for a specific device. In case an INDI server is running multiple drivers, we are only interested in one or more devices. If `watchDevice()` is not set, `INDI::BaseClient` will fetch all devices from the INDI servers and build virtual device for each.

Finally, we connect to the server. Note that we're not checking for any error messages here to make it simple.

What happens next is as following:

1. `INDI::BaseClient` will attempt to connect to an INDI server.
1. If successful, it issues a `getProperties` command to INDI server with the device CCD Simulator.
1. Upon reception of the first device property from CCD Simulator, it issues newDevice and newProperty notifications.

### myclient.cpp

Here we ensure that the device received is indeed our simulator. Then we ask the client to gives us a pointer to the driver's instance for future use in the class.

```cpp
void MyClient::newDevice(INDI::BaseDevice *dp)
{
    if (!strcmp(dp->getDeviceName(), MYCCD))
    {
        IDLog("Receiving %s Device...\n", dp->getDeviceName());
        ccd_simulator = dp;
    }
}
```

An important note to consider is that there is no way a client can tell whether or not we have received all the properties of a particular driver. This is because of the very nature of INDI protocol where devices are discovered via introspection. Therefore, the client may either choose to wait for a period of time until it begins processing the driver, or watch for a particular property of interest. Since we are planning to change the CCD temperature, we are interested in [INDI Standard Property](../drivers/README.md#standard-properties) `CCD_TEMPERATURE` which we will watch for in `newProperty()`.

```cpp
void MyClient::newProperty(INDI::Property *property)
{
    if (!strcmp(property->getDeviceName(), MYCCD) && !strcmp(property->getName(), "CONNECTION"))
    {
        connectDevice(MYCCD);
        return;
    }

    if (!strcmp(property->getDeviceName(), MYCCD) && !strcmp(property->getName(), "CCD_TEMPERATURE"))
    {
        if (ccd_simulator->isConnected())
        {
            IDLog("CCD is connected. Setting temperature to -20 C.\n");
            setTemperature();
        }
        return;
    }
}
```

Here we ensure we are receiving the correct property from the correct device, then we issue our first command to the CCD driver: `Connect`. If `setDriverConnection` is passed false instead, the client will issue a `Disconnect` command. Once the device is connected, it will define other properties, and we are interested in `CCD_TEMPERATURE` property. Once this property is defined, we ensure that the device is still connected, and then issue `setTemperature()` command.

```cpp
void MyClient::setTemperature()
{
   INumberVectorProperty *ccd_temperature = NULL;
   ccd_temperature = ccd_simulator->getNumber("CCD_TEMPERATURE");
   if (ccd_temperature == NULL)
   {
       IDLog("Error: unable to find CCD Simulator CCD_TEMPERATURE property...\n");
       return;
   }
   ccd_temperature->np[0].value = -20;
   sendNewNumber(ccd_temperature);
}
```

Here we get a pointer to the `CCD_TEMPERATURE` numeric property. If found (which we should since we just received it in `newProperty`), we set it to -20, and then call `sendNewNumber` to send the new value to the driver.

Finally, we should be expecting the driver to comply and update the `CCD_TEMPERATURE` property.

```cpp
void MyClient::newNumber(INumberVectorProperty *nvp)
{
    // Let's check if we get any new values for CCD_TEMPERATURE
    if (!strcmp(nvp->name, "CCD_TEMPERATURE"))
    {
       IDLog("Receving new CCD Temperature: %g C\n", nvp->np[0].value);
       if (nvp->np[0].value == -20)
           IDLog("CCD temperature reached desired value!\n");
   }
}
```

## Running

Open two console windows, and in each go to libindi cmake build directory (e.g. `/home/jsmith/libindi/build`) as these tutorials do not get installed to `/usr/bin`. On the first console, run `indiserver` with the CCD Simulator which is [tutorial_three](https://github.com/indilib/indi/tree/master/examples/tutorial_three):

```bash
indiserver ./tutorial_three
```

On the second console, run tutorial_client:

```bash
./tutorial_client
```

If everything works fine, you should be getting the following output:

```
Press any key to terminate the client.
Receiving CCD Simulator Device...
CCD_TEMPERATURE standard property defined. Attempting connection to CCD...
CCD is connected. Setting temperature to -20 C.
Receving new CCD Temperature: 0 C
Receving new CCD Temperature: -1 C
Receving new CCD Temperature: -2 C
Receving new CCD Temperature: -3 C
Receving new CCD Temperature: -4 C
Receving new CCD Temperature: -5 C
Receving new CCD Temperature: -6 C
Receving new CCD Temperature: -7 C
Receving new CCD Temperature: -8 C
Receving new CCD Temperature: -9 C
Receving new CCD Temperature: -10 C
Receving new CCD Temperature: -11 C
Receving new CCD Temperature: -12 C
Receving new CCD Temperature: -13 C
Receving new CCD Temperature: -14 C
Receving new CCD Temperature: -15 C
Receving new CCD Temperature: -16 C
Receving new CCD Temperature: -17 C
Receving new CCD Temperature: -18 C
Receving new CCD Temperature: -19 C
Receving new CCD Temperature: -20 C
CCD temperature reached desired value!
Receving new CCD Temperature: -20 C
CCD temperature reached desired value!
```

That's it! tt's that easy to write a client!

Receving BLOBs: By default, INDI server does not send BLOBs to client unless the client explicitly sets BLOB handling mode by making a call to [`setBLOBMode()`](http://www.indilib.org/api/classINDI_1_1BaseClient.html#adb3740470ff9f9136f9a57e5560cd0ae) function. You can limit BLOB mode to a specific device and/or property.
