// 
// 
// 

#include "config_TPM.h"
#include "tools.h"
#include "leds.h"

#include "config_fs.h"	



#include <time.h>
#include "wifi-ota.h"		// needs Wifi and co for data stuctures!!!


// Global Bool Structure soi that we only use 1 Bit and not a byte for a Bool
// use tool.h to read and write
  

uint8_t global_options[NR_GLOBAL_OPTIONS_BYTES] = { B00000001, B00000000, B00000000 };  //{ B00001111, B00000000 };


extern led_cfg_struct led_cfg;

boolean setup_controlls()
{
	pinMode(BTN_PIN, INPUT_PULLUP);
	pinMode(POTI_BRI_PIN, INPUT);
	pinMode(POTI_FPS_PIN, INPUT);
return true;
}



// Bools functions
boolean get_bool(uint8_t bit_nr)
{

	boolean return_bool = 0;
	uint8_t byte_nr = 0;
	while (bit_nr > 7)
	{
		byte_nr++;
		bit_nr = bit_nr - 8;
	}
	if (byte_nr < NR_GLOBAL_OPTIONS_BYTES)
	return_bool = bitRead(global_options[byte_nr], bit_nr);

	return return_bool;
}

boolean get_bool(uint8_t inByte, uint8_t bit_nr)
{

	boolean return_bool = 0;

	return_bool = bitRead(inByte, bit_nr);

	return return_bool;
}




void write_bool(uint8_t bit_nr, boolean value)
{

	uint8_t byte_nr = 0;
	while (bit_nr > 7)
	{
		byte_nr++;
		bit_nr = bit_nr - 8;
	}
	if (byte_nr < NR_GLOBAL_OPTIONS_BYTES)
		bitWrite(global_options[byte_nr], bit_nr, value);
	//yield();
}


void load_bool()
{		// Load the global bools from disk or defaults

	//debugMe("loading Bools");
	//if (FS_Bools_read(0) == false)
	debugMe("pre_Bool_LOAD");
	//debugMe(FS_Bools_read(0));
	//debugMe("PAST_BOOL_LOAD");
	write_bool(WIFI_EVENTS, DEF_WIFI_EVENTS);

	
	if (!FS_Bools_read(0) || OVERWRITE_INIT_CONF_ON )
	{
		debugMe("Loading default device config");
		led_cfg.ledMode 		= constrain(DEF_LED_MODE, 0, 5);
		led_cfg.max_bri 		= constrain(DEF_MAX_BRI, 1, 255);
		led_cfg.startup_bri 	= constrain(DEF_MAX_BRI, 1, 255);
		led_cfg.NrLeds			= constrain(NUM_LEDS, 1,MAX_NUM_LEDS) ;
		led_cfg.DataNR_leds[0] 	= constrain(DEF_DATA1_NR_LEDS, 0,led_cfg.NrLeds - DEF_DATA1_START_NR);
		led_cfg.DataStart_leds[0] 	= constrain(DEF_DATA1_START_NR, 0, led_cfg.NrLeds) ;
		led_cfg.DataNR_leds[1] 	= constrain(DEF_DATA2_NR_LEDS, 0,led_cfg.NrLeds - DEF_DATA2_START_NR);
		led_cfg.DataStart_leds[1]  	= constrain(DEF_DATA2_START_NR, 0, led_cfg.NrLeds) ;
		led_cfg.DataNR_leds[1] 	= constrain(DEF_DATA3_NR_LEDS, 0,led_cfg.NrLeds - DEF_DATA3_START_NR);
		led_cfg.DataStart_leds[2]  	= constrain(DEF_DATA3_START_NR, 0, led_cfg.NrLeds) ;
		led_cfg.DataNR_leds[3] 	= constrain(DEF_DATA4_NR_LEDS, 0,led_cfg.NrLeds - DEF_DATA4_START_NR);
		led_cfg.DataStart_leds[3]  	= constrain(DEF_DATA4_START_NR, 0, led_cfg.NrLeds) ;
		led_cfg.apa102data_rate = constrain(DEF_APA102_DATARATE, 0, 24) ;
		led_cfg.bootCFG = 16;

		write_bool(DATA1_ENABLE,DEF_DATA1_ENABLE);
		write_bool(DATA2_ENABLE,DEF_DATA2_ENABLE);
		write_bool(DATA3_ENABLE,DEF_DATA3_ENABLE);
		write_bool(DATA4_ENABLE,DEF_DATA4_ENABLE);
		

		write_bool(DEBUG_OUT, DEF_DEBUG_OUT);
		write_bool(DEBUG_TELNET, DEF_DEBUG_TELNET);
		write_bool(FFT_ENABLE, DEF_FFT_ENABLE);
		write_bool(FFT_MASTER, DEF_FFT_MASTER);
		write_bool(FFT_MASTER_SEND, DEF_FFT_MASTER_SEND);

		write_bool(POT_DISABLE,DEF_DISABLE_HW_POTS);

		write_bool(WIFI_EVENTS, DEF_WIFI_EVENTS);

		if (WRITE_CONF_AT_INIT || OVERWRITE_INIT_CONF_ON ) FS_Bools_write(0);
	}
	//else debugMe("Bools Loaded from SPIFFS!");

	//debugMe("http Server load = " + String(get_bool(HTTP_ENABLED)));
	
}

