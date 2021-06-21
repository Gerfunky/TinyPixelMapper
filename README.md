# TinyPixelMapper
a Pixelmapping software for the ESP32 and ESP8266 for addressible LED Strips, with a OSC controll interface and FFT data from a Mic hocked up to a MSGEQ7.

have a look what it can do: https://www.youtube.com/watch?v=r7bt4Rk3eVM  

# Note to everyone that has been using this before 06.21
The Software has gone over some major changes in the config files please be aware that when loding old config it wont work. Sorry about that but it was nessesery becouse We are running out of memory in the ESP32 with this Software. So I had to do some changes like selecting paletts and mixing will always be 1 setting for 8 Strips. Which I quite like since it was a hassle to change the palette on every strip manually to just see what another one would look like.

# 06.2021 This Readme and the Wiki Are now in major rework to get all the new info in. 
We are preparing for the crowdfunding this year (2021) for the Hardware with some Nice Lights to go with it. 
New info, pictures ect will be added in the next weeks. If you find any bugs please post them!



# TinyPixelMapper : What is it?
It is a PixelMapping software for a ESP32 Chip.

The main LED driving library is [FastLed](https://github.com/FastLED/FastLED).

Configuration is done over [Open Stage control](https://github.com/jean-emmanuel/open-stage-control/releases)

Ther are 3 main modes: Artnet Recive, Arntet Send (16 universes) and Normal Led output.

In the Artnet Recive mode it becomes a ARTNET node on the network. (hardcoded to 4 universes max, no mapping at the moment)

In the Artnet Send mode it becomes a ARTNET Master node on the network. And sends it output to Arntent nodes and does not output locally. 

In Normal mode it playes Palletes or takes an input from a MIC that is connected to a MSGEQ7 chip to get FFT data to fill the leds. You have the option to Mask the pallete over the FFT data or add/subtract it. Same for FX data. The new board will have an Audio Jack and not just a mic.

You can configre Forms Start Led Nr Leds and then selecht what to do with those Forms and mix Layers onto each other.

There are effects that can be added to each Form such as FFT data or Palletes.

It should work with any ESP32 it was designed on a [Adafruit HUZZAH32](https://www.adafruit.com/product/3405).

FFT data can be sent to other units (ESP8266) over udp multicast. (not tested on esp32) Will work with ARnet Send on the ESP32.

# Work In progress 
The SW is working this Documentation + wiki is still missing some Stuff.

I have moved to the ESP32. Therfore i have branched out the last working version for the ESP8266. I am calling that version 1.0. (Branch = ESP8266-release-1) The ESP8266 version is running rock solid, on my test Led Crystalgrid it was running for 4 months without any interuptions.

The ESP32 Version is working, and is in active development.

# Hardware
A basic PCB design is ready and will be posted after the Betatesters are done with testing. 

The PCB has 2 variable resistors, one for Brightness and the other for FPS. One button, if the button is pressed during boot the unit will go into AP mode with a hardcoded AP password (love4all) even if wifi is disabled in the configuration.

We are planning to Crowdfund the Hardware with nice Lamps that are designed to take advantage of the hardware. The Main developer is part of a Crew that builds Stages for big PSY festivals like Momento Demento in Kroatia. And we will have some nice Audio reactive Psy Lamps for you. 

# Saving Setups
All configurations are saved to the SPIFFS. And can be edited over HTTP once the editor is working again on the ESP32 (problem in the esp core/fix is tested and working).

Read the Wiki to understand whats going on. howto configure it. 
Videos are on the way. 


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
Goto the Wiki

## Configuration
The configuration is done in the config_TPM.h settings loaded from the SPIFFS will have a huger priority than what is hardcoded in this file. Only use it for initial setup.


## Where we need help
We need a real APP. Im am getting to the limits of what we can do with TouchOSC. OSC will always be available so that its possible to use a midi-keyboard to play with the TinyPixelMapper



## Donation Box
TODO
