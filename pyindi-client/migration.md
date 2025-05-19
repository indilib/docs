---
title: Migration
nav_order: 1
parent: PyIndi Client
permalink: /pyindi-client/migration/
---

# Migration to INDI Core 2.0.0

This documentation is only for those using an old version of INDI and now trying to migrate to the INDI Core 2.0.0

## For all property

Applies to all properties available in the library (Switch/Number/Text/Light/Blob)

New implementation since INDI Core 1.9.x

```python
# set
anyProperty.setDeviceName("Some device")
anyProperty.setName("Some name")
anyProperty.setLabel("Some label")
anyProperty.setGroupName("Some group")
anyProperty.setState(PyIndi.IPS_IDLE)
anyProperty.setTimestamp("123")
anyProperty.setPermission(PyIndi.IP_RO) # no effect for Light Property
anyProperty.setTimeout(123)             # no effect for Light Property

anyProperty[0].setName("Some name of widget")
anyProperty[0].setLabel("Some label of widget")

# get
device    = anyProperty.getDeviceName()
name      = anyProperty.getName()
label     = anyProperty.getLabel()
group     = anyProperty.getGroupName()
state     = anyProperty.getState()
timestamp = anyProperty.getTimestamp()
perm      = anyProperty.getPermission() # returns IP_RO for Light Property
timeout   = anyProperty.getTimeout()    # returns 0 for Light Property

name      = anyProperty[0].getName()
label     = anyProperty[0].getLabel()

# auxiliary functions
if anyProperty.isNameMatch("Some name"):
    # anyProperty.getName() is equal to "Some name"
    pass

if anyProperty.isLabelMatch("Some label"):
    # anyProperty.getLabel() is equal to "Some label"
    pass

if not anyProperty.isValid():
    # e.g. PyIndi.Property() is not valid because type is unknown
    # PyIndi.PropertyText(somePropertySwitch) is also not valid because type
    # is mismatch (invalid cast)
    pass

stringState  = anyProperty.getStateAsString()            # returns Idle/Ok/Busy/Alert
stringPerm   = anyProperty.getPermissionAsString()       # returns ro/wo/rw
someWidget   = anyProperty.findWidgetByName("Some name") # returns widget with `Some name` name
```

Deprecated implementation

```python
# set
anyProperty.device    = "Some device"
anyProperty.name      = "Some name"
anyProperty.label     = "Some label"
anyProperty.group     = "Some group"
anyProperty.s         = PyIndi.IPS_IDLE
anyProperty.timestamp = "123"
anyProperty.p         = PyIndi.IP_RO # except Light Property
anyProperty.timeout   = 123          # except Light Property

anyProperty[0].name   = "Some name of widget"
anyProperty[0].label  = "Some label of widget"

# get
device    = anyProperty.device
name      = anyProperty.name
label     = anyProperty.label
group     = anyProperty.group
state     = anyProperty.s
timestamp = anyProperty.timestamp
perm      = anyProperty.p       # except Light Property
timeout   = anyProperty.timeout # except Light Property

name  = anyProperty[0].name
label = anyProperty[0].label
```

## Property Text

New implementation since INDI Core 1.9.x

```python
# set
textProperty[0].setText("Some text")

# get
text  = textProperty[0].getText()
```

Deprecated implementation

```python
# set
textProperty[0].text  = "Some text"

# get
text  = textProperty[0].text
```

## Property Number

New implementation since INDI Core 1.9.x

```python
# set
numberProperty[0].setFormat("Some format")
numberProperty[0].setMin(0)
numberProperty[0].setMax(1000)
numberProperty[0].setMinMax(0, 1000) # simplification
numberProperty[0].setStep(1)
numberProperty[0].setValue(123)

# get
format = numberProperty[0].getFormat()
min    = numberProperty[0].getMin()
max    = numberProperty[0].getMax()
step   = numberProperty[0].getStep()
value  = numberProperty[0].getValue()
```

Deprecated implementation

```python
# set
numberProperty[0].format = "Some format"
numberProperty[0].min = 0
numberProperty[0].max = 1000
numberProperty[0].step = 1
numberProperty[0].value = 123

# get
format = numberProperty[0].format
min    = numberProperty[0].min
max    = numberProperty[0].max
step   = numberProperty[0].step
value  = numberProperty[0].value
```

## Property Switch

New implementation since INDI Core 1.9.x

```python
# set
switchProperty.setRule(PyIndi.ISR_NOFMANY)
switchProperty[0].setState(PyIndi.ISS_ON)

# get
rule  = switchProperty.getRule()
state = switchProperty[0].getState()

# auxiliary functions
stringRule  = switchProperty.getRuleAsString()     # returns OneOfMany/AtMostOne/AnyOfMany
stringState = switchProperty[0].getStateAsString() # returns On/Off
switchProperty.reset()                             # reset all widget switches to Off
switchProperty.findOnSwitchIndex()                 # find index of Widget with On state
switchProperty.findOnSwitch()                      # returns widget with On state
```

