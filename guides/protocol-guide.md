---
title: Protocol Guide
nav_order: 8
parent: Guides
---

# INDI Protocol Guide

This guide provides a comprehensive overview of the INDI protocol, including its message format, property types, and communication flow. It is intended for developers who want to understand the protocol in detail or implement their own INDI drivers or clients.

## Introduction to the INDI Protocol

INDI (Instrument-Neutral Distributed Interface) is a protocol designed for interactive and automated remote control of diverse instrumentation. It was created by Elwood Charles Downey and is currently at Protocol Version 1.7 (Document Version 1.3).

INDI is small, easy to parse, and stateless. In the INDI paradigm, each Device poses all command and status functions in terms of setting and getting Properties. Each Property is a vector of one or more named members. Properties provide timing information about how they might be sequenced with respect to other Properties to accomplish coordinated actions, and provide hints for interactive manipulation in a GUI.

A key feature of INDI is that Clients learn the Properties of a particular Device at runtime using introspection. This decouples Client and Device implementation histories. Devices have complete authority over whether to accept commands from Clients.

## Protocol Architecture

The INDI protocol follows a client-server architecture:

- **INDI Device**: Offers a service cast as a set of Properties in the INDI format.
- **INDI Client**: A process which connects to an INDI Device, queries for its set of control Properties, and possibly sends requests to change those Properties.
- **INDI Server**: An intermediary that can connect multiple Clients to multiple Devices, handling message routing and implementing policies for administrative issues.

INDI Clients and Devices need not be in direct contact. The protocol is designed to accommodate arbitration and broadcasting among several Clients and Devices. The protocol can support connection topologies ranging from one-to-one on a single system to many-to-many between systems of different types.

## INDI Properties

Properties are vectors of one of a small set of types:

- **Text**: Collections of arbitrarily ordered characters.
- **Number**: Numeric quantities sent with a printf-style format to recommend how a GUI should display them.
- **Switch**: Always in a state of On or Off. Rules may be imposed on the behavior of switches in a vector.
- **Light**: Properties that may be in one of four states: Idle, OK, Busy, or Alert (suggested colors: gray, green, yellow, and red).
- **BLOB**: Binary Large Objects such as images.

Each Property has a name for identification purposes and a label for presentation purposes. Each element of the Property vector also has a name, making it in effect an associative array, and a presentation label. Changes to a Property affect all vector elements atomically.

### Property Attributes

All Property types except Lights have a permission attribute (lights are conceptually always read-only):

- **Text and Number**: May be Read-Only, Write-Only, or Read-Write
- **Switches**: May be Read-Only or Read-Write

Permission terminology is with respect to the Client but does not bestow any true ability. Permission serves only as a hint to a Client as to whether a Device is potentially willing to allow a Property to be changed.

Each Property as a whole is always in one of four states:

- **Idle**: Gray (suggested color)
- **OK**: Green
- **Busy**: Yellow
- **Alert**: Red

Each Property has a timeout value that specifies the worst-case time it might take to change the value to something else. The Device may report changes to the timeout value depending on current device status.

Properties may be assembled into groups to suggest how Clients might organize them, for presentation purposes, but groups serve no functional purpose.

## INDI Protocol

Each command between Client and Device specifies a Device name and Property name. The Device name serves as a logical grouping of several Properties. Property names must be unique per Device, and a Server must report unique Device names to any one Client.

The INDI protocol does not have the notion of query and response. A sender does not save context when it sends a command and wait for a specific response. All INDI participants must always be prepared to receive any command at any time.

### Protocol Flow

When a Client first starts up, it knows nothing about the Devices and Properties it will control. It begins by connecting to a Device or indiserver and sending the `getProperties` command. This includes the protocol version and may include the name of a specific Device and Property if known.

The Device then sends back one `deftype` element for each matching Property it offers for control. The `deftype` element always includes all members of the vector for each Property.

To inform a Device of new target values for a Property, a Client sends one `newtype` element. The Client must send all members of Number and Text vectors, or may send just the members that change for other types. Before it does so, the Client sets its notion of the state of the Property to Busy and leaves it until told otherwise by the Device.

To inform a Client of new current values for a Property and their state, a Device sends one `settype` element. It is only required to send those members of the vector that have changed.

### BLOB Handling

