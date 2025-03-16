---
title: Tutorial Eight - Rotator
nav_order: 8
parent: INDI Tutorials
grand_parent: Driver Development
---

# Tutorial Eight: Rotator

This tutorial demonstrates how to create a simple rotator device driver. A rotator is a device that can rotate a camera or other instrument to a specific angle, which is useful for framing targets or for field de-rotation during long exposures.

## What You'll Learn

- How to create a rotator driver by inheriting from `INDI::RotatorInterface`
- How to implement angle control and movement simulation
- How to handle rotator-specific properties and commands
- How to integrate the rotator interface with the DefaultDevice class

## Source Code

The source code for this tutorial can be found in the INDI library repository under the `examples/tutorial_eight` directory.

## INDI Rotator Interface

The INDI Rotator Interface provides a standard way to control rotator devices. The key components are:

1. **Angle Control**: Setting and reporting the rotator's angle
2. **Movement Control**: Starting, stopping, and monitoring rotator movement
3. **Reverse Direction**: Ability to reverse the direction of rotation

## Code Explanation

This tutorial demonstrates:

1. **Rotator Implementation**:

   - Creating a class that inherits from `INDI::DefaultDevice` and implements `INDI::RotatorInterface`
   - Initializing rotator properties
   - Implementing the required interface methods

2. **Angle Control**:

   - Setting the target angle
   - Simulating movement to the target angle
   - Reporting the current angle

3. **Additional Features**:
   - Implementing reverse direction functionality
   - Handling abort commands
   - Simulating realistic rotator behavior

## Running the Tutorial

To run this tutorial:

1. Build the tutorial using CMake:

   ```bash
   cd /path/to/indi/build
   make
   ```

2. Run the INDI server with the tutorial driver:

   ```bash
   indiserver -v ./tutorial_eight
   ```

3. Connect to the server using an INDI client (like KStars/EKOS or INDI Control Panel).

4. You should see the rotator device in the client, and you can connect to it and control its angle.

## Conclusion

This tutorial completes our series on INDI driver development. We've covered a wide range of topics, from basic device drivers to specialized interfaces and inter-driver communication. With these examples, you should have a solid foundation for developing your own INDI drivers for various astronomical devices.

For more information and advanced topics, please refer to the [INDI Library Documentation](https://indilib.org/developers/documentation.html) and the [INDI GitHub Repository](https://github.com/indilib/indi).
