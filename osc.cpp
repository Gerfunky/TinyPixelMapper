// 
// 
// 


//  Todo : 



#include "config_TPM.h"   // include the main Defines
#include "osc.h"
#include "leds.h"

#ifndef USE_OSC
// clapse all ctl+m +o    +l = expand

//#ifdef ESP8266

//#define OSC_MC_SERVER_DISABLED

		#define OSC_BUNDLE_SEND_COUNT 16				// how many OSC messages to send in one bundle.
		#define OSC_MULTIPLY_OPTIONS 11					// how many multiply options to add to input from osc

	#include <WiFiUdp.h>

#ifdef ESP8266
	#include <ESP8266WiFi.h>
#endif
#ifdef ESP32
	#include <WiFi.h>
#endif
	#include <OSCMessage.h>
	#include <OSCBundle.h>
	#include <OSCData.h>
	#include <QueueArray.h>


	#include "tools.h"								// include the Tools enums for reading and writing bools
	#include "wifi-ota.h"
	#include "config_fs.h"
	#include "httpd.h"


// External Variables/ Structures


	extern fft_data_struct fft_data[7];
	extern wifi_Struct wifi_cfg;
	extern artnet_struct artnet_cfg;




// from leds

	extern led_cfg_struct led_cfg;
	extern led_Copy_Struct copy_leds[NR_COPY_STRIPS];
	extern Strip_FL_Struct part[NR_STRIPS];
	extern form_Part_FL_Struct form_part[NR_FORM_PARTS];
	extern byte  copy_leds_mode[NR_COPY_LED_BYTES];
	extern byte strip_menu[_M_NR_STRIP_BYTES_][_M_NR_OPTIONS_];
	extern byte form_menu[_M_NR_FORM_BYTES_][_M_NR_FORM_OPTIONS_];
	extern uint8_t global_strip_opt[_M_NR_STRIP_BYTES_][_M_NR_GLOBAL_OPTIONS_];
	extern byte fft_menu[3];
	extern byte fft_data_menu[3];
	extern byte fft_data_bri;
	extern fft_led_cfg_struct fft_led_cfg;
	extern uint8_t fft_bin_autoTrigger;
	extern byte fft_data_fps;
//extern CRGBPalette16 LEDS_pal_cur[NR_PALETTS];


//struct OSC_buffer_float master_rgb = { 255,255,255 };


//from coms
//extern void comms_S_FPS(uint8_t fps);


QueueArray <char> osc_out_float_addr;
QueueArray <float> osc_out_float_value;

osc_cfg_struct osc_cfg = { OSC_IPMULTI_ ,OSC_PORT_MULTI_,OSC_OUTPORT, OSC_INPORT, 0,1 };




	WiFiUDP osc_server;				// the normal osc server
#ifndef OSC_MC_SERVER_DISABLED
	WiFiUDP osc_mc_server; 			// the multicast server
#endif


void OSC_setup()
{
	osc_server.begin(osc_cfg.inPort);
#ifndef OSC_MC_SERVER_DISABLED
		#ifdef ESP8266
			osc_mc_server.beginMulticast(WiFi.localIP(), osc_cfg.ipMulti, osc_cfg.portMulti);
		#endif

		#ifdef ESP32
			osc_mc_server.beginMulticast(osc_cfg.ipMulti, osc_cfg.portMulti);
		#endif
#endif
}




// OSC gerneral functions
float osc_byte_tofloat(byte value, uint8_t max_value = 255) {

	float float_out = float(value) / max_value;

	return float_out;
}

void osc_queu_MSG_float(String addr_string, float value) {
	IPAddress ip_out(osc_server.remoteIP());
	osc_cfg.return_ip_LB = ip_out[3];

	//char address_out[20];
	//OSC_buffer_float msg_out_data;


	//addr_string.toCharArray(address_out, addr_string.length() + 1); //address_out 
	//msg_out_data.addr = addr_string;
	//msg_out_data.value = value;

	for (int i = 0; i < addr_string.length(); i++)
		osc_out_float_addr.enqueue(addr_string.charAt(i));

	osc_out_float_addr.enqueue(0);	// add a null on the end 
	osc_out_float_value.enqueue(value);


	/*OSCMessage msg_out(address_out);
	msg_out.add(value);
	osc_server.beginPacket(osc_server.remoteIP(), 9000);
	msg_out.send(osc_server);
	osc_server.endPacket();
	msg_out.empty();*/

}

void osc_send_MSG_String(String addr_string, String value) {
	char address_out[20];
	char message[20];



	addr_string.toCharArray(address_out, addr_string.length() + 1); //address_out 

	value.toCharArray(message, value.length() + 1);

	OSCMessage msg_out(address_out);
	msg_out.add(message);
	osc_server.beginPacket(osc_server.remoteIP(), 9000);
	msg_out.send(osc_server);
	osc_server.endPacket();
	msg_out.empty();


}
// Other Functions like Sending loop
void osc_send_out_float_MSG_buffer() 
{
	if (osc_out_float_value.isEmpty() != true)
	{

		char address_out[20] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
		uint8_t i = 0;
		float value = 0;



		//msg_out_data = osc_out_float.dequeue();

		//addr_string.toCharArray(address_out, addr_string.length() + 1); //address_out 
		//msg_out_data.addr.toCharArray(address_out, msg_out_data.addr.length() + 1); //address_out
		if (osc_out_float_value.count() >= OSC_BUNDLE_SEND_COUNT)
		{
			OSCBundle bundle_out;
			//IPAddress ip_out(WiFi.localIP());
			IPAddress ip_out(osc_server.remoteIP());
			//ip_out[3] = osc_cfg.return_ip_LB;

			for (uint8_t z = 0; z <16; z++)
			{
				i = 0;
				while (osc_out_float_addr.peek() != 0 && i <20) {
					address_out[i] = osc_out_float_addr.dequeue();
					i++;
				}
				address_out[i] = osc_out_float_addr.dequeue(); // get the null char for end of string as well. so that we can fetch the next msg next time
				value = osc_out_float_value.dequeue();

				bundle_out.add(address_out).add(float(value));

				yield();
				memset(address_out, 0, 20);

}
			osc_server.beginPacket(ip_out, 9000); //{172,16,222,104}, 8001) ; //
			bundle_out.send(osc_server);
			osc_server.endPacket();
			bundle_out.empty();
			yield();
			//delay(1);

		}
		else
		{
			OSCBundle bundle_out;

			for (uint8_t z = 0; z <osc_out_float_value.count(); z++)
			{
				i = 0;
				while (osc_out_float_addr.peek() != 0 && i <20) {
					address_out[i] = osc_out_float_addr.dequeue();
					i++;
				}
				address_out[i] = osc_out_float_addr.dequeue(); // get the null char for end of string as well. so that we can fetch the next msg next time
				value = osc_out_float_value.dequeue();

				bundle_out.add(address_out).add(float(value));
				memset(address_out, 0, 20);

			}
			osc_server.beginPacket(osc_server.remoteIP(), 9000); //{172,16,222,104}, 8001) ; //
			bundle_out.send(osc_server);
			osc_server.endPacket();
			bundle_out.empty();
			yield();




		}
	}
}




#ifndef OSC_MC_SERVER_DISABLED
void osc_mc_send(String addr, uint8_t value)
{
	if (get_bool(OSC_MC_SEND) == true)
	{
		//debugMe("osc_mcI_send:", false);
		//debugMe(addr);
		char address_out[20];
		int out_value = int(value);

		addr.toCharArray(address_out, addr.length() + 1); //address_out 
		OSCMessage msg_out(address_out);
		msg_out.add(out_value);
//		osc_mc_server.beginPacketMulticast(osc_cfg.ipMulti, osc_cfg.portMulti, WiFi.localIP());
#ifdef ESP8266
		osc_mc_server.beginPacketMulticast(osc_cfg.ipMulti, osc_cfg.portMulti, WiFi.localIP());
#endif
#ifdef ESP32
		osc_mc_server.beginMulticastPacket();
#endif
		msg_out.send(osc_mc_server);
		osc_mc_server.endPacket();
	}
}

void osc_mc_send(String addr, float value)
{
	if (get_bool(OSC_MC_SEND) == true)
	{
		//debugMe("osc_mcF_send:",false);
		//debugMe(addr);

		char address_out[20];
		//int out_value = int(value);

		addr.toCharArray(address_out, addr.length() + 1); //address_out 
		OSCMessage msg_out(address_out);
		msg_out.add(value);
#ifdef ESP8266
		osc_mc_server.beginPacketMulticast(osc_cfg.ipMulti, osc_cfg.portMulti, WiFi.localIP());
#endif
#ifdef ESP32
		osc_mc_server.beginMulticastPacket();
#endif
		msg_out.send(osc_mc_server);
		osc_mc_server.endPacket();
	}
}
#endif

// OSC Settings MSG:/s




void osc_multiply_send()
{
	for (uint8_t x = 1;x <= 11  ; x++ )
	{
		debugMe((String("/multipl/") + String(x) + String("/1")));
		if (x != osc_cfg.conf_multiply)
			osc_queu_MSG_float((String("/multipl/") + String(x) + String("/1")), 0);
		else
			osc_queu_MSG_float((String("/multipl/") + String(x) + String("/1")), 1); 
		//debugMe((String("/multipl/") + String(x) + String("/1")));
	}
}

uint16_t osc_miltiply_get()
{
	uint16_t value = 1;


	switch (osc_cfg.conf_multiply)
	{
	case 1:
		value = 1;

		break;
	case 2:
		value = 10;

		break;
	case 3:
		value = 100;

		break;
	case 4:
		value = 8;

		break;
	case 5:
		value = 16;

		break;
	case 6:
		value = 32;

		break;
	case 7:
		value = 64;

		break;
	case 8:
		value = 128;

		break;
	case 9:
		value = 256;

		break;
	case 10:
		value = 512;

		break;
	case 11:
		value = 1024;

		break;

	default:
		value = 1;
		break;
	}


	return value;
}


// recive conf multiply toggle
void osc_multipl_rec(OSCMessage &msg, int addrOffset)
{	// recive routing of Multiply ocs settings by 1 10 or 100

	// OSC MESSAGE :/m/multipl/?/1
	if (bool(msg.getFloat(0)) == true)
	{
		String select_mode_string;
		// String select_bit_string;
		char address[10];
		bool switch_bool = false;

		msg.getAddress(address, addrOffset + 1);
		for (byte i = 0; i < sizeof(address); i++)
		{
			if (address[i] == '/')
			{
				switch_bool = true;

			}
			else if (switch_bool == false)
			{
				select_mode_string = select_mode_string + address[i];

			}

		}

		osc_cfg.conf_multiply = select_mode_string.toInt();
		osc_multiply_send();
		


	} // end  new msg

}



#ifndef ARTNET_DISABLED
void osc_rec_artnet_info(OSCMessage &msg, int addrOffset)
{
	// OSC MESSAGE :/s/AN/Row/collum  

	String collum_string;
	String row_string;
	char address[4];					// to pick info aut of the msg address
										//char address_out[20];	
	int select_bit = 0;
	String select_bit_string;
	bool switch_bool = false;			// for toggels to get row and collum

	String outbuffer = "/s/ANL";		// OSC return address
	float outvalue;						// return value to labels

	String out_add_label;				// address label



	msg.getAddress(address, addrOffset - 4, 1);					// get the select-bit info	
	select_bit_string = select_bit_string + address[0];
	select_bit = select_bit_string.toInt();

	memset(address, 0, sizeof(address));				// rest the address to blank


	msg.getAddress(address, addrOffset + 1);		// get the address for row / collum


	for (byte i = 0; i < sizeof(address); i++)
	{
		if (address[i] == '/') {
			switch_bool = true;
		}
		else if (switch_bool == false)
		{
			row_string = row_string + address[i];
		}
		else
			collum_string = collum_string + address[i];
	}

	int row = row_string.toInt() - 1;
	int collum = collum_string.toInt() - 1;  // Whit CRGB value in the pallete

	memset(address, 0, sizeof(address));
	byte msg_size = msg.size();

	//debugMe("row: " + String(row) + " col: " + String(collum));

	switch (collum)
	{
	case 0:		// Artnet Start universe
		switch (row)
		{
		case 0:
			artnet_cfg.startU = constrain(artnet_cfg.startU -  osc_miltiply_get(), 0, 255);
			break;
		case 2:
			artnet_cfg.startU = constrain(artnet_cfg.startU +  osc_miltiply_get(), 0, 255);
			break;
		}
		outvalue = artnet_cfg.startU;
		out_add_label = "/ASUL";
		break;

	case 1:		// artnet NR universes
		switch (row)
		{
		case 0:
			artnet_cfg.numU = constrain(artnet_cfg.numU -  osc_miltiply_get(), 0, 4);
			break;
		case 2:
			artnet_cfg.numU = constrain(artnet_cfg.numU +  osc_miltiply_get(), 0, 4);
			break;
		}
		outvalue = artnet_cfg.numU;
		out_add_label = "/ANUL";
		break;


	}

	outbuffer = String("/DS" + out_add_label);
	osc_queu_MSG_float(outbuffer, outvalue);

}


void osc_toggle_artnet(bool value)
{
	//debugMe("in artnet");
	if (value == true) {
		debugMe("enable artnet");

		
		//FastLED.setBrightness(led_brightness);
		//leds.fadeToBlackBy(255);
		//FastLED.show();

		write_bool(ARTNET_ENABLE, true); // artnet_enabled = true;		
		//enable_artnet();
		FS_artnet_write(0);
		WiFi_artnet_enable();
		//writeESP_play_Settings();
	}
	else {
		debugMe("disable artnet");
		write_bool(ARTNET_ENABLE, false);  //artnet_enabled = false;
		FS_artnet_write(0);
		//writeESP_play_Settings(),
		//Udp.stop();
		ESP.restart();


	}



}
#endif


// Strips

void osc_strips_G_toggle_rec(OSCMessage &msg, int addrOffset) {
	// OSC MESSAGE :/strips/GO:/?/Row/collum  

	String collum_string;
	String row_string;
	char address[4];					// to pick info aut of the msg address
										//char address_out[20];	
	int select_bit = 0;
	String select_bit_string;
	bool switch_bool = false;			// for toggels to get row and collum

	String outbuffer = "/s/ANL";		// OSC return address
	float outvalue = 0;						// return value to labels

	String out_add_label;				// address label



	msg.getAddress(address, addrOffset - 1, 1);					// get the select-bit info	
	select_bit_string = select_bit_string + address[0];
	select_bit = select_bit_string.toInt();
	

	memset(address, 0, sizeof(address));				// rest the address to blank


	msg.getAddress(address, addrOffset + 3);		// get the address for row / collum


	for (byte i = 0; i < sizeof(address); i++)
	{
		if (address[i] == '/') {
			switch_bool = true;
		}
		else if (switch_bool == false)
		{
			row_string = row_string + address[i];
		}
		else
			collum_string = collum_string + address[i];
	}

	int row = row_string.toInt() - 1;
	int collum = collum_string.toInt() - 1;  // Whit CRGB value in the pallete

	//debugMe(address);
	memset(address, 0, sizeof(address));
	byte msg_size = msg.size();

	//debugMe("row: " + String(row) + " col: " + String(collum));

	bitWrite(global_strip_opt[select_bit][row], collum, bool(msg.getFloat(0)));


	//outbuffer = String("/s" + out_add_label);
	//osc_send_MSG(outbuffer, outvalue);

}


