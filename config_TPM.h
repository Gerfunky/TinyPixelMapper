#ifndef _CONFIG_TPM_h
#define _CONFIG_TPM_h

#include "arduino.h"


// This is the Config file for the TinyPixxelMapper
// It should only hold #defines for compile time replacment


//#define ARTNET_DISABLED 
//#define OSC_MC_SERVER_DISABLED


#define DEF_BOOT_DEBUGING  true  // Set to true to get DEbuging info on serial port during boot. else set to false  TODO put this in epprom
#define DEF_SERIAL_SPEED 115200   // teensy - ESP8266 working on 57600
#define DEF_SERIAL_PORT Serial


// DEFAULT settings if not loaded from the SPIFFS
	#define DEF_WIFI_POWER true
	#define DEF_DEBUG_OUT true				// serial debugging
	#define DEF_WIFI_MODE false				// false = client
	#define DEF_OTA_SERVER true				// enable the OTA server ?
	#define DEF_STATIC_IP_ENABLED false		// set static ip for startup  ?
	#define DEF_HTTP_ENABLED true			// enable the HTTP server ?
	#define DEF_ARTNET_ENABLE false			// enable Artnet  ?
	#define DEF_AUTO_FFT true				// enalbe auto FFT ?
	#define DEF_DEBUG_TELNET true			// debug to TELNET?
	#define DEF_FFT_MASTER_SEND false		//  if in master mode send out the UDP Multicast packets?

// Wifi
	// DEFAULT setting if no config is loaded from the SPIFFS
	#define DEF_AP_NAME			"TinyPixelMapperT"				// AP / Hostname
	#define DEF_SSID			"home"							// SSID to connect to 
	#define DEF_WIFI_PWD		"love4all"		// PW for wifi Client
	#define DEF_AP_PASSWD		"love4all"					// PW for AP mode   !!! no OSC config yet STATIC !!!!
	#define DEF_IP_LOCAL		{172,16,222,31}					// Static IP
	#define DEF_IP_SUBNET		{255,255,255,0}					// Subnet Mask
	#define DEF_IP_DGW			{172,16,222,1}					// DGW

	#define DEF_DNS				{172,16,222,1}					// DNS server
	#define DEF_NTP_SERVER		"0.at.pool.ntp.org"				//"0.at.pool.ntp.org"	 // only FQDN's  no ip!! 	
	#define DEF_TIMEZONE		2								// how much to add to UTC for the NTP client
	#define WIFI_CLIENT_CONNECT_TIMEOUT		1000				// how long to try to connect to the Wifi
	#define WIFI_CLIENT_CONNECT_TRYS		1					// how many times to try to connect to the wifi 

	// Artnet
	#define DEF_ARTNET_STAT_UNIVERSE 5								// Default Artnet Start universe
	#define DEF_ARTNET_NUMBER_OF_UNIVERSES 2						// Default Arnet NR of universes MAX 4 !!!! TODO 
	#define DEF_ARTNET_MAC { 0x04, 0xE9, 0xE5, 0x00, 0x69, 0xEC }   // TODO!!!!! we dont want duplicates!!! so lets generate it!!!

	//  DEFAULT FFT parameters if not loaded from SPIFFS
	#define DEF_FFT_IP_MULTICAST	{239, 0, 0, 57}			//  Multicast IP address to send the FFT data to
	#define DEF_FFT_SLAVE_PORT		431						// Multicast DEST port for FFT packets
	#define DEF_FFT_MASTER_PORT		432						// Multicast source port to send from
	#define DEF_FFT_ENABLE			false					// enalbe FFT from start?
	#define DEF_FFT_MASTER			false					// set the node to Master mode?


// END WIFI

