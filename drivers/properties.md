---
sort: 3
---
## Device Properties (Theory)

All communication in INDI is done by updating properties, so this is a really
important part of the tutorial. If you want your driver to do anything at all,
you'll need to understand this concept. This is also the longest part of the
tutorial, but stick with it, or you're gonna have a bad time.

All properties have a `name` and a `label`, as well as a `group` and `state`.

`name` is the code friendly name of the property.

`label` is the human friendly name of the property.

`group` is the human friendly name of the tab the property is on. The `DefaultDevice` class has some nice helpers for commonly used tabs, but feel free to add your own custom tab.

```cpp
const char *COMMUNICATION_TAB = "Communication";
const char *MAIN_CONTROL_TAB  = "Main Control";
const char *CONNECTION_TAB    = "Connection";
const char *MOTION_TAB        = "Motion Control";
const char *DATETIME_TAB      = "Date/Time";
const char *SITE_TAB          = "Site Management";
const char *OPTIONS_TAB       = "Options";
const char *FILTER_TAB        = "Filter Wheel";
const char *FOCUS_TAB         = "Focuser";
const char *GUIDE_TAB         = "Guide";
const char *ALIGNMENT_TAB     = "Alignment";
const char *INFO_TAB          = "General Info";
```

`state` is one of the `IPState` values:

```cpp
typedef enum
{
IPS_IDLE = 0, /*!< State is idle */
IPS_OK,       /*!< State is ok */
IPS_BUSY,     /*!< State is busy */
IPS_ALERT     /*!< State is alert */
} IPState;
```

In general, we define our properties with the idle state. When we are actively doing something, we'll change the property's state to busy, when we've succeeded at doing something, set it to ok, or if there was an error doing something, set it to alert.

All property types EXCEPT Light also have `permission` and `timeout` attributes. These are used to instruct any UI (like the INDI Control Panel) on how to handle them. Light properties are always read only, so they also never need a timeout attribute.

`permission` is one of the `IPerm` values:

```cpp
typedef enum
{
IP_RO, /*!< Read Only */
IP_WO, /*!< Write Only */
IP_RW  /*!< Read & Write */
} IPerm;
```

Read Only means only the driver can update the value of this property, a client cannot update it.

Write Only means the driver will not update the value of this property, only a client can update it.

Read Write means either a client or the driver can update the value of this property.

`timeout` tells the client how long to wait for a response from the driver before assuming the property change errored. In general, clients will set their internal state of a property to busy when the user sets it, and expect the driver to set it back to idle or ok when it was successfully processed.

### Vectors

All properties are vectors (array like object). I'll say that again: ALL PROPERTIES ARE VECTORS.
A property vector can have one or more values.
All values have a `name` and a `label` as well.
There are Number, Text, Switch, Light, and BLOB properties.

#### Number

A Number property is used to represent any numerical value in INDI.

Number values have the following attributes in addition to `name` and `label`:

* `format`
    * This can be any `printf` compatible specifier for a `double`, or for sexagesimal formatting, you can use the INDI special `%m` formatter.
* `min`
    * The minimum value allowed.
* `max`
    * The maximum value allowed.
* `step`
    * A hint for the UI to do steps of values.
* `value`
    * The actual value stored. This will always be a double.

#### Text

A Text property is used to represent any text value in INDI.

Text values have the following attributes in addition to `name` and `label`:

* `text`
    * The actual value stored.

#### Switch

A Switch property is used to represent buttons, checkboxes, and dropdown lists in INDI.

Switch values have the following attributes in addition to `name` and `label`:

* `state`
    * The `ISState` value of the switch. (On or Off)

Switch properties have an additional attribute that the other properties don't have: `rule`. This is one of the `ISRule` values:

```cpp
typedef enum
{
ISR_1OFMANY, /*!< Only 1 switch of many can be ON (e.g. radio buttons) */
ISR_ATMOST1, /*!< At most one switch can be ON, but all switches can be off. It is similar to ISR_1OFMANY with the exception that all switches can be off. */
ISR_NOFMANY  /*!< Any number of switches can be ON (e.g. check boxes) */
} ISRule;
```