void osc_strips_settings_rec(OSCMessage &msg, int addrOffset) {
	// OSC MESSAGE :/strip/??:/bit/row/collum   xx/?/?
	// :/bit/+-option/strip



	//byte addrOffset = 7+6+1 ;
	String bit_string;
	String option_string;
	String strip_string;

	int bit_int;
	int option_int;
	int strip_int;


	char address[4];
	//char address_out[20];
	bool switch_bool = false;

	String outbuffer = "/strips/sX/SIL/X";
	float outvalue = 255;

	msg.getAddress(address, addrOffset + 1, 1);
	bit_string = bit_string + address[0];

	memset(address, 0, sizeof(address));

	msg.getAddress(address, addrOffset + 3);


	for (byte i = 0; i < sizeof(address); i++) {
		if (address[i] == '/') {
			switch_bool = true;

		}
		else if (switch_bool == false) {
			option_string = option_string + address[i];

		}
		else
			strip_string = strip_string + address[i];

	}

	bit_int = bit_string.toInt();
	option_int = option_string.toInt() - 1;
	strip_int = strip_string.toInt() - 1;

	memset(address, 0, sizeof(address));

	msg.getAddress(address, addrOffset - 2, 2);


	byte msg_size = msg.size();

	char full_addr[msg_size + 1 - 4];
	//hmsg.getAddress(full_addr, 0, msg_size-4 ) ;

	//full_addr[sizeof(full_addr)- 2] = 'X'; 


	if (bool(msg.getFloat(0)) == true) {  // only on pushdown
		if (address[0] == 'S' && address[1] == 'I') 
		{
			switch (option_int) 
			{
				case 0:
					part[strip_int + bit_int * 8].index_start = constrain(part[strip_int + bit_int * 8].index_start -  osc_miltiply_get(),0, MAX_INDEX_LONG-1);
					break;
				case 2:
					part[strip_int + bit_int * 8].index_start = constrain(part[strip_int + bit_int * 8].index_start +  osc_miltiply_get(), 0, MAX_INDEX_LONG-1);
					break;
			}

			outvalue = float(part[strip_int + bit_int * 8].index_start);

		}

		if (address[0] == 'I' && address[1] == 'A') 
		{
			switch (option_int) 
			{
				case 0:
					part[strip_int + bit_int * 8].index_add -=  osc_miltiply_get();
					break;
				case 2:
					part[strip_int + bit_int * 8].index_add +=  osc_miltiply_get();
					break;
			}
			outvalue = float(part[strip_int + bit_int * 8].index_add);

		}

		if (address[0] == 'I' && address[1] == 'F') 
		{
			switch (option_int) 
			{
				case 0:
					part[strip_int + bit_int * 8].index_add_pal -=  osc_miltiply_get();
					break;
				case 2:
					part[strip_int + bit_int * 8].index_add_pal +=  osc_miltiply_get();
					break;
			}
			outvalue = float(part[strip_int + bit_int * 8].index_add_pal);


		}

		if (address[0] == 'S' && address[1] == 'L') 
		{
			if (get_bool(OSC_EDIT) == true)
				switch (option_int) 
				{
					case 0:
						part[strip_int + bit_int * 8].start_led = constrain(part[strip_int + bit_int * 8].start_led -  osc_miltiply_get(), 0, MAX_NUM_LEDS - part[strip_int + bit_int * 8].nr_leds);
						break;
					case 1:
						if (strip_int !=0 )
							part[strip_int + bit_int * 8].start_led = part[strip_int - 1 + bit_int * 8].start_led + part[strip_int-1 + bit_int * 8].nr_leds;
						else  if (bit_int != 0)
							part[strip_int + bit_int * 8].start_led = part[ 7  + (bit_int- 1) * 8].start_led + part[7 + (bit_int-1) * 8].nr_leds;
					break;
					case 2:
						part[strip_int + bit_int * 8].start_led = constrain(part[strip_int + bit_int * 8].start_led +  osc_miltiply_get(), 0, MAX_NUM_LEDS - part[strip_int + bit_int * 8].nr_leds);
						break;
				}
			outvalue = float(part[strip_int + bit_int * 8].start_led);
		}

		if (address[0] == 'N' && address[1] == 'L') 
		{
			if (get_bool(OSC_EDIT) == true)
				switch (option_int) 
				{
					case 0:
						part[strip_int + bit_int * 8].nr_leds = constrain(part[strip_int + bit_int * 8].nr_leds -  osc_miltiply_get(), 0, MAX_NUM_LEDS - part[strip_int + bit_int * 8].start_led);
						break;
					case 2:
						part[strip_int + bit_int * 8].nr_leds = constrain(part[strip_int + bit_int * 8].nr_leds +  osc_miltiply_get(), 0, MAX_NUM_LEDS - part[strip_int + bit_int * 8].start_led);
						break;
				}
			outvalue = float(part[strip_int + bit_int * 8].nr_leds);
		}

		outbuffer = String("/strips/s" + String(bit_int) + "/" + String(address) + "L/" + String(strip_int + 1));
		osc_queu_MSG_float(outbuffer, outvalue);
	}

}

void osc_strips_settings_send(byte y) {
	// OSC MESSAGE OUT :/strip/s?/xx/?/?

	for (int i = 0; i < 8; i++) {

		osc_queu_MSG_float(String("/strips/s" + String(y) + "/SLL/" + String(i + 1)), float(part[i + (y * 8)].start_led));
		osc_queu_MSG_float(String("/strips/s" + String(y) + "/NLL/" + String(i + 1)), float(part[i + (y * 8)].nr_leds));
		osc_queu_MSG_float(String("/strips/s" + String(y) + "/IAL/" + String(i + 1)), float(part[i + (y * 8)].index_add));
		osc_queu_MSG_float(String("/strips/s" + String(y) + "/IFL/" + String(i + 1)), float(part[i + (y * 8)].index_add_pal));
		osc_queu_MSG_float(String("/strips/s" + String(y) + "/SIL/" + String(i + 1)), float(part[i + (y * 8)].index_start));

		for (uint8_t row = 0; row < _M_NR_GLOBAL_OPTIONS_; row++)
		{
			osc_queu_MSG_float(String("/strips/GO/" + String(y) + "/" + String(row + 1) + String("/") + String(i + 1)), float(bitRead(global_strip_opt[y][row], i)));
		}

		yield(); //delay(1); 
	}
	osc_multiply_send();

}

void osc_strips_toggle_rec(OSCMessage &msg, int addrOffset) {
	// recive OSC message for STRIPS
	// OSC MESSAGE :/strip/T:/bit/row/collum   T/?/?
	//						:/bit/option/strip

	String bit_string;
	String option_string;
	String strip_string;

	int bit_int;
	int option_int;
	int strip_int;

	char address[6];
	bool switch_bool = false;

	msg.getAddress(address, addrOffset + 1, 1);
	bit_string = bit_string + address[0];
	bit_int = bit_string.toInt();

	msg.getAddress(address, addrOffset + 3);

	for (byte i = 0; i < sizeof(address); i++)
	{
		if (address[i] == '/')
			switch_bool = true;
		else if (switch_bool == false)
			option_string = option_string + address[i];
		else
			strip_string = strip_string + address[i];
	}

	option_int = option_string.toInt() - 1;
	strip_int = strip_string.toInt() - 1;



	bitWrite(strip_menu[bit_int][option_int], strip_int, bool(msg.getFloat(0)));

}

void osc_strips_toggle_send(byte y)
{
	// OSC MESSAGE OUT :/strip/s?/T/?/?
	for (int i = 0; i < 8; i++)
	{
		for (int z = 0; z < _M_NR_OPTIONS_; z++)
		{
			osc_queu_MSG_float(String("/strips/T/" + String(y) + "/" + String(z + 1) + "/" + String(i + 1)), float(bitRead(strip_menu[y][z], i)));
		}
		osc_queu_MSG_float(String("/strips/s" + String(y) + "/SLL/" + String(i + 1)), float(part[i + (y * 8)].start_led));
		osc_queu_MSG_float(String("/strips/s" + String(y) + "/NLL/" + String(i + 1)), float(part[i + (y * 8)].nr_leds));
	}
}

void osc_strips_routing(OSCMessage &msg, int addrOffset) {
	// OSC MESSAGE :/strip
	msg.route("/T",		osc_strips_toggle_rec, addrOffset);
	msg.route("/SI",	osc_strips_settings_rec, addrOffset);				// Start Index
	msg.route("/IA",	osc_strips_settings_rec, addrOffset);
	msg.route("/IF",	osc_strips_settings_rec, addrOffset);				// Index add
	msg.route("/SL",	osc_strips_settings_rec, addrOffset);
	msg.route("/NL",	osc_strips_settings_rec, addrOffset);
	msg.route("/GO",	osc_strips_G_toggle_rec, addrOffset);

	if (msg.fullMatch("/s0/refresh",		addrOffset) && bool(msg.getFloat(0)) == true) { osc_strips_toggle_send(0); osc_strips_toggle_send(1); }
	if (msg.fullMatch("/s2/refresh",		addrOffset) && bool(msg.getFloat(0)) == true) { osc_strips_toggle_send(2); osc_strips_toggle_send(3); }
	if (msg.fullMatch("/s0/conf_refesh",	addrOffset) && bool(msg.getFloat(0)) == true) { osc_strips_settings_send(0); osc_strips_settings_send(1); }
	if (msg.fullMatch("/s2/conf_refesh",	addrOffset) && bool(msg.getFloat(0)) == true) { osc_strips_settings_send(2); osc_strips_settings_send(3); }
}


// OSC Forms 
void osc_forms_send(byte y) {
	// OSC MESSAGE OUT :/form/f"y"/T/?/i
	debugMe("Forms - refesh Start ");
	debugMe(String("y=" + String(y)));
	for (int i = 0; i < 8; i++) 
	{
		//debugMe(String("i=" + String(i)));


		for (int z = 0; z < _M_NR_FORM_OPTIONS_ ; z++)
		{
			if(z < 16) 							osc_queu_MSG_float(String("/form/T/" + String(y) + "/" + String(z + 1) + "/" + String(i + 1)), float(bitRead(form_menu[y][z], i)));
			else if(z < 28)						osc_queu_MSG_float(String("/form/F/" + String(y) + "/" + String(z + 1 -16) + "/" + String(i + 1)), float(bitRead(form_menu[y][z], i)));
			else if(z < _M_NR_FORM_OPTIONS_)	osc_queu_MSG_float(String("/form/X/" + String(y) + "/" + String(z + 1 -28) + "/" + String(i + 1)), float(bitRead(form_menu[y][z], i)));
		}
		//debugMe(String("for 1 done"), true);
		
		osc_queu_MSG_float(String("/form/f" + String(y) + "/SLL/" + String(i + 1)), float(form_part[i + (y * 8)].start_led));
		osc_queu_MSG_float(String("/form/f" + String(y) + "/NLL/" + String(i + 1)), float(form_part[i + (y * 8)].nr_leds));


		osc_queu_MSG_float(String("/form/FF/" + String(y) + "/" + String(i + 1)), osc_byte_tofloat(form_part[i + (y * 8)].fade_value, MAX_FADE_VALUE));
		osc_queu_MSG_float(String("/form/f" + String(y) + "/FFL/" + String(i + 1)), float(form_part[i + (y * 8)].fade_value));
		
		osc_queu_MSG_float(String("/form/FA/" + String(y) + "/" + String(i + 1)), osc_byte_tofloat(form_part[i + (y * 8)].FX_level, 255));
		osc_queu_MSG_float(String("/form/f" + String(y) + "/FAL/" + String(i + 1)), float(form_part[i + (y * 8)].FX_level));
		
		osc_queu_MSG_float(String("/form/GL/" + String(y) + "/" + String(i + 1)), osc_byte_tofloat(form_part[i + (y * 8)].glitter_value, MAX_GLITTER_VALUE));
		osc_queu_MSG_float(String("/form/f" + String(y) + "/GLL/" + String(i + 1)), float(form_part[i + (y * 8)].glitter_value));
		osc_queu_MSG_float(String("/form/f" + String(y) + "/JDL/" + String(i + 1)), float(form_part[i + (y * 8)].juggle_nr_dots));
		osc_queu_MSG_float(String("/form/f" + String(y) + "/JSL/" + String(i + 1)), float(form_part[i + (y * 8)].juggle_speed));
		yield();
		osc_send_out_float_MSG_buffer();
		
	}
	//debugMe(String("forms  done"), true);
	
}

void osc_forms_config_send(byte y) {
	// OSC MESSAGE OUT :/strip/s?/xx/?/?

	for (int i = 0; i < 8; i++) {

		osc_queu_MSG_float(String("/form/f" + String(y) + "/SLL/" + String(i + 1)), float(form_part[i + (y * 8)].start_led));
		osc_queu_MSG_float(String("/form/f" + String(y) + "/NLL/" + String(i + 1)), float(form_part[i + (y * 8)].nr_leds));
		osc_queu_MSG_float(String("/form/f" + String(y) + "/IAL/" + String(i + 1)), float(form_part[i + (y * 8)].index_add));
		osc_queu_MSG_float(String("/form/f" + String(y) + "/IFL/" + String(i + 1)), float(form_part[i + (y * 8)].index_add_pal));
		osc_queu_MSG_float(String("/form/f" + String(y) + "/SIL/" + String(i + 1)), float(form_part[i + (y * 8)].index_start));
		osc_queu_MSG_float(String("/form/f" + String(y) + "/JDL/" + String(i + 1)), float(form_part[i + (y * 8)].juggle_nr_dots));
		osc_queu_MSG_float(String("/form/f" + String(y) + "/JS/" + String(i + 1)), osc_byte_tofloat(form_part[i + (y * 8)].juggle_speed, MAX_JD_SPEED_VALUE));
		osc_queu_MSG_float(String("/form/f" + String(y) + "/JSL/" + String(i + 1)), float(form_part[i + (y * 8)].juggle_speed));
		osc_queu_MSG_float(String("/form/f" + String(y) + "/RFL/" + String(i + 1)), float(form_part[i + (y * 8)].rotate));
 
	}

	osc_multiply_send();
}

