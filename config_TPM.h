#ifndef _CONFIG_TPM_h
#define _CONFIG_TPM_h

//#include "arduino.h"


// This is the Config file for the TinyPixxelMapper
// It should only hold #defines for compile time replacment
// settings where the name starts with DEF will only be loaded if there are no setting on the SPIFFS


#define ARTNET_DISABLED 				// disables artnet, wont be compiled 
//#define OSC_MC_SERVER_DISABLED
#define OVERWRITE_INIT_CONF_ON	true		//Overwrite, wifi, device and led settings to defaut

#define DEF_BOOT_DEBUGING  false  // Set to true to get Debuging info on serial port during boot. else set to false  
#define DEF_SERIAL_SPEED 115200   
#define DEF_SERIAL_PORT Serial



// DEFAULT settings if not loaded from the SPIFFS

// General booleans

	#define DEF_DEBUG_OUT 		true			// serial debugging
	#define DEF_DEBUG_TELNET 	true			// debug to TELNET?
	#define DEF_OTA_SERVER 		true			// enable the OTA server ?
	#define DEF_HTTP_ENABLED 	true			// enable the HTTP server ?
	#define WRITE_CONF_AT_INIT 	true 			// write bools/wifi def conf to SPIFFS on load if not available.

// Wifi
	#define DEF_WIFI_POWER 		true							// Enable wifi 	 holing button on boot overides this and unit goes into AP mode with DEF_AP_PASSWD as the AP password
	#define DEF_WIFI_MODE 		true							// false = client  , true = AP

	#define DEF_AP_NAME			"TinyPixelMapperManu"				// AP and Hostname
	#define DEF_SSID			"home"							// SSID to connect to 
	#define DEF_WIFI_PWD		"love4all"						// PW for wifi Client
	#define DEF_AP_PASSWD		"love4all"						// PW for AP mode   !!! no OSC config yet STATIC !!!!

	#define DEF_STATIC_IP_ENABLED true							// set static ip for startup  ?
	#define DEF_IP_LOCAL		{172,16,222,31}					// Static IP
	#define DEF_IP_SUBNET		{255,255,255,0}					// Subnet Mask
	#define DEF_IP_DGW			{172,16,222,1}					// DGW

	#define DEF_WIFI_CHANNEL    1								// the wifi channel 1-12

	#define DEF_DNS				{172,16,222,1}					// DNS server
	#define DEF_NTP_SERVER		"0.at.pool.ntp.org"				//"0.at.pool.ntp.org"	 // only FQDN's  no ip!! 	
	#define DEF_TIMEZONE		2								// how much to add to UTC for the NTP client
	#define WIFI_CLIENT_CONNECT_TIMEOUT		1000				// how long to try to connect to the Wifi
	#define WIFI_CLIENT_CONNECT_TRYS		1					// how many times to try to connect to the wifi 

	// Default Artnet
	#define DEF_ARTNET_ENABLE 				false					// enable Artnet  ?
	#define DEF_ARTNET_STAT_UNIVERSE 		5						// Default Artnet Start universe
	#define DEF_ARTNET_NUMBER_OF_UNIVERSES 	2						// Default Arnet NR of universes MAX 4 !!!! TODO 
	#define DEF_ARTNET_MAC { 0x04, 0xE9, 0xE5, 0x00, 0x69, 0xEC }   // TODO!!!!! we dont want duplicates!!! so lets generate it!!!

	//  DEFAULT FFT parameters if not loaded from SPIFFS
	#define DEF_FFT_IP_MULTICAST	{239, 0, 0, 57}			//  Multicast IP address to send the FFT data to
	#define DEF_FFT_SLAVE_PORT		431						// Multicast DEST port for FFT packets
	#define DEF_FFT_MASTER_PORT		432						// Multicast source port to send from
	#define DEF_FFT_ENABLE			false					// enalbe FFT from start?
	#define DEF_FFT_MASTER			false					// set the node to Master mode?
	#define DEF_AUTO_FFT 			true					// enalbe auto FFT ?
	#define DEF_FFT_MASTER_SEND		false					//  if in master mode send out the UDP Multicast packets?

// END WIFI