// end bools functions



uint8_t get_strip_menu_bit(int strip) 
{
	// return the bit of the strip menu

	byte strip_bit = 0;
	if (strip > 7)  strip_bit += 1;
	if (strip > 15)  strip_bit += 1;
	if (strip > 23)  strip_bit += 1;
	return strip_bit;
}

boolean isODDnumber(uint8_t number) 
{

	//if ( (number % 2) == 0) { do_something(); }
	//if ( (number & 0x01) == 0) 
	if (bitRead(number, 0) == true)
		return true;

	else
		return false;

}

uint8_t striptobit(int strip_no)						
{
	// scale down the Strip no to a bit value 0-7
	while (strip_no > 7) strip_no = strip_no - 8;


	return strip_no;

}

float byte_tofloat(uint8_t value, uint8_t max_value) 
{

	float float_out = float(value) / max_value;

	return float_out;
}





//debugging

// debugging functions 
// can take String, float, uint8_t, int and IPAddress 
// and print it to telnet or Serial



void debugMe(String input, boolean line, boolean allways)
{

	if (get_bool(DEBUG_TELNET)) WiFi_telnet_print(input, line);


	if (get_bool(DEBUG_OUT) || allways)
	{
		if (line == true)
			DEF_SERIAL_PORT.println(input);
		else
			DEF_SERIAL_PORT.print(input);
	}


}


void debugMe(tm input, boolean line, boolean allways)
{
	if (get_bool(DEBUG_TELNET)) WiFi_telnet_print(input, line);
	if (get_bool(DEBUG_OUT)|| allways)
	{
		if (line == true)
			DEF_SERIAL_PORT.println(&input, "%H:%M:%S %d.%m.%y");
		else
			DEF_SERIAL_PORT.println(&input, "%H:%M:%S %d.%m.%y");
	}
}

void debugMe(float input, boolean line, boolean allways)
{
	if (get_bool(DEBUG_TELNET)) WiFi_telnet_print(input, line);
	if (get_bool(DEBUG_OUT)|| allways)
	{
		if (line == true)
			DEF_SERIAL_PORT.println(String(input));
		else
			DEF_SERIAL_PORT.print(String(input));
	}
}

void debugMe(uint8_t input, boolean line, boolean allways)
{
	if (get_bool(DEBUG_TELNET)) WiFi_telnet_print(input, line);
	if (get_bool(DEBUG_OUT)|| allways)
	{
		if (line == true)
			DEF_SERIAL_PORT.println(String(input));
		else
			DEF_SERIAL_PORT.print(String(input));
	}

}

void debugMe(int input, boolean line, boolean allways)
{
	if (get_bool(DEBUG_TELNET)) WiFi_telnet_print(input, line);
	if (get_bool(DEBUG_OUT)|| allways)
	{
		if (line == true)
			DEF_SERIAL_PORT.println(String(input));
		else
			DEF_SERIAL_PORT.print(String(input));
	}

}