For BLOB Properties, the element `enableBLOB` allows a Client to specify whether `setBLOB` elements will arrive on a given INDI connection. The Client may send this element with a value of:

- **Only**: Only BLOB elements will be sent
- **Also**: BLOB elements and other elements will be sent
- **Never**: No BLOB elements will be sent (default)

This flow control facility allows a Client to open a separate connection and create a separate processing thread dedicated to handling BLOB data.

#### Fast BLOB Support

The INDI protocol was extended by Ludovic Pollet to support Fast BLOBs on Linux systems. This extension improves performance when transferring large binary data (like images) by using shared memory to avoid copying large binary data multiple times.

Key features of Fast BLOB support include:

- Shared memory allocation for BLOB data
- The `enclen` attribute in `oneBLOB` elements to specify the expected encoded length
- Optimized parsing of BLOB data with pre-allocated memory buffers
- Zero-copy transfer of large binary data between processes

This optimization is particularly important for high-speed imaging applications where minimizing data transfer overhead is critical.

## XML Message Format

INDI messages are formatted as XML documents. Each message consists of one or more XML elements that represent INDI commands or properties.

### Basic XML Structure

The basic structure of an INDI XML message is as follows:

```xml
<command device="device_name" name="property_name" ...>
    <element name="element_name" ...>element_value</element>
    ...
</command>
```

Where:

- `command` is the INDI command (e.g., `defNumberVector`, `newNumberVector`, `setNumberVector`).
- `device` is the name of the device.
- `name` is the name of the property.
- `element` is an element of the property.
- `element_name` is the name of the element.
- `element_value` is the value of the element.

### Data Types and Formats

All PCDATA uses character set ISO 8651-1.

The format of a numberValue can be:

- Integer
- Real
- Sexagesimal (with separators: space, colon, or semicolon)

A numberFormat can be:

- A printf-style format specification appropriate for C-type double
- An INDI style "m" to specify sexagesimal in the form "%\<w>.\<f>m" where:
  - \<w> is the total field width
  - \<f> is the width of the fraction, with valid values:
    - 9 -> :mm:ss.ss
    - 8 -> :mm:ss.s
    - 6 -> :mm:ss
    - 5 -> :mm.m
    - 3 -> :mm

A timeValue is specified in UTC in the form YYYY-MM-DDTHH:MM:SS.S, in accord with ISO 8601.

## INDI Commands

INDI defines several commands for defining, setting, and updating properties. These commands are represented as XML elements in INDI messages.

### Commands from Device to Client

- **getProperties**: Command to enable snooping messages from other devices.
- **defTextVector**, **defNumberVector**, **defSwitchVector**, **defLightVector**, **defBLOBVector**: Define properties of various types.
- **setTextVector**, **setNumberVector**, **setSwitchVector**, **setLightVector**, **setBLOBVector**: Send new values for properties.
- **message**: Send a message associated with a device or entire system.
- **delProperty**: Delete the given property, or entire device if no property is specified.

### Commands from Client to Device

- **getProperties**: Command to ask Device to define all Properties.
- **enableBLOB**: Command to control whether setBLOBs should be sent to this channel.
- **newTextVector**, **newNumberVector**, **newSwitchVector**, **newBLOBVector**: Commands to inform Device of new target values for a Property.

### Vector Member Elements

- **oneText**, **oneNumber**, **oneSwitch**, **oneLight**, **oneBLOB**: Elements describing a vector member value.

## Property Definitions

Each property type has its own XML representation for definition, update, and client commands.

### Number Properties

Number properties represent numeric values. They can have minimum, maximum, and step values, as well as a format string for display.

#### Definition

```xml
<defNumberVector device="device_name" name="property_name" label="label" group="group" state="state" perm="perm" timeout="timeout" timestamp="timestamp">
    <defNumber name="element_name" label="element_label" format="format" min="min" max="max" step="step">value</defNumber>
    ...
</defNumberVector>
```

#### Update

```xml
<setNumberVector device="device_name" name="property_name" state="state" timeout="timeout" timestamp="timestamp">
    <oneNumber name="element_name">value</oneNumber>
    ...
</setNumberVector>
```

#### Client Command

```xml
<newNumberVector device="device_name" name="property_name" timestamp="timestamp">
    <oneNumber name="element_name">value</oneNumber>
    ...
</newNumberVector>
```

