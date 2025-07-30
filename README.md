# Vector SIL Kit Adapter for Generic Linux IO
[![Vector Informatik](https://img.shields.io/badge/Vector%20Informatik-rgb(180,0,50))](https://www.vector.com/int/en/)
[![SocialNetwork](https://img.shields.io/badge/vectorgrp%20LinkedIn®-rgb(0,113,176))](https://www.linkedin.com/company/vectorgrp/)\
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/vectorgrp/sil-kit-adapters-generic-linux-io/blob/main/LICENSE)
[![Linux Builds](https://github.com/vectorgrp/sil-kit-adapters-generic-linux-io/actions/workflows/build-linux-release.yaml/badge.svg)](https://github.com/vectorgrp/sil-kit-adapters-generic-linux-io/actions/workflows/build-linux-release.yaml)
[![SIL Kit](https://img.shields.io/badge/SIL%20Kit-353b42?logo=github&logoColor=969da4)](https://github.com/vectorgrp/sil-kit)

This collection of software is provided to illustrate how the [Vector SIL Kit](https://github.com/vectorgrp/sil-kit/)
can be attached to a generic Linux IO device such as character devices or GPIO chips.

This repository contains instructions to build the adapter and set up a minimal working development environment.

The main contents are working examples of necessary software to connect the running system to a SIL Kit environment,
as well as complimentary demo applications for some communication to happen.

Those instructions assume you use a Linux OS (or a virtual machine running a Linux image) for building and running the adapter and use ``bash`` as your interactive
shell.

**Note 1:** Advalues and chardev modes are supported in WSL/WSL2 if you use the Linux filesystem (not the mounted drives like `/mnt/..`). However there is no easy way to run the adapter's GPIO mode in WSL/WSL2 without recompiling the Linux kernel with the required module.

**Note 2:** Advalues and chardev modes can be used in QNX environments. In order to acheive that, you can cross-build the adapter for QNX systems using the provided CMake toolchain files inside the `cmake` folder.

## a) Getting Started with self-built Adapter and Demos
This section specifies steps you should do if you have just cloned the repository.

To get started, please change your current directory to the top-level in the ``sil-kit-adapters-generic-linux-io``
repository:

    cd /path/to/sil-kit-adapters-generic-linux-io

### Fetch Third Party Software
The first thing that you should do is initializing the submodules to fetch the required third party software:

    git submodule update --init --recursive

### Build the Adapter and Demos
To build the demos, you'll need SIL Kit packages ``SilKit-x.y.z-$platform`` for your platform. You can download them directly from [Vector SIL Kit Releases](https://github.com/vectorgrp/sil-kit/releases).

The adapter and demos are built using ``cmake``. If you want to build the adapter against a specific downloaded release of SIL Kit, you can follow these steps:

    mkdir build
    cmake -S. -Bbuild -DSILKIT_PACKAGE_DIR=/path/to/SilKit-x.y.z-$platform/ -D CMAKE_BUILD_TYPE=Release
    cmake --build build --parallel

**Note 1:** If you have installed a self-built version of SIL Kit, you can build the adapter against it by setting SILKIT_PACKAGE_DIR to the installation path, where the bin, include and lib directories are.

**Note 2:** If you have SIL Kit installed on your system, you can build the adapter against it, even by not providing SILKIT_PACKAGE_DIR to the installation path at all. Hint: Be aware, if you are using WSL2 this may result in issue where your Windows installation of SIL Kit is found. To avoid this specify SILKIT_PACKAGE_DIR.

**Note 3:** If you don't provide a specific path for SILKIT_PACKAGE_DIR and there is no SIL Kit installation on your system, a SIL Kit release package (the default version listed in CMakeLists.txt) will be fetched from github.com and the adapter will be built against it.

The adapter and demo executables will be available in the bin directory. Additionally the ``SilKit`` shared library is copied to the lib directory next to it automatically.

### Build the adapter for Android environments 
You can use the [Android NDK](https://developer.android.com/ndk) to cross-build the adapter for Android environments. 

Begin by cloning the SIL Kit repo and building SIL Kit using the cmake toolchain file provided by Android NDK and generating a package as follows: 

    cmake -S. -Bbuild -DCMAKE_TOOLCHAIN_FILE=/path/to/android-ndk/build/cmake/android.toolchain.cmake -DANDROID_ABI=x86_64 -DSILKIT_HOST_PLATFORM=android -DANDROID_PLATFORM=android-33 -DSILKIT_BUILD_TESTS=OFF -DSILKIT_BUILD_DEMOS=OFF

    cmake --build build --parallel --target package

Unzip the package that was generated and use the same toolchain file to cross-build the adapter, ensuring it is built against the unzipped package. 

    cmake -S.  -Bbuild -DCMAKE_TOOLCHAIN_FILE=/path/to/android-ndk/build/cmake/android.toolchain.cmake  -DANDROID_ABI=x86_64 -DCMAKE_BUILD_TYPE=Release -DSILKIT_PACKAGE_DIR=/path/to/SilKit-*-Linux-x86_64-clang-Release/  -DSilKit_DIR=/path/to/SilKit-*-Linux-x86_64-clang-Release/lib/cmake/SilKit -DANDROID_PLATFORM=android-33

    cmake --build build --parallel

Lastly, update the LD_LIBRARY_PATH in your Android environment to point to the location of the SIL Kit shared library, which can be found in the generated lib folder.

## b) Getting Started with pre-built Adapter and Demos
Download a preview or release of the adapter directly from [Vector SIL Kit Adapter for Generic Linux IO Releases](https://github.com/vectorgrp/sil-kit-adapters-generic-linux-io/releases).

If not already existent on your system you should also download a SIL Kit Release directly from [Vector SIL Kit Releases](https://github.com/vectorgrp/sil-kit/releases). You will need this for being able to start a sil-kit-registry.

## Install the sil-kit-adapter-generic-linux-io (optional)

### Installation with Debian package
On Debian systems, the most straightforward way to install the sil-kit-adapter-generic-linux-io is to use the Debian package `sil-kit-adapter-generic-linux-io_*.deb` which is provided with each release (version v1.0.0 and above).
After downloading it, you can install it using the following command:
```
sudo apt install ./sil-kit-adapter-generic-linux-io_*.deb
```
To get more information about this Debian package you can refer to the SIL Kit Adapter Packaging [README](https://github.com/vectorgrp/sil-kit-adapters-pkg).

**Note 1:** To be able to run the installed adapter you will also have to install the SIL Kit library. This can be done installing the `libsilkit4_*.deb` and `libsilkit-dev_*.deb` provided in the SIL Kit releases.

**Note 2:** After installing the adapter, you can run the ``sil-kit-adapter-generic-linux-io`` from any location without specifying a path. The default installation path is ``/usr/bin``.

### Installation with CMake
To install the sil-kit-adapter-generic-linux-io using CMake, run the following command (can be done for self-built and pre-built package after cmake configure):

    sudo cmake --build build --target install

**Note 1:** Be aware that SIL Kit itself also needs to be installed to run the adapter.

**Note 2:** After installing the adapter, you can run the ``sil-kit-adapter-generic-linux-io`` from any location without specifying a path. The default installation path is ``/usr/local/bin``.

## Run the sil-kit-adapter-generic-linux-io
This application allows the user to attach character devices of any Linux system to the Vector SIL Kit.

Before you start the adapter there always needs to be a sil-kit-registry running already. Start it e.g. like this:

    /path/to/SilKit-x.y.z-$platform/SilKit/bin/sil-kit-registry --listen-uri 'silkit://0.0.0.0:8501'

It is also necessary that the involved character devices or GPIO chips exist before the ``sil-kit-adapter-generic-linux-io`` is started. 

The application *must* be started with the following command line argument:
    
    sil-kit-adapter-generic-linux-io --adapter-configuration <path to .yaml devices configuration file>

The application *optionnally* takes the following command line arguments (default between curly braces):

    sil-kit-adapter-generic-linux-io 
      [--configuration <path to .silkit.yaml or .json configuration file>]
      [--name <participant name{SilKitAdapterGenericLinuxIO}>]
      [--registry-uri silkit://<host{localhost}>:<port{8501}>]
      [--log <Trace|Debug|Warn|{Info}|Error|Critical|Off>]
      [--version]
      [--help]

### Configure the publish and subscribe topics
The adapter uses the publish/subscribe SIL Kit API in order to send and receive data through the participants. This means the user has to specify for each [file/character device/gpio line] a publish topic name
and/or a subscribe topic name in the adapter configuration file. These topics must be unique.
Some examples of configuration files can be found in the following demos.

**Note:** The publish topic is used to send data from the adapter to other participants.
The subscribe topic is used by the adapter to receive data from the other participants.

## Analog-Digital values Demo
The aim of this demo is to showcase the adapter forwarding bytes with an attached data type from and to a set of files through SIL Kit. This demo illustrates how an analog-digital-converted values can be covered by the adapter.

This demo is further explained in [advalues/README.md](advalues/README.md).

## Chardev Demo
The aim of this demo is to showcase the adapter forwarding bytes from and to a set of character devices through
SIL Kit. In this demo the character devices used are FIFOs.

This demo is further explained in [chardev/README.md](chardev/README.md).

## GPIO Demo
The aim of this demo is to showcase the adapter forwarding data from and to a GPIO chip through
SIL Kit. 

This demo is further explained in [gpio/README.md](gpio/README.md).

## GenericLinuxIO Demo
The aim of this demo is to showcase all the three modes working at the same time. 

This demo is further explained in [adapter/README.md](adapter/README.md).