// FastLed Defines
		//#define FASTLED_ESP8266_RAW_PIN_ORDER		// Set Raw pin order not NodeMCU fake pins!!! 

		#define DEF_LED_TYPE    0	// 1					// led type 1 = WS2812b, 0 = APA102 , 2= SK6822
		#define LED_DATA_PIN    18 							// DATA 1 PIN	
		#define LED_CLK_PIN     5 							// DATA 2 PIN / data1CLK pin

		#define LED_DATA_3_PIN  19							// DATA 3 pin not used at the moment 
		#define LED_DATA_4_PIN  17							// DATA 4 PIN not used at the moment 

		//#define FASTLED_ALLOW_INTERRUPTS 0
		//#define FASTLED_INTERRUPT_RETRY_COUNT 0   // dont retry to send interupted transmissions  to leds

		#define NUM_LEDS		340 //680 //321 //642 //490 //640 // 320


		#define DEF_MAX_BRI 255		// the default max bri
		#define DEF_BRI 100			// the deault Bri

		#define MAX_FADE_VALUE 90			// maximum for FADE effect on each frame in  amount 0-255 
		#define MAX_JD_SPEED_VALUE 30		// maximum BPM for Juggle and SAW dots
		#define MAX_GLITTER_VALUE 255		// max glitter value

		#define MAX_PAL_FPS 88				// maximum FPS 

		#define BMP_MAX_TIME 3000				

		#define MAX_INDEX_LONG 4096			// must stay under this number!

		// FFT
		#define FFT_MIN_AUTO_TRIGGER 11					// Deafult min auto FFT trigger
		#define FFT_MAX_AUTO_TRIGGER 222				// Default Max auto FFT trigger

		// Strip/Form settings do not change!!! 
		#define NR_FORM_PARTS	16	// how many forms? default 16
		#define NR_STRIPS		32	// how many strips  default 32
		#define NR_PALETTS 2				// how many pallets do we have = 2



// END FastLed Defines

// Variable resistors + Buttons
#define POTI_BRI_PIN 39 //35	// For Brightness 
#define POTI_FPS_PIN 36 //34	// for speed
#define BTN_PIN 4 // for a button 



// OSC Defines
	#define OSC_IPMULTI_	{239,0,0,58}			// the multicast IP we sent to to brodcast the FFT data
	#define OSC_PORT_MULTI_	420						// the multicast port to send to
	#define OSC_OUTPORT		9000					// the OSC port we sent to on responsed
	#define OSC_INPORT		8000					// The input osc port

	#define OSC_BUNDLE_SEND_COUNT 16				// how many OSC messages to send in one bundle.

	#define OSC_MULTIPLY_OPTIONS 11					// how many multiply options to add to input from osc
// END OSC


// FFT MSGEQ7 defines


#define MSGEQ7_INPUT_PIN  34 //38 // A0 = 36 ... A4=32 sparkfun  (=18) //A1    // input from mic   Huzzah ESP32 A2 = 34		q
#define MSGEQ7_STROBE_PIN 21 //25 // 16 //3		// stobe pin
#define MSGEQ7_RESET_PIN  26 //15 //4		// reset pin

#define ANALOG_IN_DEVIDER 16 // devide analog in by this value to get into a 0-255 range 

/*

MSGEQ7
						--------\_/-------
	3v && GND-0	.1uF-	|1-VDDA 	 CKIN-8|	-- 200k - VVC &&  -- 33pF - GND
	 			GND		|2-VSSA		RESET-7|	- Reset PIN (17)
				to A4	|3-OUT		  CG -6|	- 0.01uF - GND
		Strobe pin (19) |4-STROBE	   IN-5|	- 0.01uF -22k - Mic (??33pF to input
						------------------

ADAfruit MAX9814
|GND
|V+
|Gain
|out
|AR


for a gain between :
� 20db to 40 db, connect the gain pin to the VDD
� 30db to 50 db, connect the gain pin to the GND
� 40db to 60 db, leave the gain pin unconnected
for a A/R of :
� 1:500 ms/ms, connect the AR pin to the GND
� 1:2000 ms/ms, connect the AR pin to the VDD
� 1:4000 ms/ms, leave the AR pin unconnected




Teensy-LC
9 = RX2
10 = TX2




*/







#endif