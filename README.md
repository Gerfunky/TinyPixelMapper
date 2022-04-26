# TinyPixelMapper
a Pixelmapping software for the ESP32 and ESP8266 for addressible LED Strips, with a OSC controll interface and FFT data from a Mic or Audio Jack.

have a look what it can do: https://www.youtube.com/watch?v=r7bt4Rk3eVM  


# Update 26.04.2022 
We are ready to release Version 1.0 for the ESP32
We wanted to do a Crowdfunding but due to the current things happening in the World we are contemplating to just go straight for a Shop and let it grow gradually. 
We had to do another Major Change in the Code. It is now possible to set Pallets to auto calculate the Compression based on the NR of LEDs in the Strip. So that its possible to Load the same config into different lamps (last layer has different amount of leds in my Lamps) without having to update the compression or make a different config for each Lamp. In the Load Save options it possible to make a Pattern Load override whats in the Lamp config. Since we still save the entire led setup on each Pattern Save. 
We have also updated the Open Stage Controll Interface to 1.16.0 So you can use a new Version of Open Stage Controll.

# Note to everyone that has been using this before 06.21
The Software has gone over some major changes in the config files please be aware that when lodaing old config it wont work. Sorry about that but it was nessesery becouse We are running out of memory in the ESP32 with this Software. So I had to do some changes like selecting paletts and mixing will always be 1 setting for 8 Strips. Which I quite like since it was a hassle to change the palette on every strip manually to just see what another one would look like.

# 06.2021 This Readme and the Wiki Are now in major rework to get all the new info in. 
We are preparing for the crowdfunding this year (2021) for the Hardware with some Nice Lights to go with it. 
New info, pictures ect will be added in the next weeks. If you find any bugs please post them!

# TinyPixelMapper : What is it?
It is a PixelMapping software for a ESP32 Chip.

The main LED driving library is [FastLed](https://github.com/FastLED/FastLED).

# Crowdfunding Campaign
We were planning on starting a crowdfunding campaign in the summer 2021. But do to the current situation in the Word wer are Thinking of just making a Shop.
We are negotiation with an artist for permission to use his music. Once this is done we will start posting videos of the different lamps that will be available. 

# Configuration Interface:
Configuration is done over OSC, with the opensource Software [Open Stage Control](https://openstagecontrol.ammd.net/)
We are now on the Open Stage Control version 1.16.0 
In the Settings make shure to set the OSC port to 9000
Open Stage Control creates a HTTP Server, so you can connect from any device on your network over a web browser. To configure the Tinypixelmapper.
There are 2 Version of the Interface one for edeting Pattern and one just to Play around and Load Paterns (Phone version)
There is also a configuration file with just simple options like loading a save file, available for [TouchOSC](https://hexler.net/touchosc) a App available in the Google/ Apple App stores. 

Any OSC Software can be used to configure the System. So it is also possible to use Midi Controllers with faders to Play with the unit.

# Modes
There are different modes of operation: Artnet and Normal.

# Artnet Mode:
In the Artnet mode it becomes a ARTNET sender or receiver node on the network. 

## Artnet Reciver
As an Artnet receiver there is a special sub mode called “ARTNET REMAPPING” In this mode the Artnet data replaces the FFT data from the MIC / Audio IN. Instead of addressing the LEDs directly. This allows us to use almost all the features of the SW like mirroring, rotation or other effects. So you only need to send 1 Universe (170 pixel) an Audio Color Scroll  from any Mapping Software and let the Tinypixelmapper mix it against pallets, Since we don’t have any real FFT data the FFT DATA BIN triggers don’t work in this mode.

## Artnet Sender 
As an Artnet sender, the unit does not output anything to the locally connected LED outputs. Instead it sends ARTNET packets to other units controlling them. Since its not outputting anything locally we have more time to do calculations so its possible to use one unit to control many units in sync and only the sender need a MIC or Audio in to do the FFT data calculations.

# Normal mode:
The unit plays Palettes or takes an input from a MIC / audio In  that is connected to a MSGEQ7 chip to get FFT data to fill the LEDs.

There are effects that can be added to each strip

A very important and strong feature is the Mixing, You can shoose how and in what order the different layers should be mixed.

It should work with any ESP32 it was designed for a [Adafruit HUZZAH32](https://www.adafruit.com/product/3405).
But we have decided to switch to an [OLIMEX  ESP32-POE](https://www.olimex.com/Products/IoT/ESP32/ESP32-POE/open-source-hardware). board since Ethernet and and SD card are alot more user friendly. For professional lighting solutions Wifi is not possible since the Party crowd with there mobile phones would interrupt the led output. 

# Work In progress 
The SW is working this Documentation + wiki is still missing some Stuff.

I have finished to move to the ESP32. Therfore i have branched out the last working version for the ESP8266. I am calling that version 1.0. (Branch = ESP8266-release-1) The ESP8266 version is running rock solid, on my test Led Crystalgrid it was running for 4 months without any interruptions. This old version does not have many of the new cool features such as Mixing layers.

The ESP32 Version is working, and is in active development.

A basic PCB design is ready but we have decided to do a new Version for the Crowdfunding campaign that includes Ethernet, SD card and an audio jack.

The PCB has 2 variable resistors, one for Brightness and the other for FPS. One button, if the button is pressed during boot the unit will go into AP mode with a hardcoded AP password (love4all) even if wifi is disabled in the configuration.

All configurations are saved to the SPIFFS or SD card. And can be edited over HTTP. "http://IP/edit"



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
More FX ideas.
A custom fast purpose built interface instead of using Open stage control. We have started implementing it in Unreal Engine but would need help from un Unreal engine expert.


## Credit where credit is due
We used code sniblets from:
TODO

## Donation Box
TODO
