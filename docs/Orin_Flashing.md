# Flashing the Jetson Orin Nano & Other/Older Variants

## Introduction
The Jetson Nano can prove difficult to flash due to the lack of documentation available online. This
document serves to fill in the gaps and aid in flashing all three developer kit models that exist at the
time of writing.

### Required Computer/Software
* A device with at least 64GB of storage running Ubuntu is required to
flash a Jetson Nano and its variants. Other linux distributions may work, though are untested.

  * Jetson Orin Nano: Ubuntu 20.04 or 22.04
  * (Older) Jetson Nano: Ubuntu 18.04

* An Nvidia account https://developer.nvidia.com/login

'''NOTE: It would be advised to simply install Ubuntu to a removable storage device (such as an SD card
or USB drive) to avoid issues with installing to your main computer. A Live-CD cannot be used for
flashing a Jetson due to its lack in storage capacity.'''

### Required Hardware
Jetson Nano Developer Kit (variants are listed below)
• P3448-0003
◦ 2GB model, without EMMC. Requires an SD card for the OS
to be installed to.
• P3448-0000
◦ 4GB model, without EMMC. Requires an SD card for the OS
to be installed to.
• P3448-0002
◦ 4GB model, with 16GB EMMC. Does NOT require an SD
card.
Standard Computer Jumper
• These can be borrowed from an old computer motherboard you
may have laying around.
◦ NOTE: A Female to Female breadboard wire will work in a
pinch.


(Micro-USB or USB-C) cable
• Depending on the model daughterboard to be flashed, a Micro-
USB or USB-C cable is needed to connect the board to another
computer.
Power Supply (12V barrel jack or Micro-USB or USB-C)
• Depending on the model daughterboard to be flashed, a 12V barrel
jack supply, Micro-USB, or USB-C power supply is necessary to
ensure a stable power supply while flashing.
Installation Process
Navigate to https://developer.nvidia.com/sdk-manager and download the .deb file.
Flashing Nvidia Jetson Nano Developer Kit
By: Gunnar Fandrich
April 30, 2023
Open a terminal and cd to the location of the downloaded file:
cd ~/Downloads
Attempt to install the sdkmanager:
sudo dpkg -i sdkmanager*.deb



It is likely some dependencies will be missing. Common dependencies can be installed with the
following command.
sudo apt-get install libgconf-2-4 libcanberra-gtk-module gconf-service gconf2-common
libcanberra-gtk0 gconf-service-backend
NOTE: If any additional dependencies are required, simply review the terminal log to find the missing
packages and install with:
sudo apt-get install <package name>
ex: sudo apt-get install libgconf-2-4


After installing necessary dependencies, attempt to install the sdkmanager again.
sudo dpkg -i sdkmanager*.deb
If no errors occur, proceed to the next step. Else, install any missing packages and repeat the install step
prior.
Once the sdkmanager is successfully installed without any errors, it may be launched with:
sdkmanager
On first launch, it will be necessary to sign into an Nvidia account to proceed. Once signed in, an
update window may open. Install updates as necessary and allow the sdkmanager to reopen.
Flashing Nvidia Jetson Nano Developer Kit
By: Gunnar Fandrich
April 30, 2023
Set the Jetson Nano Development kit to boot in recovery mode. This is done by utilizing a computer
jumper or female to female jumper wire to bridge the ‘FC_REC’ pin to any GND pin. The module has
been removed from its daughterboard in the following pictures to illustrate such process.


Once the recovery pin has been connected to ground, connect the power supply and Micro-USB or
USB-C cable to the computer running Ubuntu.


The sdkmanager should now auto-detect that a compatible device was connected.
Select your appropriate device as listed on Pg. 1
Pick an OS- Jetpack 4.6.3 will suffice.
No additional SDKs are necessary to install.
Proceed to Step 02.


Now select the software to be installed. It is fine to select all components. Read and accept the terms
and conditions given by Nvidia to proceed.
The device will now download the required software before flashing. Once complete, Step 03 will be
available to proceed to.


A window will open prompting the configuration of the Jetson. Your device should be the same as
selected prior. If not, select the correct device in the top drop-down menu.
Select the manual setup pertaining to the model you selected prior.
Select Pre-Config and choose a username and password.
NOTE: in this example, the Username and Password are “nvidia” and a 2GB board without EMMC is
being flashed. MAKE SURE to select your correct board type and MANUAL method, otherwise you
will have to repeat the process. If flashing a Jetson without EMMC, ensure an SD card is inserted.


The device will now flash. The execution process can be monitored via the sdkmanager.
NOTE: Be sure to not unplug the power supply or the connection to the computer from the device
during flashing. It would be wise to also ensure the flashing computer is not set to sleep, to avoid
flashing problems. Sleep can be configured from Settings/Power in Ubuntu.
Let the device flash and follow any prompts from the sdkmanager. The sdkmanager will create a usbinternet
tunnel into the device and configure the username and password as entered.
Post-Installation
As a tunnel to the device was created during installation, now is an optimal time to install any missing
packages. The IP of the device can be found with nmcli via the terminal. The default IP is:
192.168.55.1
SSH into the device as necessary nvidia@192.168.55.1 and install missing packages, such as WIFI, etc.
