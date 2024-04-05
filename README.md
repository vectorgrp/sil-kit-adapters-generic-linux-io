# Vector SIL Kit Adapter for Generic Linux IO
This collection of software is provided to illustrate how the [Vector SIL Kit](https://github.com/vectorgrp/sil-kit/)
can be attached to a generic Linux IO device such as character devices or GPIO chips.

This repository contains instructions to build the adapter and set up a minimal working development environment.

The main contents are working examples of necessary software to connect the running system to a SIL Kit environment,
as well as complimentary demo applications for some communication to happen.

Those instructions assume you use a Linux OS (or a virtual machine running a Linux image) for building and running the adapter and use ``bash`` as your interactive
shell.

## a) Getting Started with self build Adapters and Demos
This section specifies steps you should do if you have just cloned the repository.

To get started, please change your current directory to the top-level in the ``sil-kit-adapters-generic-linux-io``
repository:

    cd /path/to/sil-kit-adapters-generic-linux-io

### Fetch Third Party Software
The first thing that you should do is initializing the submodules to fetch the required third party software:

    git submodule update --init --recursive

### Build the Adapters and Demos
To build the demos, you'll need SIL Kit packages ``SilKit-x.y.z-$platform`` for your platform. You can download them directly from [Vector SIL Kit Releases](https://github.com/vectorgrp/sil-kit/releases).

The adapters and demos are built using ``cmake``. If you want to build the adapter against a specific downloaded release of SIL Kit, you can follow these steps:

    mkdir build
    cmake -S. -Bbuild -DSILKIT_PACKAGE_DIR=/path/to/SilKit-x.y.z-$platform/ -D CMAKE_BUILD_TYPE=Release
    cmake --build build --parallel

**Note 1:** If you have installed a self-built version of SIL Kit, you can build the adapter against it by setting SILKIT_PACKAGE_DIR to the installation path, where the bin, include and lib directories are.

**Note 2:** If you have SIL Kit installed on your system, you can build the adapter against it, even by not providing SILKIT_PACKAGE_DIR to the installation path at all. Hint: Be aware, if you are using WSL2 this may result in issue where your Windows installation of SIL Kit is found. To avoid this specify SILKIT_PACKAGE_DIR.

**Note 3:** If you don't provide a specific path for SILKIT_PACKAGE_DIR and there is no SIL Kit installation on your system, a SIL Kit release package (the default version listed in CMakeLists.txt) will be fetched from github.com and the adapter will be built against it.

The adapters and demo executables will be available in the bin directory. Additionally the SilKit shared library is copied to the lib directory next to it automatically.

## b) Getting Started with pre-built Adapters and Demos
Download a preview or release of the Adapters directly from [Vector SIL Kit Adapters Releases](https://github.com/vectorgrp/sil-kit-adapters-generic-linux-io/releases).

If not already existent on your system you should also download a SIL Kit Release directly from [Vector SIL Kit Releases](https://github.com/vectorgrp/sil-kit/releases). You will need this for being able to start a sil-kit-registry.

## Install the sil-kit-adapter-generic-linux-io (optional)
If you call the following command (can be done for self build and pre build package after cmake configure) ``sil-kit-adapter-generic-linux-io`` can be called from everywhere without defining a path:  

    sudo cmake --build build --target install

The default installation path will be ``/usr/local/bin``. Be aware that SIL Kit itself also needs to be installed to make this work.

## Run the sil-kit-adapter-generic-linux-io
This application allows the user to attach character devices of any Linux system to the Vector SIL Kit.

Before you start the adapter there always needs to be a sil-kit-registry running already. Start it e.g. like this:

    ./path/to/SilKit-x.y.z-$platform/SilKit/bin/sil-kit-registry --listen-uri 'silkit://0.0.0.0:8501'

It is also necessary that the involved character devices or GPIO chips exist before the ``sil-kit-adapter-generic-linux-io`` is started. 

The application *must* be started with the following command line argument:
    
    sil-kit-adapter-generic-linux-io --adapter-configuration <path to .yaml devices configuration file>

The application *optionnally* takes the following command line arguments (default between curly braces):

    sil-kit-adapter-generic-linux-io 
      [--configuration <path to .silkit.yaml or .json configuration file>]
      [--name <participant name{SilKitAdapterGenericLinuxIO}>]
      [--registry-uri silkit://<host{localhost}>:<port{8501}>]
      [--log <Trace|Debug|Warn|{Info}|Error|Critical|Off>]
      [--help]

### Configure the publish and subscribe topics
The adapter uses the publish/subscribe SIL Kit API in order to send and receive data through the participants. This means the user has to specify for each [file/character device/gpio line] a publish topic name
and/or a subscribe topic name in the adapter configuration file. These topics must be unique.
Some examples of configuration files can be found in the following demos.

**Note:** The publish topic is used to send data from the adapter to other participants.
The subscribe topic is used by the adapter to receive data from the other participants.

## Analog-Digital values Demo
The aim of this demo is to showcase the adapter forwarding bytes with an attached data type from and to a set of files through Vector SIL Kit. This demo illustrates how an analog-digital-converted values can be covered by the adapter.

This demo is further explained in [advalues/README.md](advalues/README.md).

## Chardev Demo
The aim of this demo is to showcase the adapter forwarding bytes from and to a set of character devices through
Vector SIL Kit. In this demo the character devices used are FIFOs.

This demo is further explained in [chardev/README.md](chardev/README.md).

## GPIO Demo
The aim of this demo is to showcase the adapter forwarding data from and to a GPIO chip through
Vector SIL Kit. 

This demo is further explained in [gpio/README.md](gpio/README.md).

## GenericLinuxIO Demo
The aim of this demo is to showcase all the three modes working at the same time. 

This demo is further explained in [adapter/README.md](adapter/README.md).