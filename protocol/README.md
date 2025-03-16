---
title: Protocol
nav_order: 3
has_children: true
permalink: /protocol/
---

# INDI Protocol

The INDI (Instrument-Neutral Distributed Interface) protocol is an XML-based protocol designed for controlling astronomical devices. This section provides detailed information about the protocol, its structure, and how it works.

## Introduction to the INDI Protocol

The INDI protocol is designed to provide a standard way for astronomical software to communicate with astronomical hardware. It is:

- **XML-based**: All messages are formatted as XML elements
- **Stateful**: The protocol maintains state information about devices and their properties
- **Asynchronous**: Commands and responses are not necessarily paired or immediate
- **Extensible**: The protocol can be extended to support new types of devices and functionality

The protocol defines a set of messages that clients and drivers can exchange to discover and control devices. These messages are grouped into several categories:

- **Device and property discovery**: Messages for discovering devices and their properties
- **Property definition**: Messages for defining properties and their characteristics
- **Property update**: Messages for updating property values
- **Property state**: Messages for indicating the state of properties
- **Binary data**: Messages for transferring binary data (e.g., images)

## Protocol Version History

The INDI protocol has evolved over time to support new features and improve existing ones. Here's a brief history of the protocol versions:

- **1.7**: Added getProperties from Devices to add snooping functionality, and added message attribute to getProperties from Clients.
- **1.6**: Clarification regarding when all members of a vector must be sent, even members that changed.
- **1.5**: Made the size attribute of the oneBLOB element required.
- **1.3**: Added BLOB elements for transferring binary data.
- **1.0**: Initial release of the INDI protocol.

## Protocol Architecture

The INDI protocol follows a client-server architecture:

- **INDI Device**: Offers a service cast as a set of Properties in the INDI format.
- **INDI Client**: A process which connects to an INDI Device, queries for its set of control Properties, and possibly sends requests to change those Properties.
- **INDI Server**: An intermediary that can connect multiple Clients to multiple Devices, handling message routing and implementing policies for administrative issues.

INDI Clients and Devices need not be in direct contact. The protocol is designed to accommodate arbitration and broadcasting among several Clients and Devices. The protocol can support connection topologies ranging from one-to-one on a single system to many-to-many between systems of different types.

## Protocol Structure

### XML Message Format

INDI messages are formatted as XML documents. Each message consists of one or more XML elements that represent INDI commands or properties.

#### Basic XML Structure

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

#### Data Types and Formats

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

### XML Elements

The INDI protocol defines several XML elements for different types of messages:

- `<defTextVector>`, `<defNumberVector>`, `<defSwitchVector>`, `<defLightVector>`, `<defBLOBVector>`: Define property vectors
- `<setTextVector>`, `<setNumberVector>`, `<setSwitchVector>`, `<setLightVector>`, `<setBLOBVector>`: Set property values
- `<newTextVector>`, `<newNumberVector>`, `<newSwitchVector>`, `<newLightVector>`, `<newBLOBVector>`: Report new property values
- `<message>`: Send a message to the client
- `<delProperty>`: Delete a property

Each element has attributes that provide additional information about the message, such as the device name, property name, timestamp, and state.

### INDI Commands

INDI defines several commands for defining, setting, and updating properties. These commands are represented as XML elements in INDI messages.

#### Commands from Device to Client

- **getProperties**: Command to enable snooping messages from other devices.
- **defTextVector**, **defNumberVector**, **defSwitchVector**, **defLightVector**, **defBLOBVector**: Define properties of various types.
- **setTextVector**, **setNumberVector**, **setSwitchVector**, **setLightVector**, **setBLOBVector**: Send new values for properties.
- **message**: Send a message associated with a device or entire system.
- **delProperty**: Delete the given property, or entire device if no property is specified.

#### Commands from Client to Device

- **getProperties**: Command to ask Device to define all Properties.
- **enableBLOB**: Command to control whether setBLOBs should be sent to this channel.
- **newTextVector**, **newNumberVector**, **newSwitchVector**, **newBLOBVector**: Commands to inform Device of new target values for a Property.

#### Vector Member Elements