#### Light

A Light property is a readonly light that shows in the INDI Control Panel. This is typically used as a status indicator of some sort.

Light values have the following additional attributes in addition to `name` and `label`:

* `state`
    * An `IPState` value (Idle, Ok, Busy, Alert)

#### BLOB

A BLOB property is a way to send binary data through INDI. If you aren't dealing with image data, you likely won't need to touch on it. BLOBs will be covered in a later tutorial.

## Device Properties (Practice)

Now that we've gone through the theory of properties, it's time to see some examples. You did understand all that up there, right? Right!?

So, let's start with something simple, Hello, world!

First we define the class members on our driver responsible for holding the properties.

### indi_mycustomdriver.h

An established naming convention is to end the value list with the first letter of the property type, so `S` in this case, and to end the vector with the first letter of the property type and `P` for property, so `SP` in this case.

We also override the `initProperties` method.

```cpp
public:
    virtual bool initProperties() override;

private:
    ISwitch SayHelloS[1] {}; // This is our array of values for the property.
    ISwitchVectorProperty SayHelloSP; // This is the actual property vector.
```

Now our class has a way to store it, but we need to tell INDI about it, so we move over to our `cpp` file.

### indi_mycustomdriver.cpp

```cpp
bool MyCustomDriver::initProperties()
{
    // initialize the parent's properties first
    INDI::DefaultDevice::initProperties();

    IUFillSwitch(
        &SayHelloS[0], // A reference to the switch VALUE
        "SAY_HELLO",   // The name of the VALUE
        "Say Hello",   // The label of the VALUE
        ISS_OFF        // The switch state
    );

    IUFillSwitchVector(
        &SayHelloSP,      // A reference to the switch PROPERTY
        SayHelloS,        // The list of switch values on this PROPERTY
        1,                // How many switch values are there?
        getDeviceName(),  // The name of the device
        "SAY_HELLO",      // The name of the PROPERTY
        "Hello Commands", // The label of the PROPERTY
        MAIN_CONTROL_TAB, // What tab should we be on?
        IP_RW,            // Let's make it read/write.
        ISR_ATMOST1,      // At most 1 can be on
        60,               // With a timeout of 60 seconds
        IPS_IDLE          // and an initial state of idle.
    );

    // now we register the property with the DefaultDevice
    // without this, the property won't show up on the control panel
    // NOTE: you don't have to call defineProperty here. You can call it at
    // any time. Maybe you don't want it to show until you are connected, or
    // until the user does something else? Maybe you want to connect, query your
    // device, then call this. It's up to you.
    defineProperty(&SayHelloSP);

    return true;
}
```

Now if we build and install the driver again, when we load it up in Ekos and look at the control panel, we see our shiny new button.

But it doesn't do anything at the moment. Let's fix that.

When a user clicks on a switch, a command get's sent through INDI that will trigger the `ISNewSwitch` method on our class (these methods also exist for the other property types; can you guess what they are named?). So we need to override it.

```cpp
public:
    virtual bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[],
                             int n) override;
```

```cpp
bool MyCustomDriver::ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[],
                                 int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        if (strcmp(name, SayHelloSP.name) == 0)
        {
            // Log a message. This will show up in the control panel.
            LOG_INFO("Hello, world!");

            // Turn the switch back off
            SayHelloS[0].s = ISS_OFF;

            // Set the property state back to idle
            SayHelloSP.s = IPS_IDLE;

            // And actually inform INDI of those two operations
            IDSetSwitch(&SayHelloSP, nullptr);

            return true;
        }
    }

    // Nobody has claimed this, so let the parent handle it
    return INDI::DefaultDevice::ISNewSwitch(dev, name, states, names, n);
}
```

Now if we run our driver and click the switch, we can see our message printed in the log!

But what about getting more info into the driver? Let's add a way to customize the text.

