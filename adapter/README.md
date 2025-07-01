# GenericLinuxIO Demo and Adapter Setup
This demo showcases the three modes working at the same time. To setup the demo environment you can refer to the three demos [Analog-Digital values Demo](../advalues/README.md), [Chardev Demo](../chardev/README.md) and [GPIO Demo](../gpio/README.md).

In the following diagram you can see the whole setup. It illustrates the data flow going through each component involved.
```
            +----[ adchip1 ]----+
            |                   |
            +-------------------+
                           |                                                 SIL Kit topics:
    +----[ adchip0 ]----+  |
    |                   |  |
    +-------------------+  |                                       |  -- >         ...        > --  |
                    |      |   +---[ SIL Kit Participant ]---+     |  -- >    fromVoltage15   > --  |     +--[ CANoe ]--+
+--[ fifo1 ]--+     |      |__ |                             |     |  -- >     fromFifo2      > --  | --- |             |
|             | __  |_________ |                             |     |  -- > fromGpiochip0Line2 > --  |     +-------------+
+-------------+   \___________ |                             |     |  -- > fromGpiochip1Line2 > --  |     
                   ___________ | SilKitAdapterGenericLinuxIO | --- |                                |     +--[ Demos ]--+
+--[ fifo2 ]--+   /   ________ |                             |     |  -- <     toVoltage32    < --  | --- |   Forward   |
|             |__/   |      __ |                             |     |  -- <       toFifo1      < --  |     +-------------+
+-------------+      |     |   +-----------------------------+     |  -- <   toGpiochip1Line1 < --  |
                     |     |                                       |  -- <         ...        < --  |
    +---[ gpiochip0 ]---+  |
    |                   |  |
    +-------------------+  |
                           |
            +---[ gpiochip1 ]---+
            |                   |
            +-------------------+
```

## YAML devices configuration file
### Configure the Linux IO paths
In order to match the above setup, you need to adapt Linux IO ``path`` attributes in the adapter configuration file ``./adapter/demos/DevicesConfig.yaml``. 

# Running the Demo
Now is a good point to start the ``sil-kit-registry`` and the ``sil-kit-adapter-generic-linux-io``. In separate terminals:
```
/path/to/SilKit-x.y.z-$platform/SilKit/bin/sil-kit-registry --listen-uri 'silkit://0.0.0.0:8501'
    
./bin/sil-kit-adapter-generic-linux-io --log Debug --adapter-configuration ./adapter/demos/DevicesConfig.yaml
```

You should see the following output:
```
[date time] [SilKitAdapterGenericLinuxIO] [info] Creating participant 'SilKitAdapterGenericLinuxIO' at 'silkit://localhost:8501', SIL Kit version: <version>
[date time] [SilKitAdapterGenericLinuxIO] [info] Connected to registry at 'tcp://127.0.0.1:8501' via 'tcp://127.0.0.1:37918' (local:///tmp/SilKitRegic044495579071f55.silkit, silkit://localhost:8501)
[date time] [SilKitAdapterGenericLinuxIO] [info] Creating participant SilKitAdapterGenericLinuxIO with registry silkit://localhost:8501
...
[date time] [SilKitAdapterGenericLinuxIO] [debug] Serializing and publishing initial values
[date time] [SilKitAdapterGenericLinuxIO] [debug] Serializing data and publishing on topic: fromFifo1
[date time] [SilKitAdapterGenericLinuxIO] [debug] Serializing data and publishing on topic: fromFifo2
[date time] [SilKitAdapterGenericLinuxIO] [debug] Serializing data and publishing on topic: fromVoltage15
[date time] [SilKitAdapterGenericLinuxIO] [debug] Serializing data and publishing on topic: fromVoltage32
[date time] [SilKitAdapterGenericLinuxIO] [debug] Serializing data and publishing on topic: fromPIN12Value
[date time] [SilKitAdapterGenericLinuxIO] [debug] Serializing data and publishing on topic: fromPIN12Dir
[date time] [SilKitAdapterGenericLinuxIO] [debug] Serializing data and publishing on topic: fromGpiochip0Line0
[date time] [SilKitAdapterGenericLinuxIO] [debug] Serializing data and publishing on topic: fromGpiochip0Line1
[date time] [SilKitAdapterGenericLinuxIO] [debug] Serializing data and publishing on topic: fromGpiochip0Line4
[date time] [SilKitAdapterGenericLinuxIO] [debug] Serializing data and publishing on topic: fromGpiochip1Line1
...
Press CTRL + C to stop the process...
```

When the adapter starts it sends the initial values of the handled Linux IO.

The adapter will then produce output when data is written into the Linux IO or when the other participants send new values to them.

## Basic Forward Demo
First you can start each demo in separte terminals:
```
./bin/sil-kit-demo-glio-advalues-forward-device
    
./bin/sil-kit-demo-glio-chardev-forward-device

./bin/sil-kit-demo-glio-gpio-forward-device
```

