# Vector SIL Kit Adapter for GPIOs
This collection of software is provided to illustrate how the [Vector SIL Kit](https://github.com/vectorgrp/sil-kit/)
can be attached to a GPIO device.

This repository contains instructions to build the adapter and set up a minimal working development environment.

The main contents are working examples of necessary software to connect the running system to a SIL Kit environment,
as well as complimentary demo applications for some communication to happen.

Those instructions assume you use a Linux OS (or a virtual machine running a Linux image) for building and running the adapter and use ``bash`` as your interactive
shell.

## a) Getting Started with self build Adapters and Demos
This section specifies steps you should do if you have just cloned the repository.

Before any of those topics, please change your current directory to the top-level in the ``sil-kit-adapters-gpio``
repository:

    cd /path/to/sil-kit-adapters-gpio

### Fetch Third Party Software
The first thing that you should do is initializing the submodules to fetch the required third party software:

    git submodule update --init --recursive

### Build the Adapters and Demos
To build the demos, you'll need SIL Kit packages ``SilKit-x.y.z-$platform`` for your platform. You can download them directly from [Vector SIL Kit Releases](https://github.com/vectorgrp/sil-kit/releases).

Before building the adapter you have to install the package ``autoconf-archive`` on your machine. For ubuntu you can do it with the following command : 

    sudo apt install autoconf-archive -y

The adapters and demos are built using ``cmake``. If you want to build the adapter against a specific downloaded release of SIL Kit, you can follow these steps:

    mkdir build
    cmake -S. -Bbuild -DSILKIT_PACKAGE_DIR=/path/to/SilKit-x.y.z-$platform/ -D CMAKE_BUILD_TYPE=Release
    cmake --build build --parallel

**Note 1:** If you have installed a self-built version of SIL Kit, you can build the adapter against it by setting SILKIT_PACKAGE_DIR to the installation path, where the bin, include and lib directories are.

**Note 2:** If you don't provide a specific path for SILKIT_PACKAGE_DIR, a SIL Kit release package [SilKit-4.0.17-ubuntu-18.04-x86_64-gcc] will be fetched from github.com and the adapter will be built against it.

  
The adapters and demo executables will be available in ``build/bin`` (depending on the configured build directory).
Additionally the ``SilKit`` shared library is copied to that directory automatically.