---
title: Inter-Driver Communication
nav_order: 1
parent: Advanced
---

## Inter-Driver Communication

INDI Library provides the facility to snoop on properties in other drivers. This permits robust inter-driver communication that can be useful in many applications. Typically, all the properties defined by a driver are available for snooping by other drivers. Therefore, all drivers are by default publishers, whereas the snooping drivers are subscribers.

![snoop](../images/snoop.png)

When a subscriber driver needs to subscribe to a property, it calls an API function with the snooped (publisher) device and property names:

```cpp
IDSnoopDevice (snooped_device, snooped_property)
```

Whenever the snooped_device updates its snooped_property, the INDI framework notifies the subscriber of the change by calling the ISSnoopDevice() function which is defined by the subscriber driver.

In ISSnoopDevice, the driver receives an XML element for the new property value which it can pass to a number of handy utility routines for processing and information extraction.

For instance, suppose a driver defines a simple switch MountType to switch between mount types:

Header file:

```cpp
INDI::PropertySwitch MountTypeSP {2};
enum
{
  MOUNT_GEM,
  MOUNT_SINGLE_ARM
};
```

Implementation file under initProperties():

```cpp
MountTypeSP[MOUNT_GEM].fill("MOUNT_GEM", "GEM", ISS_ON);
MountTypeSP[MOUNT_SINGLE_ARM].fill("MOUNT_SINGLE_ARM", "Single ARM", ISS_OFF);
MountTypeSP.fill(getDeviceName(), "MOUNT_TYPE", "Mount Type", MAIN_CONTROL_TAB, IP_RW, ISR_1OFMANY, 60, IPS_IDLE);
```

The subscribed driver can start _snooping_ on the property by issuing the following command, usually in its initProperties() function:

```
IDSnoopDevice("AstroTrac", "MOUNT_TYPE")
```

This will cause the server to start monitoring this particular property as it gets updated, and would relay all subsequent updates to the subscribed driver.

When the snooped property arrives at the subscribed driver, it arrives as an XML INDI Protocol message, for example:

```xml
<setSwitchVector device="AstroTrac" name="MOUNT_TYPE" state="Ok" timeout="60" timestamp="2021-07-12T07:04:31">
    <oneSwitch name="MOUNT_GEM">
Off
    </oneSwitch>
    <oneSwitch name="MOUNT_SINGLE_ARM">
On
    </oneSwitch>
</setSwitchVector>
```

There are two methods to parse the snooped property in the subscriber driver:

## Process XML Message

The subscriber driver processes the root XML elements for properties it is interested in fetching. In ISSnoopDevice, we can have the following:

```
void ISSnoopDevice (XMLEle *root)
{
   const char *propName = findXMLAttValu(root, "name");
   if (!strcmp(propName, "MOUNT_TYPE"))
   {
        // set default value
        int mountType = MOUNT_GEM;
        //  crack the message
        for (ep = nextXMLEle(root, 1); ep != nullptr; ep = nextXMLEle(root, 0))
        {
            // Get element name
            const char * elemName = findXMLAttValu(ep, "name");
            // Check if element and state match
            if (!strcmp(elemName, "MOUNT_GEM") && !strcmp(pcdataXMLEle(ep), "On"))
                mountType = MOUNT_GEM;
            else if (!strcmp(elemName, "MOUNT_SIGNEL_ARM") && !strcmp(pcdataXMLEle(ep), "On"))
                mountType = MOUNT_SINGLE_ARM;
        }
   }
}
```

## Carbon Copy

Alternatively, the subscriber driver may define a _carbon_ copy of the snooped property so that it can use the utility functions to fetch the XML element into it. The subscriber driver simply needs to copy that definition to its source file
In ISSnoopDevice, we can have the following:

```
void ISSnoopDevice (XMLEle *root)
{
   if (IUSnoopSwitch(root, &MountTypeSP) == 0)
   {
       if (MountTypeSP[MOUNT_GEM].getState() == ISS_ON)
          LOG_INFO("Mount is GEM!);
       else
          LOG_INFO("Mount is Single ARM");
    }
}
```

In addition to snooping on text, number, switch, and light properties, the subscriber driver can also snoop BLOBs sent by other drivers. Like clients, it can select how it receives BLOBs. The driver can choose to never receive BLOBs, or receive them intermixed with other traffic, or exclusively receive BLOBs while ignoring all other type of traffic.

Refer to the inter-driver communication tutorial under the examples directory of INDI for a typical implementation involving a dome driver that monitors the status of a rain collector. The dome driver subscribes to a LIGHT property called rain collector in the rain driver. When the property changes its status to alert, the dome driver promptly closes down the dome.
