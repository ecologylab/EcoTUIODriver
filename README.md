WinTUIODriver
=============

Diver to convert tuio touch events into windows touch events. Started as GSoC 2012 project.


##INSTALL 

###Method 1 : 
1.Download the [source](https://github.com/ecologylab/WinTUIODriver/zipball/master) or checkout from "git://github.com/ecologylab/WinTUIODriver.git"

2.Extract .

3.Install using winTUIODriver.exe


###Method 2 :

1.Download the [source](https://github.com/ecologylab/WinTUIODriver/zipball/master) 

2.Extract .

3.Go to vmulti-x64 directory if your computer is 64bit otherwise goto vmulti-x86 .

4.Right click on installvmulti.cmd and chooose run as administrator .

5.Run the configuration utility from the $INSALL_DIR/executables folder . 


##Configuration Utility 

If you used the installer , the configuration utility will automatically be added in your startmenu and startup . Ohterwise you can manually locate it at $INSALL_DIR/executables/Configuration/Configuration_Utility.exe

![ "Screenshot" ](http://img802.imageshack.us/img802/504/wintuiodriver.png)
				
Start the service to convert TUIO events to windows touch events . 


***Important Note : The configuration utlity supports 5 multiple monitor and 5 multiple touch sensor but multiple sensors are not supported by the driver yet , so this will work only for single monitor . 