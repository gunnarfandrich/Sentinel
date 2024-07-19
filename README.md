# Sentinel

The Sentinel project aims to create an all-encompassing, deployable field solution to monitor plant growth and crop disease over extended periods of time.

## Features

### Components
* (Two) Intel RealSense D415
* Nvidia Jetson Orin Nano

### Compute
#### Jetson Orin Nano Developer Kit
* 40 TOPS
* 1024-core NVIDIA Ampere architecture GPU with 32 Tensor Cores
* GPU: 625 MHz
* CPU: 6-core Arm® Cortex®-A78AE v8.2 64-bit CPU
  * Cache: 1.5MB L2 + 4MB L3
  * Max Frequency: 1.5 GHz
* Memory: 8GB LPDDR5
  * 128-bit Width, 68 GB/s

#### Particle Photon 2
* Realtek RTL8721DM MCU
  * ARM Cortex M33 CPU
  * Max Frequency: 200 MHz
  * Memory: 3072 KB
  * Program Storage: 2048 KB
* 802.11a/b/g/n Wi-Fi, 2.4 GHz and 5 GHz
* Bluetooth Low Energy 5.0

### Storage
* 512gb PCIe M.2 Gen3 M-Key x4 NVME

### Connectivity
* 802.11AC WiFi
* LoRaWAN
* (Not currently in use) Bluetooth 5.0
* (One) RJ45 Gigabit Ethernet Connection

### Expansion (Not currently in use)
* M.2 Gen3 M-key x2 Slot
* UHS-1 with SDR104 Mode TF/SD Card Slot
* (Two) MIPI CSI-2 22-pin Camera Connector

### Power Consumption
* 7W to 15W, dependent on CPU power configuration.



## Documentation
* [Jetson Orin Flashing](./docs/Flashing_Jetson.pdf)

Last Updated: 7/19/24