void osc_forms_config_rec(OSCMessage &msg, int addrOffset) {
	// OSC MESSAGE :/form/??:/bit/row/collum   xx/?/?
	//	:/bit/option/form
 

	String bit_string;
	String option_string;
	String form_string;

	int bit_int;
	int option_int;
	int form_int;
	char address[5];
	//char address_out[20];
	bool switch_bool = false;

	String outbuffer = "/form/fx/SIL/X";
	float outvalue;


	memset(address, 0, sizeof(address));
	//memset(address_out, 0, sizeof(address_out));


	msg.getAddress(address, addrOffset + 1, 1);
	bit_string = bit_string + address[0];

	memset(address, 0, sizeof(address));

	msg.getAddress(address, addrOffset + 3);
	debugMe("Form-Addr1 : " + String(address));

	for (byte i = 0; i < sizeof(address); i++) {
		if (address[i] == '/') {
			switch_bool = true;

		}
		else if (switch_bool == false) {
			option_string = option_string + address[i];   // first part

		}
		else
			form_string = form_string + address[i];    //second part

	}
	bit_int = bit_string.toInt();
	option_int = option_string.toInt() - 1;
	form_int = form_string.toInt() - 1;

	memset(address, 0, sizeof(address));

	msg.getAddress(address, addrOffset - 2, 2);
	debugMe("Form-type : " + String(address));

	if (bool(msg.getFloat(0)) == true) 
	{  // only on pushdown
		if (address[0] == 'S' && address[1] == 'I') {
			switch (option_int) {


			case 0:
				form_part[form_int + bit_int * 8].index_start = constrain(form_part[form_int + bit_int * 8].index_start -  osc_miltiply_get(), 0, MAX_INDEX_LONG-1);
				break;
			case 2:
				form_part[form_int + bit_int * 8].index_start = constrain(form_part[form_int + bit_int * 8].index_start +  osc_miltiply_get(), 0, MAX_INDEX_LONG-1);
				break;
			}
			outvalue = float(form_part[form_int + bit_int * 8].index_start);
		}

		if (address[0] == 'I' && address[1] == 'A') {
			switch (option_int) {
			case 0:
				form_part[form_int + bit_int * 8].index_add -=  osc_miltiply_get();
				break;
			case 2:
				form_part[form_int + bit_int * 8].index_add +=  osc_miltiply_get();
				break;
			}
			//outbuffer = String("/strips/s" + String(z) + "/AIL/" + String(select_bit_int+1));

			outvalue = float(form_part[form_int + bit_int * 8].index_add);
			//osc_send_MSG(outbuffer , float(part[select_bit_int + z *8 ].index_add)) ;   

		}

		else if (address[0] == 'I' && address[1] == 'F') {
			switch (option_int) {
			case 0:
				form_part[form_int + bit_int * 8].index_add_pal -=  osc_miltiply_get();
				break;
			case 2:
				form_part[form_int + bit_int * 8].index_add_pal +=  osc_miltiply_get();
				break;
			}
			//outbuffer = String("/strips/s" + String(z) + "/AIL/" + String(select_bit_int+1));

			outvalue = float(form_part[form_int + bit_int * 8].index_add_pal);
			//osc_send_MSG(outbuffer , float(part[select_bit_int + z *8 ].index_add)) ;   

		}

		else  if (address[0] == 'S' && address[1] == 'L') {
			if ((get_bool(OSC_EDIT) == true) )
				switch (option_int)
				{
				case 0:
					//form_part[select_bit_int + z * 8].start_led -=  osc_miltiply_get();

					form_part[form_int + bit_int * 8].start_led = constrain(form_part[form_int + bit_int * 8].start_led -  osc_miltiply_get(), 0, MAX_NUM_LEDS - form_part[form_int + bit_int * 8].nr_leds);
					break;
				case 1:
					if (form_int != 0)
						form_part[form_int + bit_int * 8].start_led = form_part[form_int - 1 + bit_int * 8].start_led + form_part[form_int - 1 + bit_int * 8].nr_leds;
					else  if (bit_int != 0)
						form_part[form_int + bit_int * 8].start_led = form_part[7 + (bit_int - 1) * 8].start_led + form_part[7 + (bit_int - 1) * 8].nr_leds;
					break;

				case 2:
					//form_part[select_bit_int + z * 8].start_led +=  osc_miltiply_get();
					form_part[form_int + bit_int * 8].start_led = constrain(form_part[form_int + bit_int * 8].start_led +  osc_miltiply_get(), 0, MAX_NUM_LEDS - form_part[form_int + bit_int * 8].nr_leds);
					break;
				}
			//outbuffer = String("/strips/s" + String(z) + "/AIL/" + String(select_bit_int+1));

			outvalue = float(form_part[form_int + bit_int * 8].start_led);
			//osc_send_MSG(outbuffer , float(part[select_bit_int + z *8 ].index_add)) ;   

		}

		else  if (address[0] == 'N' && address[1] == 'L') {
			if ((get_bool(OSC_EDIT) == true) )
				switch (option_int)
				{
				case 0:
					form_part[form_int + bit_int * 8].nr_leds = constrain(form_part[form_int + bit_int * 8].nr_leds -  osc_miltiply_get(), 0, MAX_NUM_LEDS - form_part[form_int + bit_int * 8].start_led);
					break;
				case 2:
					form_part[form_int + bit_int * 8].nr_leds = constrain(form_part[form_int + bit_int * 8].nr_leds +  osc_miltiply_get(), 0, MAX_NUM_LEDS - form_part[form_int + bit_int * 8].start_led);
					break;
				}

			outvalue = float(form_part[form_int + bit_int * 8].nr_leds);

		}

		else if (address[0] == 'J' && address[1] == 'D') {
			switch (option_int) {
			case 0:
				form_part[form_int + bit_int * 8].juggle_nr_dots--;
				break;
			case 2:
				form_part[form_int + bit_int * 8].juggle_nr_dots++;
				break;
			}


			outvalue = float(form_part[form_int + bit_int * 8].juggle_nr_dots);
			//osc_send_MSG(outbuffer , float(part[select_bit_int + z *8 ].index_add)) ;   

		}
		else if (address[0] == 'R' && address[1] == 'F') 
		{
			if (get_bool(OSC_EDIT) == true)
				if (form_part[form_int + bit_int * 8].nr_leds != 0) 
				{
					switch (option_int) 
					{
					case 0:
						form_part[form_int + bit_int * 8].rotate = constrain(form_part[form_int + bit_int * 8].rotate -  osc_miltiply_get(), -(form_part[form_int + bit_int * 8].nr_leds) + 1, (form_part[form_int + bit_int * 8].nr_leds) - 1);
						break;
					case 2:

						form_part[form_int + bit_int * 8].rotate = constrain(form_part[form_int + bit_int * 8].rotate +  osc_miltiply_get(), -(form_part[form_int + bit_int * 8].nr_leds) + 1, (form_part[form_int + bit_int * 8].nr_leds) - 1);
						break;
					}
				}

			outvalue = float(form_part[form_int + bit_int * 8].rotate);
			//osc_send_MSG(outbuffer , float(part[select_bit_int + z *8 ].index_add)) ;   

		}


		
		
				// no else since other osc send 
		if (address[0] == 'M' && address[1] == 'A') 
		{

				debugMe(bit_int , true );
				debugMe(option_int , true );
				debugMe(form_int , true );

				//form_menu[bit_int][option_int] = ~form_menu[bit_int][option_int];
				
				for (uint8_t formX = bit_int * 8 ; formX < (bit_int+1) * 8  ; formX++ )
				{
				if (form_part[formX].nr_leds != 0) 
				{
					debugMe(formX , true );
					boolean trun_value = !bitRead(form_menu[bit_int][option_int], formX- 8*bit_int);

					bitWrite(form_menu[bit_int][option_int], formX - 8*bit_int, trun_value ) ;
				
				outbuffer = String("/form/T/" + String(bit_int) + "/" + String(option_int+1) + "/" + String(formX- 8*bit_int + 1));
				osc_queu_MSG_float(outbuffer, bitRead(form_menu[bit_int][option_int], formX- 8*bit_int));
				}
				}  
			
		}
		else
		{
			outbuffer = String("/form/f" + String(bit_int) + "/" + String(address) + "L/" + String(form_int + 1));
			osc_queu_MSG_float(outbuffer, outvalue);

		}
		
	}

}

void osc_forms_fader_rec(OSCMessage &msg, int addrOffset) 
{
	// OSC MESSAGE :/form/FA:/?   

	String select_mode_string;		// Fader NR
	String select_strip_addr;		// form NR
									//String select_bit_string;
	char address[3];
	char address_out[20];
	bool switch_bool = false;
	uint8_t z = 0;						// form NR in uint8_t
	String outbuffer = "/form/fx/FA/X";
	float outvalue;

	msg.getAddress(address, addrOffset + 1, 1);
	select_strip_addr = select_strip_addr + address[0];
	z = select_strip_addr.toInt();
	memset(address, 0, sizeof(address));

	msg.getAddress(address, addrOffset + 3);

	for (byte i = 0; i < sizeof(address); i++) 
	{
		select_mode_string = select_mode_string + address[i];
	}

	//int select_bit_int = select_bit_string.toInt() - 1;
	int select_mode_int = select_mode_string.toInt() - 1;

	memset(address, 0, sizeof(address));

	msg.getAddress(address, addrOffset - 2, 2);

	byte msg_size = msg.size();

	if (address[0] == 'F' && address[1] == 'A') 
	{
		form_part[select_mode_int + z * 8].FX_level = (msg.getFloat(0) * 255);  //byte(msg.getFloat(0) * 255);
		outvalue = float(form_part[select_mode_int + z * 8].FX_level);
	}

	if (address[0] == 'F' && address[1] == 'F') 
	{
		form_part[select_mode_int + z * 8].fade_value = (msg.getFloat(0) * MAX_FADE_VALUE);  //byte(msg.getFloat(0) * 255);
		outvalue = float(form_part[select_mode_int + z * 8].fade_value);
	}




	if (address[0] == 'G' && address[1] == 'L') 
	{				//map(msg.getFloat(0), 0,1,0,MAX_FADE) 

		form_part[select_mode_int + z * 8].glitter_value = (msg.getFloat(0) *  MAX_GLITTER_VALUE); //byte(msg.getFloat(0) * 255);

																								   //outbuffer = String("/strips/s" + String(z) + "/AIL/" + String(select_bit_int+1));

		outvalue = float(form_part[select_mode_int + z * 8].glitter_value);
		//osc_send_MSG(outbuffer , float(part[select_bit_int + z *8 ].index_add)) ;   

	}

	if (address[0] == 'J' && address[1] == 'S') 
	{

		form_part[select_mode_int + z * 8].juggle_speed = constrain(msg.getFloat(0) * MAX_JD_SPEED_VALUE, 0, MAX_JD_SPEED_VALUE);  //byte(msg.getFloat(0) * 255);

																								 //outbuffer = String("/strips/s" + String(z) + "/AIL/" + String(select_bit_int+1));

		outvalue = float(form_part[select_mode_int + z * 8].juggle_speed);
		//osc_send_MSG(outbuffer , float(part[select_bit_int + z *8 ].index_add)) ;   

	}

	{
		outbuffer = String("/form/f" + String(z) + "/" + String(address) + "L/" + String(select_mode_int + 1));
		osc_queu_MSG_float(outbuffer, outvalue);
	}


}

void osc_forms_toggle_rec(OSCMessage &msg, int addrOffset) // OSC: /form/T:/bit/row/collum 
{															
				// :/bit/option/form
				// recive OSC message for STRIPS
				// OSC MESSAGE :/form/f?/   T/?/?

				//DBG_OUTPUT_PORT.println("lol"); 

				//byte addrOffset = 7+6+1 ;																				

	String bit_string;
	String option_string;
	String form_string;

	int bit_int;
	int option_int;
	int form_int;

	char address[6];
	bool switch_bool = false;

	msg.getAddress(address, addrOffset + 1, 1);
	bit_string = bit_string + address[0];


	msg.getAddress(address, addrOffset + 3);
	//DBG_OUTPUT_PORT.println(address);

	for (byte i = 0; i < sizeof(address); i++) {
		if (address[i] == '/') {
			switch_bool = true;  

		}
		else if (switch_bool == false) {
			option_string = option_string + address[i];   // row

		}
		else
			form_string = form_string + address[i];      // col

	}

	bit_int = bit_string.toInt();
	option_int = option_string.toInt() - 1;
	form_int = form_string.toInt() - 1;


	bitWrite(form_menu[bit_int][option_int], form_int, bool(msg.getFloat(0)));

}

void osc_forms_toggle_rec_fx(OSCMessage &msg, int addrOffset) // OSC: /form/T:/bit/row/collum 
{															
				// :/bit/option/form
				// recive OSC message for STRIPS
				// OSC MESSAGE :/form/f?/   T/?/?

				//DBG_OUTPUT_PORT.println("lol"); 

				//byte addrOffset = 7+6+1 ;																				

	String bit_string;
	String option_string;
	String form_string;

	int bit_int;
	int option_int;
	int form_int;

	char address[6];
	bool switch_bool = false;

	msg.getAddress(address, addrOffset + 1, 1);
	bit_string = bit_string + address[0];


	msg.getAddress(address, addrOffset + 3);
	//DBG_OUTPUT_PORT.println(address);

	for (byte i = 0; i < sizeof(address); i++) {
		if (address[i] == '/') {
			switch_bool = true;  

		}
		else if (switch_bool == false) {
			option_string = option_string + address[i];   // row

		}
		else
			form_string = form_string + address[i];      // col

	}

	bit_int = bit_string.toInt();
	option_int = option_string.toInt() - 1;
	form_int = form_string.toInt() - 1;

	option_int = option_int + 16;

	bitWrite(form_menu[bit_int][option_int], form_int, bool(msg.getFloat(0)));

}

void osc_forms_toggle_rec_fx2(OSCMessage &msg, int addrOffset) // OSC: /form/T:/bit/row/collum 
{															
				// :/bit/option/form
				// recive OSC message for STRIPS
				// OSC MESSAGE :/form/f?/   T/?/?

				//DBG_OUTPUT_PORT.println("lol"); 

				//byte addrOffset = 7+6+1 ;																				

	String bit_string;
	String option_string;
	String form_string;

	int bit_int;
	int option_int;
	int form_int;

	char address[6];
	bool switch_bool = false;

	msg.getAddress(address, addrOffset + 1, 1);
	bit_string = bit_string + address[0];


	msg.getAddress(address, addrOffset + 3);
	//DBG_OUTPUT_PORT.println(address);

	for (byte i = 0; i < sizeof(address); i++) {
		if (address[i] == '/') {
			switch_bool = true;  

		}
		else if (switch_bool == false) {
			option_string = option_string + address[i];   // row

		}
		else
			form_string = form_string + address[i];      // col

	}

	bit_int = bit_string.toInt();
	option_int = option_string.toInt() - 1;
	form_int = form_string.toInt() - 1;

	option_int = option_int + 28;

	bitWrite(form_menu[bit_int][option_int], form_int, bool(msg.getFloat(0)));

}


void osc_forms_routing(OSCMessage &msg, int addrOffset) {
	// OSC MESSAGE :/form
	debugMe("in Form Routing");

	if (msg.fullMatch("/f/refresh", addrOffset) && bool(msg.getFloat(0)) == true) { osc_forms_send(0); osc_forms_send(1); }
	if (msg.fullMatch("/f/conf_refesh", addrOffset) && bool(msg.getFloat(0)) == true) { osc_forms_config_send(0); osc_forms_config_send(1); }

	//toggle buttons 
	msg.route("/T", osc_forms_toggle_rec, addrOffset);			// form menu toggles
	msg.route("/F", osc_forms_toggle_rec_fx, addrOffset);			// form menu toggles
	msg.route("/X", osc_forms_toggle_rec_fx2, addrOffset);			// form menu toggles
														//msg.route("/f1/T",  osc_rec_forms, addrOffset);
	msg.route("/MA", osc_forms_config_rec, addrOffset);
														// push buttons
	msg.route("/SI", osc_forms_config_rec, addrOffset);	// Start Index
	msg.route("/IA", osc_forms_config_rec, addrOffset);	// Index Add led
	msg.route("/IF", osc_forms_config_rec, addrOffset);	// Index Add pal
	msg.route("/SL", osc_forms_config_rec, addrOffset);	// Start Led
	msg.route("/NL", osc_forms_config_rec, addrOffset);	// NR Leds
	msg.route("/JD", osc_forms_config_rec, addrOffset);	// Juggle Dots
	msg.route("/RF", osc_forms_config_rec, addrOffset);

	// Faders
	msg.route("/FA", osc_forms_fader_rec, addrOffset);	// Fade
	msg.route("/FF", osc_forms_fader_rec, addrOffset);	// Fade
	msg.route("/GL", osc_forms_fader_rec, addrOffset);	// glitter
	msg.route("/JS", osc_forms_fader_rec, addrOffset);	// juggle speed

														//msg.route("/f1/FA", osc_rec_forms_fader, addrOffset);
														//msg.route("/f0/GL", osc_rec_forms_fader, addrOffset);	// Glitter Value
														//msg.route("/f1/GL", osc_rec_forms_fader, addrOffset);
														//msg.route("/f0/JS", osc_rec_forms_fader, addrOffset);	// Juggle Speed
														//msg.route("/f1/JS", osc_rec_forms_fader, addrOffset);
														//
														//DBG_OUTPUT_PORT.println("yeah");      
}


// OSC COPY Strips
void osc_copy_settings_send() {
	// OSC MESSAGE OUT :/copy/s?/xx/?/?
	for (uint8_t y = 0; y < NR_COPY_LED_BYTES; y++) {
		//DBG_OUTPUT_PORT.println("ohssssh-info");

		/*OSCBundle bundle_out;
		String addr = "/strips/s0/SSL/0";
		char   addr_char[20];*/
		//for(int y = 0 ; y < 2  ; y++){



		for (int i = 0; i < 8; i++) {

			osc_queu_MSG_float(String("/copy/c" + String(y) + "/SLL/" + String(i + 1)), float(copy_leds[i + (y * 8)].start_led));
			osc_queu_MSG_float(String("/copy/c" + String(y) + "/NLL/" + String(i + 1)), float(copy_leds[i + (y * 8)].nr_leds));
			osc_queu_MSG_float(String("/copy/c" + String(y) + "/RLL/" + String(i + 1)), float(copy_leds[i + (y * 8)].Ref_LED));
			osc_queu_MSG_float(String("/copy/c" + String(y) + "/T/1/" + String(i + 1)), float(bitRead(copy_leds_mode[y], i)));


			//addr = String("/copy/c" + String(y) + "/SLL/" + String(i + 1));
			//addr.toCharArray(addr_char, 17);
			//bundle_out.add(addr_char).add(float(copy_leds[i + (y * 8)].start_led));

			//addr = String("/copy/c" + String(y) + "/NLL/" + String(i + 1));
			//addr.toCharArray(addr_char, 17);
			//bundle_out.add(addr_char).add(float(copy_leds[i + (y * 8)].nr_leds));

			//addr = String("/copy/c" + String(y) + "/RLL/" + String(i + 1));
			//addr.toCharArray(addr_char, 17);
			//bundle_out.add(addr_char).add(float(copy_leds[i + (y * 8)].Ref_LED));

			//addr = String("/copy/c" + String(y) + "/T/1/" + String(i + 1));
			//addr.toCharArray(addr_char, 17);
			//bundle_out.add(addr_char).add(float(bitRead(copy_leds_mode[y], i)));

			//// DBG_OUTPUT_PORT.print("---s--");
			//// DBG_OUTPUT_PORT.println(part[i + ( y * 8 ) ].index_start);

			//osc_server.beginPacket(osc_server.remoteIP(), 9000); //{172,16,222,104}, 8001) ; //
			//bundle_out.send(osc_server);
			//osc_server.endPacket();
			//bundle_out.empty();
			//delay(1);
			//do_fastled();


			//DBG_OUTPUT_PORT.println(addr_char);  
		}

	}

}

