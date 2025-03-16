---
title: Tutorial Four - Skeleton Files
nav_order: 4
parent: INDI Tutorials
grand_parent: Driver Development
---

# Tutorial Four: Skeleton Files

This tutorial demonstrates how to use skeleton files to define properties in INDI drivers. It provides an alternative approach to defining properties compared to hard-coding them in the driver's source code.

## What You'll Learn

- How to use skeleton files to define INDI properties
- How to load and use skeleton files in your driver
- The advantages of using skeleton files for property definitions

## Source Code

The source code for this tutorial can be found in the INDI library repository under the `examples/tutorial_four` directory.

## What are Skeleton Files?

Skeleton files are XML files that define the properties of an INDI driver. They provide a way to separate the property definitions from the driver's source code, making it easier to modify the properties without recompiling the driver.

A skeleton file contains a list of `defXXX` XML statements enclosed by `<INDIDriver>` opening and closing tags. These statements define the properties, their types, and their default values.

## Code Explanation

This tutorial demonstrates:

1. **Loading Skeleton Files**: How to load property definitions from an external XML file
2. **Processing Property Updates**: How to handle property updates from clients
3. **Advantages of Skeleton Files**: Separation of property definitions from driver logic

## Running the Tutorial

To run this tutorial:

1. Make sure the skeleton file is installed:

   ```bash
   # Either install to the system directory
   sudo cp tutorial_four_sk.xml /usr/share/indi/

   # Or set the INDISKEL environment variable
   export INDISKEL=/path/to/tutorial_four_sk.xml
   ```

2. Build the tutorial using CMake:

   ```bash
   cd /path/to/indi/build
   make
   ```

3. Run the INDI server with the tutorial driver:

   ```bash
   indiserver -v ./tutorial_four
   ```

4. Connect to the server using an INDI client (like KStars/EKOS or INDI Control Panel).

## Next Steps

This tutorial demonstrates using skeleton files for property definitions. In [Tutorial Five](tutorial-five.md), we'll learn about inter-driver communication between a Rain Detector and a Dome driver.
