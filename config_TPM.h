#ifndef _CONFIG_TPM_h
#define _CONFIG_TPM_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

// This is the Config file for the TinyPixxelMapper
// It should only hold #defines for compile time replacment


#define ARTNET_DISABLED 

#define DEF_BOOT_DEBUGING  true  // Set to true to get DEbuging info on serial port during boot. else set to false  TODO put this in epprom
#define DEF_SERIAL_SPEED 115200   // teensy - ESP8266 working on 57600

// DEFAULT settings if not loaded from the SPIFFS
	#define DEF_DEBUG_OUT true				// serial debugging
	#define DEF_WIFI_MODE false				// false = client
	#define DEF_OTA_SERVER true				// enable the OTA server ?
	#define DEF_STATIC_IP_ENABLED true		// set static ip for startup  ?
	#define DEF_HTTP_ENABLED true			// enable the HTTP server ?
	#define DEF_ARTNET_ENABLE false			// enable Artnet  ?
	#define DEF_AUTO_FFT true				// enalbe auto FFT ?
	#define DEF_LED_TYPE 1					// led type 1 = WS2812b, 0 = APA102
	#define DEF_DEBUG_TELNET true			// debug to TELNET?
	#define DEF_FFT_MASTER_SEND false		//  if in master mode send out the UDP Multicast packets?


// FastLed Defines
		//#ifdef ESP32 
		#define FASTLED_ESP8266_RAW_PIN_ORDER		// Set Raw pin order not NodeMCU fake pins!!! 
		//#define WS2812_LED						// LED Type = 1 this is the default led type can be changes online in OSC
		#define APA102_LED							// Led type = 0
		//#define SK6822_LED						// Led type = 2
		#define LED_DATA_PIN    19 //12	
		#define LED_CLK_PIN     18 //13	// used with APA102
		//#endif


		//#define FASTLED_ALLOW_INTERRUPTS 0
		#define FASTLED_INTERRUPT_RETRY_COUNT 0   // dont retry to send interupted transmissions  to leds

		#define NUM_LEDS		680 //321 //642 //490 //640 // 320
		#define NR_FORM_PARTS	16	// how many forms? default 16
		#define NR_STRIPS		32	// how many strips  default 32

		#define DEF_MAX_BRI 255		// the default max bri
		#define DEF_BRI 100			// the deault Bri

		#define MAX_FADE_VALUE 90			// maximum for FADE effect on each frame in  amount 0-255 
		#define MAX_JD_SPEED_VALUE 30		// maximum BPM for Juggle and SAW dots
		#define MAX_GLITTER_VALUE 255		// max glitter value

		#define MAX_PAL_FPS 60				// maximum FPS 

		#define NR_PALETTS 2				// how many pallets do we have = 2

		#define BMP_MAX_TIME 3000				

		#define MAX_INDEX_LONG 4096			// must stay under this number!

		// FFT
		#define FFT_MIN_AUTO_TRIGGER 11					// Deafult min auto FFT trigger
		#define FFT_MAX_AUTO_TRIGGER 222				// Default Max auto FFT trigger

// END FastLed Defines




// OSC Defines
	#define OSC_IPMULTI_	{239,0,0,58}			// the multicast IP we sent to to brodcast the FFT data
	#define OSC_PORT_MULTI_	420						// the multicast port to send to
	#define OSC_OUTPORT		9000					// the OSC port we sent to on responsed
	#define OSC_INPORT		8000					// The input osc port

	#define OSC_BUNDLE_SEND_COUNT 16				// how many OSC messages to send in one bundle.

#define OSC_MULTIPLY_OPTIONS 11					// 
// END OSC




#endif