void osc_copy_settings_rec(OSCMessage &msg, int addrOffset) {
	// OSC MESSAGE :/copy/c?/   xx/?/?

	//DBG_OUTPUT_PORT.println("lol"); 

	//byte addrOffset = 7+6+1 ;
	String select_mode_string;
	String select_strip_addr;
	String select_bit_string;
	char address[3];
	char address_out[20];
	bool switch_bool = false;
	byte z = 0;
	String outbuffer = "/copy/c?/SIL/X";
	float outvalue;

	msg.getAddress(address, addrOffset - 4, 1);
	select_strip_addr = select_strip_addr + address[0];
	z = select_strip_addr.toInt();
	//DBG_OUTPUT_PORT.println(address);


	memset(address, 0, sizeof(address));

	msg.getAddress(address, addrOffset + 1);
	//DBG_OUTPUT_PORT.println(address);

	for (byte i = 0; i < sizeof(address); i++) {
		if (address[i] == '/') {
			switch_bool = true;

		}
		else if (switch_bool == false) {
			select_mode_string = select_mode_string + address[i];

		}
		else
			select_bit_string = select_bit_string + address[i];

	}

	int select_bit_int = select_bit_string.toInt() - 1;
	int select_mode_int = select_mode_string.toInt() - 1;

	memset(address, 0, sizeof(address));

	msg.getAddress(address, addrOffset - 2, 2);
	/*DBG_OUTPUT_PORT.println(select_mode_int);
	DBG_OUTPUT_PORT.println("--"); */

	byte msg_size = msg.size();

	char full_addr[msg_size + 1 - 4];
	//hmsg.getAddress(full_addr, 0, msg_size-4 ) ;

	//full_addr[sizeof(full_addr)- 2] = 'X'; 


	if (bool(msg.getFloat(0)) == true) {  // only on pushdown


		if (address[0] == 'S' && address[1] == 'L') {
			if (get_bool(OSC_EDIT) == true)
				switch (select_mode_int) {
				case 0:
					//copy_leds[select_bit_int + z * 8].start_led -=  osc_miltiply_get();
					copy_leds[select_bit_int + z * 8].start_led = constrain(copy_leds[select_bit_int + z * 8].start_led -  osc_miltiply_get(), 0, MAX_NUM_LEDS - copy_leds[select_bit_int + z * 8].nr_leds);
					break;
				case 2:
					copy_leds[select_bit_int + z * 8].start_led = constrain(copy_leds[select_bit_int + z * 8].start_led +  osc_miltiply_get(), 0, MAX_NUM_LEDS + copy_leds[select_bit_int + z * 8].nr_leds);
					//copy_leds[select_bit_int + z * 8].start_led +=  osc_miltiply_get();
					break;
				}
			//outbuffer = String("/strips/s" + String(z) + "/AIL/" + String(select_bit_int+1));

			outvalue = float(copy_leds[select_bit_int + z * 8].start_led);
			//osc_send_MSG(outbuffer , float(part[select_bit_int + z *8 ].index_add)) ;   

		}

		if (address[0] == 'N' && address[1] == 'L') {
			if (get_bool(OSC_EDIT) == true)
				switch (select_mode_int) {
				case 0:
					//copy_leds[select_bit_int + z * 8].nr_leds -=  osc_miltiply_get();
					copy_leds[select_bit_int + z * 8].nr_leds = constrain(copy_leds[select_bit_int + z * 8].nr_leds -  osc_miltiply_get(), -MAX_NUM_LEDS + copy_leds[select_bit_int + z * 8].start_led, MAX_NUM_LEDS - copy_leds[select_bit_int + z * 8].start_led);
					break;
				case 2:
					copy_leds[select_bit_int + z * 8].nr_leds = constrain(copy_leds[select_bit_int + z * 8].nr_leds +  osc_miltiply_get(), -MAX_NUM_LEDS + copy_leds[select_bit_int + z * 8].start_led, MAX_NUM_LEDS + copy_leds[select_bit_int + z * 8].start_led);
					//copy_leds[select_bit_int + z * 8].nr_leds +=  osc_miltiply_get();
					break;
				}
			//outbuffer = String("/strips/s" + String(z) + "/AIL/" + String(select_bit_int+1));

			outvalue = float(copy_leds[select_bit_int + z * 8].nr_leds);
			//osc_send_MSG(outbuffer , float(part[select_bit_int + z *8 ].index_add)) ;   

		}

		if (address[0] == 'R' && address[1] == 'L') {
			if (get_bool(OSC_EDIT) == true)
				switch (select_mode_int) {
				case 0:
					//copy_leds[select_bit_int + z * 8].Ref_LED -=  osc_miltiply_get();
					copy_leds[select_bit_int + z * 8].Ref_LED = constrain(copy_leds[select_bit_int + z * 8].Ref_LED -  osc_miltiply_get(), 0, MAX_NUM_LEDS);
					break;
				case 2:
					//copy_leds[select_bit_int + z * 8].Ref_LED +=  osc_miltiply_get();
					copy_leds[select_bit_int + z * 8].Ref_LED = constrain(copy_leds[select_bit_int + z * 8].Ref_LED +  osc_miltiply_get(), 0, MAX_NUM_LEDS);
					break;
				}


			outvalue = float(copy_leds[select_bit_int + z * 8].Ref_LED);
			//osc_send_MSG(outbuffer , float(part[select_bit_int + z *8 ].index_add)) ;   

		}

		{
			outbuffer = String("/copy/c" + String(z) + "/" + String(address) + "L/" + String(select_bit_int + 1));
			osc_queu_MSG_float(outbuffer, outvalue);
		}
	}

}

void osc_copy_toggle_rec(OSCMessage &msg, int addrOffset) {
	// recive OSC message for STRIPS
	// OSC MESSAGE :/copy/c?/   T/1/?

	//DBG_OUTPUT_PORT.println("lol"); 

	//byte addrOffset = 7+6+1 ;
	String select_mode_string;
	String select_bit_string;
	String select_strip_addr;
	char address[6];
	bool switch_bool = false;
	byte z = 0;

	msg.getAddress(address, addrOffset - 3, 1);
	select_strip_addr = select_strip_addr + address[0];
	z = select_strip_addr.toInt();

	msg.getAddress(address, addrOffset + 1);
	//DBG_OUTPUT_PORT.println(address);

	for (byte i = 0; i < sizeof(address); i++) {
		if (address[i] == '/') {
			switch_bool = true;

		}
		else if (switch_bool == false) {
			select_mode_string = select_mode_string + address[i];

		}
		else
			select_bit_string = select_bit_string + address[i];

	}

	int select_bit_int = select_bit_string.toInt() - 1;
	int select_mode_int = select_mode_string.toInt() - 1;



	/*   DBG_OUTPUT_PORT.print("menu :");
	DBG_OUTPUT_PORT.println(select_bit_int);
	DBG_OUTPUT_PORT.print("type :");
	DBG_OUTPUT_PORT.println(select_mode_int);
	DBG_OUTPUT_PORT.println(msg.getFloat(0));
	DBG_OUTPUT_PORT.println(strip_menu[z][select_mode_int], BIN);   //*/



	bitWrite(copy_leds_mode[z], select_bit_int, bool(msg.getFloat(0)));

}

void osc_copy_routing(OSCMessage &msg, int addrOffset) {
	// OSC MESSAGE :/form

	if (msg.fullMatch("/conf_refesh", addrOffset) && bool(msg.getFloat(0)) == true) osc_copy_settings_send();
	msg.route("/c0/T", osc_copy_toggle_rec, addrOffset);
	msg.route("/c1/T", osc_copy_toggle_rec, addrOffset);
	msg.route("/c0/SL", osc_copy_settings_rec, addrOffset);
	msg.route("/c1/SL", osc_copy_settings_rec, addrOffset);
	msg.route("/c0/NL", osc_copy_settings_rec, addrOffset);
	msg.route("/c1/NL", osc_copy_settings_rec, addrOffset);
	msg.route("/c0/RL", osc_copy_settings_rec, addrOffset);
	msg.route("/c1/RL", osc_copy_settings_rec, addrOffset);

	//DBG_OUTPUT_PORT.println("yeah");      
}




// OSC fft   MSG/fft


/* MULTICAST Server !!!* */
/* * */
void osc_fft_rec_toggle_byte(OSCMessage &msg, int addrOffset)
{
	char address[3];
	msg.getAddress(address, addrOffset + 1, 1);
	byte fft_bit = String(address).toInt();

	if (address[0] == '0')
		fft_menu[0] = byte(msg.getInt(0));
	else if (address[0] == '1')
		fft_menu[1] = byte(msg.getInt(0));
	else if (address[0] == '2')
		fft_menu[2] = byte(msg.getInt(0));

}



void  osc_fft_send_info() {
	/*OSCBundle bundle_out;
	String addr = "/fft/binTA/0";
	char   addr_char[20];*/



	//addr = String("/fft/FPSL" );
	//addr.toCharArray(addr_char, 13);
	//bundle_out.add(addr_char).add(float(fft_led_cfg.fps));
	osc_queu_MSG_float(String("/fft/FPSL"), float(fft_led_cfg.fps));
	
	osc_queu_MSG_float(String("/fft/auto"), float(get_bool(FFT_AUTO)));
	osc_queu_MSG_float(String("/fft/ATMIL"), float(fft_led_cfg.fftAutoMin));
	osc_queu_MSG_float(String("/fft/ATMXL"), float(fft_led_cfg.fftAutoMax));

	osc_queu_MSG_float(String("/fft/scale"), float(fft_led_cfg.Scale));
	osc_queu_MSG_float(String("/fft/scaleL"), float(fft_led_cfg.Scale));

	for (uint8_t i = 0; i < 7; i++)
	{
		osc_queu_MSG_float(String("/fft/binTL/" + String(i)), float(fft_data[i].trigger));
		osc_queu_MSG_float(String("/fft/binTA/" + String(i)), float(fft_data[i].avarage));
		osc_queu_MSG_float(String("/fft/fader/" + String(i + 1)), osc_byte_tofloat(fft_data[i].trigger));

		/*addr = String("/fft/binTL/" + String(i) );
		addr.toCharArray(addr_char, 13);
		bundle_out.add(addr_char).add(float(fft_data[i].trigger));

		addr = String("/fft/binTA/" + String(i));
		addr.toCharArray(addr_char, 13);
		bundle_out.add(addr_char).add(float(fft_data[i].avarage));

		addr = String("/fft/fader/" + String(i+1));
		addr.toCharArray(addr_char, 13);
		bundle_out.add(addr_char).add(osc_byte_tofloat(fft_data[i].trigger));*/
	

		for (uint8_t z = 0; z < 3; z++)
		{
			osc_queu_MSG_float(String("/fft/tg/" + String(z) + "/" + String(i + 1) + "/1"), float(bitRead(fft_menu[z], i)));
			/*addr = String("/fft/tg/" + String(z) + "/" + String(i+1) + "/1");
			addr.toCharArray(addr_char, 14);
			bundle_out.add(addr_char).add(float(bitRead(fft_menu[z], i)));*/
			osc_queu_MSG_float(String("/fft/FD/" + String(z) + "/" + String(i + 1) + "/1"), float(bitRead(fft_data_menu[z], i)));
			
		}
		osc_queu_MSG_float(String("/fft/FB/0/" + String(i + 1) + "/1"), float(bitRead(fft_data_bri, i)));
		osc_queu_MSG_float(String("/fft/FT/0/" + String(i + 1) + "/1"), float(bitRead(fft_bin_autoTrigger, i)));
		osc_queu_MSG_float(String("/fft/FS/0/" + String(i + 1) + "/1"), float(bitRead(fft_data_fps, i)));
	

	}




	//osc_server.beginPacket(osc_server.remoteIP(), 9000); //{172,16,222,104}, 8001) ; //
	//bundle_out.send(osc_server);
	//osc_server.endPacket();
	//bundle_out.empty();

}

void  osc_fft_send_avg() {
	/*OSCBundle bundle_out;
	String addr = "/fft/binTA/0";
	char   addr_char[20];*/


	for (uint8_t i = 0; i < 7; i++)
	{
		osc_queu_MSG_float(String("/fft/binTA/" + String(i)), float(fft_data[i].avarage));

		/*addr = String("/fft/binTA/" + String(i));
		addr.toCharArray(addr_char, 13);
		bundle_out.add(addr_char).add(float(fft_data[i].avarage));*/



	}





	//osc_server.beginPacket(osc_server.remoteIP(), 9000); //{172,16,222,104}, 8001) ; //
	//bundle_out.send(osc_server);
	//osc_server.endPacket();
	//bundle_out.empty();

}


void osc_fft_rec_toggle(OSCMessage &msg, int addrOffset)
{
	// OSC MESSAGE :/s/AN/Row/collum  

	String collum_string;
	String row_string;
	char address[4];					// to pick info aut of the msg address
										//char address_out[20];	
	int select_bit = 0;
	String select_bit_string;
	bool switch_bool = false;			// for toggels to get row and collum

	String outbuffer = "/s/ATMXL";		// OSC return address
	float outvalue;						// return value to labels

	String out_add_label;				// address label



	//msg.getAddress(address, addrOffset - 4, 1);					// get the select-bit info	
	//select_bit_string = select_bit_string + address[0];
	//select_bit = select_bit_string.toInt();
	//DBG_OUTPUT_PORT.println(address);

	//memset(address, 0, sizeof(address));				// rest the address to blank


	msg.getAddress(address, addrOffset + 1);		// get the address for row / collum
													//DBG_OUTPUT_PORT.println(address);

	for (byte i = 0; i < sizeof(address); i++)
	{
		if (address[i] == '/') {
			switch_bool = true;
		}
		else if (switch_bool == false)
		{
			row_string = row_string + address[i];
		}
		else
			collum_string = collum_string + address[i];
	}

	int row = row_string.toInt() - 1;
	int collum = collum_string.toInt() - 1;  // 

	memset(address, 0, sizeof(address));
	byte msg_size = msg.size();

	debugMe("row: " + String(row) + " col: " + String(collum));

	switch (collum)
	{
	case 0:		// Max fft ato trigger
		switch (row)
		{
		case 0:
			fft_led_cfg.fftAutoMin = constrain(fft_led_cfg.fftAutoMin -  osc_miltiply_get(), 0, 255);
			break;
		case 2:
			fft_led_cfg.fftAutoMin = constrain(fft_led_cfg.fftAutoMin +  osc_miltiply_get(), 0, 255);
			break;
		}
		outvalue = fft_led_cfg.fftAutoMin;
		out_add_label = "/ATMIL";
		break;

	case 1:		// min fft auto trigger
		switch (row)
		{
		case 0:
			fft_led_cfg.fftAutoMax = constrain(fft_led_cfg.fftAutoMax -  osc_miltiply_get(), 0, 255);
			break;
		case 2:
			fft_led_cfg.fftAutoMax = constrain(fft_led_cfg.fftAutoMax +  osc_miltiply_get(), 0, 255);
			break;
		}
		outvalue = fft_led_cfg.fftAutoMax;
		out_add_label = "/ATMXL";
		break;


	}

	debugMe(fft_led_cfg.fftAutoMin);
	outbuffer = String("/fft" + out_add_label);
	osc_queu_MSG_float(outbuffer, outvalue);

}



