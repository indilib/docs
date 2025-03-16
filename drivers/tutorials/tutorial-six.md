---
title: Tutorial Six - Simple Client
nav_order: 6
parent: INDI Tutorials
grand_parent: Driver Development
---

# Tutorial Six: Simple Client

This tutorial demonstrates how to create a simple INDI client that can control a CCD simulator. It shows how to connect to an INDI server, access device properties, and perform operations like setting the camera temperature and taking exposures.

## What You'll Learn

- How to create an INDI client by inheriting from `INDI::BaseClient`
- How to connect to an INDI server
- How to access and modify device properties
- How to handle property updates and BLOB data

## Source Code

The source code for this tutorial can be found in the INDI library repository under the `examples/tutorial_six` directory.

## INDI Client Development

INDI clients connect to INDI servers to control devices. The key steps for implementing an INDI client are:

1. Create a class that inherits from `INDI::BaseClient`
2. Connect to an INDI server
3. Watch for devices and properties of interest
4. Handle property updates and perform operations

## Code Explanation

This tutorial demonstrates:

1. **Client Setup**:

   - Creating a client class that inherits from `INDI::BaseClient`
   - Connecting to an INDI server
   - Watching for the "Simple CCD" device

2. **Device Control**:

   - Setting the camera temperature
   - Taking exposures
   - Receiving and saving FITS images

3. **Property Handling**:
   - Watching for specific properties (CONNECTION, CCD_TEMPERATURE, CCD1)
   - Handling property updates
   - Processing BLOB data

## Running the Tutorial

To run this tutorial:

1. First, run the CCD simulator (tutorial_three):

   ```bash
   cd /path/to/indi/build
   indiserver -v ./tutorial_three
   ```

2. In another terminal, run the client:

   ```bash
   cd /path/to/indi/build
   ./tutorial_client
   ```

3. You should see the client connect to the server, set the temperature to -20°C, and take an exposure when the temperature is reached.

4. The client will save the received image as "ccd_simulator.fits" in the current directory.

## Next Steps

This tutorial demonstrates how to create a simple INDI client. In [Tutorial Seven](tutorial-seven.md), we'll learn how to create a telescope simulator that uses the INDI Alignment Subsystem to improve pointing performance.