```cpp
public:
    virtual bool ISNewText(const char *dev, const char *name, char *texts[], char *names[],
                           int n) override;
private:
    IText WhatToSayT[1] {};
    ITextVectorProperty WhatToSayTP;
```

```cpp
bool MyCustomDriver::initProperties()
{
    ...

    IUFillText(&WhatToSayT[0], "WHAT_TO_SAY", "What to say?", "Hello, world!");
    IUFillTextVector(&WhatToSayTP, WhatToSayT, 1, getDeviceName(), "WHAT_TO_SAY", "Got something to say?", MAIN_CONTROL_TAB, IP_RW, 60, IPS_IDLE);
    defineProperty(&WhatToSayTP);

    return true;
}

// change our log message to
LOG_INFO(WhatToSayT[0].text);


bool MyCustomDriver::ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    // Make sure it is for us.
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        if (strcmp(name, WhatToSayTP.name) == 0)
        {
            WhatToSayTP.s = IPS_IDLE;
            // This is a helper method to just update the values
            // on the property.
            if (!IUUpdateText(&WhatToSayTP, texts, names, n))
            {
                return false;
            }
            IDSetText(&WhatToSayTP, nullptr);
            return true;
        }
    }

    // Nobody has claimed this, so let the parent handle it
    return INDI::DefaultDevice::ISNewText(dev, name, texts, names, n);
}
```

Now when we run the driver, we can change what we say when the button is clicked!

But what if we want to have multiple switch values on a property?

Let's have both a `Say Hello` and `Say Custom` button.

```cpp
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
    ISwitch SayHelloS[SAY_HELLO_N] {};
    ISwitchVectorProperty SayHelloSP;
```

So, now `SayHelloS` is an array with 2 values, instead of the 1 value we had before.
But we need to update `initProperties` to define the new switch value.

```cpp
    IUFillSwitch(
        &SayHelloS[SAY_HELLO_DEFAULT], // A reference to the switch VALUE
        "SAY_HELLO_DEFAULT",           // The name of the VALUE
        "Say Hello",                   // The label of the VALUE
        ISS_OFF                        // The switch state
    );
    IUFillSwitch(
        &SayHelloS[SAY_HELLO_CUSTOM], // A reference to the switch VALUE
        "SAY_HELLO_CUSTOM",           // The name of the VALUE
        "Say Custom",                 // The label of the VALUE
        ISS_OFF                       // The switch state
    );

    IUFillSwitchVector(
        &SayHelloSP,      // A reference to the switch PROPERTY
        SayHelloS,        // The list of switch values on this PROPERTY
        SAY_HELLO_N,      // How many switch values are there?
        getDeviceName(),  // The name of the device
        "SAY_HELLO",      // The name of the PROPERTY
        "Hello Commands", // The label of the PROPERTY
        MAIN_CONTROL_TAB, // What tab should we be on?
        IP_RW,            // Let's make it read/write.
        ISR_ATMOST1,      // At most 1 can be on
        60,               // With a timeout of 60 seconds
        IPS_IDLE          // and an initial state of idle.
    );
```

If you'll notice, we are using the enum values here so we don't have to remember indexes.

And we need to handle the multiple switches in `ISNewSwitch`.

```cpp
        if (strcmp(name, SayHelloSP.name) == 0)
        {
            // Accept what we received.
            IUUpdateSwitch(&SayHelloSP, states, names, n);

            // Find out what switch was clicked.
            int index = IUFindOnSwitchIndex(&SayHelloSP);
            switch (index)
            {
            case SAY_HELLO_DEFAULT: // see how much better this is than direct indexes? USE AN ENUM!
                LOG_INFO("Hello, world!");
                break;
            case SAY_HELLO_CUSTOM:
                LOG_INFO(WhatToSayT[0].text);
                break;
            }

            // Turn all switches back off.
            IUResetSwitch(&SayHelloSP);

            // Set the property state back to idle
            SayHelloSP.s = IPS_IDLE;

            // And actually inform INDI of those two operations
            IDSetSwitch(&SayHelloSP, nullptr);

            return true;
        }
```