void debugMe(IPAddress input, boolean line , boolean allways)
{
	if (get_bool(DEBUG_TELNET)) WiFi_telnet_print(input, line);
	if (get_bool(DEBUG_OUT) || allways)
	{
		if (line == true)
			DEF_SERIAL_PORT.println(input);
		else
			DEF_SERIAL_PORT.print(input);
	}

}

// end Debug functions


#include <rom/rtc.h>		// required to get the reset reason
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */

String print_reset_reason(RESET_REASON reason)
{
	String stringy_reason;
		
	switch (reason)
	{
	case 1: stringy_reason = ("POWERON_RESET : Vbat power on reset"); break;									/**<1,  Vbat power on reset*/
	//case 2: stringy_reason = ("Nr 2 who knows!!!"); break;
	case 3: stringy_reason = ("SW_RESET : Software reset digital core"); break;									/**<3,  Software reset digital core*/
	case 4: stringy_reason = ("OWDT_RESET :Legacy watch dog reset digital core"); break;						/**<4,  Legacy watch dog reset digital core*/
	case 5: stringy_reason = ("DEEPSLEEP_RESET : Deep Sleep reset digital core"); break;						/**<5,  Deep Sleep reset digital core*/
	case 6: stringy_reason = ("SDIO_RESET : Reset by SLC module, reset digital core"); break;					/**<6,  Reset by SLC module, reset digital core*/
	case 7: stringy_reason = ("TG0WDT_SYS_RESET : Timer Group0 Watch dog reset digital core"); break;			/**<7,  Timer Group0 Watch dog reset digital core*/
	case 8: stringy_reason = ("TG1WDT_SYS_RESET : Timer Group1 Watch dog reset digital core"); break;			/**<8,  Timer Group1 Watch dog reset digital core*/
	case 9: stringy_reason = ("RTCWDT_SYS_RESET : RTC Watch dog Reset digital core"); break;					/**<9,  RTC Watch dog Reset digital core*/
	case 10: stringy_reason = ("INTRUSION_RESET : Instrusion tested to reset CPU"); break;						/**<10, Instrusion tested to reset CPU*/
	case 11: stringy_reason = ("TGWDT_CPU_RESET : Time Group reset CPU"); break;								/**<11, Time Group reset CPU*/
	case 12: stringy_reason = ("SW_CPU_RESET : Software reset CPU"); break;										/**<12, Software reset CPU*/
	case 13: stringy_reason = ("RTCWDT_CPU_RESET : RTC Watch dog Reset CPU"); break;							/**<13, RTC Watch dog Reset CPU*/
	case 14: stringy_reason = ("EXT_CPU_RESET : for APP CPU, reseted by PRO CPU"); break;						/**<14, for APP CPU, reseted by PRO CPU*/
	case 15: stringy_reason = ("RTCWDT_BROWN_OUT_RESET : Reset when the vdd voltage is not stable"); break;		/**<15, Reset when the vdd voltage is not stable*/
	case 16: stringy_reason = ("RTCWDT_RTC_RESET : RTC Watch dog reset digital core and rtc module"); break;    /**<16, RTC Watch dog reset digital core and rtc module*/
	default: stringy_reason = ("NO_MEAN");
	}
	return stringy_reason;
}



String debug_ResetReason(boolean core)
{
	String the_reason = ("CPU" + String(core) + " : " + print_reset_reason(rtc_get_reset_reason(core)));
	
	//debugMe("Reset reason:"+ the_reason);
	//debugMe("-------------------------------------------");

		return the_reason;
}

String debug_GetChipID()
{

	uint64_t chipid = ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
	Serial.printf("ESP32 Chip ID = %04X", (uint16_t)(chipid >> 32));//print High 2 bytes
	Serial.printf("%08X\n", (uint32_t)chipid);//print Low 4bytes.
	//debugMe(String(chipid));
	//String TEST = printf("ESP32 Chip ID = %04X", (uint16_t)(chipid >> 32));//print High 2 bytes
	return "uuuups";
}
