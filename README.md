Giant Gecko 11
Prerequisites
You'll need the following software:
	• mbed CLI
	• Toolchain, this comes packaged with the Windows mbed CLI installer
	• Git
	• Segger J-Link Software
	• VS Code, or any text editor
   
Checkout
git clone https://github.com/PeterFoy/mbed-os-gg11-Si7201.git
cd mbed-os-gg11-Si7201
mbed deploy

Compiling
mbed compile --target EFM32GG11_STK3701 --toolchain GCC_ARM
Flashing
flash.bat (or ./flash.sh)
WARNING make sure you update attached flash.bat and flash.jlink files

flash.bat
JLink.exe -device EFM32GG11B820F2048 -if SWD -speed 2700 -commanderscript flash.jlink

flash.jlink
loadfile BUILD/EFM32GG11_STK3701/GCC_ARM/mbed-os-gg11-ethernet.bin 0x0000
r
exit

Export to Keil uVision (Only if Keil Project files are not included)
mbed export -i uvision5 -m EFM32GG11_STK3701

This project contains code from other projects. The original license text is included in those source files. They must comply with our license guide.
