---
title: Driver Submission Guide
nav_order: 6
parent: Driver Development
permalink: /drivers/driver-submission-guide/
---

# INDI Driver Submission Guide

This guide provides instructions for submitting your INDI driver to the official INDI repositories and documenting it for the INDI Device Portal. Following these guidelines will help ensure your driver is properly integrated into the INDI ecosystem and is accessible to users.

## Choosing the Right Repository

INDI maintains two main repositories for drivers:

1. **INDI Core Repository**: [https://github.com/indilib/indi](https://github.com/indilib/indi)
2. **INDI 3rd Party Repository**: [https://github.com/indilib/indi-3rdparty](https://github.com/indilib/indi-3rdparty)

### INDI Core Repository

The INDI Core Repository is the preferred destination for most drivers. Submit your driver to the core repository if:

- Your driver's dependencies are already satisfied by the INDI core library
- Your driver doesn't require any external libraries beyond what's already used in the core
- Your driver follows the standard INDI release cycle

Examples of drivers suitable for the core repository include:

- Simple serial/USB device drivers
- Drivers that use standard communication protocols
- Drivers that implement standard INDI interfaces without exotic dependencies

### INDI 3rd Party Repository

The INDI 3rd Party Repository is for drivers that:

- Require additional dependencies not present in the core repository
- Need a release cycle independent of the core INDI library
- Interface with proprietary SDKs or libraries
- Have complex or unusual build requirements

Examples of drivers suitable for the 3rd party repository include:

- Drivers that use vendor-specific SDKs
- Drivers with dependencies on specialized libraries
- Drivers that need frequent updates independent of the core INDI release cycle

### Dependency Analysis

Before submitting your driver, analyze its dependencies:

1. List all libraries your driver depends on
2. Check if these dependencies are already present in the INDI core repository
3. Determine if your driver needs a release cycle different from the core

```bash
# Example command to check dependencies of an existing driver
ldd /usr/bin/indi_mydriver
```

## Preparing Your Driver for Submission

### Code Organization

Ensure your driver follows the standard INDI code organization:

1. **Header Files**: Include all necessary header files
2. **Source Files**: Organize your source code logically
3. **Build System**: Provide CMake files for building your driver
4. **XML Files**: Include the driver XML file for client discovery

### Documentation

Prepare comprehensive documentation for your driver:

1. **README.md**: Include basic information about the driver
2. **INSTALL.md**: Provide installation instructions
3. **Driver Documentation**: Create detailed documentation for the INDI Device Portal

### Testing

Before submission, thoroughly test your driver:

1. Test with actual hardware if possible
2. Test in simulation mode
3. Verify compatibility with popular INDI clients (KStars/Ekos, INDI Web Manager, etc.)
4. Check for memory leaks and other issues

## Submitting Your Driver

### GitHub Pull Request

1. Fork the appropriate repository (core or 3rd party)
2. Create a branch for your driver
3. Add your driver code
4. Submit a pull request with a clear description of your driver

Example pull request description:

```
Driver: MyNewDevice Driver

This PR adds support for MyNewDevice, a [type of device] that communicates via [protocol].

Features:
- Feature 1
- Feature 2
- Feature 3

Dependencies:
- libusb-1.0
- Standard INDI libraries

Tested with:
- Actual MyNewDevice hardware
- KStars/Ekos client
```

### Code Review Process

After submission, your code will go through a review process:

1. INDI maintainers will review your code
2. You may be asked to make changes or improvements
3. Once approved, your driver will be merged into the repository

## Documenting Your Driver for the INDI Device Portal

The [INDI Device Portal](https://indilib.org/devices) is the central location for users to find information about supported devices. Proper documentation is essential for users to understand how to use your driver.

### Required Documentation

Your driver documentation should include:

1. **Metadata**:

   - Driver name
   - Executable name
   - Author information
   - Version
   - Device type
   - Connection type
   - Supported platforms

2. **Overview**:

   - Brief description of the device
   - Key features
   - Supported models

3. **Installation**:

   - Installation instructions
   - Dependencies
   - Configuration requirements

4. **Usage Guide**:

   - Connection instructions
   - Configuration options
   - Operation instructions
   - Screenshots of the driver interface
   - Tips and best practices

5. **Troubleshooting**:
   - Common issues and solutions
   - Debugging tips

### Documentation Format

The INDI Device Portal accepts documentation in Markdown format. Here's a template for your driver documentation:

````markdown
# MyDevice Driver

## Metadata

- **Driver Name**: MyDevice
- **Executable**: indi_mydevice
- **Author**: Your Name
- **Version**: 1.0
- **Device Type**: CCD Camera (or Telescope, Focuser, etc.)
- **Connection Type**: USB, Serial, Network, etc.
- **Platforms**: Linux, macOS, etc.

## Overview

Brief description of the device and what it does. Include key features and supported models.

## Installation

### Dependencies

- Dependency 1
- Dependency 2

### Installation Steps

```bash
sudo apt-get install indi-mydevice
```

## Usage Guide

### Connecting to the Device

Instructions for connecting to the device, including port settings, etc.

### Main Control Panel

![Main Control Panel](images/mydevice_main.png)

Description of the main control panel and its features.

### Options Panel

![Options Panel](images/mydevice_options.png)

Description of the options panel and available settings.

### Operation

Step-by-step instructions for using the device with this driver.

### Tips and Best Practices

- Tip 1
- Tip 2
- Tip 3

## Troubleshooting

### Common Issues

- Issue 1: Solution 1
- Issue 2: Solution 2

### Getting Help

Where to get help if you encounter problems not covered in this documentation.
````

### Screenshots

Include screenshots of all relevant driver tabs and dialogs:

1. **Connection Panel**: Show the connection options
2. **Main Control Panel**: Show the primary controls
3. **Options Panel**: Show configuration options
4. **Other Panels**: Include any other relevant panels

Screenshots should be:

- Clear and readable
- Showing the driver in actual use
- Demonstrating key features
- Properly cropped and sized

### Submitting Documentation to the Device Portal

To submit your documentation to the INDI Device Portal:

1. Prepare your documentation in Markdown format
2. Gather all necessary screenshots
3. Submit your documentation through the [INDI website](https://indilib.org/forum.html) or as part of your GitHub pull request

## Example: Complete Driver Documentation

Here's an example of complete documentation for a fictional INDI driver:

````markdown
# ACME Deluxe CCD Camera Driver

## Metadata

- **Driver Name**: ACME Deluxe CCD
- **Executable**: indi_acme_ccd
- **Author**: Jane Doe
- **Version**: 1.2
- **Device Type**: CCD Camera
- **Connection Type**: USB
- **Platforms**: Linux, macOS

## Overview

The ACME Deluxe CCD is a high-sensitivity astronomy camera with a Sony IMX294 sensor. This driver supports all features of the camera including cooling, filter wheel control, and high-speed readout modes.

Supported models:

- ACME Deluxe CCD
- ACME Deluxe CCD Pro
- ACME Deluxe CCD Lite

## Installation

### Dependencies

- libusb-1.0
- libcfitsio

### Installation Steps

```bash
# For Ubuntu/Debian
sudo apt-get install indi-acme-ccd

# For Fedora
sudo dnf install indi-acme-ccd

# From source
git clone https://github.com/indilib/indi-3rdparty
cd indi-3rdparty
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_ACME=ON ..
make
sudo make install
```
````

## Usage Guide

### Connecting to the Device

1. Connect the camera to your computer via USB
2. In your INDI client, select "ACME Deluxe CCD" from the device list
3. Click "Connect"

The driver will automatically detect the camera model and configure itself accordingly.

### Main Control Panel

![Main Control Panel](images/acme_main.png)

The main control panel shows:

1. Current exposure settings
2. Temperature control
3. Image download progress
4. Cooling status

### Options Panel

![Options Panel](images/acme_options.png)

The options panel allows you to configure:

1. Readout mode (High Quality, Fast, Binned)
2. Gain and offset
3. Fan speed
4. Filter wheel position (if attached)

### Filter Wheel Control

![Filter Wheel](images/acme_filter.png)

If you have the ACME filter wheel attached, you can:

1. Select filters from the dropdown
2. Configure filter names
3. Set up filter offsets for auto-focusing

### Operation

#### Taking an Exposure

1. Set the exposure time in seconds
2. Select the frame type (Light, Dark, Flat, Bias)
3. Click "Start Exposure"
4. The image will download automatically when complete

#### Cooling the Camera

1. Set the target temperature
2. Click "Set"
3. The cooling system will gradually reach the target temperature
4. Monitor the current temperature and cooling power

### Tips and Best Practices

- Allow the camera to cool for at least 10 minutes before taking calibration frames
- Use the "High Quality" readout mode for the best image quality
- For planetary imaging, use the "Fast" readout mode
- The camera performs best at gain settings between 100-200
- Always disconnect the camera before powering it off

## Troubleshooting

### Common Issues

- **Camera not detected**: Ensure the USB cable is firmly connected and try a different USB port
- **Cooling not working**: Check that the camera has external power connected
- **Noisy images**: Reduce the gain or increase the exposure time
- **Download errors**: Try reducing the USB traffic setting in the options panel

### Getting Help

If you encounter issues not covered here, please:

1. Check the [INDI forums](https://indilib.org/forum.html)
2. Report bugs on the [GitHub issue tracker](https://github.com/indilib/indi-3rdparty/issues)
3. Contact the driver author at jane.doe@example.com

```

## Conclusion

Submitting your driver to the appropriate INDI repository and providing comprehensive documentation helps integrate your driver into the INDI ecosystem and makes it accessible to users. By following these guidelines, you contribute to the growth and improvement of the INDI project.

Remember:
- Submit to the core repository when possible
- Use the 3rd party repository for drivers with special dependencies
- Provide complete and clear documentation
- Include screenshots of all relevant driver interfaces
- Test thoroughly before submission

For more information, visit the [INDI website](https://indilib.org) or contact the INDI development team.
```
