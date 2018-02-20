/*
	TinyPixelMapper
	a Pixelmapping software for the ESP8266 for addressible LED Strips, with a OSC controll interface.
	
	Git site https://github.com/Gerfunky/TinyPixelMapper
	
	License (GPL-3) : https://github.com/Gerfunky/TinyPixelMapper/blob/master/LICENSE
	
	TODO: more info here!

*/

#include "Header.h"				// add the main Header file
#include "config_TPM.h"			// add the Config.h where the main Settings a DEFINED



		// add the Debug functions   --     send to debug   MSG to  Serial or telnet --- Line == true  add a CR at the end.
		//extern void debugMe(String input, boolean line = true);				// debug funtions from wifi-ota 
		//extern void debugMe(float input, boolean line = true);
		//extern void debugMe(uint8_t input, boolean line = true);
		//extern void debugMe(int input, boolean line = true);


		// add wifi funtions from wifi-ota.cpp
		extern void wifi_setup();
		extern void wifi_loop();
		extern void setup_wifi_Vars();


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





#ifdef ESP322  //did not want to startup correctly in own file
		#include <WiFi.h>
		#include "wifi-ota.h"
		extern wifi_Struct wifi_cfg;

 

		void ESP32_startWiFi()
		{
			//*
			WiFi.begin(wifi_cfg.ssid, wifi_cfg.pwd);

			while (WiFi.status() != WL_CONNECTED) {
				delay(500);
				Serial.print(".g.");
			}

			Serial.println("");
			Serial.println("WiFi connected.");
			Serial.println("IP address: ");
			Serial.println(WiFi.localIP());
			//	*/



		}


#endif



void setup()
{		
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
	//write_bool(DEBUG_OUT, DEF_BOOT_DEBUGING);   // Set the Boot debuging level will be overwriten when loading config from SPIFFS

	FS_setup_SPIFFS();  // includes loadbool()
	
	LEDS_setup();
	


#ifdef ESP32
	//ESP32_startWiFi();
#endif

	wifi_setup();


		

	//setup_comms(DEF_BOOT_DEBUGING, DEF_SERIAL_SPEED);   // Start CMDmessanger and the Serial if DEF_BOOT_DEBUGING == false
	debugMe("DONE Setup");


}

void loop() 
{

	wifi_loop();
	//LEDS_loop();
	//debugMe("running.");
}
