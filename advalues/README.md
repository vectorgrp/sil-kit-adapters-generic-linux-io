# Analog-Digital values and Adapter Setup
This demo consists of a set of files, illustrating a chip device which is connected to the SIL Kit via ``sil-kit-adapter-generic-linux-io`` as a SIL Kit participant. Bytes are transmitted to and from the other participants through SIL Kit publish/subscribe API.

In the following diagram you can see the whole setup. It illustrates the data flow going through each component involved.
```
+----[ Chip files ]----+
|  ./adchips/adchip0/  |
|  |-- in_voltage103   |
|  |-- in_voltage15    |
|  |-- out_voltage32   |                                            SIL Kit topics:
+----------------------+
                |                                            |  -- >       ...      > --  |     +--[ SIL Kit Participant ]--+
                 \______ +---[ SIL Kit Participant ]---+     |  -- > fromVoltage103 > --  | --- |          CANoe            |
                         |                             |     |  -- >  fromPIN12Dir  > --  |     +---------------------------+
                         | SilKitAdapterGenericLinuxIO | --- |                            |
                  ______ |                             |     |  -- <   toVoltage15  < --  |     +--[ SIL Kit Participant ]--+
                 /       +-----------------------------+     |  -- <   toVoltage5   < --  | --- |       ForwardDevice       |
                |                                            |  -- <       ...      < --  |     +---------------------------+
+----[ Chip files ]----+
|  ./adchips/adchip1/  |
|  |-- PIN12           |
|  |   |-- value       |
|  |   |-- direction   |
|  |-- out_voltage5    |
+----------------------+
```

**Note:** In order to trigger the events happening on the files, Linux inotify watchers are instanciated with the ``IN_CLOSE_WRITE`` flag. This means events are only triggered when a file opened for writing is closed again afterwards.

## Create the adchips
In order to get the same setup on your local machine, you can run the following script:
```
./advalues/demos/create_adchips.sh
```

This script will create adchip0 and adchip1 with their respective files into a main folder ``adchips``.

## YAML devices configuration file
### Configure the adchip paths
In order to match the above setup, you need to adapt the adchip ``path`` attributes in the adapter configuration file ``./advalues/demos/DevicesConfig.yaml``.

### Configure the data types
Each file can also be configured to deal with a specific data type. The handled data types are the ones available in SIL Kit publish/subscribe API:
- Integer values: ``uint8_t``/``int8_t`` to ``uint64_t``/``int64_t``
- Floating-point values: ``float``, ``double``

# Running the Demos
Now is a good point to start the ``sil-kit-registry`` and the ``sil-kit-adapter-generic-linux-io``. In separate terminals:
```
/path/to/SilKit-x.y.z-$platform/SilKit/bin/sil-kit-registry --listen-uri 'silkit://0.0.0.0:8501'
    
./bin/sil-kit-adapter-generic-linux-io --adapter-configuration ./advalues/demos/DevicesConfig.yaml --log Debug
```

You should see the following output:
```
[2024-01-31 14:48:53.138] [SilKitAdapterGenericLinuxIO] [info] Creating participant 'SilKitAdapterGenericLinuxIO' at 'silkit://localhost:8501', SIL Kit version: 4.0.44
[2024-01-31 14:48:53.139] [SilKitAdapterGenericLinuxIO] [info] Connected to registry at 'tcp://127.0.0.1:8501' via 'tcp://127.0.0.1:55898' (local:///tmp/SilKitRegic044495579071f55.silkit, silkit://localhost:8501)
[2024-01-31 14:48:53.141] [SilKitAdapterGenericLinuxIO] [info] Creating participant SilKitAdapterGenericLinuxIO with registry silkit://localhost:8501
...
[2024-01-31 14:48:53.144] [SilKitAdapterGenericLinuxIO] [debug] Serializing and publishing initial values
[2024-01-31 14:48:53.144] [SilKitAdapterGenericLinuxIO] [debug] Serializing data and publishing on topic: fromVoltage15
[2024-01-31 14:48:53.144] [SilKitAdapterGenericLinuxIO] [debug] Serializing data and publishing on topic: fromVoltage32
[2024-01-31 14:48:53.144] [SilKitAdapterGenericLinuxIO] [debug] Serializing data and publishing on topic: fromPIN12Value
[2024-01-31 14:48:53.144] [SilKitAdapterGenericLinuxIO] [debug] Serializing data and publishing on topic: fromPIN12Dir
...
Press CTRL + C to stop the process...
```
When the adapter starts it sends the initial values of the handled files.

