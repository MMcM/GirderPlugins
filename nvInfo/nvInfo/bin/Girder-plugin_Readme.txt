This lua plugin will go parse the list of ACTIVE displays on an nVidia video card.  I have tested successfully on my 6800GT with Dual DVI monitors attached.  I am using this plugin on my HTPC with one digital DVI projector attached and one Analog CRT monitor attached.  It works on these.  I use it to detect which display is active and display it on my front LCD 2X16 screen.  

This code is just provided as a starting point to those who find it useful. There is a lot of functionality one could add quite simply. I might add some more functionality as I need it.

This code is based in part on ref code from Promixis (sdks) and nVidia...ref code at(specifically NVIDIA Control Panel API sample): http://download.nvidia.com/developer/SDK/Individual_Samples/samples.html

nvInfo returns an ACTIVE display list of the format:
#<Display type>,<Display index>
Display type = A (analog) or D (digital/DVI) or ? for TVout
Display index = diplay number..it will be A, B, C ...
Example with two active DVI monitors attached:
#DA,DB

NOTED LIMITATION: If you have two analog CRT displays attached to your system and have one or the other active, I believe each will display #AA, which effectively makes this plugin useless. 

You need to drop nvInfo.dll into your girder plugins direcory.

the included GML is some lua code to show you how to call, parse and use this.

developers: 
The Visual studio workspace you want is located at:
this thing is missing the girder.h file.  Seems everybody else does this to make sure you are compiling with the newest.  It can be gotten from the promixis website.
Girder 3.x plugin\refcodelua\nvDisplay
there is a dummy arg that is parsed but not used right now.  It could be used to put an option of what data a user wants returned instead of just the current active display.

Disclaimer: This software has no warranties expressed or implied.  Use at your own risk
-Sundance