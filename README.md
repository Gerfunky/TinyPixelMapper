# TinyPixelMapper
a Pixelmapping software for the ESP8266 for addressible LED Strips, with a OSC controll interface.

# Work In progress 
The SW is working this Documentation + wiki is still missing.

# TinyPixelMapper : What is it?
It is a PixelMapping software for a ESP8266 chip. (will be changend to also support the ESP32 in the future)

The main LED driving library is FastLED.

Configuration is done over OSC, TouchOSC interfaces are included.

Ther are 2 main modes: Artnet and Normal.

In the Artnet mode it becomes a ARTNET node on the network. (hardcoded to 4 universes max, no mapping at the moment)

In Normal mode it playes Palletes or Colors from FFT data.

You can configre Strips, Forms(a collection of strips).

There are effects that can be added to each strip/form.

It should work with any ESP8266 it was designed on a Adafruit Huzzah and is configured to use a real ESP8266 PIN numbering.
default pins are LED_DATA_PIN 12  and  LED_CLK_PIN 13  (leds.h).

FFT data can be sent to the unit over udp. There is an app in programming to send the FFT data to the unit. 
The file  extras/TD-TinyPixelMapper/TD-TinyPixelMapper.toe is a TouchDesigner Sketch to send the audio imput from a mic out over udp to the units.
it is possible to send the fft from a other Microcontroller over serial to the ESP8266, when the ESP32 setup is ready the system will setup to query a MSGEQ7 for the FFT data. an example is in the project [TinyPixelMapper-Teensy-FFT](https://github.com/Gerfunky/TinyPixelMapper-Teensy-FFT)
it uses a Teensy LC and a MSGEQ7 with a mic to get the FFT data and then sends it over serial to the ESP8266.



## Required Library's
[Arduino core for ESP82662.2.0, not tested on 2.3.0] (http://arduino.esp8266.com/stable/package_esp8266com_index.json)

[FastLed](https://github.com/FastLED/FastLED "FastLED git Page ")

[Arduino-CmdMessenger](https://github.com/thijse/Arduino-CmdMessenger)

[RunningAverage](https://github.com/RobTillaart/Arduino/tree/master/libraries/RunningAverage)

[QueueArray](http://playground.arduino.cc/Code/QueueArray)

[OSC](https://github.com/CNMAT/OSC)

[RemoteDebug](https://github.com/JoaoLopesF/RemoteDebug)

[Artnet](https://github.com/natcl/Artnet)

[time](http://playground.arduino.cc/Code/Time)


## Installation 
TODO in wiki ?


## Where we need help
the editor under http://ip.address/edit takes to long to load.
a nice fast loading editor would be nice.


## Credit where credit is due
We used code sniblets from:
TODO

## Donation Box
TODO