You should see the following output in the GLIO Adapter window:
```
[date time] [SilKitAdapterGenericLinuxIO] [debug] New value received on toVoltage103
[date time] [SilKitAdapterGenericLinuxIO] [debug] Updating ./adchips/adchip0/in_voltage103
[date time] [SilKitAdapterGenericLinuxIO] [debug] New value received on toFifo2
[date time] [SilKitAdapterGenericLinuxIO] [debug] Updating ./chardevs/fifo2
[date time] [SilKitAdapterGenericLinuxIO] [debug] Serializing data and publishing on topic: fromFifo2
[date time] [SilKitAdapterGenericLinuxIO] [debug] New values received on topic: toGpiochip1Line2
[date time] [SilKitAdapterGenericLinuxIO] [debug] Updating gpiochip1 line 2
```

For running the forward demo with all Linux IOs connected, just repeat the manual steps (*cat* and *echo* commands) to observe and stimulate the values for each mode ([Advalues](../advalues/README.md#basic-forward-demo), [Chardev](../chardev/README.md#basic-forward-demo) and [GPIO](../gpio/README.md#basic-forward-demo)).

## Adding CANoe (17 SP3 or newer) as a participant
Before you can connect CANoe to the SIL Kit network you should adapt the ``RegistryUri`` in ``./adapter/demos/SilKitConfig_CANoe.silkit.yaml`` to the IP address of your system where your sil-kit-registry is running.

### CANoe Desktop Edition
Load the ``GLIOControl_device.cfg`` from the ``./adapter/demos/CANoe`` directory. After starting the demo, the current file states appear in the *Data Window*. Then you can see the files being updated and you can send new values to files using the *DemoPanel*. Optionally you can also start the test unit execution of included test configuration. While the demo is running these tests should be successful. They bring together tests from the three modes.

### CANoe4SW Server Edition (Windows)
You can also run the same test set with ``CANoe4SW SE`` by executing the following powershell script ``./adapter/demos/CANoe4SW_SE/run.ps1``. The test cases are executed automatically and you should see a short test report in powershell after execution.

### CANoe4SW Server Edition (Linux)
You can also run the same test set with ``CANoe4SW SE (Linux)``. At first you have to execute the powershell script ``./adapter/demos/CANoe4SW_SE/createEnvForLinux.ps1`` on your windows system by using tools of ``CANoe4SW SE (Windows)`` to prepare your test environment for Linux. In ``./adapter/demos/CANoe4SW_SE/run.sh`` you should adapt ``canoe4sw_se_install_dir`` to the path of your ``CANoe4SW SE`` installation in your Linux system. Afterwards you can execute ``./adapter/demos/CANoe4SW_SE/run.sh`` in your Linux system. The test cases are executed automatically and you should see a short test report in your terminal after execution.

## Running the demo applications inside a Docker container (Optional)
*Note: This section provides an alternative method for running the demo applications - apart from CANoe Desktop Edition and CANoe4SW Server Edition - inside a Docker container and using the `devcontainers` Visual Studio Code extension. The steps outlined here are optional and not required if you prefer to run the applications directly and manually on your host machine.*

The following tools are needed:
* Visual Studio Code in Windows
* An Ubuntu Virtual Machine with Linux kernel version v5.17-rc1 or higher and Docker running as a daemon (a standard WSL2 is not compatible because it does not support GPIO by default)
    > You can use the *ms-vscode-remote.remote-ssh* Visual Studio Code extension to connect your Visual Studio Code to the Ubuntu Virtual Machine.
* *ms-vscode-remote.remote-containers* Visual Studio Code extension

>In WSL2, it is advisable to use the native filesystem (such as `/home/`) rather than the mounted `/mnt/` filesystem to prevent any performance issues.

### Steps:


1- Clone the repo on your Virtual Machine (if you have not done that yet).
 
2- On your Ubuntu host, run the `gpio/demos/create_gpio_sim.sh` script as root to instantiate the GPIO chips.
    
    sudo ./gpio/demos/create_gpio_sim.sh 
 
These GPIO chips (`gpiochip0` and `gpiochip1`) will be automatically mounted and accessible on the `/dev` directory of the Docker container. 

3- Open the cloned folder remotely from an instance of Visual Studio Code via SSH (using the *ms-vscode-remote.remote-ssh* Visual Studio Code Extension).

4- A pop-up will appear and propose to open the project in a container.

![Dev Containers popup](../adapter/demos/images/dev-container-popup.png)

Alternatively, you can click on the Dev Containers button at the bottom-left corner of Visual Studio Code, then click on `Reopen in Container`. 

Wait for the Docker image to be built and for the container to start. After that, you can launch the available pre-defined tasks in the Advalues section to acheive the demo setup. 

> The Docker container exposes the TCP/IP port 8501 to the host, which means that adding CANoe as a participant in the following steps shall work out-of-the box if you set SIL Kit's registry-uri to `silkit://localhost:8501`.  