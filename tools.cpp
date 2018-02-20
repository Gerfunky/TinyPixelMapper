// 
// 
// 

#include "config_TPM.h"


#include "tools.h"

#include "leds.h"


//extern void debugMe(String input, boolean line = true);
/*
extern void debugMe(float input, boolean line = true);
extern void debugMe(uint8_t input, boolean line = true);
extern void debugMe(int input, boolean line = true);
// */

// Global Bool Structure soi that we only use 1 Bit and not a byte for a Bool
// use tool.h to read and write
  

uint8_t global_options[NR_GLOBAL_OPTIONS_BYTES] = { B00001111, B00000000 };

extern boolean FS_Bools_read(uint8_t conf_nr);
extern led_cfg_struct led_cfg;


//
//	Functions
//




// Boolean functions
boolean get_bool(uint8_t bit_nr)
{

	boolean return_bool = 0;
	uint8_t byte_nr = 0;
	while (bit_nr > 7)
	{
		byte_nr++;
		bit_nr = bit_nr - 8;
	}
	return_bool = bitRead(global_options[byte_nr], bit_nr);

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
	bitWrite(global_options[byte_nr], bit_nr, value);
}


void load_bool()
{		// Load the global bools from disk or defaults

	//debugMe("loading Bools");
	//if (FS_Bools_read(0) == false)
	if (false == false)
	{
		debugMe("Loading default BOOLS");
		led_cfg.ledType = DEF_LED_TYPE;
		write_bool(DEBUG_OUT, DEF_DEBUG_OUT);
		write_bool(OTA_SERVER, DEF_OTA_SERVER);
		write_bool(STATIC_IP_ENABLED, DEF_STATIC_IP_ENABLED);
		write_bool(HTTP_ENABLED, DEF_HTTP_ENABLED);
		write_bool(ARTNET_ENABLE, DEF_ARTNET_ENABLE);
		write_bool(FFT_AUTO, DEF_AUTO_FFT);
		write_bool(DEBUG_TELNET, DEF_DEBUG_TELNET);
		write_bool(FFT_MASTER_SEND, DEF_FFT_MASTER_SEND);
	}
	
}



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

void debugMe(String input, boolean line)
{
	//debugMe(input);
	/*
	if ((TelnetDebug.isActive(TelnetDebug.VERBOSE)) && get_bool(DEBUG_TELNET))
	{
	if (line == true)
	TelnetDebug.println(input);
	else
	TelnetDebug.print(input);

	}// */
	if (get_bool(DEBUG_OUT))
	{
		if (line == true)
			Serial.println(input);
		else
			Serial.print(input);
	}


}

void debugMe(float input, boolean line)
{
	//debugMe(input);
	/*
	if ((TelnetDebug.isActive(TelnetDebug.VERBOSE)) && get_bool(DEBUG_TELNET))
	{
	if (line == true)
	TelnetDebug.println(String(input));
	else
	TelnetDebug.print(String(input));

	} // */

	if (get_bool(DEBUG_OUT))
	{
		if (line == true)
			Serial.println(String(input));
		else
			Serial.print(String(input));
	}
}

void debugMe(uint8_t input, boolean line)
{
	//debugMe(input);
	/*
	if ((TelnetDebug.isActive(TelnetDebug.VERBOSE)) && get_bool(DEBUG_TELNET))
	{
	if (line == true)
	TelnetDebug.println(String(input));
	else
	TelnetDebug.print(String(input));

	} // */
	if (get_bool(DEBUG_OUT))
	{
		if (line == true)
			Serial.println(String(input));
		else
			Serial.print(String(input));
	}

}

void debugMe(int input, boolean line)
{
	//debugMe(input);
	/*
	if ((TelnetDebug.isActive(TelnetDebug.VERBOSE)) && get_bool(DEBUG_TELNET))
	{
	if (line == true)
	TelnetDebug.println(String(input));
	else
	TelnetDebug.print(String(input));

	} // */
	if (get_bool(DEBUG_OUT))
	{
		if (line == true)
			Serial.println(String(input));
		else
			Serial.print(String(input));
	}

}

void debugMe(IPAddress input, boolean line)
{
	//debugMe(input);
	/*
	if ((TelnetDebug.isActive(TelnetDebug.VERBOSE)) && get_bool(DEBUG_TELNET))
	{
	if (line == true)
	TelnetDebug.println(input);
	else
	TelnetDebug.print(input);

	}
	// */
	if (get_bool(DEBUG_OUT))
	{
		if (line == true)
			Serial.println(input);
		else
			Serial.print(input);
	}

}

// end Debug functions