Now we can click either button, and either get `Hello, world!` or our custom text. Hooray!

But every time we restart the driver, we go back to the default of `Hello, world!`.
Can we remember these between sessions? Yes we can!

This part's easy. We just need to override `saveConfigItems` to tell it what to save.

```cpp
protected:
    virtual bool saveConfigItems(FILE *fp) override;
```

```cpp
bool MyCustomDriver::saveConfigItems(FILE *fp)
{
    INDI::DefaultDevice::saveConfigItems(fp);
    IUSaveConfigText(fp, &WhatToSayTP);

    return true;
}
```

Now when the user clicks the `Save` button on the `Options` tab, we will save the
current value of WhatToSayTP. Then they can click the `Load` button the next time
the driver starts. If you want to load it for them, you can override ISGetProperties,
and load it there.

```cpp
void MyCustomDriver::ISGetProperties(const char *dev)
{
    DefaultDevice::ISGetProperties(dev);
    loadConfig(true, WhatToSayTP.name);
}
```

So we've handled getting user input from the client into the driver, but how does
the driver inform the client about updates?

To illustrate this, lets add a new "Say Count" property to our driver.

```cpp
// header file
    INumber SayCountN[1]{};
    INumberVectorProperty SayCountNP;
```

```cpp
bool MyCustomDriver::initProperties()
{
    ...
    // and now let's add a counter of how many times the user clicks the button
    IUFillNumber(&SayCountN[0], // First number VALUE in the property (and the only one)
                 "SAY_COUNT",   // name of the VALUE
                 "Count",       // label of the VALUE
                 "%0.f",        // format specifier to show the value to the user; this should be a format specifier for a double
                 0,             // minimum value; used by the client to render the UI
                 0,             // maximum value; used by the client to render the UI
                 0,             // step value; used by the client to render the UI
                 0);            // current value

    IUFillNumberVector(&SayCountNP,      // reference to the number PROPERTY
                       SayCountN,        // Array of number values
                       1,                // count of number values in the array
                       getDeviceName(),  // device name
                       "SAY_COUNT",      // PROPERTY name
                       "Say Count",      // PROPERTY label
                       MAIN_CONTROL_TAB, // What tab should we be on?
                       IP_RO,            // Make this read-only
                       0,                // With no timeout
                       IPS_IDLE);        // and an initial state of idle
    ...
}

bool MyCustomDriver::updateProperties()
{
    INDI::DefaultDevice::updateProperties();

    if (isConnected())
    {
        ...
        defineProperty(&SayCountNP);
    }
    else
    {
        ...
        deleteProperty(SayCountNP.name);
    }

    return true;
}
```

Now we need to update the count every time the user clicks one of the "Say..."
switches.

We know from above that our driver's `ISNewSwitch` method is called when the user
activates a switch, so let's add some code there.

```cpp
bool MyCustomDriver::ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[],
                                 int n)
{
    ...
        if (strcmp(name, SayHelloSP.name) == 0)
        {
            ...
            // Increment our "Say Count" counter.
            // Here we update the value on the property.
            SayCountN[0].value = int(SayCountN[0].value) + 1;
            // And then send a message to the clients to let them know it is updated.
            IDSetNumber(&SayCountNP, nullptr);
            ...
        }
    ...
}
```

We are updating the value of the first VALUE on our PROPERTY, then sending the client
a message showing it was updated, so the client can update the UI.

All of the `IDSet*` functions are used to let the client know that a property was updated.
We are telling the client that "[I]NDI [D]evice [Set] a [Number]" when we call `IDSetNumber`.

See [helpful functions](helpful-functions.md) for more info.

Okay, so now we understand properties a bit. We can use them, save, and load them.
But most drivers will want to connect to something else, and in this hobby, that is
usually over a serial connection.

So let's move on to [the next tutorial](serialconnection.md).
