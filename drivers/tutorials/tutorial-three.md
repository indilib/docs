---
title: Tutorial Three - Simple CCD
nav_order: 3
parent: INDI Tutorials
grand_parent: Driver Development
---

# Tutorial Three: Simple Camera

This tutorial demonstrates how to create a simple Camera simulator that can generate and transmit FITS images. It builds on the concepts introduced in the previous tutorials and introduces BLOB (Binary Large Object) handling for image data.

## What You'll Learn

- How to create a Camera driver by inheriting from `INDI::CCD`
- How to generate and transmit FITS images
- How to handle BLOB properties
- How to simulate CCD temperature control

## Source Code

The source code for this tutorial can be found in the INDI library repository under the `examples/tutorial_three` directory.

## Code Explanation

This driver simulates a simple camera that can:

- Connect/disconnect to the device
- Control the TEC temperature
- Capture images with different exposure times
- Generate and transmit FITS images

The key components include:

1. **CCD Properties**: Temperature, exposure time, and image data
2. **FITS Generation**: Creating simulated FITS images with random data
3. **BLOB Handling**: Transmitting binary image data to clients

## Running the Tutorial

To run this tutorial:

1. Build the tutorial using CMake:

   ```bash
   cd /path/to/indi/build
   make
   ```

2. Run the INDI server with the tutorial driver:

   ```bash
   indiserver -v ./tutorial_three
   ```

3. Connect to the server using an INDI client (like KStars/EKOS or INDI Control Panel).

4. You should see the "Simple CCD" in the client, and you can connect to it, set the temperature, and take exposures.

## Next Steps

This tutorial demonstrates a simple Camera simulator. In [Tutorial Four](tutorial-four.md), we'll learn how to use skeleton files to define properties.
