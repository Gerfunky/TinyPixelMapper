  /*
	TinyPixelMapper
	a Pixelmapping software for the ESP32 for addressible LED Strips, with a OSC controll interface.
	
	Git site https://github.com/Gerfunky/TinyPixelMapper
	
	License (GPL-3) : https://github.com/Gerfunky/TinyPixelMapper/blob/master/LICENSE
	
	TODO: more info here!

*/

#include "config_TPM.h"			// add the Config.h where the main Settings a DEFINED
#include "tools.h"
	

#include "config_fs.h"	



#include "wifi-ota.h"
#include "leds.h"
#include "mmqt.h"

//float temperatureRead();

void setup()
{		
	

	
		DEF_SERIAL_PORT.begin(DEF_SERIAL_SPEED);

	if (DEF_BOOT_DEBUGING == true)
	{
		write_bool(DEBUG_OUT, DEF_BOOT_DEBUGING);   // Set the normal debuging level will be overwriten when loading config from SPIFFS

		DEF_SERIAL_PORT.setDebugOutput(true);

		debugMe(debug_ResetReason(0));
		debugMe(debug_ResetReason(1));
	
		debugMe("Starting Setup - Light Fractal");
	}

	btStop(); // disable bluetooth
	
	setup_controlls();	// Pottis and Button
	yield();
	FS_setup();  // includes loadbool()
	yield();
	
	
	LEDS_setup();
	wifi_setup();
	yield();
 	

	MMQT_setup() ;

	wifi_start_IP_services();


	debugMe("DONE Setup");
	


} // end setup

void loop() 
{

	if (Network_connected_check()) 
	{
		wifi_loop();
		yield();
		MMQT_loop();
	}
	LEDS_loop();

	

} // end loop
