# "BeagleWhisperer"-"Beaglo" Duo program
I'm developing a multi-purpose a program to automate tasks for the Beaglebone Black, intended to facilitate experimenting with the Beaglebone. It is built as a dual program. It consistst on the Beaglebone side of the program Beaglo and in the desktop computer side (until now: Windows-side) of the program BeagleWhisperer.exe.
#Beaglo
When 'Beaglo' runs on the Beaglebone it can accept orders from the command line to readout time/ ADC values/ . When the program runs on the Beaglebone it can read portvalues with values from the command line The 'Beaglo' program is developed , on a Beaglebone Black (BBB) with Ǻgstrom 3.8.13 installed. It uses some library functions of the gcc C++ libraries. Install gcc on your Beaglebone(opkg install gcc) to load the libraries. When the program runs under Debian on the Beaglebone you need to copy the ‘libstdc++.so.6’ library to the directory :’ /user/lib/arm-linux-gnueabihf/’ on the BBB It is developed with Eclipse Indigo (Service Release 2), in C++.

#BeagleWisperer.
The program ‘BeagleWhisperer’ for Windows is developed with the multiplatform RadStudio C++ development environment under Firemonkey. With a few minor adaptations it can be compiled for Apple IOS and Android. The desktop-computer program is in the stage of development. I’m planning to make a beta version in the first half of 2014. 

Visit http://www.bram-vanleeuwen.nl/BeagleWhisperer/BeagleWhisperer.htm to see the full description of both programs. There is also the newest version of the code to download.