### Text Properties

Text properties represent text values.

#### Definition

```xml
<defTextVector device="device_name" name="property_name" label="label" group="group" state="state" perm="perm" timeout="timeout" timestamp="timestamp">
    <defText name="element_name" label="element_label">value</defText>
    ...
</defTextVector>
```

#### Update

```xml
<setTextVector device="device_name" name="property_name" state="state" timeout="timeout" timestamp="timestamp">
    <oneText name="element_name">value</oneText>
    ...
</setTextVector>
```

#### Client Command

```xml
<newTextVector device="device_name" name="property_name" timestamp="timestamp">
    <oneText name="element_name">value</oneText>
    ...
</newTextVector>
```

### Switch Properties

Switch properties represent boolean values or sets of mutually exclusive options. They can have different rules for how switches can be set:

- **OneOfMany**: Only one switch can be on at a time.
- **AtMostOne**: At most one switch can be on at a time.
- **AnyOfMany**: Any number of switches can be on at a time.

#### Definition

```xml
<defSwitchVector device="device_name" name="property_name" label="label" group="group" state="state" perm="perm" rule="rule" timeout="timeout" timestamp="timestamp">
    <defSwitch name="element_name" label="element_label">value</defSwitch>
    ...
</defSwitchVector>
```

#### Update

```xml
<setSwitchVector device="device_name" name="property_name" state="state" timeout="timeout" timestamp="timestamp">
    <oneSwitch name="element_name">value</oneSwitch>
    ...
</setSwitchVector>
```

#### Client Command

```xml
<newSwitchVector device="device_name" name="property_name" timestamp="timestamp">
    <oneSwitch name="element_name">value</oneSwitch>
    ...
</newSwitchVector>
```

### Light Properties

Light properties represent status indicators. They can be in one of four states: Idle, OK, Busy, or Alert.

#### Definition

```xml
<defLightVector device="device_name" name="property_name" label="label" group="group" state="state" timestamp="timestamp">
    <defLight name="element_name" label="element_label">value</defLight>
    ...
</defLightVector>
```

#### Update

```xml
<setLightVector device="device_name" name="property_name" state="state" timestamp="timestamp">
    <oneLight name="element_name">value</oneLight>
    ...
</setLightVector>
```

### BLOB Properties

BLOB (Binary Large Object) properties represent binary data, such as images.

#### Definition

```xml
<defBLOBVector device="device_name" name="property_name" label="label" group="group" state="state" perm="perm" timeout="timeout" timestamp="timestamp">
    <defBLOB name="element_name" label="element_label"/>
    ...
</defBLOBVector>
```

#### Update

```xml
<setBLOBVector device="device_name" name="property_name" state="state" timeout="timeout" timestamp="timestamp">
    <oneBLOB name="element_name" size="size" format="format" enclen="enclen">encoded_data</oneBLOB>
    ...
</setBLOBVector>
```

The contents of the oneBLOB element must always be encoded using base64. The format attribute consists of one or more file name suffixes, each preceded with a period, which indicate how the decoded data is to be interpreted (e.g., .fits, .fits.z). The optional `enclen` attribute specifies the length of the encoded data, which enables optimized parsing with Fast BLOB support.

#### Client Command

```xml
<newBLOBVector device="device_name" name="property_name" timestamp="timestamp">
    <oneBLOB name="element_name" size="size" format="format">encoded_data</oneBLOB>
    ...
</newBLOBVector>
```

### Message Command

The message command is used by devices to send messages to clients:

```xml
<message device="device_name" timestamp="timestamp" message="message"/>
```

## Communication Flow

The communication flow between INDI clients and drivers follows a specific pattern:

1. **Device Discovery**: Clients connect to the INDI server and discover available devices.
2. **Property Definition**: Drivers define properties to clients.
3. **Property Control**: Clients set property values, and drivers update property values.
4. **Event Notification**: Drivers send messages and property updates to clients.

### Client Processing

```
if receive <setXXX> from Device
  change record of value and/or state for the specified Property
if receive <defProperty> from Device
  if first time to see this device=
    create new Device record
  if first time to see this device+name combination
    create new Property record within given Device
if receive <delProperty> from Device
  if includes device= attribute
    if includes name= attribute
      delete record for just the given Device+name
    else
      delete all records the given Device
  else
    delete all records for all devices
if Client wants to learn all Devices and all their Properties
  send <defProperties>
if Client wants to change a Property value or state
  set State to Busy
  send <newXXX> with device, name and value
```

