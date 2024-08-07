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
[date time] [SilKitAdapterGenericLinuxIO] [info] Creating participant 'SilKitAdapterGenericLinuxIO' at 'silkit://localhost:8501', SIL Kit version: 4.0.44
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
