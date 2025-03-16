---
title: Tutorial Five - Inter-Driver Communication
nav_order: 5
parent: INDI Tutorials
grand_parent: Driver Development
---

# Tutorial Five: Inter-Driver Communication

This tutorial demonstrates how to implement communication between different INDI drivers. It shows how a Rain Detector driver can communicate with a Dome driver to close the dome when rain is detected.

## What You'll Learn

- How to implement inter-driver communication using INDI's snooping mechanism
- How to register for property updates from other drivers
- How to react to property changes from other drivers
- How to create a practical example of device automation

## Source Code

The source code for this tutorial can be found in the INDI library repository under the `examples/tutorial_five` directory. It consists of two drivers:

- `tutorial_dome`: A simple dome driver that can open and close
- `tutorial_rain`: A rain detector that can detect rain and notify the dome

## Inter-Driver Communication in INDI

INDI provides a mechanism called "snooping" that allows drivers to listen for property updates from other drivers. This enables drivers to react to changes in other devices without direct coupling.

The key steps for implementing inter-driver communication are:

1. Register for property updates using `IDSnoopDevice`
2. Process property updates in the `ISSnoopDevice` function
3. Take appropriate actions based on the received updates

## Code Explanation

This tutorial demonstrates:

1. **Rain Detector Driver**:
   - Simulates rain detection
   - Broadcasts rain status through a property
2. **Dome Driver**:

   - Registers to receive updates from the Rain Detector
   - Automatically closes the dome when rain is detected
   - Provides manual control for opening and closing

3. **Communication Flow**:
   - Rain Detector detects rain and updates its property
   - Dome driver receives the update through snooping
   - Dome driver closes the dome to protect the equipment

## Running the Tutorial

To run this tutorial:

1. Build the tutorial using CMake:

   ```bash
   cd /path/to/indi/build
   make
   ```

2. Run the INDI server with both tutorial drivers:

   ```bash
   indiserver -v ./tutorial_dome ./tutorial_rain
   ```

3. Connect to the server using an INDI client (like KStars/EKOS or INDI Control Panel).

4. You should see both the Dome and Rain Detector devices in the client.

5. When you simulate rain detection in the Rain Detector, you should see the Dome automatically close.

## Next Steps

This tutorial demonstrates inter-driver communication for device automation. In [Tutorial Six](tutorial-six.md), we'll learn how to create a simple INDI client that can control a CCD simulator.
