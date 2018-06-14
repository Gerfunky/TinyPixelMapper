# TinyPixelMapper
a Pixelmapping software for the ESP32 and ESP8266 for addressible LED Strips, with a OSC controll interface and FFT data from a Mic hocked up to a MSGEQ7.

# TinyPixelMapper : What is it?
It is a PixelMapping software for a ESP32 Chip.

The main LED driving library is [FastLed](https://github.com/FastLED/FastLED).

Configuration is done over OSC, [TouchOSC](https://hexler.net/touchosc) interfaces are included.

Ther are 2 main modes: Artnet and Normal.

In the Artnet mode it becomes a ARTNET node on the network. (hardcoded to 4 universes max, no mapping at the moment)

In Normal mode it playes Palletes or takes an input from a MIC that is connected to a MSGEQ7 chip to get FFT data to fill the leds.

You can configre Strips or Forms(a collection of strips).

There are effects that can be added to each strip/form.

It should work with any ESP32 it was designed on a [Adafruit HUZZAH32](https://www.adafruit.com/product/3405).

FFT data can be sent to other units (ESP8266) over udp multicast. (not tested on esp32)

# Work In progress 
The SW is working this Documentation + wiki is still missing some Stuff.

I have started to move to the ESP32. Therfore i have branched out the last working version for the ESP8266. I am calling that version 1.0. (Branch = ESP8266-release-1) The ESP8266 version is running rock solid, on my test Led Crystalgrid it was running for 4 months without any interuptions.

The ESP32 Version is working, and is in active development.

A basic PCB design is ready and will be posted after the Betatesters are done with testing.

The PCB has 2 variable resistors, one for Brightness and the other for FPS. One button, if the button is pressed during boot the unit will go into AP mode with a hardcoded AP password (love4all) even if wifi is disabled in the configuration.

All configurations are saved to the SPIFFS. And can be edited over HTTP once the editor is working again on the ESP32.

Some of the newest settings are still missing from the OSC configuration, like WIFI_POWER so you can let it run without wifi.

APA102 leds are not working 100% on the ESP32 still trying to figure it out.



## Required Library's
[Arduino for ESP32](https://github.com/espressif/arduino-esp32)

[FastLed](https://github.com/FastLED/FastLED)

[RunningAverage](https://github.com/RobTillaart/Arduino/tree/master/libraries/RunningAverage)

[QueueArray](http://playground.arduino.cc/Code/QueueArray)

[OSC](https://github.com/CNMAT/OSC)

[RemoteDebug](https://github.com/JoaoLopesF/RemoteDebug)

[Artnet](https://github.com/natcl/Artnet)


### Additional for ESP8266
[Arduino core for ESP82662.2.0, not tested on 2.3.0](http://arduino.esp8266.com/stable/package_esp8266com_index.json)

[time](http://playground.arduino.cc/Code/Time)

[Arduino-CmdMessenger](https://github.com/thijse/Arduino-CmdMessenger)



## Installation 
TODO in wiki ?

## Configuration
The configuration is done in the config_TPM.h


## Where we need help
the editor under http://ip.address/edit takes to long to load.
a nice fast loading editor is in the works. (old editor not working on ESP32) 


## Credit where credit is due
We used code sniblets from:
TODO

## Donation Box
TODO