// FastLed Defines
		//#define FASTLED_ESP8266_RAW_PIN_ORDER		// Set Raw pin order not NodeMCU fake pins!!! 


		/* DEF_LED_MODE:
							This select the way the controller outputs are setup!
							in Line mode they are serial. So set the Nr of leds correctly and the correct starled for that output.
							in Mirror mode all start al led 0 and do the same.
							This is loaded at boot!

						Type : Mode   : led_type  
							0: Mirror :     MIX : Data1 + CLk 1 = APA102 ; Data3 = WS2812 ; Data4 = SK8622
							5: Line	  :	    MIX : Data1 + CLk 1 = APA102 ; Data3 = WS2812 ; Data4 = SK8622
							1: Line   :  APA102 : Data1+CLK & Data3+Data4(clk) = APA102
							2: Mirror :  APA102 : Data1+CLK & Data3+Data4(clk) = APA102
							3: Line   : WS2812b : Data1 to Data4 = WS2812b
							4: Mirror : WS2812b : Data1 to Data4 = WS2812b
							
		*/
		#define DEF_LED_MODE    0
														

		#define NUM_LEDS		352 //w170*4		// NR of leds for mirror mode.												
		#define DEF_DATA1_START_NR 0
		#define DEF_DATA1_NR_LEDS  340 //170*4
		#define DEF_DATA2_START_NR 0
		#define DEF_DATA2_NR_LEDS  10 //170*4
		#define DEF_DATA3_START_NR 50
		#define DEF_DATA3_NR_LEDS  340 //170*4
		#define DEF_DATA4_START_NR 0
		#define DEF_DATA4_NR_LEDS  340 //170*4
		
		#define DEF_DATA1_ENABLE true
		#define DEF_DATA2_ENABLE false
		#define DEF_DATA3_ENABLE true
		#define DEF_DATA4_ENABLE false


		#define DEF_PLAY_MODE 0 

		#define DEF_FIRE_SPARKING 50
		#define DEF_FIRE_COOLING 30 

		#define LED_DATA_PIN    18 							// DATA 1 PIN	
		#define LED_CLK_PIN     5 							// DATA 2 PIN / data1CLK pin

		#define LED_DATA_3_PIN  19							// DATA 3 PIN = WS2812 
		#define LED_DATA_4_PIN  17							// DATA 4 PIN = SK6822 

			

		#define DEF_MAX_BRI 255		// the default max bri
		#define DEF_BRI 240			// the deault Bri



		// FFT
		#define FFT_MIN_AUTO_TRIGGER 11					// Deafult min auto FFT trigger
		#define FFT_MAX_AUTO_TRIGGER 222				// Default Max auto FFT trigger



		#define FASTLED_ALLOW_INTERRUPTS 0
		#define FASTLED_INTERRUPT_RETRY_COUNT 0   // dont retry to send interupted transmissions  to leds


// END FastLed Defines

// Variable resistors + Buttons
		#define POTI_BRI_PIN 39 	// For Brightness 
		#define POTI_FPS_PIN 36 	// for speed
		#define BTN_PIN 4 			// for a button 



// OSC Defines
		#define OSC_IPMULTI_	{239,0,0,58}			// the multicast IP we sent to to brodcast the FFT data
		#define OSC_PORT_MULTI_	420						// the multicast port to send to
		#define OSC_OUTPORT		9000					// the OSC port we sent to on responsed
		#define OSC_INPORT		8000					// The input osc port


// END OSC


// FFT MSGEQ7 defines


		#define MSGEQ7_INPUT_PIN  34 // input from mic  
		#define MSGEQ7_STROBE_PIN 21 // stobe pin
		#define MSGEQ7_RESET_PIN  26 // reset pin

/*

MSGEQ7
						--------\_/-------
	3v && GND-0	.1uF-	|1-VDDA 	 CKIN-8|	-- 200k - VVC &&  -- 33pF - GND
	 			GND		|2-VSSA		RESET-7|	- Reset PIN (17)
				to A4	|3-OUT		  CG -6|	- 0.01uF - GND
		Strobe pin (19) |4-STROBE	   IN-5|	- 0.01uF -22k - Mic (??33pF to input
						------------------

ADAfruit MAX9814
-|GND
-|V+
-|Gain
-|out
-|AR

for a gain between :
  20db to 40 db, connect the gain pin to the VDD
  30db to 50 db, connect the gain pin to the GND
  40db to 60 db, leave the gain pin unconnected
for a A/R of :
  1:500 ms/ms, connect the AR pin to the GND
  1:2000 ms/ms, connect the AR pin to the VDD
  1:4000 ms/ms, leave the AR pin unconnected

*/







#endif