### Device Processing

```
if receive <newXXX> from Client
  if element contains acceptable device, name and value
    set new target value and commence
if receive <getProperties> from Client
  if element contains recognized device
    send one <defProperty> for each name for specified device
  else if element contains no device attribute
    send one <defProperty for each name for each device
if any Property's value changes, even as a result of a <newXXX>
  send <setXXX> specifying device+name with new value and state
```

## Example Communication

Here's an example of the communication flow between a client and a driver:

1. **Client connects to the INDI server**

2. **Driver defines a number property**

```xml
<defNumberVector device="OTA" name="Focus" state="Idle" perm="rw" timeout="50" label="Focus position, μM">
    <defNumber name="Focus" label="" format="%4.0f" min="-100" max="100" step="10">50</defNumber>
</defNumberVector>
```

3. **Client sets the focus value**

```xml
<newNumberVector device="OTA" name="Focus" timestamp="2023-01-01T00:00:01">
    <oneNumber name="Focus">75</oneNumber>
</newNumberVector>
```

4. **Driver updates the property state to Busy**

```xml
<setNumberVector device="OTA" name="Focus" state="Busy" timeout="50" timestamp="2023-01-01T00:00:01">
    <oneNumber name="Focus">75</oneNumber>
</setNumberVector>
```

5. **Driver updates the property state to OK when the focus is complete**

```xml
<setNumberVector device="OTA" name="Focus" state="OK" timeout="50" timestamp="2023-01-01T00:00:03">
    <oneNumber name="Focus">75</oneNumber>
</setNumberVector>
```

## Scripting and Automation

INDI Clients need not be GUI programs; they can also be command line programs. For example, a program could be written to get and display one or more INDI Properties specified on its command line. These sorts of programs could be put together to perform complex functionality within the convenience of a scripting language.

The INDI self-describing paradigm can also be used to operate any system automatically. For automated telescope applications, two additional XML elements are defined for timing actions:

- **by**: Wraps a newtype command with an attribute t that specifies when the Property should achieve status OK.
- **at**: Wraps a newtype command with an attribute t that specifies the exact moment when the newtype should be issued.

## Network Behavior

INDI is a session layer protocol. It presumes the existence of a reliable sequenced byte stream between each INDI participant. The transport mechanism for the INDI protocol may be anything that satisfies these requirements, such as:

- Direct driver calls
- Local pipes or FIFOs
- UNIX sockets
- TCP sockets (possibly secured with SSL or tunneling through SSH)
- As a payload within frameworks like P2P, JXTA, Jabber, XML-RPC, or SOAP

If a straight TCP/IP Socket implementation is used, IANA has assigned INDI to TCP port 7624.

## Server Behavior

An INDI Server presents the behavior of a Client to all Devices and a Device to all Clients. In its simplest form, each command the Server receives from any Device might be sent unaltered to all Clients, and each command the Server receives from any Client is sent unaltered to all Devices.

INDI Servers must take special precautions to deal with large BLOB Properties:

- They must maintain and honor the enableBLOB state for each Client connection
- They may drop BLOBs if they arrive faster than slow recipients can accept them
- They must take care not to block while writing large BLOBs to slow Clients

It is possible to build INDI Servers such that they can be chained together, allowing the INDI network to be distributed and take advantage of multiprocessing.

## Protocol Version History

- **1.7**: Added getProperties from Devices to add snooping functionality, and added the name attribute to getProperties from Clients.
- **1.6**: Clarification regarding when all members of a vector must be transmitted or just those members that changed.
- **1.5**: Made the size attribute of the oneBLOB element required instead of optional.
- **1.3**: Added BLOB elements for transferring binary data.

## Conclusion

The INDI protocol provides a flexible and extensible way to control astronomical equipment. By understanding the protocol's message format, property types, and communication flow, you can implement your own INDI drivers or clients, or extend existing ones to support new devices or functionality.

INDI Clients know nothing about the meaning of the Devices and Properties with which they deal, and Devices know nothing of how or why Clients are using their services. This decoupling allows Devices to be written once and accessed from anywhere, making INDI truly an Instrument-Neutral Distributed Interface.
