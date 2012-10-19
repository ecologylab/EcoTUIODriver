EcoTUIODriver
=============

Diver to convert tuio touch events into windows touch events. Supports both Windows 7 & 8. Started as GSoC 2012 project.
EcoTUIODriver uses the [vmulti](http://code.google.com/p/vmulti/) project.

Original Authors: Rajat Goyal and Bill Hamilton
Liscense: [MIT](https://github.com/ecologylab/EcoTUIODriver/blob/master/LISCENSE.txt)

##INSTALL 
1. Download the [installer](https://github.com/downloads/ecologylab/EcoTUIODriver/EcoTuioDriver.exe)

2. Run the installer.

##Configuration Utility 

If you used the installer , the configuration utility will automatically be added in your startmenu and desktop. Otherwise you can manually locate it at $INSALL_DIR/executables/Configuration/Configuration_Utility.exe

![ "Screenshot" ](http://imageshack.us/a/img827/8/ecotuioconf.png)
				
Start the service to convert TUIO events to windows touch events. 

EcoTUIODriver supports up to 5 TUIO sensors simultaneously on distinct UDP ports.
Sensors may be mapped to displays by hitting the "ASSIGN DISPLAYS TO SENSORS" button, which runs a native windows mapping utility.


*** Note: On uninstall we are currently unable to uninstall the actual virtual drivers. This has been an issue with vmulti the virtual driver that we use.