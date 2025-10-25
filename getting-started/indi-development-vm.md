---
title: Development VM
nav_order: 3
has_children: false
permalink: /getting-started/indi-development-vm/
---

# INDI Development VirtualBox Machine

The INDI Development VM  offers a pre-configured operating system environment that can can run on Windows, MacOS, or Linux using [Oracle Virtual Box](https://virtualbox.org). If you wish to get started immediately and want to avoid installing dependencies, this VM provides an immediate start to INDI driver development without the need for extensive setup.

It is assumed that you are already familiar with INDI architecture and how it works.

### Virtual Machine Configuration

Upon importing the VirtualBox image, you will need to configure the network adapter. By default, the VM network is set to **Bridged Adapter** mode, requiring you to select a specific network adapter from your host machine. You can also switch the network mode to **NAT** if desired, depending on your network setup and requirements.

The VM is pre-configured to utilize **8GB of RAM** from the host machine and is set to use up to **16 CPU cores** for optimal performance during INDI driver development. As it is a development machine that requires heavy resources, please make sure the RAM is at least 8GB and assign a minimum of 4 CPU cores. You can adjust these settings in VirtualBox Manager after importing the VM to match your host machine's resources and your specific development needs.

The default username and password for the Virtual Machine is `indi`.

### Download the Virtual Machine Image

The 14GB VirtualBox image is available for download via Google Drive. To prevent abuse, access is granted upon request. When requesting the image from Google Drive, specify why you want to use the Virtual Machine for INDI driver development, and we will grant you access.

[![Download](https://img.shields.io/badge/Google%20Drive-Download-4285F4?style=for-the-badge&logo=googledrive&logoColor=white)](https://drive.google.com/file/d/13YbfyRZrUSArsJEasKeUzk-atR7ovn-6/view?usp=sharing)

<a href="" class="btn btn-primary">Download INDI Development VM</a>

### Virtual Machine Tutorial

Learn how to get started with the VM and setup your first project.

[![Watch the video](https://img.youtube.com/vi/RWlWkAJWHgY/maxresdefault.jpg)](https://youtu.be/RWlWkAJWHgY)

