s2jmidi
=======

Serial to Jack-MIDI conversion program

This program reads music informaiton form /dev/ttyAMA0, and sends it to JACK 
(Jack Audio Connection Kit). Currently the serial port /dev/ttyAMA0 is defined
in the source file, and corresponds to the serial port at GPIO 10 of a
Raspberry Pi B, running Raspbian Wheezy. 

This program expects a four byte frame at the serial input of the device 
running this program. The format at the serial input should be as follows:

+------------------+------------------+------------------+------------------+
| Frame Header     | Keying- Press/   | Note identifier  | Velocity/        |
|                  |   Release        |                  |    Intensity     |
| 0xFA             | 0x90 - Key Press | eg: C4 = 60      | eg: for medium,  |
|                  | 0x80 - Release   |     C5 = 72      |        60        |
+------------------+------------------+------------------+------------------+

