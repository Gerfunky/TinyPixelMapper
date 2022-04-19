// tools.h

#ifndef _TOOLS_h
#define _TOOLS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#define DEF_SERIAL_SPEED 115200   
#define DEF_SERIAL_PORT Serial
#define DEF_BOOT_DEBUGING  true 


#define NR_GLOBAL_OPTIONS_BYTES 5     // to hold the bools in bits not bytes! Enum below must fit in here!!!

enum GLOBAL_OPTIONS_ENUM
{
	DEBUG_OUT			= 0		// Debug to Serial ?
	,WIFI_MODE_TPM		= 1 	// True = Client, False = AP
	,OTA_SERVER			= 2		// Enable Arduino OTA
	,STATIC_IP_ENABLED	= 3		// Static IP?
	,HTTP_ENABLED		= 4		// enable the HTTP server
	,WIFI_EVENTS      	= 5		// Show all Wifi Events not only 7 Ipaddress .
	,OSC_EDIT			= 6		// alow Edditing of NR LEds and Start Led ?
	,FFT_MASTER			= 7		// Master FFT-Server ? if not = slave!
	,FFT_ENABLE			= 8		// FFT enabled
	,BLEND_INVERT		= 9		// invert all blend modes!
	,OSC_MC_SEND		= 10	// send OSC to slave OSC devices ?
	,UPDATE_LEDS		= 11	// 
	,BPM_COUNTER		= 12	// Pallete BPM mode ?
	,FFT_OSTC_VIZ		= 13	// send FFT data to Open Stage Controll for analisys ?
	,DEBUG_TELNET		= 14	// Debug to telnet ?
	,FFT_MASTER_SEND	= 15	// if in master mode send out the UDP Multicast packets?
	,WIFI_POWER 		= 16    // enable WIFI power.
	,BTN_LASTSTATE		= 17 	// what was the last bttn state
	,DATA1_ENABLE		= 18 	// enable Data line 1
	,DATA2_ENABLE		= 19	// enable Data line 2
	,DATA3_ENABLE		= 20	// enable Data line 3
	,DATA4_ENABLE		= 21	// enable Data line 4
	,WIFI_POWER_ON_BOOT = 22 	// was wifi on on boot?
	,POT_DISABLE 		= 23  	// Disable HW Pots BRI & FPS!
	,SEQUENCER_ON 		= 24	// Is the sequencer switched on?
	,MQTT_ON 			= 25     // Enable Mqtt.
	,ARTNET_SEND		= 26	// Are we a artnet sender (no direct output only calculation unit)
	,ARTNET_RECIVE		= 27    // Is it a artnet recive node (exclusive)
	,WIFI_MODE_BOOT		= 28	// What was the wifimode at boot time.
	,FADE_INOUT			= 29	// Trigger for save/load loop
	,FADE_INOUT_SAVE	= 30	// are we saving or loading for the fade.  flase = load
	,FADE_INOUT_FADEBACK = 31   // used for SAve load loop Fadebackin
	,POTS_LVL_MASTER	= 32	// override the LVL from the save to the value set on the pot
	,PAUSE_DISPLAY		= 33 	// frezze the display but continue to calculate in the background
	,ARTNET_REMAPPING   = 34	// remap artnet universe to FFT data and use almost all other features.
	,MANUAL_REFRESH  = 35   // is it a custom load.

};



// Functions


boolean setup_controlls();
// The main DEbuging Functions.

void debugMe(String input, boolean line = true, boolean allways = false);
void debugMe(float input, boolean line = true, boolean allways = false);
void debugMe(uint8_t input, boolean line = true, boolean allways = false);
void debugMe(int input, boolean line = true, boolean allways = false);
void debugMe(IPAddress input, boolean line = true , boolean allways = false);
void debugMe(tm input, boolean line = true, boolean allways = false);


String debug_ResetReason(boolean core);   // core 0 or 1 
//void verbose_print_reset_reason(RESET_REASON reason);
//void print_reset_reason(RESET_REASON reason);

// Boolean functions

boolean get_bool(uint8_t bit_nr);
void write_bool(uint8_t bit_nr, boolean value);
void load_bool();

// Get bit for bools
uint8_t get_strip_menu_bit(int strip);
uint8_t striptobit(int strip_no);


//generic functions

uint16_t calc_rounded_devision(uint16_t value, uint16_t divider);
boolean isODDnumber(uint8_t number);
float byte_tofloat(uint8_t value, uint8_t max_value = 255);



#endif