- **oneText**, **oneNumber**, **oneSwitch**, **oneLight**, **oneBLOB**: Elements describing a vector member value.

### Property Types

The INDI protocol defines five types of properties:

- **Text**: String values
- **Number**: Numeric values with optional format, minimum, maximum, and step
- **Switch**: Boolean or enumerated values with different rule types (OneOfMany, AtMostOne, AnyOfMany)
- **Light**: Read-only status indicators with different states (Idle, OK, Busy, Alert)
- **BLOB**: Binary data with format information

Each property type has its own set of attributes and elements for defining and updating values.

### Message Flow

The typical message flow in the INDI protocol follows this pattern:

1. **Device Discovery**: Clients send `<getProperties>` to discover devices and their properties
2. **Property Definition**: Drivers send `<defXXXVector>` messages to define properties
3. **Property Update**: Clients send `<setXXXVector>` messages to update property values
4. **Property State**: Drivers send `<newXXXVector>` messages to report new property values and states

This flow is asynchronous, meaning that drivers can send property updates at any time, not just in response to client requests.

## Standard Properties

The INDI protocol defines a set of standard properties that all drivers should implement if applicable. These properties provide a common interface for clients to interact with devices, regardless of the specific hardware.

Some examples of standard properties include:

- `CONNECTION`: Connect/disconnect from the device
- `EQUATORIAL_EOD_COORD`: Telescope equatorial coordinates
- `CCD_EXPOSURE`: Camera exposure time
- `FILTER_SLOT`: Filter wheel slot number

For a complete list of standard properties, see the [Standard Properties](../drivers/standard-properties.md) documentation.

## Protocol Examples

Here are some examples of INDI protocol messages:

### Device Discovery

```xml
<getProperties version="1.7"/>
```

### Property Definition

```xml
<defNumberVector device="CCD Simulator" name="CCD_EXPOSURE" label="Expose" group="Main Control" state="Idle" perm="rw" timeout="60" timestamp="2023-01-01T12:00:00">
    <defNumber name="CCD_EXPOSURE_VALUE" label="Duration (s)" format="%5.2f" min="0" max="36000" step="0.01">
        1.0
    </defNumber>
</defNumberVector>
```

### Property Update

```xml
<setNumberVector device="CCD Simulator" name="CCD_EXPOSURE" state="Busy" timeout="60" timestamp="2023-01-01T12:01:00">
    <oneNumber name="CCD_EXPOSURE_VALUE">
        5.0
    </oneNumber>
</setNumberVector>
```

### Property State

```xml
<newNumberVector device="CCD Simulator" name="CCD_EXPOSURE" state="Busy" timeout="60" timestamp="2023-01-01T12:01:00">
    <oneNumber name="CCD_EXPOSURE_VALUE">
        5.0
    </oneNumber>
</newNumberVector>
```

## Protocol Documentation

For more detailed information about the INDI protocol, refer to the following resources:

- [INDI Protocol White Paper](INDI.pdf): The original white paper describing the INDI protocol
- [INDI Protocol Specification](https://www.indilib.org/develop/developer-manual/104-indi-protocol.html): The official INDI protocol specification
- [INDI Library API Documentation](https://www.indilib.org/api/index.html): Documentation for the INDI library API

## Protocol Implementation

The INDI protocol is implemented in the INDI library, which provides a C/C++ API for developing INDI drivers and clients. The library handles the XML parsing and generation, as well as the network communication between clients and drivers.

For information on how to use the INDI library to implement drivers, see the [Driver Development](../drivers/) documentation.

For information on how to use the INDI library to implement clients, see the [Client Development](../clients/) documentation.

## Protocol Extensions

The INDI protocol can be extended to support new types of devices and functionality. Extensions can be implemented by:

- Adding new standard properties
- Adding new property types
- Adding new message types

Extensions should be backward compatible with existing clients and drivers, and should follow the INDI protocol design principles.

## Conclusion

The INDI protocol provides a flexible and extensible way for astronomical software to communicate with astronomical hardware. By understanding the protocol structure and message flow, you can develop INDI drivers and clients that work seamlessly with the INDI ecosystem.
