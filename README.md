# TinyPixelMapper
a Pixelmapping software for the ESP8266 for addressible LED Strips, with a OSC controll interface.

# Work In progress 
The SW is working the Documentation is still missing.

# TinyPixelMapper : What is it?
It is a PixelMapping software for a ESP8266 chip. (will be changend to ESP32 in the future)

The main LED driving library is FastLED.

Configuration is done over OSC, TouchOSC interfaces are included.

Ther are 2 main modes: Artnet and Normal.

In the Artnet mode it becomes a ARTNET node on the network. (hardcoded to 4 universes max, no mapping at the moment)

In Normal mode it playes Palletes or Colors from FFT data.

You can configre Strips, Forms(a collection of strips).

There are effects that can be added to each strip/form.




