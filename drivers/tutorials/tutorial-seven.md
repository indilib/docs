---
title: Tutorial Seven - Alignment Subsystem
nav_order: 7
parent: INDI Tutorials
grand_parent: Driver Development
---

# Tutorial Seven: Alignment Subsystem

This tutorial demonstrates how to create a telescope simulator that uses the INDI Alignment Subsystem to improve pointing performance. The Alignment Subsystem helps correct for mechanical and optical misalignments in telescope mounts.

## What You'll Learn

- How to integrate the INDI Alignment Subsystem into a telescope driver
- How to handle alignment points and sync operations
- How to transform coordinates using the alignment model
- How to improve telescope pointing accuracy

## Source Code

The source code for this tutorial can be found in the INDI library repository under the `examples/tutorial_seven` directory.

## INDI Alignment Subsystem

The INDI Alignment Subsystem provides a framework for improving telescope pointing accuracy by building a mathematical model of the telescope's pointing errors. The key components are:

1. **Alignment Database**: Stores alignment points (pairs of requested and actual coordinates)
2. **Math Plugins**: Implement different mathematical models for coordinate transformation
3. **Coordinate Conversion**: Transforms between different coordinate systems (e.g., equatorial to horizontal)

## Code Explanation

This tutorial demonstrates:

1. **Alignment Integration**:

   - Initializing the Alignment Subsystem
   - Adding alignment-related properties to the driver
   - Handling alignment commands

2. **Coordinate Transformation**:

   - Converting between coordinate systems
   - Applying the alignment model to correct coordinates
   - Handling sync operations to build the alignment model

3. **Telescope Control**:
   - Implementing GOTO operations with alignment correction
   - Simulating telescope movement
   - Reporting telescope status

## Running the Tutorial

To run this tutorial:

1. Build the tutorial using CMake:

   ```bash
   cd /path/to/indi/build
   make
   ```

2. Run the INDI server with the tutorial driver:

   ```bash
   indiserver -v ./tutorial_seven
   ```

3. Connect to the server using an INDI client that supports the Alignment Subsystem (like KStars/EKOS).

4. You can now perform alignment operations:
   - Slew to a known object
   - Sync the telescope to the object's actual position
   - Repeat for multiple objects to build an alignment model
   - Observe improved pointing accuracy in subsequent slews

## Next Steps

This tutorial demonstrates how to use the INDI Alignment Subsystem to improve telescope pointing accuracy. In [Tutorial Eight](tutorial-eight.md), we'll learn how to create a simple rotator device.
