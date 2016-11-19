/*
	Teensy-LC to ESP8266 FFT data sender for 
	TinyPixelMapper a Pixelmapping software for the ESP8266 for addressible LED Strips, with a OSC controll interface.
	
	Git site https://github.com/Gerfunky/TinyPixelMapper
	
	License (GPL-3) : https://github.com/Gerfunky/TinyPixelMapper/blob/master/LICENSE
	
	TODO: more info here!

	This takes the input from a MSGEQ7 chip and sends speed and data out over serial to the esp8266
	When we can port TinyPixelMapper to the esp32 ths code will be moved there.
	
	
*/




#include "comms.h"
#include "msgeq7.h"



extern void setup_MSGEQ7();
extern void run_MSGEQ7();
extern void comms_loop();
extern void setup_comms();

void setup()
{
	setup_comms();
	setup_MSGEQ7();

	/*//Serial.begin(115200);
	delay(1000);
	Serial.println("hello");
	delay(1000);
	Serial.println("Anyone There?");


}

void loop()
{
	run_MSGEQ7();
	comms_loop();

}


