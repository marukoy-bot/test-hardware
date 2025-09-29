//Create a "debug" folder inside the library folder and copy-paste this file inside that folder
// PlatformIO: C:\Users\<username>\.platformio\lib
// Arduino IDE: C:\Users\<username>\Documents\Arduino\libraries
#ifndef DEBUG_H
#define DEBUG_H

//Set 1 to enable, 0 to disable
#define DEBUG 1

#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#elif DEBUG == 0
#define debug(x)
#define debugln(x)
#endif

#endif