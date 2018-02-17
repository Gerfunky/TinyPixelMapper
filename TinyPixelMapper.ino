/*
	TinyPixelMapper
	a Pixelmapping software for the ESP8266 for addressible LED Strips, with a OSC controll interface.
	
	Git site https://github.com/Gerfunky/TinyPixelMapper
	
	License (GPL-3) : https://github.com/Gerfunky/TinyPixelMapper/blob/master/LICENSE
	
	TODO: more info here!

*/

#include "Header.h"				// add the main Header file

#define ARTNET_DISABLED 

#define DEF_BOOT_DEBUGING  true  // Set to true to get DEbuging info on serial port during boot. else set to false  TODO put this in epprom
#define DEF_SERIAL_SPEED 57600   // teensy - ESP8266 working on 57600

		// add the Debug functions   --     send to debug   MSG to  Serial or telnet --- Line == true  add a CR at the end.
		extern void debugMe(String input, boolean line = true);				// debug funtions from wifi-ota 
		//extern void debugMe(float input, boolean line = true);
		//extern void debugMe(uint8_t input, boolean line = true);
		//extern void debugMe(int input, boolean line = true);


		// add wifi funtions from wifi-ota.cpp
		extern void wifi_setup();
		extern void wifi_loop();


		// add Comms functions for CMD messanger comms.cpp
		extern void startSerial(int Speed);		// function to start the serial --> from comms
		extern void setup_comms(boolean bootDebug, int Speed);  // setup CMD messanger

		// add SPIFFS-Functions from config-fs.cpp
		extern void FS_setup_SPIFFS();


		// add led functions from  leds.cpp
		extern void LEDS_setup();
		extern void LEDS_loop();

		// From tools.cpp
		//extern boolean get_bool(uint8_t bit_nr);
		extern void write_bool(uint8_t bit_nr, boolean value);


 
void setup()
{
	write_bool(DEBUG_OUT, DEF_BOOT_DEBUGING);   // Set the Boot debuging level will be overwriten when loading config from SPIFFS
	
	
	if (DEF_BOOT_DEBUGING == true)
	{

		startSerial(DEF_SERIAL_SPEED);

		//Serial.begin(DEF_SERIAL_SPEED);						// enable serial for debugging nand CMDmesanger if using local FFT from teensy
#ifdef ESP8266
		debugMe(ESP.getResetReason());
		debugMe(ESP.getResetInfo());
#endif //ESP8266
		debugMe("Starting Setup - Light Fractal");
	}

	FS_setup_SPIFFS();  // includes loadbool()
	
	LEDS_setup();

	wifi_setup();


		

	setup_comms(DEF_BOOT_DEBUGING, DEF_SERIAL_SPEED);   // Start CMDmessanger and the Serial if DEF_BOOT_DEBUGING == false
	debugMe("DONE Setup");


}

void loop() 
{

	wifi_loop();
	LEDS_loop();

}
