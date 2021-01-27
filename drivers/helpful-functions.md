## Helpful Functions

### class DefaultDevice

#### Adding common controls to your driver.

These would be called in `initProperties`.

* `addDebugControl`
* `addSimulationControl`
* `addConfigurationControl`
* `addPollPeriodControl`
* `addAuxControls`
    * This one calls all 4 of the above. This is usually the one you want.

#### Defining/Deleting properties.

* `initProperties`
    * Initialize all your properties here.
* `updateProperties`
    * Called when there is a change to the connection state.
* `define*`
    * Define a property to the client & register it.
    * Usually called in either `initProperties` or `updateProperties`.
* `deleteProperty`
    * Delete a property and unregister it. It will also be deleted from all clients.
    * Usually called in `updateProperties` when we get disconnected.

#### Driver Interface

* `setDriverInterface`
    * Used to tell the client what base classes we implement. Usually called in
    `initProperties`.

#### Saving Configuration

* `saveConfigItems`
    * Used to save your properties when the `Save` button is pushed.
    * You will typically call `IUSaveConfig*` for each property you want to save.
    * Be sure to call the base class's method if you override this.
* `saveConfig(bool silent = false, const char *property = nullptr)`
    * Used to save a single (or all) property.
    * Can be used to silently save a property when it is set, so you don't need
    to rely on the user clicking the "Save" button in "Options".

#### Communication

* `registerConnection`
    * Registers a new `Connection::Interface` with the driver.
* `getActiveConnection`
    * Get the active `Connection::Interface`. Most base classes that implement
    connections will call this for you, and set the value of a `PortFD` file
    descriptor.
* `setSupportedConnections`
    * Most device base classes have this method, and it allows you to define
    whether you can connect via serial, tcp, usb, custom, or any combination.

### <libindi/indicom.h>

#### Serial Communication

If a function starts with `tty_` it is used for serial communication. Here are some
useful ones you will probably need.

* `tty_read`
* `tty_read_section`
* `tty_write`
* `tty_write_string`
* `tty_error_msg`

#### Formatting functions

* `numberFormat`
    * This will properly format a number value given the format on the number property.

#### Range functions

* `rangeHA`
    * Limits the hour angle value to be between -12 ---> 12
* `range24`
    * Limits a number to be between 0-24 range.
* `range360`
    * Limits an angle to be between 0-360 degrees.
* `rangeDec`
    * Limits declination value to be in -90 to 90 range.

### <libindi/indidevapi.h>

#### INDI Driver Functions

* `IDSet*` Functions
    * These are used to inform the client about updates to a property. If you
    change the property's values or state, and don't call this function, the
    client won't see it.
    * `IDSetText`
    * `IDSetNumber`
    * `IDSetSwitch`
    * `IDSetLight`
    * `IDSetBLOB`
* `IDLog`
    * Used to log a message locally (not sent to the client).
    * You will not see output from this function unless you add a newline manually.
* `IDSnoopDevice`
    * Used to snoop on another device's property. When the other device updates
    the matching property, `ISSnoopDevice` will be called in your driver.

#### INDI Event Functions

* `int IEAddCallback(int readfiledes, IE_CBF *fp, void *userpointer)`
    * This will call the function `fp` when the file descriptor `readfiledes`
    is ready to read. `userpointer` will be passed to `fp` when called.
    * The return value is an identifier that can be used to...
* `void IERmCallback(int callbackid)`
    * Removes a callback.

* `int IEAddTimer(int millisecs, IE_TCF *fp, void *userpointer)`
    * This will call the function `fp` after `millisecs`. It will only call
    `fp` once. You will need to add the timer again if you want a loop.
    * The return value is an identifier that can be used to...
* `void IERmTimer(int timerid)`
    * Removes a timer.

#### INDI Utility Functions

These are meant to be called by a driver. These do NOT send any messages to a client.

* `IUFind*`
    * Finds a property value in a property vector with the given name.
* `IUFindOnSwitch`
    * Find the first switch in a property vector that is On.
* `IUResetSwitch`
    * Turns all switch values in the property vector Off.
* `IUUpdate*`
    * Useful to set multiple values on a property vector at the same time.
    * Really handy in the `ISNew*` methods of your driver if you just want to
    accept the values the client sent you.
* `IUSaveText`
    * Used to set a text property value. Handles a `realloc` for you.
* `IUFill*`
    * Used in `initProperties` to initialize your properties and their values.
* `IUSnoop*`
    * Used in `ISSnoopDevice` to set the value of a local property to match the
    snooped value you just received.
    * If you use `IDSnoopDevice`, you'll probably want to use this as well.

#### IS Functions

If a function starts with `IS`, it should NEVER be called by a driver, but is used
to receive messages from a client. All drivers MUST define these.

* `ISGetProperties`
    * This function is called by the framework whenever the driver has received a
    getProperties message from an INDI client.
* `ISNew*`
    * Update the value of an existing property.
