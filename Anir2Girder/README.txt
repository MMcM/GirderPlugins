Animax Anir Remote plugin for Girder 3.2.x
==========================================

The serial communication is based on code made by PJ Naughter (See Anir32.h/cpp)
http://www.naughter.com/.


The rest is plain and simple straight forward using the Girder 3.2 plugin API.
Check out http://www.girder.nl


Event Strings
=============
The event string Girder receives has the following format:

$name:$key:$repeat

name	- The name of the device (changeable, default "Anir")
key		- Key value
repeat	- Holding buttun gives "1", otherwise "0"
		  (This is always "0" if repeat events are disabled)



Where to get it
===============
http://girderplugins.sourceforge.net/
http://sourceforge.net/projects/girderplugins/

Gider can be found at
http://www.girder.nl/