void osc_fft_rec_toggleRGB(OSCMessage &msg, int addrOffset)
{
	// OSC MESSAGE :/fft/tg/z/Row/collum  

	String collum_string;
	String row_string;
	char address[14];					// to pick info aut of the msg address
										//char address_out[20];	
	int select_bit = 0;
	String select_bit_string;
	bool switch_bool = false;			// for toggels to get row and collum

	String outbuffer = "/s/ANL";		// OSC return address
	float outvalue;						// return value to labels

	String out_add_label;				// address label

	memset(address, 0, sizeof(address));

	msg.getAddress(address, addrOffset + 1, 1);					// get the select-bit info	
	select_bit_string = select_bit_string + address[0];
	select_bit = select_bit_string.toInt();
	
	debugMe(address);
	memset(address, 0, sizeof(address));				// rest the address to blank


	msg.getAddress(address, addrOffset + 3);		// get the address for row / collum
													//DBG_OUTPUT_PORT.println(address);
	debugMe(address);
	for (byte i = 0; i < sizeof(address); i++)
	{
		if (address[i] == '/') {
			switch_bool = true;
		}
		else if (switch_bool == false)
		{
			row_string = row_string + address[i];
		}
		else
			collum_string = collum_string + address[i];
	}

	int row = row_string.toInt() - 1;
	int collum = collum_string.toInt() - 1;  // Whit CRGB value in the pallete

	memset(address, 0, sizeof(address));
	byte msg_size = msg.size();

	//DBG_OUTPUT_PORT.println("row: " + String(row) + " col: " + String(collum));
	boolean value = msg.getFloat(0);
	//bitWrite(fft_menu[select_bit], collum, bool(msg.getFloat(0));
	bitWrite(fft_menu[select_bit], row, value);

	msg.getAddress(address, 0);






#ifndef OSC_MC_SERVER_DISABLED
	if (address[1] != 'x')
		osc_mc_send(String("/x/fft/tga/" + String(select_bit)), byte(fft_menu[select_bit]));
#endif

	//outbuffer = String("/fft/tg/" + String(select_bit) + "" + String() );
	//osc_send_MSG(outbuffer, outvalue);

}


void osc_fft_rec_toggleData(OSCMessage &msg, int addrOffset)
{
	// OSC MESSAGE :/fft/tg/z/Row/collum  

	String collum_string;
	String row_string;
	char address[14];					// to pick info aut of the msg address
										//char address_out[20];	
	int select_bit = 0;
	String select_bit_string;
	bool switch_bool = false;			// for toggels to get row and collum

	String outbuffer = "/s/ANL";		// OSC return address
	float outvalue;						// return value to labels

	String out_add_label;				// address label

	memset(address, 0, sizeof(address));

	msg.getAddress(address, addrOffset + 1, 1);					// get the select-bit info	
	select_bit_string = select_bit_string + address[0];
	select_bit = select_bit_string.toInt();
	
	debugMe(address);
	memset(address, 0, sizeof(address));				// rest the address to blank


	msg.getAddress(address, addrOffset + 3);		// get the address for row / collum
													//DBG_OUTPUT_PORT.println(address);
	debugMe(address);
	for (byte i = 0; i < sizeof(address); i++)
	{
		if (address[i] == '/') {
			switch_bool = true;
		}
		else if (switch_bool == false)
		{
			row_string = row_string + address[i];
		}
		else
			collum_string = collum_string + address[i];
	}

	int row = row_string.toInt() - 1;
	int collum = collum_string.toInt() - 1;  // Whit CRGB value in the pallete

	memset(address, 0, sizeof(address));
	byte msg_size = msg.size();

	//DBG_OUTPUT_PORT.println("row: " + String(row) + " col: " + String(collum));
	boolean value = msg.getFloat(0);
	//bitWrite(fft_menu[select_bit], collum, bool(msg.getFloat(0));
	bitWrite(fft_data_menu[select_bit], row, value);

	msg.getAddress(address, 0);






//#ifndef OSC_MC_SERVER_DISABLED
//	if (address[1] != 'x')
//		osc_mc_send(String("/x/fft/tga/" + String(select_bit)), byte(fft_menu[select_bit]));
//#endif

	//outbuffer = String("/fft/tg/" + String(select_bit) + "" + String() );
	//osc_send_MSG(outbuffer, outvalue);

}

void osc_fft_rec_toggleBri(OSCMessage &msg, int addrOffset)
{
	// OSC MESSAGE :/fft/tg/z/Row/collum  

	String collum_string;
	String row_string;
	char address[14];					// to pick info aut of the msg address
										//char address_out[20];	
	int select_bit = 0;
	String select_bit_string;
	bool switch_bool = false;			// for toggels to get row and collum

	String outbuffer = "/s/ANL";		// OSC return address
	float outvalue;						// return value to labels

	String out_add_label;				// address label

	memset(address, 0, sizeof(address));

	msg.getAddress(address, addrOffset + 1, 1);					// get the select-bit info	
	select_bit_string = select_bit_string + address[0];
	select_bit = select_bit_string.toInt();
	
	debugMe(address);
	memset(address, 0, sizeof(address));				// rest the address to blank


	msg.getAddress(address, addrOffset + 3);		// get the address for row / collum
													//DBG_OUTPUT_PORT.println(address);
	debugMe(address);
	for (byte i = 0; i < sizeof(address); i++)
	{
		if (address[i] == '/') {
			switch_bool = true;
		}
		else if (switch_bool == false)
		{
			row_string = row_string + address[i];
		}
		else
			collum_string = collum_string + address[i];
	}

	int row = row_string.toInt() - 1;
	int collum = collum_string.toInt() - 1;  // Whit CRGB value in the pallete

	memset(address, 0, sizeof(address));
	byte msg_size = msg.size();

	//DBG_OUTPUT_PORT.println("row: " + String(row) + " col: " + String(collum));
	boolean value = msg.getFloat(0);
	//bitWrite(fft_menu[select_bit], collum, bool(msg.getFloat(0));
	bitWrite(fft_data_bri, row, value);

	msg.getAddress(address, 0);


//#ifndef OSC_MC_SERVER_DISABLED
//	if (address[1] != 'x')
//		osc_mc_send(String("/x/fft/tga/" + String(select_bit)), byte(fft_menu[select_bit]));
//#endif

	//outbuffer = String("/fft/tg/" + String(select_bit) + "" + String() );
	//osc_send_MSG(outbuffer, outvalue);

}

void osc_fft_rec_toggleAutoTrigger(OSCMessage &msg, int addrOffset)
{
	// OSC MESSAGE :/fft/tg/z/Row/collum  

	String collum_string;
	String row_string;
	char address[14];					// to pick info aut of the msg address
										//char address_out[20];	
	int select_bit = 0;
	String select_bit_string;
	bool switch_bool = false;			// for toggels to get row and collum

	String outbuffer = "/s/ANL";		// OSC return address
	float outvalue;						// return value to labels

	String out_add_label;				// address label

	memset(address, 0, sizeof(address));

	msg.getAddress(address, addrOffset + 1, 1);					// get the select-bit info	
	select_bit_string = select_bit_string + address[0];
	select_bit = select_bit_string.toInt();
	
	debugMe(address);
	memset(address, 0, sizeof(address));				// rest the address to blank


	msg.getAddress(address, addrOffset + 3);		// get the address for row / collum
													//DBG_OUTPUT_PORT.println(address);
	debugMe(address);
	for (byte i = 0; i < sizeof(address); i++)
	{
		if (address[i] == '/') {
			switch_bool = true;
		}
		else if (switch_bool == false)
		{
			row_string = row_string + address[i];
		}
		else
			collum_string = collum_string + address[i];
	}

	int row = row_string.toInt() - 1;
	int collum = collum_string.toInt() - 1;  // Whit CRGB value in the pallete

	memset(address, 0, sizeof(address));
	byte msg_size = msg.size();

	//DBG_OUTPUT_PORT.println("row: " + String(row) + " col: " + String(collum));
	boolean value = msg.getFloat(0);
	//bitWrite(fft_menu[select_bit], collum, bool(msg.getFloat(0));
	bitWrite(fft_bin_autoTrigger, row, value);

	msg.getAddress(address, 0);


//#ifndef OSC_MC_SERVER_DISABLED
//	if (address[1] != 'x')
//		osc_mc_send(String("/x/fft/tga/" + String(select_bit)), byte(fft_menu[select_bit]));
//#endif

	//outbuffer = String("/fft/tg/" + String(select_bit) + "" + String() );
	//osc_send_MSG(outbuffer, outvalue);

}

void osc_fft_rec_toggleFPS(OSCMessage &msg, int addrOffset)
{
	// OSC MESSAGE :/fft/tg/z/Row/collum  

	String collum_string;
	String row_string;
	char address[14];					// to pick info aut of the msg address
										//char address_out[20];	
	int select_bit = 0;
	String select_bit_string;
	bool switch_bool = false;			// for toggels to get row and collum

	//String outbuffer = "/s/ANL";		// OSC return address
	//float outvalue;						// return value to labels

	//String out_add_label;				// address label

	memset(address, 0, sizeof(address));

	msg.getAddress(address, addrOffset + 1, 1);					// get the select-bit info	
	select_bit_string = select_bit_string + address[0];
	select_bit = select_bit_string.toInt();
	
	debugMe(address);
	memset(address, 0, sizeof(address));				// rest the address to blank


	msg.getAddress(address, addrOffset + 3);		// get the address for row / collum
													//DBG_OUTPUT_PORT.println(address);
	debugMe(address);
	for (byte i = 0; i < sizeof(address); i++)
	{
		if (address[i] == '/') {
			switch_bool = true;
		}
		else if (switch_bool == false)
		{
			row_string = row_string + address[i];
		}
		else
			collum_string = collum_string + address[i];
	}

	int row = row_string.toInt() - 1;
	int collum = collum_string.toInt() - 1;  // Whit CRGB value in the pallete

	memset(address, 0, sizeof(address));
	byte msg_size = msg.size();

	//DBG_OUTPUT_PORT.println("row: " + String(row) + " col: " + String(collum));
	boolean value = msg.getFloat(0);
	//bitWrite(fft_menu[select_bit], collum, bool(msg.getFloat(0));
	bitWrite(fft_data_fps, row, value);

	msg.getAddress(address, 0);


//#ifndef OSC_MC_SERVER_DISABLED
//	if (address[1] != 'x')
//		osc_mc_send(String("/x/fft/tga/" + String(select_bit)), byte(fft_menu[select_bit]));
//#endif

	//outbuffer = String("/fft/tg/" + String(select_bit) + "" + String() );
	//osc_send_MSG(outbuffer, outvalue);

}


void osc_fft_rec_fader(OSCMessage &msg, int addrOffset) {
	// OSC MESSAGE :/fft/fader/1-7      /pal/?/?/1-3   


	String fader_no_string;
	//String select_bit_string;
	char address[4];
	char address_out[20];
	//String outbuffer = "/pal/0/x/1-3";
	float outvalue;
	char   addr_char[20];
	String addr;

	memset(address, 0, sizeof(address));
	memset(address_out, 0, sizeof(address_out));

	msg.getAddress(address_out, 0);
	//DBG_OUTPUT_PORT.println("rec_fader:");
	//DBG_OUTPUT_PORT.println(address_out);


	msg.getAddress(address, addrOffset + 1);
	//DBG_OUTPUT_PORT.println(address);
	fader_no_string = fader_no_string + address[0];



	int fader_no = fader_no_string.toInt() - 1;
	memset(address, 0, sizeof(address));

	if (!bitRead(fft_bin_autoTrigger, fader_no))							// only update if auto is off
		fft_data[fader_no].trigger = constrain(byte(msg.getFloat(0) * 255), 0 , 255);


	if (address_out[1] != 'x')				// dont send out reply if incoimming from master 
	{
		outvalue = float(fft_data[fader_no].trigger);
		addr = String("/fft/binTL/" + String(fader_no));
		addr.toCharArray(addr_char, 13);
		osc_queu_MSG_float(addr, outvalue);

#ifndef OSC_MC_SERVER_DISABLED
		osc_mc_send(String("/x/fft/fader/" + String(fader_no + 1)), outvalue / 255);
#endif		
	}
	
}


void osc_fft_save(OSCMessage &msg, int addrOffset)
{
	// MSG /fft/save/x

	char address[4];					// to pick info aut of the msg address
										//char address_out[20];	
	int select_bit = 0;
	String select_bit_string;

	msg.getAddress(address, addrOffset + 1, 1);					// get the select-bit info	
	select_bit_string = select_bit_string + address[0];
	select_bit = select_bit_string.toInt();
	//DBG_OUTPUT_PORT.println(address);

	memset(address, 0, sizeof(address));				// rest the address to blank

	FS_FFT_write(select_bit);
	osc_send_MSG_String("/fft/INFO", String("Saved FFT :  " + String(select_bit)));

}

void osc_fft_load(OSCMessage &msg, int addrOffset)
{
	// MSG /fft/save/x

	char address[4];					// to pick info aut of the msg address
										//char address_out[20];	
	int select_bit = 0;
	String select_bit_string;


	msg.getAddress(address, addrOffset + 1, 1);					// get the select-bit info	
	select_bit_string = select_bit_string + address[0];
	select_bit = select_bit_string.toInt();
	//DBG_OUTPUT_PORT.println(address);

	memset(address, 0, sizeof(address));				// rest the address to blank

	if (FS_FFT_read(select_bit) == true)
		osc_send_MSG_String("/fft/INFO", String("Loaded FFT :  " + String(select_bit)));
	else
		osc_send_MSG_String("/fft/INFO", String("failed Load FFT :  " + String(select_bit)));
}

void osc_fft_routing(OSCMessage &msg, int addrOffset) {
	// OSC MESSAGE :/fft

	
	if (msg.fullMatch("/conf_refesh", addrOffset) && bool(msg.getFloat(0)) == true)		 osc_fft_send_info();
	if (msg.fullMatch("/avg_refesh", addrOffset) && bool(msg.getFloat(0)) == true)		osc_fft_send_avg();

	if (msg.fullMatch("/FPS", addrOffset))			osc_queu_MSG_float("/fft/FPSL", float(fft_led_cfg.fps));
	if (msg.fullMatch("/auto", addrOffset))			write_bool(FFT_AUTO,bool(msg.getFloat(0)));
	if (msg.fullMatch("/scale", addrOffset))		{ fft_led_cfg.Scale = constrain(msg.getFloat(0), 0, 500); osc_queu_MSG_float("/fft/scaleL", float(fft_led_cfg.Scale)); }

	msg.route("/fader", osc_fft_rec_fader, addrOffset);
	
	msg.route("/tg", osc_fft_rec_toggleRGB, addrOffset);
	
	msg.route("/FD", osc_fft_rec_toggleData, addrOffset);

	msg.route("/FB", osc_fft_rec_toggleBri, addrOffset);

	msg.route("/FT", osc_fft_rec_toggleAutoTrigger, addrOffset);

	msg.route("/FS", osc_fft_rec_toggleFPS, addrOffset);

	if (bool(msg.getFloat(0)) == true)
	{

		msg.route("/AT", osc_fft_rec_toggle, addrOffset);
		msg.route("/save", osc_fft_save, addrOffset);
		msg.route("/load", osc_fft_load, addrOffset);
	}
	//DBG_OUTPUT_PORT.println("yeah");      
}





// MASTER    OSC: /m


void osc_master_conf_play_save(OSCMessage &msg, int addrOffset) {
	// OSC MESSAGE :/m/save/row/coll=1
	if (bool(msg.getFloat(0)) == true) 
	{
		String select_mode_string;
		// String select_bit_string;
		char address[6];
		bool switch_bool = false;

		msg.getAddress(address, addrOffset + 1);
		for (byte i = 0; i < sizeof(address); i++) {
			if (address[i] == '/') {
				switch_bool = true;

			}
			else if (switch_bool == false) {
				select_mode_string = select_mode_string + address[i];

			}

		}

		int select_mode_int = select_mode_string.toInt() - 1;
		FS_play_conf_write(select_mode_int);

		msg.getAddress(address, 0,3);
		if (address[1] != 'x')
		{
#ifndef OSC_MC_SERVER_DISABLED
			//if (get_bool(OSC_MC_SEND) == true)
			osc_mc_send(String("/x/m/save/" + select_mode_string + "/1"), float(1));
			
#endif
			osc_send_MSG_String("/m/INFO", String("Saved :  " + String(select_mode_int)));
		}
	}
}

