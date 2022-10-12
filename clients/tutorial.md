---
sort: 1
---
# Client Development Tutorial

In this short tutorial, our client (`tutorial_client`) needs connect to an INDI server which may be running several drivers.
However, we only need to receive information regarding the "Simple CCD" driver. Once received, we try to set the temperature to -20 C.
In order to establish a client, we need to subclass `INDI::BaseClient`.

To manipulate the selected device, we must first access it using the `watchDevice` method as the device can appear dynamically.
The properties of the device are also dynamic, that is, they can come and go. To obtain access to selected properties, we use the `watchProperty` method, which informs us about availability and passes an object on which we can perform the operation of reading and writing the value.

The source for this tutorial can be found [here](https://github.com/indilib/indi/tree/master/examples/tutorial_six).

## Building
Let's follow the example code:
### tutorial_client.h

```cpp
class MyClient : public INDI::BaseClient
{
    public:
        MyClient();
        ~MyClient() = default;

    public:
        void setTemperature(double value);
        void takeExposure(double seconds);

    protected:
        void newMessage(INDI::BaseDevice *dp, int messageID) override;

    private:
        INDI::BaseDevice mSimpleCCD;
};
```
Above, we declare our class with the methods of interest to us.
By inheriting from the `INDI::BaseClient` class, we get a whole set of functionalities that will allow us to communicate with the INDI server and to manipulate drivers.
Additionally, the overwritten method `newMessage` will allow us to read messages from the server.

### tutorial_client.cpp

```cpp
int main(int, char *[])
{
    MyClient myClient;
    myClient.setServer("localhost", 7624);

    myClient.connectServer();

    myClient.setBLOBMode(B_ALSO, "Simple CCD", nullptr);

    myClient.enableDirectBlobAccess("Simple CCD", nullptr);

    std::cout << "Press Enter key to terminate the client.\n";
    std::cin.ignore();
}
```

In `main()`, we initiate the client object as myClient, then we set the server parameters.
Note that by default, `INDI::BaseClient` will connect to an INDI server running on localhost and on port 7624.

The whole process of detecting the device and properties we are interested in is entered in the constructor of our class,
so that after connecting to the server, the `INDI::BaseClient` class can read the received information and inform us.

```cpp
MyClient::MyClient()
{
    // wait for the availability of the "Simple CCD" device
    watchDevice("Simple CCD", [](INDI::BaseDevice device)
    {
        // wait for the availability of the "CONNECTION" property
        device.watchProperty("CONNECTION", [](INDI::Property)
        {
            /* ... */
        });

        // wait for the availability of the "CCD_TEMPERATURE" property
        device.watchProperty("CCD_TEMPERATURE", [](INDI::PropertyNumber)
        {
            /* ... */
        });

        // wait for the availability of the "CCD1" property
        device.watchProperty("CCD1", [](INDI::PropertyBlob)
        {
            /* ... */
        });
    });
}
```

The names of the observed properties result from the structure of the given driver.
In our case, for the `Simple CCD` driver, we are interested in:
- `CONNECTION` - property of type `INDI::PropertySwitch`, however, if we are not interested in the property, but in its appearance, we can omit the variable name in the argument as its type, using the generic `INDI::Property`
- `CCD_TEMPERATURE` - property of type `INDI::PropertyNumber`, to which we can enter the set temperature of the camera and read the current one
- `CCD1` - property of type `INDI::PropertyBlob`, which stores the captured photo from the camera

An important note to consider is that there is no way a client can tell whether or not we have received all the properties of a particular driver. This is because of the very nature of INDI protocol where devices are discovered via introspection. Therefore, the client may either choose to wait for a period of time until it begins processing the driver, or watch for a particular property of interest. Since we are planning to change the CCD temperature, we are interested in [INDI Standard Property](../drivers/README.md#standard-properties) `CCD_TEMPERATURE` which we will watch for.

To detect the change of a given property, we use the `onUpdate` method, which calls the function we specified when the property is updated:
```cpp
device.watchProperty("CCD_TEMPERATURE", [](INDI::PropertyNumber property)
{
    // call lambda function if property changed
    property.onUpdate([]()
    {
        // property changed!
    });
});

```

Now let's add an implementation that provides us with:
1. after the appearance of the `CONNECT` property, connect to the device
1. after the appearance of the property `CCD_TEMPERATURE`, set the temperature to -20 C
1. when changing the property `CCD_TEMPERATURE`, display the current temperature, and when it reaches -20 C, take a picture
1. if there is a new value for the property `CCD1`, save the picture to disk

```cpp
MyClient::MyClient()
{
    // wait for the availability of the device
    watchDevice("Simple CCD", [this](INDI::BaseDevice device)
    {
        mSimpleCCD = device; // save device

        // wait for the availability of the "CONNECTION" property
        device.watchProperty("CONNECTION", [this](INDI::Property)
        {
            IDLog("Connecting to INDI Driver...\n");
            connectDevice("Simple CCD");
        });

        // wait for the availability of the "CCD_TEMPERATURE" property
        device.watchProperty("CCD_TEMPERATURE", [this](INDI::PropertyNumber property)
        {
            if (mSimpleCCD.isConnected())
            {
                IDLog("CCD is connected.\n");
                setTemperature(-20);
            }

            // call lambda function if property changed
            property.onUpdate([property, this]()
            {
                IDLog("Receving new CCD Temperature: %g C\n", property[0].getValue());
                if (property[0].getValue() == -20)
                {
                    IDLog("CCD temperature reached desired value!\n");
                    takeExposure(1);
                }
            });
        });

        // wait for the availability of the "CCD1" property
        device.watchProperty("CCD1", [](INDI::PropertyBlob property)
        {
            // call lambda function if property changed
            property.onUpdate([property]()
            {
                // Save FITS file to disk
                std::ofstream myfile;

                myfile.open("ccd_simulator.fits", std::ios::out | std::ios::binary);
                myfile.write(static_cast<char *>(property[0].getBlob()), property[0].getBlobLen());
                myfile.close();

                IDLog("Received image, saved as ccd_simulator.fits\n");
            });
        });
    });
}
```

We have already implemented device detection, reading properties and information about its change.
Now let's move on to setting property values.

```cpp
void MyClient::setTemperature(double value)
{
    INDI::PropertyNumber ccdTemperature = mSimpleCCD.getProperty("CCD_TEMPERATURE");

    if (!ccdTemperature.isValid())
    {
        IDLog("Error: unable to find Simple CCD, CCD_TEMPERATURE property...\n");
        return;
    }

    IDLog("Setting temperature to %g C.\n", value);
    ccdTemperature[0].setValue(value);
    sendNewProperty(ccdTemperature);
}
```

Here we use the `mSimpleCCD` variable that we got while watching the device to look for the property `CCD_TEMPERATURE` in order to change it.
The `INDI::BaseDevice` and `INDI::PropertyXXX` classes are constructed in such a way that in the absence of a given property, the `isValid` method will return false.
Then we set a new value and send it to the driver.

Finally, we should be expecting the driver to comply and update the `CCD_TEMPERATURE` property.

## Running

Open two console windows, and in each go to libindi cmake build directory (e.g. `/home/jsmith/libindi/build`) as these tutorials do not get installed to `/usr/bin`. On the first console, run `indiserver` with the Simple CCD which is [tutorial_three](https://github.com/indilib/indi/tree/master/examples/tutorial_three):

```bash
indiserver ./tutorial_three
```

On the second console, run tutorial_client:

```bash
./tutorial_client
```

If everything works fine, you should be getting the following output:

```
Press Enter key to terminate the client.
Connecting to INDI Driver...
CCD is connected.
Setting temperature to -20 C.
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
Taking a 1 second exposure.
Received image, saved as ccd_simulator.fits
```

That's it! It's that easy to write a client!

Receving BLOBs: By default, INDI server does not send BLOBs to client unless the client explicitly sets BLOB handling mode by making a call to [`setBLOBMode()`](http://www.indilib.org/api/classINDI_1_1BaseClient.html#adb3740470ff9f9136f9a57e5560cd0ae) function. You can limit BLOB mode to a specific device and/or property.