Further traffic will pass trough the adapter whenever data is written into the files handled by it or when the other participants send new values to the corresponding topics.

## Basic Forward Demo
The basic forward demo consists of forwarding the data from ``out_voltage32`` to ``in_voltage103`` using their respective topics ``fromVoltage32``, ``toVoltage103``.

The data flow is illustrated in the following drawing:
```
                                                                SIL Kit topics:

+--[ out_voltage32 ]--+      +---[ SIL Kit Participant ]---+   > fromVoltage32 >
|                     | ---> |                             |       ---------- 
+---------------------+      |                             |     /            \     +--[ SIL Kit Participant ]--+
                             | SilKitAdapterGenericLinuxIO | ---                --- |       ForwardDevice       |
+--[ in_voltage103 ]--+      |                             |     \            /     +---------------------------+
|                     | <--- |                             |       ----------
+---------------------+      +-----------------------------+    < toVoltage103 <
```

You can start the forward device:
```
./bin/sil-kit-demo-glio-advalues-forward-device
```

You should see the following output:
```
Creating participant 'ForwardDevice' at silkit://localhost:850
[2024-01-31 14:49:00.110] [ForwardDevice] [info] Creating participant 'ForwardDevice' at 'silkit://localhost:8501', SIL Kit version: 4.0.44
[2024-01-31 14:49:00.111] [ForwardDevice] [info] Connected to registry at 'tcp://127.0.0.1:8501' via 'tcp://127.0.0.1:45286' (local:///tmp/SilKitRegic044495579071f55.silkit, silkit://localhost:8501)
Press enter to stop the process...
GLIO Adapter  >> ForwardDevice: 3
ForwardDevice >> GLIO Adapter : 3
```

The GLIO Adapter published the initial values, it means ``3`` is the initial value from the file ``out_voltage32``. Then the forward demo is publishing the value in order to update ``in_voltage103``.

On the GLIO Adapter terminal you should see two new lines:
```
[2024-01-31 14:49:00.116] [SilKitAdapterGenericLinuxIO] [debug] New value received on toVoltage103
[2024-01-31 14:49:00.116] [SilKitAdapterGenericLinuxIO] [debug] Updating ./adchips/adchip0/in_voltage103
```

In your adchip0 folder you can see the updated value in ``in_voltage103``:
```
cat ./adchips/adchip0/in_voltage103
3
```

Then you can update ``out_voltage32``:
```
echo 2 > ./adchips/adchip0/out_voltage32
cat ./adchips/adchip0/in_voltage103
2
```

The new value should be reported by the ForwardDevice as well.

**Note:** Using a text editor to read and write to the previously-mentioned files is not recommended. You should stick to using *cat* and *echo* Linux commands.

## Adding CANoe (17 SP3 or newer) as a participant
Before you can connect CANoe to the SIL Kit network you should adapt the ``RegistryUri`` in ``./advalues/demos/SilKitConfig_CANoe.silkit.yaml`` to the IP address of your system where your sil-kit-registry is running.

### CANoe Desktop Edition
Load the ``GLIOControl_advalues_device.cfg`` from the ``./advalues/demos/CANoe`` directory. After starting the demo, the current file states appears in the *Data Window*. Then you can see the files update in this *Data Window*, and you can send data to files through the boxes available in the *DemoPanel*. Optionally you can also start the test unit execution of included test configuration. While the demo is running these tests should be successful. They test several data types by setting 3 to in_voltage15 and 0 to PIN12/value, then send these new values and receive them.

### CANoe4SW Server Edition (Windows)
You can also run the same test set with ``CANoe4SW SE`` by executing the following powershell script ``./advalues/demos/CANoe4SW_SE/run.ps1``. The test cases are executed automatically and you should see a short test report in powershell after execution.

### CANoe4SW Server Edition (Linux)
You can also run the same test set with ``CANoe4SW SE (Linux)``. At first you have to execute the powershell script ``./advalues/demos/CANoe4SW_SE/createEnvForLinux.ps1`` on your windows system by using tools of ``CANoe4SW SE (Windows)`` to prepare your test environment for Linux. In ``./advalues/demos/CANoe4SW_SE/run.sh`` you should adapt ``canoe4sw_se_install_dir`` to the path of your ``CANoe4SW SE`` installation in your Linux system. Afterwards you can execute ``./advalues/demos/CANoe4SW_SE/run.sh`` in your Linux system. The test cases are executed automatically and you should see a short test report in your terminal after execution.