void osc_master_settings_send()
{
	//float test = float(led_cfg.bri) / float(led_cfg.max_bri);
	// debugMe("ohoho : ");
	//debugMe(String(test));
	//debugMe(led_cfg.bri);
	//debugMe(led_cfg.max_bri);

	osc_queu_MSG_float("/m/bri", byte_tofloat(led_cfg.bri, 255)) ; //float(led_cfg.bri) / float(led_cfg.max_bri) );
	osc_queu_MSG_float("/m/r", byte_tofloat(led_cfg.r,255));
	osc_queu_MSG_float("/m/g", byte_tofloat(led_cfg.g,255));
	osc_queu_MSG_float("/m/b", byte_tofloat(led_cfg.b,255));
	osc_queu_MSG_float("/m/palbri", byte_tofloat(led_cfg.pal_bri, 255));
	osc_queu_MSG_float("/m/ups", byte_tofloat(led_cfg.pal_fps, MAX_PAL_FPS));
	osc_queu_MSG_float("/m/fftups", byte_tofloat(fft_led_cfg.fps, MAX_PAL_FPS));

	osc_queu_MSG_float("/m/rl", float(led_cfg.r));
	osc_queu_MSG_float("/m/gl", float(led_cfg.g));
	osc_queu_MSG_float("/m/bl", float(led_cfg.b));
	osc_queu_MSG_float("/m/bril", float(led_cfg.bri));
	osc_queu_MSG_float("/m/palbril",float(led_cfg.pal_bri));
	osc_queu_MSG_float("/m/upsl", float(led_cfg.pal_fps));
	osc_queu_MSG_float("/m/fftupsl", float(fft_led_cfg.fps));

	osc_queu_MSG_float("/m/fire/coolL", float(led_cfg.fire_cooling));
	osc_queu_MSG_float("/m/fire/sparkL", float(led_cfg.fire_sparking));

	osc_queu_MSG_float("/m/blend", float(get_bool(BLEND_INVERT)));
	yield();
}

void osc_master_basic_reply(String address , uint8_t value)
{
		 // send back label info
#ifndef OSC_MC_SERVER_DISABLED
	//if (get_bool(OSC_MC_SEND) == true)
	osc_mc_send(String("/x" + address), value );
	
#endif
	osc_queu_MSG_float(String(address + "l"), value);
}

void osc_master_bpm_manual(OSCMessage &msg, int addrOffset) 
{
	if (bool(msg.getFloat(0)) == true)			// only on pushdown
	{
		unsigned long curT = millis();

		if (led_cfg.bpm_lastT == 1)			led_cfg.bpm_lastT = curT;
		else if (led_cfg.bpm_diff == 1)		led_cfg.bpm_diff = curT - led_cfg.bpm_lastT;
		else
		{
			unsigned long diff = curT - led_cfg.bpm_lastT;

			if (diff > BMP_MAX_TIME) 	diff = led_cfg.bpm_diff;
			else						led_cfg.bpm_diff = (led_cfg.bpm_diff + diff) / 2;
			
			led_cfg.bpm = 60000 / led_cfg.bpm_diff;
			//DBG_OUTPUT_PORT.println(bpm_counter.bpm);


			led_cfg.bpm_lastT = curT;
			osc_send_MSG_String("/m/bpm/bpml", String(led_cfg.bpm));

			if (get_bool(BPM_COUNTER) == true)
			{
				led_cfg.pal_fps = (led_cfg.bpm * 16) / 60;
				osc_send_MSG_String("/m/upsl", String(led_cfg.pal_fps));
				//DBG_OUTPUT_PORT.println("ups?");
				//DBG_OUTPUT_PORT.println(pal_updates_second);
			}

		}
	} // bool only on pushdown
}

void osc_master_bpm_toggle(OSCMessage &msg, int addrOffset) 
{
	// OSC MESSAGE :/m/bpm/toggle/x/x
	if (bool(msg.getFloat(0)) == true) 
	{
		String select_row_string;			// old mode
		// String select_bit_string;
		char address[6];
		bool switch_bool = false;

		msg.getAddress(address, addrOffset + 1);
		for (byte i = 0; i < sizeof(address); i++) 
		{
			if (address[i] == '/')  switch_bool = true;
			else if (switch_bool == false) 	select_row_string = select_row_string + address[i];
		}
		//int select_bit_int = select_bit_string.toInt()-1;
		int select_row_int = select_row_string.toInt() - 1;
		//DBG_OUTPUT_PORT.print("select_mode ") ;
		//DBG_OUTPUT_PORT.println(select_mode_int) ;

		switch (select_row_int)
		{
		case 0:
			led_cfg.bpm = constrain((led_cfg.bpm -  osc_miltiply_get()), 1 , 255)   ;
			break;
		case 1:
			osc_queu_MSG_float("/m/bpm/enabled", float(get_bool(BPM_COUNTER)));

		break;
		case 2:
			led_cfg.bpm = constrain((led_cfg.bpm +  osc_miltiply_get()),1,255) ;
			break;
		default:
			break;
		}



		osc_send_MSG_String("/m/bpm/bpml", String(led_cfg.bpm));

			
		if (get_bool(BPM_COUNTER) == true)
		{
			led_cfg.pal_fps = (led_cfg.bpm * 16 )/60 ;
			osc_send_MSG_String("/m/upsl", String(led_cfg.pal_fps));
			//DBG_OUTPUT_PORT.println("ups?");
			//DBG_OUTPUT_PORT.println(pal_updates_second);
		}


	}
}


void osc_master_conf_play_load(OSCMessage &msg, int addrOffset) {
	// OSC MESSAGE :/m/load
	if (bool(msg.getFloat(0)) == true)
	{
		String select_mode_string;
		// String select_bit_string;
		char address[15];
		bool switch_bool = false;

		msg.getAddress(address, addrOffset + 1);
		for (byte i = 0; i < sizeof(address); i++) {
			if (address[i] == '/') {
				switch_bool = true;

			}
			else if (switch_bool == false) {
				select_mode_string = select_mode_string + address[i];

			}
		}
		int select_mode_int = select_mode_string.toInt() - 1;

		FS_play_conf_read(select_mode_int);
		LEDS_pal_reset_index();

		msg.getAddress(address, 0, 3);
		if (address[1] != 'x')
		{
#ifndef OSC_MC_SERVER_DISABLED
			//if (get_bool(OSC_MC_SEND) == true)
			osc_mc_send(String("/x/m/load/" + select_mode_string + "/1"), float(1));

#endif
			osc_send_MSG_String("/m/INFO", String("Loaded :  " + String(select_mode_int)));
		}
		osc_master_settings_send();
	}

}


void osc_master_routing(OSCMessage &msg, int addrOffset)
{

		msg.route("/load", osc_master_conf_play_load, addrOffset);
		msg.route("/save", osc_master_conf_play_save, addrOffset);

		if (msg.fullMatch("/ref", addrOffset) && bool(msg.getFloat(0)) == true)		osc_master_settings_send();

		if (msg.fullMatch("/bri",addrOffset))			{ led_cfg.bri	= byte(msg.getFloat(0)	* 255); osc_master_basic_reply("/m/bri",led_cfg.bri);}// osc_mc_send("/x/bri",		led_brightness);}
        if (msg.fullMatch("/r",addrOffset))				{ led_cfg.r		= byte(msg.getFloat(0)	* 255); osc_master_basic_reply("/m/r", led_cfg.r);}// osc_mc_send("/x/r",		master_rgb.r);}
        if (msg.fullMatch("/g",addrOffset))				{ led_cfg.g		= byte(msg.getFloat(0)	* 255); osc_master_basic_reply("/m/g", led_cfg.g);}// osc_mc_send("/x/g",		master_rgb.g);}
        if (msg.fullMatch("/b",addrOffset))				{ led_cfg.b		= byte(msg.getFloat(0)	* 255); osc_master_basic_reply("/m/b", led_cfg.b);}// osc_mc_send("/x/b" ,		master_rgb.b) ; }
		if (msg.fullMatch("/palbri", addrOffset))		{ led_cfg.pal_bri = byte(msg.getFloat(0) * 255); osc_master_basic_reply("/m/palbri", led_cfg.pal_bri); }// osc_mc_send("/x/b" ,		master_rgb.b) ; }

		if (msg.fullMatch("/blend", addrOffset))		{ write_bool(BLEND_INVERT, bool(msg.getFloat(0))); osc_queu_MSG_float("/m/blend", float(get_bool(BLEND_INVERT))    ); }  

		if (msg.fullMatch("/ups", addrOffset))		{ led_cfg.pal_fps = constrain(byte(msg.getFloat(0) * MAX_PAL_FPS), 1, MAX_PAL_FPS); osc_master_basic_reply("/m/ups", led_cfg.pal_fps); osc_queu_MSG_float("/m/FPSL", float(LEDS_get_FPS()));}
		//if (msg.fullMatch("/fftups", addrOffset)) { fft_led_cfg.fps = constrain(byte(msg.getFloat(0) * MAX_PAL_FPS), 1, MAX_PAL_FPS); osc_queu_MSG_float("/m/fftupsl", float(fft_led_cfg.fps)); yield();  comms_S_FPS(fft_led_cfg.fps); }
		if (msg.fullMatch("/FPS", addrOffset))			osc_queu_MSG_float("/m/FPSL", float(LEDS_get_FPS()));

		if (msg.fullMatch("/fire/cool", addrOffset)) { led_cfg.fire_cooling = constrain(byte(msg.getFloat(0)), 20, 100); osc_queu_MSG_float("/m/fire/coolL", float(led_cfg.fire_cooling)); }
		if (msg.fullMatch("/fire/spark", addrOffset)) { led_cfg.fire_sparking = constrain(byte(msg.getFloat(0)), 20, 100); osc_queu_MSG_float("/m/fire/sparkL", float(led_cfg.fire_sparking)); }
		

		if (msg.fullMatch("/bpm/enabled", addrOffset))	write_bool(BPM_COUNTER, bool(msg.getFloat(0)));
		msg.route("/bpm/toggle",						osc_master_bpm_toggle, addrOffset);
		msg.route("/bpm/man",							osc_master_bpm_manual, addrOffset);

}


void osc_send_pal_info(uint8_t pal) {
	// OSC MESSAGE OUT :/pal/?/?/1-3


	byte outvalue = 0;


	for (int i = 0; i < 16; i++) {
		for (uint8_t fader_no = 0; fader_no < 3; fader_no++)

			switch (fader_no) {

			case 0:
				osc_queu_MSG_float(String("/pal/" + String(pal) + "/" + String(i) + "/RL"), float(LEDS_pal_read(pal,i,0)));
				osc_queu_MSG_float(String("/pal/" + String(pal) + "/" + String(i) + "/1"), osc_byte_tofloat(LEDS_pal_read(pal, i, 0)));


				break;

			case 1:
				osc_queu_MSG_float(String("/pal/" + String(pal) + "/" + String(i) + "/GL"), float(LEDS_pal_read(pal, i, 1)));
				osc_queu_MSG_float(String("/pal/" + String(pal) + "/" + String(i) + "/2"), osc_byte_tofloat(LEDS_pal_read(pal, i, 1)));
				break;

			case 2:
				osc_queu_MSG_float(String("/pal/" + String(pal) + "/" + String(i) + "/BL"), float(LEDS_pal_read(pal, i, 2)));
				osc_queu_MSG_float(String("/pal/" + String(pal) + "/" + String(i) + "/3"), osc_byte_tofloat(LEDS_pal_read(pal, i, 2)));
				break;


			}

	}

	//}

}





void osc_rec_pal_fader(OSCMessage &msg, int addrOffset) {
	// OSC MESSAGE :/pal/?/?/1-3   

	String pallete_option_string;		// Fader NR
	String pal_no_string;		// Pallete NO
	String fader_no_string;
	//String select_bit_string;
	char address[5];
	//char address_out[20];
	bool switch_bool = false;
	uint8_t pal_no = 0;						// form NR in uint8_t
	String outbuffer = "/pal/0/x/1-3";
	float outvalue;

	msg.getAddress(address, addrOffset - 1, 1);
	pal_no_string = pal_no_string + address[0];
	pal_no = pal_no_string.toInt();
	//DBG_OUTPUT_PORT.println(address);
	//DBG_OUTPUT_PORT.println(pal_no);

	memset(address, 0, sizeof(address));

	msg.getAddress(address, addrOffset + 1);
	//DBG_OUTPUT_PORT.println(address);

	for (byte i = 0; i < sizeof(address); i++) 
	{
		if (address[i] == '/') 
		{
			switch_bool = true;

		}
		else if (switch_bool == false) 
		{
			//select_mode_string = select_mode_string + address[i];
			pallete_option_string = pallete_option_string + address[i];
		}
		else
			fader_no_string = fader_no_string + address[i];



	}

	int fader_no = fader_no_string.toInt() - 1;
	int pallete_option = pallete_option_string.toInt();  // Whit CRGB value in the pallete

	memset(address, 0, sizeof(address));

	byte msg_size = msg.size();
	String out_add_label;

	switch (fader_no) 
	{
			case 0:
				LEDS_pal_write(pal_no, pallete_option, 0, byte(msg.getFloat(0) * 255));
				//LEDS_pal_cur[pal_no][pallete_option].r = byte(msg.getFloat(0) * 255);
				outvalue = LEDS_pal_read(pal_no, pallete_option, 0 ); // [pal_no][pallete_option].r;
				out_add_label = "/RL";
				//DBG_OUTPUT_PORT.println("RED");
				break;

			case 1:
				LEDS_pal_write(pal_no, pallete_option, 1, byte(msg.getFloat(0) * 255));
				outvalue = LEDS_pal_read(pal_no, pallete_option, 1 );
				//LEDS_pal_cur[pal_no][pallete_option].g = byte(msg.getFloat(0) * 255);
				//outvalue = LEDS_pal_cur[pal_no][pallete_option].g;
				out_add_label = "/GL";
				break;

			case 2:
				LEDS_pal_write(pal_no, pallete_option, 2, byte(msg.getFloat(0) * 255));
				outvalue = LEDS_pal_read(pal_no, pallete_option, 2 );
				//LEDS_pal_cur[pal_no][pallete_option].b = byte(msg.getFloat(0) * 255);
				//outvalue = LEDS_pal_cur[pal_no][pallete_option].b;
				out_add_label = "/BL";
				break;
	}

	{
		outbuffer = String("/pal/" + String(pal_no) + "/" + String(pallete_option) + out_add_label); //+ String(select_mode_int + 1));
																									 //DBG_OUTPUT_PORT.println(outbuffer);
		osc_queu_MSG_float(outbuffer, outvalue);
	}

}

void osc_rec_pal_load(OSCMessage &msg, int addrOffset) {
	// OSC MESSAGE :/pal/load/0/x/y
	if (bool(msg.getFloat(0)) == true)
	{
		String select_mode_string;
		String select_pal_string;
		// String select_bit_string;
		char address[15];
		bool switch_bool = false;


		msg.getAddress(address, addrOffset + 1, 1);

		select_pal_string = select_pal_string + address[0];

		int select_pal_int = select_pal_string.toInt();   // convert the sting to an int to get the pallete Nr

		memset(address, 0, sizeof(address));

		msg.getAddress(address, addrOffset + 3);
		for (byte i = 0; i < sizeof(address); i++) {
			if (address[i] == '/') {
				switch_bool = true;

			}
			else if (switch_bool == false) {
				select_mode_string = select_mode_string + address[i];

			}
		}
		int select_mode_int = select_mode_string.toInt() ;


		
		LEDS_pal_load(select_pal_int, select_mode_int);  // send the command to leds to load the pallete


		//LEDS_pal_reset_index();

		msg.getAddress(address, 0, 3);
		if (address[1] != 'x')
		{
			osc_send_MSG_String("/m/INFO", String("Loaded pal :  " + String(select_mode_int)));
		}
		

		osc_send_pal_info(select_pal_int); // send out a refresh of the display
 	}

}


void osc_pal_routing(OSCMessage &msg, int addrOffset) {
	// OSC MESSAGE :/form

	debugMe("pal1");
	if (msg.fullMatch("/ref/0", addrOffset) && bool(msg.getFloat(0)) == true) osc_send_pal_info(0);
	if (msg.fullMatch("/ref/1", addrOffset) && bool(msg.getFloat(0)) == true) osc_send_pal_info(1);
	debugMe("pal2");
	msg.route("/0", osc_rec_pal_fader, addrOffset);
	msg.route("/1", osc_rec_pal_fader, addrOffset);
	debugMe("pal3");
	
	if (msg.fullMatch("/inv/0", addrOffset) && bool(msg.getFloat(0)) == true) {LEDS_PAL_invert(0) ; osc_send_pal_info(0); }
	if (msg.fullMatch("/inv/1", addrOffset) && bool(msg.getFloat(0)) == true) {LEDS_PAL_invert(1) ; osc_send_pal_info(1); }

	msg.route("/load", osc_rec_pal_load, addrOffset);
	debugMe("pal4");
	//DBG_OUTPUT_PORT.println("yeah");      
}



