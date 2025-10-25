---
title: Dome Interface
nav_order: 12
parent: Device Interfaces
---

# Implementing the Dome Interface

This guide provides a comprehensive overview of implementing the dome interface in INDI drivers. It covers the basic structure of a dome driver, how to implement the required methods, and how to handle device-specific functionality.

## Introduction to the Dome Interface

The INDI Dome Interface (`INDI::Dome`) is designed for devices that control astronomical observatory domes, including both rotating domes and roll-off observatories. It provides a standardized way for INDI clients to control dome azimuth, shutter operations, parking, and synchronization with a telescope mount. Both relative and absolute position domes are supported, as well as open-loop control for domes without position feedback.

## Prerequisites

Before implementing the dome interface, you should have:

- Basic knowledge of C++ programming
- Understanding of the INDI protocol and architecture
- Familiarity with the device's communication protocol
- Development environment set up (compiler, build tools, etc.)
- INDI library installed

## Dome Interface Structure

The dome interface consists of several key components:

-   **Base Class**: `INDI::Dome` is the base class for all dome drivers.
-   **Capabilities**: Defines the features supported by the dome.
-   **Standard Properties**: Properties for controlling the dome's state, motion, and other functionalities.
-   **Virtual Methods**: A set of virtual methods that must be implemented by the driver.
-   **Helper Methods**: Methods for common dome operations.

### Base Class

The `INDI::Dome` base class inherits from `INDI::DefaultDevice` and provides additional functionality specific to dome devices. It defines standard properties for dome control, shutter operation, parking, and synchronization.

### Dome Capabilities

The `DomeCapability` flags define the features supported by a dome device. A driver sets these capabilities to inform clients about the device's functionalities.

| Capability Flag         | Value | Description