Deprecated implementation

```python
# set
switchProperty.r = PyIndi.ISR_NOFMANY
switchProperty[0].s = PyIndi.ISS_ON

# get
rule  = switchProperty.r
state = switchProperty[0].s
```

## Property Light

New implementation since INDI Core 1.9.x

```python
# set
lightProperty[0].setState(PyIndi.IPS_BUSY)

# get
lightState = lightProperty[0].getState()

# auxiliary functions
stringState = lightProperty[0].getStateAsString() # returns Idle/Ok/Busy/Alert
```

Deprecated implementation

```python
# set
lightProperty[0].s = PyIndi.IPS_BUSY

# get
lightState = lightProperty[0].s
```

## Property Blob

New implementation since INDI Core 1.9.x

```python
# set
blobProperty[0].setFormat("Some format")
blobProperty[0].setBlob("Some blob")
blobProperty[0].setBlobLen(123)
blobProperty[0].setSize(123)

# get
format  = blobProperty[0].getFormat()
blob    = blobProperty[0].getBlob()
bloblen = blobProperty[0].getBlobLen()
size    = blobProperty[0].getSize()
```

Deprecated implementation

```python
# set
blobProperty[0].format  = "Some format"
blobProperty[0].blob    = "Some blob"
blobProperty[0].bloblen = 123
blobProperty[0].size    = 123

# get
format  = blobProperty[0].format
blob    = blobProperty[0].blob
bloblen = blobProperty[0].bloblen
size    = blobProperty[0].size
```

## BaseClient virual methods

As of INDI Core 2.0.0, there are no features like:

- newText
- newSwitch
- newNumber
- newLight
- newBlob

They have been replaced with a single `updateProperty` function to differentiate the behavior between `newProperty` and to provide more memory control in the INDI Core library itself.

It is not required to implement all functions. Please use as needed.

```python
class IndiClient(PyIndi.BaseClient):
    def __init__(self):
        super(IndiClient, self).__init__()

    # Emmited when a new device is created from INDI server.
    def newDevice(self, dev):
        pass

    # Emmited when a device is deleted from INDI server.
    def removeDevice(self, dev):
        pass

    # Emmited when a new property is created for an INDI driver.
    def newProperty(self, genericProperty):
        pass

    # Emmited when a new property value arrives from INDI server.
    def updateProperty(self, genericProperty):
        pass

    # Emmited when a property is deleted for an INDI driver.
    def removeProperty(self, genericProperty):
        pass

    # Emmited when a new message arrives from INDI server.
    def newMessage(self, device, messageId):
        pass

    # Emmited when the server is connected.
    def serverConnected(self):
        pass

    # Emmited when the server gets disconnected.
    def serverDisconnected(self, exitCode):
        pass
```

The function `newProperty` and `updateProperty` provide in argument a generic type in which you can read common information for all properties such as name, group, device, type, state. To read values associated with a specific type, e.g. `Switch`, you need to perform cast.

```python
switchProperty = PyIndi.PropertySwitch(genericProperty)
```

If successful, the function `switchProperty.isValid()` will return `true`.

A similar mechanism existed before version 2.0.0 for the `newProperty` function.

### BaseClient Example

With Python, migration can be simple. Nothing stands in the way of creating a function for a given property type and naming it the same as for the INDI Core 1.x.x version

```python
class IndiClient(PyIndi.BaseClient):
    def __init__(self):
        super(IndiClient, self).__init__()

    # Call functions in old style
    def updateProperty(self, prop):
        if prop.getType() == PyIndi.INDI_NUMBER:
            self.newNumber(PyIndi.PropertyNumber(prop))
        elif prop.getType() == PyIndi.INDI_SWITCH:
            self.newSwitch(PyIndi.PropertySwitch(prop))
        elif prop.getType() == PyIndi.INDI_TEXT:
            self.newText(PyIndi.PropertyText(prop))
        elif prop.getType() == PyIndi.INDI_LIGHT:
            self.newLight(PyIndi.PropertyLight(prop))
        elif prop.getType() == PyIndi.INDI_BLOB:
            self.newBLOB(PyIndi.PropertyBlob(prop)[0])

    # The methods work again in INDI Core 2.0.0!
    def newBLOB(self, prop):
        print(f"new BLOB {prop.getName()}")
    def newSwitch(self, prop):
        print(f"new Switch {prop.getName()} for device {prop.getDeviceName()}")
    def newNumber(self, prop):
        print(f"new Number {prop.getName()} for device {prop.getDeviceName()}")
    def newText(self, prop):
        print(f"new Text {prop.getName()} for device {prop.getDeviceName()}")
    def newLight(self, prop):
        print(f"new Light {prop.getName()} for device {prop.getDeviceName()}")
```