// Device Settings OSC:/DS
void osc_DS_refresh()
{
		
			for (uint8_t i = 0; i < 4; i++)
			{
				// IP Config
				osc_queu_MSG_float(String("/DS/SIPL/" + String(i)), float(wifi_cfg.ipStaticLocal[i]));
				osc_queu_MSG_float(String("/DS/SNML/" + String(i)), float(wifi_cfg.ipSubnet[i]));
				osc_queu_MSG_float(String("/DS/DGWL/" + String(i)), float(wifi_cfg.ipDGW[i]));
				osc_queu_MSG_float(String("/DS/DNSL/" + String(i)), float(wifi_cfg.ipDNS[i]));
			}

			osc_send_out_float_MSG_buffer();  // send out some of it and yield
			yield();
			osc_queu_MSG_float(String("/DS/ESIP"), float(get_bool(STATIC_IP_ENABLED)));
			osc_queu_MSG_float(String("/DS/httpd"), float(get_bool(HTTP_ENABLED)));
			osc_queu_MSG_float(String("/DS/debug"), float(get_bool(DEBUG_OUT)));
			osc_queu_MSG_float(String("/DS/TNdebug"), float(get_bool(DEBUG_TELNET)));


			osc_queu_MSG_float(String("/DS/WP"), float(get_bool(WIFI_POWER))); 
			osc_queu_MSG_float(String("/DS/WAP"), float(get_bool(WIFI_MODE)));  //debugMe(get_bool(WIFI_MODE));
			osc_send_MSG_String("/DS/SSID", String(wifi_cfg.ssid));
			osc_send_MSG_String("/DS/WPW", String(wifi_cfg.pwd));
			osc_send_MSG_String("/DS/WAPNL", String(wifi_cfg.APname));

			// Artnet
			osc_queu_MSG_float("/DS/ASUL", float(artnet_cfg.startU));
			osc_queu_MSG_float("/DS/ANUL", float(artnet_cfg.numU));
			osc_queu_MSG_float("/DS/ANE", float(get_bool(ARTNET_ENABLE)));

			osc_queu_MSG_float("/DS/NL/0", float(led_cfg.NrLeds));
			osc_queu_MSG_float("/DS/NL/1", float(led_cfg.Data1NrLeds));
			osc_queu_MSG_float("/DS/NL/2", float(led_cfg.Data2NrLeds));
			osc_queu_MSG_float("/DS/NL/3", float(led_cfg.Data3NrLeds));
			osc_queu_MSG_float("/DS/NL/4", float(led_cfg.Data4NrLeds));

			osc_queu_MSG_float("/DS/SL/1", float(led_cfg.Data1StartLed));
			osc_queu_MSG_float("/DS/SL/2", float(led_cfg.Data2StartLed));
			osc_queu_MSG_float("/DS/SL/3", float(led_cfg.Data3StartLed));
			osc_queu_MSG_float("/DS/SL/4", float(led_cfg.Data4StartLed));

			//FFT
			osc_queu_MSG_float("/DS/fft_master", float(get_bool(FFT_MASTER)));
			osc_queu_MSG_float("/DS/fft_enable", float(get_bool(FFT_ENABLE)));
			osc_queu_MSG_float(String("/fft/auto"), float(get_bool(FFT_AUTO)));

			// General
			osc_queu_MSG_float("/DS/EDIT", float(get_bool(OSC_EDIT)));
			osc_queu_MSG_float("/DS/OSC_M", float(get_bool(OSC_MC_SEND)));
			osc_queu_MSG_float("/DS/Mbril", float(led_cfg.max_bri));
			osc_queu_MSG_float("/DS/STARTbril", float(led_cfg.startup_bri));
			
			osc_send_out_float_MSG_buffer();   // send out some of it and yield
			yield();

			osc_queu_MSG_float("/DS/ledType/1/1", 0);
			osc_queu_MSG_float("/DS/ledType/2/1", 0);
			osc_queu_MSG_float("/DS/ledType/3/1", 0);
			osc_queu_MSG_float("/DS/ledType/4/1", 0);
			osc_queu_MSG_float("/DS/ledType/5/1", 0);
			osc_queu_MSG_float("/DS/ledType/6/1", 0);

			switch (led_cfg.ledMode)
			{
			case 0:
				osc_queu_MSG_float("/DS/ledType/1/1", 1);
				break;
			case 1:
				osc_queu_MSG_float("/DS/ledType/2/1", 1);
				break;
			case 2:
				osc_queu_MSG_float("/DS/ledType/3/1", 1);
				break;
			case 3:;
				osc_queu_MSG_float("/DS/ledType/4/1", 1);
			break;	
			case 4:
				osc_queu_MSG_float("/DS/ledType/5/1", 1);
			break;
			case 5:
				osc_queu_MSG_float("/DS/ledType/6/1", 1);
			break;
			
			}

			osc_queu_MSG_float("/DS/data/1/1", get_bool(DATA1_ENABLE));
			osc_queu_MSG_float("/DS/data/2/1", get_bool(DATA2_ENABLE));
			osc_queu_MSG_float("/DS/data/3/1", get_bool(DATA3_ENABLE));
			osc_queu_MSG_float("/DS/data/4/1", get_bool(DATA4_ENABLE));


		osc_multiply_send();

		//debugMe(wifi_cfg.ipStaticLocal[i]);

	 
}

void osc_DS_max_bri(OSCMessage &msg, int addrOffset) 
{
	// OSC input MAX bri Multipush 1 collum 3 row's
	// 

	// OSC MESSAGE :/m/multipl/?/1
	if (bool(msg.getFloat(0)) == true) {
		String select_mode_string;
		// String select_bit_string;
		char address[10];
		bool switch_bool = false;

		msg.getAddress(address, addrOffset + 1);
		for (byte i = 0; i < sizeof(address); i++) 
		{
			if (address[i] == '/') 
			{
				switch_bool = true;

			}
			else if (switch_bool == false) 
			{
				select_mode_string = select_mode_string + address[i];

			}

		}

		int select_mode_int = select_mode_string.toInt();


		if (bool(msg.getFloat(0)) == true) 
		{
			switch (select_mode_int) 
			{
			case 1:
				led_cfg.max_bri	= led_cfg.max_bri -  osc_miltiply_get();

				break;
			case 3:
				led_cfg.max_bri = led_cfg.max_bri +  osc_miltiply_get();			
				break;

			}
			osc_queu_MSG_float("/DS/Mbril", led_cfg.max_bri);
		} // end switch

		  //outbuffer = String("/multipl/1/1");



	} // end  new msg

}

void osc_DS_start_bri(OSCMessage &msg, int addrOffset)
{
	// OSC input MAX bri Multipush 1 collum 3 row's
	// 

	// OSC MESSAGE :/m/multipl/?/1
	if (bool(msg.getFloat(0)) == true) {
		String select_mode_string;
		// String select_bit_string;
		char address[10];
		bool switch_bool = false;

		msg.getAddress(address, addrOffset + 1);
		for (byte i = 0; i < sizeof(address); i++)
		{
			if (address[i] == '/')
			{
				switch_bool = true;

			}
			else if (switch_bool == false)
			{
				select_mode_string = select_mode_string + address[i];

			}

		}

		int select_mode_int = select_mode_string.toInt();


		if (bool(msg.getFloat(0)) == true)
		{
			switch (select_mode_int)
			{
			case 1:
				led_cfg.startup_bri = led_cfg.startup_bri -  osc_miltiply_get();

				break;
			case 3:
				led_cfg.startup_bri = led_cfg.startup_bri +  osc_miltiply_get();
				break;

			}
			osc_queu_MSG_float("/DS/STARTbril", led_cfg.startup_bri);
		} // end switch

		  //outbuffer = String("/multipl/1/1");



	} // end  new msg

}



void osc_DS_ip_in(OSCMessage &msg, int addrOffset)
{	// OSC IN Device settings IP
	// OSC MESSAGE :/DS/?type?:/row/collum   xx/?/?
		// OSC MESSAGE :/DS/???:/bit/row/collum   xx/?/?
		// :/bit/+-option/strip

		//DBG_OUTPUT_PORT.println("lol"); 

		//byte addrOffset = 7+6+1 ;
		String type_string;
		String option_string;
		String byte_string;

		int type_int;
		int option_int;
		int byte_int;


		char address[4];
		char address_out[20];
		bool switch_bool = false;

		String outbuffer = "/strips/sX/SIL/X";
		float outvalue = 255;

		msg.getAddress(address, addrOffset -3 , 3);
		type_string = type_string + address[0];


		//debugMe(address);

		memset(address, 0, sizeof(address));

		msg.getAddress(address, addrOffset + 1);
		//DBG_OUTPUT_PORT.println(address);

		//debugMe(address);

		for (byte i = 0; i < sizeof(address); i++) {
			if (address[i] == '/') {
				switch_bool = true;

			}
			else if (switch_bool == false) {
				option_string = option_string + address[i];

			}
			else
				byte_string = byte_string + address[i];

		}

		type_int = type_string.toInt();
		option_int = option_string.toInt() - 1;
		byte_int = byte_string.toInt() - 1;

		memset(address, 0, sizeof(address));

		msg.getAddress(address, addrOffset - 3, 3);

		/*DBG_OUTPUT_PORT.println(select_mode_int);
		DBG_OUTPUT_PORT.println("--"); */

		//byte msg_size = msg.size();

		//char full_addr[msg_size + 1 - 4];
		


		if (bool(msg.getFloat(0)) == true) {  // only on pushdown
			if (address[0] == 'S' && address[1] == 'I' && address[2] == 'P') {
				switch (option_int) 
				{
					case 0:
						wifi_cfg.ipStaticLocal[byte_int] -=  osc_miltiply_get();
					
						break;
					case 2:
						wifi_cfg.ipStaticLocal[byte_int] +=  osc_miltiply_get();
						break;
				}

				outvalue = float(wifi_cfg.ipStaticLocal[byte_int]);


			}

	
			if (address[0] == 'S' && address[1] == 'N' && address[2] == 'M') {
					switch (option_int)
					{
					case 0:
						wifi_cfg.ipSubnet[byte_int] -=  osc_miltiply_get();

						break;
					case 2:
						wifi_cfg.ipSubnet[byte_int] +=  osc_miltiply_get();
						break;
					}

					outvalue = float(wifi_cfg.ipSubnet[byte_int]);


				}

			if (address[0] == 'D' && address[1] == 'G' && address[2] == 'W') {
				switch (option_int)
				{
				case 0:
					wifi_cfg.ipDGW[byte_int] -=  osc_miltiply_get();

					break;
				case 2:
					wifi_cfg.ipDGW[byte_int] +=  osc_miltiply_get();
					break;
				}

				outvalue = float(wifi_cfg.ipDGW[byte_int]);


			}
			
			if (address[0] == 'D' && address[1] == 'N' && address[2] == 'S') {
				switch (option_int)
				{
				case 0:
					wifi_cfg.ipDNS[byte_int] -=  osc_miltiply_get();

					break;
				case 2:
					wifi_cfg.ipDNS[byte_int] +=  osc_miltiply_get();
					break;
				}

				outvalue = float(wifi_cfg.ipDNS[byte_int]);


			}

			outbuffer = String("/DS/" + String(address) + "L/" + String(byte_int ));
			osc_queu_MSG_float(outbuffer, outvalue);
			//debugMe(outbuffer);
		}







}

void osc_DS_led_type(OSCMessage &msg, int addrOffset) 
{		// OSC IN Device settings led type 1collum 3 rows
	
	
	// OSC MESSAGE :/m/multipl/?/1
	if (bool(msg.getFloat(0)) == true) 
	{
		String select_mode_string;
		// String select_bit_string;
		char address[10];
		debugMe("T3");
		
		bool switch_bool = false;

		msg.getAddress(address, addrOffset + 1);
		debugMe(String(address));
		for (byte i = 0; i < sizeof(address); i++) {
			if (address[i] == '/') {
				switch_bool = true;

			}
			else if (switch_bool == false) {
				select_mode_string = select_mode_string + address[i];

			}

		}

		int select_mode_int = select_mode_string.toInt();
		osc_queu_MSG_float("/DS/ledType/1/1", 0);		
		osc_queu_MSG_float("/DS/ledType/2/1", 0);
		osc_queu_MSG_float("/DS/ledType/3/1", 0);
		osc_queu_MSG_float("/DS/ledType/4/1", 0);
		osc_queu_MSG_float("/DS/ledType/5/1", 0);
		osc_queu_MSG_float("/DS/ledType/6/1", 0);

		if (bool(msg.getFloat(0)) == true) {
			switch (select_mode_int) {
			case 1:
				led_cfg.ledMode = 0;
				osc_queu_MSG_float("/DS/ledType/1/1", 1);

				break;
			case 2:
				led_cfg.ledMode = 1;
				osc_queu_MSG_float("/DS/ledType/2/1", 1);
				break;
			case 3:
				led_cfg.ledMode = 2;
				osc_queu_MSG_float("/DS/ledType/3/1", 1);
				break;
			case 4:
				led_cfg.ledMode = 3;
				osc_queu_MSG_float("/DS/ledType/4/1", 1);
				break;
			case 5:
				led_cfg.ledMode =4 ;
				osc_queu_MSG_float("/DS/ledType/5/1", 1);
				break;	
			case 6:
				led_cfg.ledMode =5 ;
				osc_queu_MSG_float("/DS/ledType/6/1", 1);
				break;	
			}
		} // end switch

		  //outbuffer = String("/multipl/1/1");



	} // end  new msg

}


void osc_DS_DATA_NL_in(OSCMessage &msg, int addrOffset)
{
	// OSC MESSAGE :/s/AN/Row/collum  

	String collum_string;
	String row_string;
	char address[4];					// to pick info aut of the msg address
										//char address_out[20];	
	int select_bit = 0;
	String select_bit_string;
	bool switch_bool = false;			// for toggels to get row and collum

	msg.getAddress(address, addrOffset - 4, 1);					// get the select-bit info	
	select_bit_string = select_bit_string + address[0];
	select_bit = select_bit_string.toInt();

	memset(address, 0, sizeof(address));				// rest the address to blank


	msg.getAddress(address, addrOffset + 1);		// get the address for row / collum


	for (byte i = 0; i < sizeof(address); i++)
	{
		if (address[i] == '/') {
			switch_bool = true;
		}
		else if (switch_bool == false)
		{
			row_string = row_string + address[i];
		}
		else
			collum_string = collum_string + address[i];
	}

	int row = row_string.toInt() - 1;
	int collum = collum_string.toInt() - 1;  // Whit CRGB value in the pallete

	memset(address, 0, sizeof(address));
	byte msg_size = msg.size();

	//debugMe("row: " + String(row) + " col: " + String(collum));
	if (bool(msg.getFloat(0)) == true) 
	switch (collum)
	{
	case 0:		// DATA1 NL
		switch (row)
		{
			case 0:
				led_cfg.Data1NrLeds = constrain(led_cfg.Data1NrLeds-  osc_miltiply_get(), 0, MAX_NUM_LEDS-led_cfg.Data1StartLed);
				break;
			case 2:
				led_cfg.Data1NrLeds = constrain(led_cfg.Data1NrLeds +  osc_miltiply_get(), 0,  MAX_NUM_LEDS-led_cfg.Data1StartLed);
			break;
		}
		osc_queu_MSG_float("/DS/NL/1", float(led_cfg.Data1NrLeds));
		break;

	case 1:		// artnet NR universes
		switch (row)
		{
			case 0:
				led_cfg.Data2NrLeds = constrain(led_cfg.Data2NrLeds-  osc_miltiply_get(), 0, MAX_NUM_LEDS-led_cfg.Data2StartLed);
				break;
			case 2:
				led_cfg.Data2NrLeds = constrain(led_cfg.Data2NrLeds +  osc_miltiply_get(), 0, MAX_NUM_LEDS-led_cfg.Data2StartLed);
			break;
		}
		osc_queu_MSG_float("/DS/NL/2", float(led_cfg.Data2NrLeds));
	break;
	case 2:		// artnet NR universes
		switch (row)
		{
			case 0:
				led_cfg.Data3NrLeds = constrain(led_cfg.Data3NrLeds-  osc_miltiply_get(), 0, MAX_NUM_LEDS-led_cfg.Data3StartLed);
				break;
			case 2:
				led_cfg.Data3NrLeds = constrain(led_cfg.Data3NrLeds +  osc_miltiply_get(), 0, MAX_NUM_LEDS-led_cfg.Data3StartLed);
			break;
		}
		osc_queu_MSG_float("/DS/NL/3", float(led_cfg.Data3NrLeds));
	break;
	case 3:		// artnet NR universes
		switch (row)
		{
			case 0:
				led_cfg.Data4NrLeds = constrain(led_cfg.Data4NrLeds-  osc_miltiply_get(), 0, MAX_NUM_LEDS-led_cfg.Data4StartLed);
				break;
			case 2:
				led_cfg.Data4NrLeds = constrain(led_cfg.Data4NrLeds +  osc_miltiply_get(), 0, MAX_NUM_LEDS-led_cfg.Data4StartLed);
			break;
		}
		osc_queu_MSG_float("/DS/NL/4", float(led_cfg.Data4NrLeds));
	break;
	case 4:		// artnet NR universes
		switch (row)
		{
			case 0:
				led_cfg.NrLeds = constrain(led_cfg.NrLeds -  osc_miltiply_get(), 0, MAX_NUM_LEDS);
				break;
			case 2:
				led_cfg.NrLeds = constrain(led_cfg.NrLeds +  osc_miltiply_get(), 0, MAX_NUM_LEDS);
			break;
		}
		osc_queu_MSG_float("/DS/NL/0", float(led_cfg.NrLeds));
	break;
	}
}

