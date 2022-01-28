# Electra PAS.17A Smart Intercom Adapter (Nano33Iot based)

## Special thanks to [Nicolae Simpalean for inspiration](https://simpalean.site/interfon/)
You can contact him for a ready made solution adapted to you, which you can install yourself or have Mr. Simpalean send you a new device that "just works" :)
If, however you like DIY work and have time on your hands, you can stay here and read a bit :) and use these resources free of change :).

If you like my work, and want to give something back, you can also [Buy me a beer](https://www.paypal.com/donate/?hosted_button_id=LH4JS85SDZPKN)

## What's this about
This is a "smart" adapter tailor made for Electra PAS.17A units, that allows you to get Push Notifications on your phone whenever your Intercom device is ringing and also control de device via Talk/Open commands.

Note: It will NOT forward audio to/from device.

## How does it work

The solution is composed from 3 components:

1. A device that interfaces with the Electra device, it needs to be installed inside the device and connected to the PAS.17A board via 3 singnal wires and 2 power wires (see below). This will detect device signals (ring, talk, open) and send commands (talk, listen, open) and will communicate via WebSockets with an application server. - this repository contains the source code and instructions for the Hadware part.

2. An application server, [see this page](https://github.com/adrian-dobre/Intercom-Server)

3. A mobile application - to be updated.

![App Demo](./board/images/app_demo.gif?raw=true)
![App Settings](./board/images/app_settings.png?raw=true)
![App Call Log](./board/images/app_call_log.png?raw=true)

## The "hardware part"
This section will try to describe how this works, how it needs to be connected and how it can be adapted, possibly, to other devices.


### Short overview on how it works
The device detects when the Intercom is ringing by monitoring a HIGH digital signal on one of the pins. Two other pins, associated to Talk/Listen and Open commands are pulled LOW to activate the controls (the Talk and Open buttons). These two other pins are also monitored (LOW means control is active) to provide feedback even if the physical Intercom (or another Mobile Client) uses the app - this way you know if somebody is already using the device and you don't need to do anything.

Current sollution is based on an Arduino Nano33Iot, but you can use any other device you want, you just probably need to change the WiFi/WebSocket libraries to your board, and, maybe change the pin configuration.

In the same way, you could probably adapt the solution to a different intercom models, you _just_ need to identify which pins you need to pull HIGH/LOW - this can be achieved by using a multimeter and checking which circuits activate when you press the buttons - of course different devices may work in entirely different ways and this task may not be that easy... 

### What do I need?

1. [Nano33Iot](https://store.arduino.cc/products/arduino-nano-33-iot) (or other boards but you need to tinker a bit with the code)
2. [A bidirectional logic level shifter](https://www.sigmanortec.ro/Modul-Translator-nivel-logic-I2C-IIC-bidirectional-4-canale-3-3V-p126421751) - Nano33Iot operates on 3.3v while the Electra PAS.17A works on 5v, which will fry your Nano33Iot - I used t
3. [A buck converter](https://cleste.ro/modul-coborare-tensiune-dc-dc-mini-mp1584en.html) to power your device from the Intercom power line directly (14v)
4. The code from this repo :)

### How do I connect everything

1. Use my board and just connect the wires :) - I made a small PCB (this is my first attempt at a PCB and diy electronics in general, sooo... please have mercy :P ) for the device, so it's easy to connect. Here are the [Gerber files](./board/pcb) for you to use - just connect the labeled pads to the intercom board as shown below. You can also try to [etch it yourself](./board/pcb/etch).

PCB
![PCB](./board/images/pcb.jpg?raw=true)
Device
![Device](./board/images/device.jpg?raw=true)
Connected Device
![Connected Device](./board/images/connected_device.jpg?raw=true)
Connections
![Connections](./board/images/connections.png?raw=true)

2. Use wires - Check out the breadboard below

Breadboard
![Breadboard](./board/images/breadboard.png?raw=true)
![Wired Device](./board/images/wired_connections.jpg?raw=true)
For device connections, see above.

### How do I use it
Connect eveything as shown above, compile the code, upload it to the board, configure WiFi credentials and WebSocket server address and credentials and that should be about it.

#### WiFi Configuration
On the first boot (or on double reset) ([see more here](https://github.com/khoih-prog/WiFiManager_NINA_Lite)) a new "Intercom" (password: "intercom") network should appear, connect to it and then navigate to 192.168.1.4 using a web browser and configure the WiFi Crendentials and WebSocket server settings.


### Library Dependencies for this project
I hope not to forget something, if I do, please let me know :) You can also check c_cpp_properties.json file for inclusions, but it contains more than needed - maybe I'll manage to clean it up at some point :|
1. [Adafruit Sleepy Dog](https://github.com/adafruit/Adafruit_SleepyDog)
2. [WiFiNINA Generic](https://github.com/khoih-prog/WiFiNINA_Generic)
3. [WebSockets Generic](https://github.com/khoih-prog/WebSockets_Generic)
4. [WiFiManager Nina Lite](https://github.com/khoih-prog/WiFiManager_NINA_Lite)

Of course, special thanks to all the devs that made those libs and made all this possible.