void osc_DS_DATA_SL_in(OSCMessage &msg, int addrOffset)
{
	// OSC MESSAGE :/s/AN/Row/collum  

	String collum_string;
	String row_string;
	char address[4];					// to pick info aut of the msg address
										//char address_out[20];	
	int select_bit = 0;
	String select_bit_string;
	bool switch_bool = false;			// for toggels to get row and collum

	msg.getAddress(address, addrOffset - 4, 1);					// get the select-bit info	
	select_bit_string = select_bit_string + address[0];
	select_bit = select_bit_string.toInt();

	memset(address, 0, sizeof(address));				// rest the address to blank


	msg.getAddress(address, addrOffset + 1);		// get the address for row / collum


	for (byte i = 0; i < sizeof(address); i++)
	{
		if (address[i] == '/') {
			switch_bool = true;
		}
		else if (switch_bool == false)
		{
			row_string = row_string + address[i];
		}
		else
			collum_string = collum_string + address[i];
	}

	int row = row_string.toInt() - 1;
	int collum = collum_string.toInt() - 1;  // Whit CRGB value in the pallete

	memset(address, 0, sizeof(address));
	byte msg_size = msg.size();

	//debugMe("row: " + String(row) + " col: " + String(collum));
	if (bool(msg.getFloat(0)) == true) 	
	switch (collum)
	{
	case 0:		// DATA1 NL
		switch (row)
		{
			case 0:
				led_cfg.Data1StartLed = constrain(led_cfg.Data1StartLed-  osc_miltiply_get(), 0, MAX_NUM_LEDS - led_cfg.Data1NrLeds );
				break;
			case 1:
				led_cfg.Data1StartLed = 0;
			break;
			case 2:
				led_cfg.Data1StartLed = constrain(led_cfg.Data1StartLed +  osc_miltiply_get(), 0, MAX_NUM_LEDS - led_cfg.Data1NrLeds );
			break;
		}
		osc_queu_MSG_float("/DS/SL/1", float(led_cfg.Data1StartLed));
		break;

	case 1:		// artnet NR universes
		switch (row)
		{
			case 0:
				led_cfg.Data2StartLed = constrain(led_cfg.Data2StartLed-  osc_miltiply_get(), 0, MAX_NUM_LEDS - led_cfg.Data2NrLeds);
				break;
			case 2:
				led_cfg.Data2StartLed = constrain(led_cfg.Data2StartLed +  osc_miltiply_get(), 0, MAX_NUM_LEDS - led_cfg.Data2NrLeds);
			break;
		}
		osc_queu_MSG_float("/DS/SL/2", float(led_cfg.Data2StartLed));
	break;
	case 2:		// artnet NR universes
		switch (row)
		{
			case 0:
				led_cfg.Data3StartLed = constrain(led_cfg.Data3StartLed -  osc_miltiply_get(), 0, MAX_NUM_LEDS - led_cfg.Data3NrLeds);
				break;
			case 2:
				led_cfg.Data3StartLed = constrain(led_cfg.Data3StartLed +  osc_miltiply_get(), 0, MAX_NUM_LEDS - led_cfg.Data3NrLeds);
			break;
		}
		debugMe("in2");
		osc_queu_MSG_float("/DS/SL/3", float(led_cfg.Data3StartLed));
	break;
		case 3:		// artnet NR universes
		switch (row)
		{
			case 0:
				led_cfg.Data4StartLed = constrain(led_cfg.Data4StartLed-  osc_miltiply_get(), 0, MAX_NUM_LEDS - led_cfg.Data4NrLeds);
				break;
			case 2:
				led_cfg.Data4StartLed = constrain(led_cfg.Data4StartLed +  osc_miltiply_get(), 0, MAX_NUM_LEDS - led_cfg.Data4NrLeds);
			break;
		}
		osc_queu_MSG_float("/DS/SL/4", float(led_cfg.Data4StartLed));
	break;
	
	}
}

void osc_DS_led_data_on(OSCMessage &msg, int addrOffset) 
{		// OSC IN Device settings led type 1collum 3 rows
	
	
	// OSC MESSAGE :/m/multipl/?/1
	//if (bool(msg.getFloat(0)) == true) 
	{
		String select_mode_string;
		// String select_bit_string;
		char address[10];
		//debugMe("T3");
		
		bool switch_bool = false;

		msg.getAddress(address, addrOffset + 1);
		debugMe(String(address));
		for (byte i = 0; i < sizeof(address); i++) {
			if (address[i] == '/') {
				switch_bool = true;

			}
			else if (switch_bool == false) {
				select_mode_string = select_mode_string + address[i];

			}

		}

		int select_mode_int = select_mode_string.toInt();
		//select_mode_int = select_mode_int-1;
		//boolean value = bool(msg.getFloat(0));
		//if (bool(msg.getFloat(0)) == true) 
		{
			switch (select_mode_int) 
			{
			case 1:
				write_bool(DATA1_ENABLE,bool(msg.getFloat(0)));
				debugMe("Data1-toggle");
				break;
			case 2:
				write_bool(DATA2_ENABLE,bool(msg.getFloat(0)));
				debugMe("Data2-toggle");
				break;
			case 3:
				write_bool(DATA3_ENABLE,bool(msg.getFloat(0)));
				debugMe("Data3-toggle");
				break;
			case 4:
				debugMe("Data4-toggle");
				write_bool(DATA4_ENABLE,bool(msg.getFloat(0)));
				break;	
			default:
				
				break;
			}
			//osc_queu_MSG_float("/DS/data/1/1", get_bool(DATA1_ENABLE));
			//osc_queu_MSG_float("/DS/data/2/1", get_bool(DATA2_ENABLE));
			//osc_queu_MSG_float("/DS/data/3/1", get_bool(DATA3_ENABLE));
			//osc_queu_MSG_float("/DS/data/4/1", get_bool(DATA4_ENABLE));
		} // end switch

		  //outbuffer = String("/multipl/1/1");



	} // end  new msg

}

void osc_device_settings_routing(OSCMessage &msg, int addrOffset) 
{	// routing of Device settings OSC messages

	char address[18];

	memset(address, 0, sizeof(address));
	debugMe("T1");

	msg.getAddress(address);
	debugMe(String(address));

	
	// OSC MESSAGE :/DS
	if (msg.fullMatch("/DEL_ALL", addrOffset) && bool(msg.getFloat(0)) == true)		{ osc_send_MSG_String("/DS/INFO", String("ALL DELETED "));	FS_osc_delete_all_saves(); };
	if (msg.fullMatch("/RESET", addrOffset) && bool(msg.getFloat(0)) == true)		{ osc_send_MSG_String("/DS/INFO", String("Resetting")); delay(100);		ESP.restart(); };

	if (msg.fullMatch("/ref", addrOffset) && bool(msg.getFloat(0)) == true)		osc_DS_refresh();
	if (msg.fullMatch("/httpd", addrOffset))		httpd_toggle_webserver();
		
	if (msg.fullMatch("/debug", addrOffset))		{ osc_send_MSG_String("/DS/INFO", String("Debug : "+ String(bool(msg.getFloat(0)))));	 debugMe(String("Debug : "+ String(bool(msg.getFloat(0))))); write_bool(DEBUG_OUT, bool(msg.getFloat(0)));}
	if (msg.fullMatch("/TNdebug", addrOffset))			{ write_bool(DEBUG_TELNET, bool(msg.getFloat(0))); }
	if (msg.fullMatch("/ESIP", addrOffset))			{ osc_send_MSG_String("/DS/INFO", String("Static IP : " + String(bool(msg.getFloat(0)))));	write_bool(STATIC_IP_ENABLED, bool(msg.getFloat(0))); }
	if (msg.fullMatch("/WAP", addrOffset))			{ write_bool(WIFI_MODE, bool(msg.getFloat(0))); }//debugMe("BLAH!!!");debugMe(get_bool(WIFI_MODE)); }
	if (msg.fullMatch("/WP", addrOffset))			{ write_bool(WIFI_POWER, bool(msg.getFloat(0)));}

	if (msg.fullMatch("/IPSAVE", addrOffset) && bool(msg.getFloat(0)) == true) 		{FS_wifi_write(0); osc_send_MSG_String("/DS/INFO", String("IP saved")); }
	if (msg.fullMatch("/BSAVE", addrOffset) && bool(msg.getFloat(0)) == true) 		{FS_Bools_write(0); osc_send_MSG_String("/DS/INFO", String("Settings saved")); }
	if (msg.fullMatch("/RSSI", addrOffset) && bool(msg.getFloat(0)) == true)		osc_queu_MSG_float("/DS/RSSIL", float(WiFi.RSSI()));



	if (msg.fullMatch("/Fheap", addrOffset) && bool(msg.getFloat(0)) == true)		osc_queu_MSG_float("/DS/FheapL", float(ESP.getFreeHeap()));
	if (msg.fullMatch("/heap", addrOffset) && bool(msg.getFloat(0)) == true)		osc_queu_MSG_float("/DS/heapL", float(ESP.getFreeHeap()));

	if (msg.fullMatch("/fft_enable", addrOffset)  )  								WIFI_FFT_toggle( bool(msg.getFloat(0))) ;
	if (msg.fullMatch("/fft_master", addrOffset))  									WIFI_FFT_toggle_master(bool(msg.getFloat(0)));
	if (msg.fullMatch("/fft_send_master", addrOffset))  							write_bool(FFT_MASTER_SEND, bool(msg.getFloat(0)));

	if (msg.fullMatch("/OSC_M", addrOffset))										write_bool(OSC_MC_SEND, bool(msg.getFloat(0)));

	msg.route("/Mbri", osc_DS_max_bri, addrOffset);
	msg.route("/STARTbri", osc_DS_start_bri, addrOffset);

	#ifndef ARTNET_DISABLED
		if (msg.fullMatch("/ANE", addrOffset)) osc_toggle_artnet(bool(msg.getFloat(0)));
		else if (bool(msg.getFloat(0) == true))  msg.route("/AN", osc_rec_artnet_info, addrOffset);
	#endif
	
	if (msg.fullMatch("/FPS", addrOffset))				osc_queu_MSG_float("/DS/FPSL", LEDS_get_FPS());
	if (msg.fullMatch("/EDIT", addrOffset))				write_bool(OSC_EDIT, bool(msg.getFloat(0)));

	msg.route("/SIP", osc_DS_ip_in, addrOffset);
	msg.route("/SNM", osc_DS_ip_in, addrOffset);
	msg.route("/DGW", osc_DS_ip_in, addrOffset);
	msg.route("/DNS", osc_DS_ip_in, addrOffset);

	debugMe("T2");
	msg.route("/ledType", osc_DS_led_type, addrOffset);
	msg.route("/data", osc_DS_led_data_on, addrOffset);
	debugMe("TXXX");


	msg.route("/LNL", osc_DS_DATA_NL_in, addrOffset);
	msg.route("/LSL", osc_DS_DATA_SL_in, addrOffset);


	//debugMe("DS routing END");
}



// Main OSC loop
void OSC_loop() 
{	// the main osc loop
	// it hast 2 main parts, 
	// part 1 is for the unicast OSC communication
	// part 2 is for the multicast OSC communication


	OSCMessage oscMSG;
	OSCMessage oscMSG_MC;


	int size = osc_server.parsePacket();

	if (size > 0) {
		while (size--) {
			oscMSG.fill(osc_server.read());
		}
		if (!oscMSG.hasError())
		{
			//debugMe("osc: rem , locel");
			//debugMe(osc_mc_server.remoteIP());
			//debugMe(WiFi.localIP());
			char address[30];
			memset(address, 0, sizeof(address));

			oscMSG.getAddress(address);
			debugMe(address);


			oscMSG.route("/m", osc_master_routing);
			oscMSG.route("/DS", osc_device_settings_routing);
			oscMSG.route("/form", osc_forms_routing);   //osc_form_effects);
			oscMSG.route("/strips", osc_strips_routing);   //"/{a,b}/[0-9]"
			//oscMSG.route("/copy", osc_copy);
			oscMSG.route("/multipl", osc_multipl_rec);
			oscMSG.route("/pal", osc_pal_routing);
			oscMSG.route("/fft", osc_fft_routing);
			oscMSG.route("/copy", osc_copy_routing);
			// if (oscMSG.fullMatch("/FULL_REF", 0) && bool(oscMSG.getFloat(0)) == true) osc_send_all();
			if (oscMSG.fullMatch("/reset-index", 0) && bool(oscMSG.getFloat(0)) == true) LEDS_pal_reset_index();
		}
		else {
			//error = bundle.getError();
			Serial.print("OSC error: ");
			//Serial.println( bundle.getError());
		}
	}
	osc_send_out_float_MSG_buffer();
#ifndef OSC_MC_SERVER_DISABLED
	size = osc_mc_server.parsePacket();
	if (size > 0)
	{
		while (size--)
		{
			oscMSG_MC.fill(osc_mc_server.read());
		}
		if (!oscMSG_MC.hasError() && osc_mc_server.remoteIP() != WiFi.localIP()) {
			char   addr_char[20];
			oscMSG_MC.getAddress(addr_char, 0, 15);
			//debugMe("MC: rem , locel" );
			
			//debugMe(osc_mc_server.remoteIP());
			//debugMe(WiFi.localIP());

			if (oscMSG_MC.fullMatch("/x/m/bri", 0))			led_cfg.bri = byte(oscMSG_MC.getInt(0));
			if (oscMSG_MC.fullMatch("/x/m/r", 0))			led_cfg.r = byte(oscMSG_MC.getInt(0));
			if (oscMSG_MC.fullMatch("/x/m/g", 0))			led_cfg.g = byte(oscMSG_MC.getInt(0));
			if (oscMSG_MC.fullMatch("/x/m/b", 0))			led_cfg.b = byte(oscMSG_MC.getInt(0));
			if (oscMSG_MC.fullMatch("/x/m/palbri", 0))		led_cfg.pal_bri = byte(oscMSG_MC.getInt(0));

			oscMSG_MC.route("/x/fft/tga", osc_fft_rec_toggle_byte, 0);
			oscMSG_MC.route("/x/fft/fader", osc_fft_rec_fader, 0);

			oscMSG_MC.route("/x/m/load", osc_master_conf_play_load, 0);
			oscMSG_MC.route("/x/m/save", osc_master_conf_play_save, 0);
			//oscMSG_MC.route("/fft", osc_master_MC_rgb);

			//oscMSG_MC.route("/m", osc_master_MC_rgb);
			//oscMSG_MC.route("/s", osc_settings, 0);
			//oscMSG_MC.route("/form", osc_forms);   //osc_form_effects);
			//oscMSG_MC.route("/strips", osc_strips);   //"/{a,b}/[0-9]"
			//oscMSG_MC.route("/copy", osc_copy);
			//oscMSG_MC.route("/multipl", osc_rec_multipl);
			//oscMSG_MC.route("/pal", osc_pal);
			//oscMSG_MC.route("/fft", osc_fft);

			//// if (oscMSG.fullMatch("/FULL_REF", 0) && bool(oscMSG.getFloat(0)) == true) osc_send_all();
			//if (oscMSG.fullMatch("/reset-index", 0) && bool(oscMSG.getFloat(0)) == true) leds_reset_index();
		}
		else {
			//error = bundle.getError();
			Serial.print("osc MC : error: ");
			//Serial.println( bundle.getError());
		}
	}
#endif // #ifdef OSC_MC_SERVER_ENABLE




}


#endif // OAC
//
