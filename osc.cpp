// 
// collapse all ctr+k+0  vis studoip code
// expand all ctl+k+j
// 


//  Todo : 



#include "config_TPM.h"   // include the main Defines
#include "osc.h"
#include "leds.h"

#ifndef USE_OSC
// clapse all ctl+m +o    +l = expand

//#ifdef ESP8266

//#define OSC_MC_SERVER_DISABLED

		#define OSC_BUNDLE_SEND_COUNT 20				// how many OSC messages to send in one bundle.
		#define OSC_MULTIPLY_OPTIONS 11					// how many multiply options to add to input from touchosc

		#define OSC_CONF_MAX_SAVES 	16					// what is the max amount of saves
		#define VIZ_FPS_MAX 15
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
	#include "tpm_artnet.h"
	

	#define NO_OF_PALLETS 2
	#define OSC_QEUE_ADD_LEN 30
// External Variables/ Structures


	extern fft_data_struct fft_data[7];
	extern wifi_Struct wifi_cfg;
	extern artnet_struct artnet_cfg;




// from leds
	extern  form_Led_Setup_Struct form_cfg[NR_FORM_PARTS];
	extern  form_fx_pal_struct form_fx_pal[NR_FORM_PARTS] ;
	extern  form_fx_shim_struct form_fx_shim[NR_FORM_PARTS];
	extern  form_fx_fire_struct form_fx_fire[NR_FORM_PARTS];
	extern  form_fx_fft_struct form_fx_fft[NR_FORM_PARTS];
	extern  form_fx1_struct form_fx1[NR_FORM_PARTS];
	extern  form_fx_glitter_struct form_fx_glitter[NR_FORM_PARTS];
	extern  form_fx_dots_struct form_fx_dots[NR_FORM_PARTS] ;

	extern byte form_menu_pal[_M_NR_FORM_BYTES_][_M_NR_FORM_PAL_OPTIONS_];
	extern byte form_menu_fft[_M_NR_FORM_BYTES_][_M_NR_FORM_FFT_OPTIONS_];
	extern byte form_menu_fire[_M_NR_FORM_BYTES_][_M_NR_FORM_FIRE_OPTIONS_];
	extern byte form_menu_glitter[_M_NR_FORM_BYTES_][_M_NR_FORM_GLITTER_OPTIONS_];
	extern byte form_menu_dot[_M_NR_FORM_BYTES_][_M_NR_FORM_DOT_OPTIONS_];
	extern byte form_menu_shimmer[_M_NR_FORM_BYTES_][_M_NR_FORM_SHIMMER_OPTIONS_];
	extern byte form_menu_fx1[_M_NR_FORM_BYTES_][_M_NR_FORM_FX1_OPTIONS_];

	extern uint8_t LEDS_fft_get_fxbin_result(uint8_t fxbin);


	extern led_cfg_struct led_cfg;
	extern led_Copy_Struct copy_leds[NR_COPY_STRIPS];
	//extern Strip_FL_Struct part[NR_STRIPS];

	extern byte  copy_leds_mode[NR_COPY_LED_BYTES];
	//extern byte strip_menu[_M_NR_STRIP_BYTES_][_M_NR_OPTIONS_];
	//extern byte form_menu[_M_NR_FORM_BYTES_][_M_NR_FORM_OPTIONS_];
	//extern uint8_t global_strip_opt[_M_NR_STRIP_BYTES_][_M_NR_GLOBAL_OPTIONS_];
	extern byte fft_menu[3];
	//extern byte fft_data_menu[3];
	extern byte fft_data_bri;
	extern fft_led_cfg_struct fft_led_cfg;
	extern uint8_t fft_bin_autoTrigger;
	extern byte fft_data_fps;

	extern fft_fxbin_struct  fft_fxbin[FFT_FX_NR_OF_BINS];
	//extern uint8_t fft_color_result_data[3]; // 
	extern uint8_t fft_color_result_bri ;
	//extern uint8_t fft_bin_autoTrigger;
	extern uint8_t fft_color_fps;

	extern form_fx_test_val form_fx_test;

	extern uint16_t play_conf_time_min[MAX_NR_SAVES];
	extern uint8_t layer_select[MAX_LAYERS_SELECT] ;
//extern CRGBPalette16 LEDS_pal_cur[NR_PALETTS];

	extern artnet_node_struct artnetNode[ARTNET_NR_NODES_TPM] ;
	

// from mmqt.cpp
 #include "mmqt.h"
 extern mqtt_Struct mqtt_cfg;




//struct OSC_buffer_float master_rgb = { 255,255,255 };


//from coms
//extern void comms_S_FPS(uint8_t fps);


QueueArray <char> osc_out_float_addr;
QueueArray <float> osc_out_float_value;
QueueArray <char> osc_out_rgb_addr;
QueueArray <uint8_t> osc_out_rgb_value;
QueueArray <char> osc_out_SelVal_addr;
QueueArray <int> osc_out_SelVal_value;
QueueArray <char> osc_out_int_addr;
QueueArray <int> osc_out_int_value;



osc_cfg_struct osc_cfg = { OSC_IPMULTI_ ,OSC_PORT_MULTI_,OSC_OUTPORT, OSC_INPORT, 0,1 };




	WiFiUDP osc_server;				// the normal osc server



void OSC_setup()
{
	osc_server.begin(osc_cfg.inPort);
	debugMe("OSC Setup Done Port:", false ); debugMe(String(osc_cfg.inPort) );
	
}




/////////////////////////////////////////// OSC  SEND / gerneral functions
//
//
float osc_byte_tofloat(byte value, uint8_t max_value = 255) {

	float float_out = float(value) / max_value;

	return float_out;
}


void osc_send_MSG_rgb(String addr_string , uint8_t  red = 128, uint8_t  green = 128, uint8_t  blue = 128  ) 
{
	char address_out[30];
	addr_string.toCharArray(address_out, addr_string.length() + 1); //address_out 

	IPAddress ip_out(osc_server.remoteIP());
	OSCMessage msg_out(address_out);

	//debugMe("out: ", false);
	//debugMe(String(address_out	));

	msg_out.add(red);
	msg_out.add(green);
	msg_out.add(blue);

	msg_out.send(osc_server);
	osc_server.endPacket();
	msg_out.empty();
	


}



void osc_send_MSG_led(String addr_string , uint8_t  state  )    // set a led to a color  0 = black , 1 = red, 2 = green, 3 = blue, 4 = white
{
	char address_out[30];
	addr_string.toCharArray(address_out, addr_string.length() + 1); //address_out 

	//IPAddress ip_out(osc_server.remoteIP());
	OSCMessage msg_out(address_out);
	//debugMe("o2: ",false);
	//debugMe(String(address_out));
	switch(state)
	{
		case 0:   // black 
			msg_out.add(uint8_t(0));
			msg_out.add(uint8_t(0));
			msg_out.add(uint8_t(0));
		break;

		case 1:  // red
			msg_out.add(uint8_t(255));
			msg_out.add(uint8_t(0));
			msg_out.add(uint8_t(0));
		break;

		case 2:   // green 
			msg_out.add(uint8_t(0));
			msg_out.add(uint8_t(255));
			msg_out.add(uint8_t(0));
		break;

		case 3:   // blue 
			msg_out.add(uint8_t(0));
			msg_out.add(uint8_t(0));
			msg_out.add(uint8_t(255));
		break;

		case 4:   // white 
			msg_out.add(uint8_t(255));
			msg_out.add(uint8_t(255));
			msg_out.add(uint8_t(255));
		break;

	}

	msg_out.send(osc_server);
	osc_server.endPacket();
	msg_out.empty();
	yield();
}


void osc_queu_MSG_SEL_VAL(String addr_string, int select, int value)
{
	for (int i = 0; i < addr_string.length(); i++)
		osc_out_SelVal_addr.enqueue(addr_string.charAt(i));

	osc_out_SelVal_addr.enqueue(0);	// add a null on the end 
	osc_out_SelVal_value.enqueue(select);
	osc_out_SelVal_value.enqueue(value);


}


void osc_queu_MSG_float(String addr_string, float value) {
	IPAddress ip_out(osc_server.remoteIP());
	osc_cfg.return_ip_LB = ip_out[3];

	for (int i = 0; i < addr_string.length(); i++)
		osc_out_float_addr.enqueue(addr_string.charAt(i));

	osc_out_float_addr.enqueue(0);	// add a null on the end 
	osc_out_float_value.enqueue(value);



}


void osc_queu_MSG_int(String addr_string, int value) {
	IPAddress ip_out(osc_server.remoteIP());
	osc_cfg.return_ip_LB = ip_out[3];

	for (int i = 0; i < addr_string.length(); i++)
		osc_out_int_addr.enqueue(addr_string.charAt(i));

	osc_out_int_addr.enqueue(0);	// add a null on the end 
	osc_out_int_value.enqueue(value);



}



void osc_queu_MSG_rgb(String addr_string, uint8_t red,uint8_t green,uint8_t blue) {
	IPAddress ip_out(osc_server.remoteIP());
	osc_cfg.return_ip_LB = ip_out[3];

	for (int i = 0; i < addr_string.length(); i++)
		osc_out_rgb_addr.enqueue(addr_string.charAt(i));

	osc_out_rgb_addr.enqueue(0);	// add a null on the end 
	osc_out_rgb_value.enqueue(red);
	osc_out_rgb_value.enqueue(green);
	osc_out_rgb_value.enqueue(blue);


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
	yield();

}
// Other Functions like Sending loop
void osc_send_out_float_MSG_buffer() 
{
	if (osc_out_float_value.isEmpty() != true || osc_out_rgb_value .isEmpty() != true || osc_out_SelVal_value.isEmpty() != true || osc_out_int_value.isEmpty() != true) 
	{

		char address_out[OSC_QEUE_ADD_LEN] ;
		memset(address_out, 0, OSC_QEUE_ADD_LEN);
		uint8_t i = 0;
		float value = 0;
		uint8_t red_val = 0;
		uint8_t green_val = 0;
		uint8_t blue_val = 0;

		int menu_select = 0;
		int menu_value = 0;


		//msg_out_data = osc_out_float.dequeue();

		//addr_string.toCharArray(address_out, addr_string.length() + 1); //address_out 
		//msg_out_data.addr.toCharArray(address_out, msg_out_data.addr.length() + 1); //address_out
		if (osc_out_float_value.count() != 0 ||  (osc_out_rgb_value.count()) != 0 ||  (osc_out_SelVal_value.count())   != 0 ||  (osc_out_int_value.count())   != 0)
		{
			//debugMe(String(osc_out_float_value.count()));
			//debugMe(String(osc_out_rgb_value.count()));
			//debugMe(String(osc_out_SelVal_value.count()));



			OSCBundle bundle_out;
			//IPAddress ip_out(WiFi.localIP());
			IPAddress ip_out(osc_server.remoteIP());
			//ip_out[3] = osc_cfg.return_ip_LB;
			uint8_t bundlecount = 0;
			while ( bundlecount <= OSC_BUNDLE_SEND_COUNT)
			{
				for (uint8_t z = 0; z <osc_out_float_value.count(); z++)
				{
					if(osc_out_float_value.isEmpty() != true)
					{
						i = 0;
						while (osc_out_float_addr.peek() != 0 && i <OSC_QEUE_ADD_LEN)
						{
							address_out[i] = osc_out_float_addr.dequeue();
							i++;
						}
						address_out[i] = osc_out_float_addr.dequeue(); // get the null char for end of string as well. so that we can fetch the next msg next time
						value = osc_out_float_value.dequeue();
						//debugMe("float-Addr: " + String(address_out));
						bundle_out.add(address_out).add(float(value));
						bundlecount++;
						yield();
						memset(address_out, 0, OSC_QEUE_ADD_LEN);
					}
				if	( bundlecount >= OSC_BUNDLE_SEND_COUNT) { break; }
				}
				if	( bundlecount >= OSC_BUNDLE_SEND_COUNT) { break; }
				for (uint8_t z = 0; z < (osc_out_rgb_value.count() /3 ); z++)
				{
					if(osc_out_rgb_value.isEmpty() != true)
					{
						i = 0;
						while (osc_out_rgb_addr.peek() != 0 && i <OSC_QEUE_ADD_LEN) {
							address_out[i] = osc_out_rgb_addr.dequeue();
							i++;
						}
						address_out[i] = osc_out_rgb_addr.dequeue(); // get the null char for end of string as well. so that we can fetch the next msg next time
						red_val = osc_out_rgb_value.dequeue();
						green_val = osc_out_rgb_value.dequeue();
						blue_val = osc_out_rgb_value.dequeue();

						//debugMe("rgb-Addr: " + String(address_out));
						bundle_out.add(address_out).add(red_val).add(green_val).add(blue_val);
						bundlecount++;
						yield();
						memset(address_out, 0, OSC_QEUE_ADD_LEN);
					}
					if	( bundlecount >= OSC_BUNDLE_SEND_COUNT) { break; }
				}
				if	( bundlecount >= OSC_BUNDLE_SEND_COUNT) { break; }
				for (uint8_t z = 0; z < (osc_out_SelVal_value.count() /2 ); z++)
				{
					if(osc_out_SelVal_value.isEmpty() != true)
					{
						i = 0;
						while (osc_out_SelVal_addr.peek() != 0 && i <OSC_QEUE_ADD_LEN) {
							address_out[i] = osc_out_SelVal_addr.dequeue();
							i++;
						}
						address_out[i] = osc_out_SelVal_addr.dequeue(); // get the null char for end of string as well. so that we can fetch the next msg next time
						menu_select = osc_out_SelVal_value.dequeue();
						menu_value = osc_out_SelVal_value.dequeue();
						
						//debugMe("SelVal-Addr: " + String(address_out));
						bundle_out.add(address_out).add(menu_select).add(menu_value);
						bundlecount++;
						yield();
						memset(address_out, 0, OSC_QEUE_ADD_LEN);
					}
					if	( bundlecount >= OSC_BUNDLE_SEND_COUNT) { break; }
				}
				if	( bundlecount >= OSC_BUNDLE_SEND_COUNT) { break; }
				for (uint8_t z = 0; z < (osc_out_int_value.count() ); z++)
				{
					if(osc_out_int_value.isEmpty() != true)
					{
						i = 0;
						while (osc_out_int_addr.peek() != 0 && i <OSC_QEUE_ADD_LEN) {
							address_out[i] = osc_out_int_addr.dequeue();
							i++;
						}
						address_out[i] = osc_out_int_addr.dequeue(); // get the null char for end of string as well. so that we can fetch the next msg next time
						menu_value = osc_out_int_value.dequeue();
						
						
						//debugMe("SelVal-Addr: " + String(address_out));
						bundle_out.add(address_out).add(menu_value);
						bundlecount++;
						yield();
						memset(address_out, 0, OSC_QEUE_ADD_LEN);
					}
					if	( bundlecount >= OSC_BUNDLE_SEND_COUNT) { break; }
				}

			if (osc_out_float_value.isEmpty() &&  (osc_out_rgb_value.isEmpty()) &&  (osc_out_SelVal_value.isEmpty())   &&  (osc_out_int_value.isEmpty())) break;
			}	// end while

			osc_server.beginPacket(ip_out, OSC_OUTPORT); //{172,16,222,104}, 8001) ; //
			bundle_out.send(osc_server);
			osc_server.endPacket();
			bundle_out.empty();
			yield();
			//delay(1);

		}
		/*else
		{
			OSCBundle bundle_out;

			for (uint8_t z = 0; z <osc_out_float_value.count(); z++)
			{
				i = 0;
				while (osc_out_float_addr.peek() != 0 && i <OSC_QEUE_ADD_LEN) {
					address_out[i] = osc_out_float_addr.dequeue();
					i++;
				}
				address_out[i] = osc_out_float_addr.dequeue(); // get the null char for end of string as well. so that we can fetch the next msg next time
				value = osc_out_float_value.dequeue();

				bundle_out.add(address_out).add(float(value));
				memset(address_out, 0, OSC_QEUE_ADD_LEN);

			}

			for (uint8_t z = 0; z < (osc_out_rgb_value.count() /3 ); z++)
			{
				i = 0;
				while (osc_out_rgb_addr.peek() != 0 && i <OSC_QEUE_ADD_LEN) {
					address_out[i] = osc_out_rgb_addr.dequeue();
					i++;
				}
				address_out[i] = osc_out_rgb_addr.dequeue(); // get the null char for end of string as well. so that we can fetch the next msg next time
					red_val = osc_out_rgb_value.dequeue();
					green_val = osc_out_rgb_value.dequeue();
					blue_val = osc_out_rgb_value.dequeue();

				bundle_out.add(address_out).add(red_val).add(green_val).add(blue_val);
				memset(address_out, 0, OSC_QEUE_ADD_LEN);

			}




			osc_server.beginPacket(osc_server.remoteIP(), 9000); //{172,16,222,104}, 8001) ; //
			bundle_out.send(osc_server);
			osc_server.endPacket();
			bundle_out.empty();
			yield();
		}
		*/	
	}


}






// OSC Settings MSG:/s




void osc_multiply_send()
{
	for (uint8_t x = 1;x <= 11  ; x++ )
	{
		//debugMe((String("/multipl/") + String(x) + String("/1")));
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
	//int select_bit = 0;
	String select_bit_string;
	bool switch_bool = false;			// for toggels to get row and collum

	String outbuffer = "/s/ANL";		// OSC return address
	float outvalue = 0;						// return value to labels

	String out_add_label;				// address label



	msg.getAddress(address, addrOffset - 4, 1);					// get the select-bit info	
	//select_bit_string = select_bit_string + address[0];
	//select_bit = select_bit_string.toInt();

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
	//byte msg_size = msg.size();

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
		debugMe("enable artnet recive");

		
		//FastLED.setBrightness(led_brightness);
		//leds.fadeToBlackBy(255);
		//FastLED.show();

		write_bool(ARTNET_RECIVE, true); // artnet_enabled = true;		
		//enable_artnet();
		FS_artnet_write();
		led_cfg.bri = led_cfg.max_bri;
		WiFi_artnet_rc_enable();
		//writeESP_play_Settings();
	}
	else {
		debugMe("disable artnet recive");
		write_bool(ARTNET_RECIVE, false);  //artnet_enabled = false;
		FS_artnet_write();
		//writeESP_play_Settings(),
		//Udp.stop();
		ESP.restart();


	}



}
#endif


void osc_StC_FFT_vizIt()
{
	osc_queu_MSG_int("/ostc/audio/rfps", 		LEDS_get_FPS());
	osc_queu_MSG_int("/ostc/audio/rbri", 		LEDS_get_real_bri()); 
	osc_queu_MSG_int("/ostc/audio/sum/bri", 	fft_color_result_bri);
	osc_queu_MSG_int("/ostc/audio/sum/fps", 	fft_color_fps);

	osc_queu_MSG_int("/ostc/audio/sum/red", 	LEDS_FFT_get_color_result(0));
	osc_queu_MSG_int("/ostc/audio/sum/green", 	LEDS_FFT_get_color_result(1));
	osc_queu_MSG_int("/ostc/audio/sum/blue",	LEDS_FFT_get_color_result(2));


	for(uint8_t fxbin = 0; fxbin < FFT_FX_NR_OF_BINS; fxbin++)
	{
		osc_queu_MSG_int("/ostc/audio/fxb/sum/" + String(fxbin) , 		fft_fxbin[fxbin].sum);
		osc_queu_MSG_int("/ostc/audio/fxb/res/" + String(fxbin) , 		LEDS_fft_get_fxbin_result(fxbin));
		osc_queu_MSG_int("/ostc/audio/fxbin/tg/" + String(fxbin) , 		fft_fxbin[fxbin].trrig_val);
		osc_queu_MSG_int("/ostc/audio/fxbin/vl/" + String(fxbin) , 		fft_fxbin[fxbin].set_val);
		
	}


	for(uint8_t bin = 0 ; bin < 7 ; bin++)
	{
		osc_queu_MSG_int("/ostc/audio/abin/"+ String(bin) ,  	fft_data[6-bin].avarage );
		osc_queu_MSG_int("/ostc/audio/mbin/"+ String(bin) ,  	fft_data[6-bin].max );
		osc_queu_MSG_int("/ostc/audio/trig/"+ String(bin) ,  	fft_data[6-bin].trigger );
		osc_queu_MSG_int("/ostc/audio/lbin/"+ String(bin) ,  	fft_data[6-bin].value );
		osc_queu_MSG_int("/ostc/audio/rbin/"+ String(bin) ,  	constrain((fft_data[6-bin].value - fft_data[6-bin].trigger) , 0 ,255  ) );		
	}
	
 
//debugMe("invizz");
}





////////////////////////////////////////////-------------- OPEN STage Controll



// REFRESH 
void osc_StC_menu_form_led_adv_ref()
{


		for (uint8_t formNr = 0; formNr < NR_FORM_PARTS; formNr++)
		{
			
		osc_queu_MSG_int("/ostc/form/sys/sld/" + String(formNr), form_cfg[formNr].start_led );
		osc_queu_MSG_int("/ostc/form/sys/nld/" + String(formNr), form_cfg[formNr].nr_leds );

		}	
}

void osc_StC_menu_form_fft_adv_ref()
{
	
		uint8_t bit = 0;

		for (uint8_t formNr = 0; formNr < NR_FORM_PARTS; formNr++)
		{
			
				uint8_t real_formNr = formNr;
				 bit = 0;
				while (real_formNr >= 8 )
				{
					bit++;
					real_formNr = real_formNr - 8;	

				}
					
					osc_queu_MSG_int( "/ostc/form/fft/rev/" + String(formNr),	(bitRead(form_menu_fft[bit][_M_FORM_FFT_REVERSED], 		real_formNr)));  		
					osc_queu_MSG_int( "/ostc/form/fft/run/" + String(formNr),	(bitRead(form_menu_fft[bit][_M_FORM_FFT_RUN], 			real_formNr)));  		
					osc_queu_MSG_int( "/ostc/form/fft/ocl/" + String(formNr),	(bitRead(form_menu_fft[bit][_M_FORM_FFT_ONECOLOR], 		real_formNr)));  		
					osc_queu_MSG_int( "/ostc/form/fft/mir/" + String(formNr),	(bitRead(form_menu_fft[bit][_M_FORM_FFT_MIRROR], 		real_formNr)));  

					



				
	
		osc_queu_MSG_int("/ostc/form/fft/ofs/" + String(formNr), form_fx_fft[formNr].offset );
		osc_queu_MSG_int("/ostc/form/fft/exd/" + String(formNr), form_fx_fft[formNr].extend );
		osc_queu_MSG_int("/ostc/form/fft/mix/" + String(formNr), form_fx_fft[formNr].mix_mode );
		osc_queu_MSG_int("/ostc/form/fft/lvl/"+	String(formNr), form_fx_fft[formNr].level );

		}		
}

void osc_StC_menu_form_play_ref()
{
uint8_t bit = 0;	

		for (uint8_t formNr = 0; formNr < NR_FORM_PARTS; formNr++)
		{
		
				uint8_t real_formNr = formNr;
				 bit = 0;
				while (real_formNr >= 8 )
				{
					bit++;
					real_formNr = real_formNr - 8;	

				}

		osc_queu_MSG_int( "/ostc/form/pal/run/" + String(formNr), 	(bitRead(form_menu_pal[bit][_M_FORM_PAL_RUN], 	real_formNr)));		
		osc_queu_MSG_int( "/ostc/form/fft/run/" + 	String(formNr),		(bitRead(form_menu_fft[bit][_M_FORM_FFT_RUN], 	real_formNr))); 


		osc_queu_MSG_int( "/ostc/form/fx/fire/run/" + String(formNr),		(bitRead(form_menu_fire[bit][_M_FORM_FIRE_RUN], 			real_formNr)));  		
		osc_queu_MSG_int( "/ostc/form/fx/shim/run/" +	String(formNr),		(bitRead(form_menu_shimmer[bit][_M_FORM_SHIMMER_RUN], 	real_formNr)));  		

		osc_queu_MSG_int( "/ostc/form/fx/fx01/run/" +	String(formNr),		(bitRead(form_menu_fx1[bit][_M_FORM_FX1_RUN], 		real_formNr)));  	
		osc_queu_MSG_int("/ostc/form/fx/dott/run/" + String(formNr), 	(bitRead(form_menu_dot[bit][_M_FORM_DOT_RUN], 		real_formNr)) );
		osc_queu_MSG_int( "/ostc/form/fx/glit/run/" +	String(formNr),		(bitRead(form_menu_glitter[bit][_M_FORM_GLITTER_RUN], 	real_formNr)));  		

		
		}

}



void osc_StC_menu_form_fx_fire_adv_ref()
{
uint8_t bit = 0;	

		for (uint8_t formNr = 0; formNr < NR_FORM_PARTS; formNr++)
		{
		
				uint8_t real_formNr = formNr;
				 bit = 0;
				while (real_formNr >= 8 )
				{
					bit++;
					real_formNr = real_formNr - 8;	

				}
					
					osc_queu_MSG_int( "/ostc/form/fx/fire/run/" + String(formNr),		(bitRead(form_menu_fire[bit][_M_FORM_FIRE_RUN], 			real_formNr)));  		
					osc_queu_MSG_int( "/ostc/form/fx/fire/mir/" + String(formNr),	(bitRead(form_menu_fire[bit][_M_FORM_FIRE_MIRROR], 	real_formNr)));  		
					osc_queu_MSG_int( "/ostc/form/fx/fire/rev/" + String(formNr),	(bitRead(form_menu_fire[bit][_M_FORM_FIRE_REVERSED], 		real_formNr)));  		
				
										
		
			osc_queu_MSG_int("/ostc/form/fx/fire/lvl/" + String(formNr) , form_fx_fire[formNr].level );
			osc_queu_MSG_int("/ostc/form/fx/fire/mix/" + String(formNr), form_fx_fire[formNr].mix_mode );
			osc_queu_MSG_int("/ostc/form/fx/fire/pal/" + String(formNr), form_fx_fire[formNr].pal );
			osc_queu_MSG_int("/ostc/form/fx/fire/col/" + String(formNr), form_fx_fire[formNr].cooling );
			osc_queu_MSG_int("/ostc/form/fx/fire/spk/" + String(formNr), form_fx_fire[formNr].sparking );
		}

	//	osc_queu_MSG_int("/ostc/master/fireCool", 			led_cfg.fire_cooling );
	//	osc_queu_MSG_int("/ostc/master/fireSpark",			led_cfg.fire_sparking );
}


void osc_StC_menu_form_dot_adv_ref()
{
	uint8_t bit = 0;	

		for (uint8_t formNr = 0; formNr < NR_FORM_PARTS; formNr++)
		{
			
				uint8_t real_formNr = formNr;
				 bit = 0;
				while (real_formNr >= 8 )
				{
					bit++;
					real_formNr = real_formNr - 8;	

				}

		
		//osc_queu_MSG_int( "/ostc/form/fx/dott/saw/" + 	String(formNr),		(bitRead(form_menu_dot[bit][_M_FORM_DOT_SAW], 		real_formNr)));  		
		//osc_queu_MSG_int( "/ostc/form/fx/dott/jug/" + 	String(formNr),		(bitRead(form_menu_dot[bit][_M_FORM_DOT_SINE], 		real_formNr)));  	
		//osc_queu_MSG_int( "/ostc/form/fx/dott/fft/" + 	String(formNr),		(bitRead(form_menu_dot[bit][_M_FORM_DOT_FFT], 	real_formNr)));  	

		osc_queu_MSG_int("/ostc/form/fx/dott/pal/" + String(formNr), form_fx_dots[formNr].pal );
		osc_queu_MSG_int("/ostc/form/fx/dott/lvl/" + String(formNr), form_fx_dots[formNr].level );
		//osc_queu_MSG_int("/ostc/form/fx/dott/mix/" + String(formNr), form_fx_dots[formNr].mix );
		osc_queu_MSG_int("/ostc/form/fx/dott/num/" + String(formNr), form_fx_dots[formNr].nr_dots );
		osc_queu_MSG_int("/ostc/form/fx/dott/bpm/" + String(formNr), form_fx_dots[formNr].speed );
		osc_queu_MSG_int("/ostc/form/fx/dott/pbm/" + String(formNr), form_fx_dots[formNr].index_add );

		osc_queu_MSG_int("/ostc/form/fx/dott/run/" + String(formNr), 	(bitRead(form_menu_dot[bit][_M_FORM_DOT_RUN], 		real_formNr)) );
		osc_queu_MSG_int("/ostc/form/fx/dott/typ/" + String(formNr), 	(bitRead(form_menu_dot[bit][_M_FORM_DOT_TYPE], 		real_formNr)));

		}		
}

void osc_StC_menu_form_glit_adv_ref()
{
	uint8_t bit = 0;	

		for (uint8_t formNr = 0; formNr < NR_FORM_PARTS; formNr++)
		{
			
				uint8_t real_formNr = formNr;
				 bit = 0;
				while (real_formNr >= 8 )
				{
					bit++;
					real_formNr = real_formNr - 8;	

				}

		osc_queu_MSG_int( "/ostc/form/fx/glit/run/" +	String(formNr),		(bitRead(form_menu_glitter[bit][_M_FORM_GLITTER_RUN], 	real_formNr)));  		
		osc_queu_MSG_int( "/ostc/form/fx/glit/gdb/" +	String(formNr),		(bitRead(form_menu_glitter[bit][_M_FORM_GLITTER_FFT], 	real_formNr)));  		
		//osc_queu_MSG_int( "/ostc/form/fx/glit/fft/" + String(formNr),		(bitRead(form_menu_glitter[bit][_M_FORM_GLITTER_FFT], 	real_formNr)));  		
		osc_queu_MSG_int( "/ostc/form/fx/glit/lvl/" + String(formNr),	 form_fx_glitter[formNr].level);
		//osc_queu_MSG_int( "/ostc/form/fx/glit/mix/" + String(formNr),	 form_fx_glitter[real_formNr].mix_mode);
		osc_queu_MSG_int( "/ostc/form/fx/glit/pal/" + String(formNr),	 form_fx_glitter[formNr].pal);
		osc_queu_MSG_int( "/ostc/form/fx/glit/val/" + String(formNr),	 form_fx_glitter[formNr].value);
		}		
}

void osc_StC_menu_form_shim_adv_ref()
{
	uint8_t bit = 0;	

		for (uint8_t formNr = 0; formNr < NR_FORM_PARTS; formNr++)
		{
			
				uint8_t real_formNr = formNr;
				 bit = 0;
				while (real_formNr >= 8 )			
				{
					bit++;
					real_formNr = real_formNr - 8;	

				}

		osc_queu_MSG_int( "/ostc/form/fx/shim/run/" +	String(formNr),		(bitRead(form_menu_shimmer[bit][_M_FORM_SHIMMER_RUN], 	real_formNr)));  		
		osc_queu_MSG_int( "/ostc/form/fx/shim/bld/" + String(formNr),		(bitRead(form_menu_shimmer[bit][_M_FORM_SHIMMER_BLEND], 	real_formNr)));  		
		osc_queu_MSG_int( "/ostc/form/fx/shim/lvl/" + String(formNr),	 form_fx_shim[formNr].level);
		osc_queu_MSG_int( "/ostc/form/fx/shim/mix/" + String(formNr),	 form_fx_shim[formNr].mix_mode);
		osc_queu_MSG_int( "/ostc/form/fx/shim/pal/" + String(formNr),	 form_fx_shim[formNr].pal);
		osc_queu_MSG_int( "/ostc/form/fx/shim/x_s/" + String(formNr),	 form_fx_shim[formNr].xscale);
		osc_queu_MSG_int( "/ostc/form/fx/shim/y_s/" + String(formNr),	 form_fx_shim[formNr].yscale);
		osc_queu_MSG_int( "/ostc/form/fx/shim/bet/" + String(formNr),	 form_fx_shim[formNr].beater);
		}		
}
/*
void osc_StC_menu_form_fx_shim_adv_ref()
{
	uint8_t bit = 0;	
	for (uint8_t formNr = 0; formNr < NR_FORM_PARTS; formNr++)
		{
			
				uint8_t real_formNr = formNr;
				 bit = 0;
				while (real_formNr >= 8 )
				{
					bit++;
					real_formNr = real_formNr - 8;	

				}
		
				
					osc_queu_MSG_SEL_VAL( "/ostc/form/fx/shimmer" , formNr,		(bitRead(form_menu_shimmer[bit][_M_FORM_SHIMMER_RUN], 	real_formNr)));  		
					osc_queu_MSG_SEL_VAL( "/ostc/form/fx/shimblend",formNr,		(bitRead(form_menu_shimmer[bit][_M_FORM_SHIMMER_BLEND], 	real_formNr)));  		

					osc_queu_MSG_SEL_VAL( "/ostc/form/fx/layon" , 	formNr,		(bitRead(form_menu_fx1[bit][_M_FORM_FX1_RUN], 		real_formNr)));  		
				


	
			

			osc_queu_MSG_SEL_VAL("/ostc/form/fx/shim/level" , (formNr), form_fx_shim[formNr].level );
			osc_queu_MSG_int("/ostc/form/fx/shm_xs/" + String(formNr), form_fx_shim[formNr].xscale );
			osc_queu_MSG_int("/ostc/form/fx/shm_ys/" + String(formNr), form_fx_shim[formNr].yscale );
			osc_queu_MSG_int("/ostc/form/fx/shm_bt/" + String(formNr), form_fx_shim[formNr].beater );

			osc_queu_MSG_int("/ostc/form/mix/shi/" + String(formNr), form_fx_shim[formNr].mix_mode );
			osc_queu_MSG_int("/ostc/form/shi/pal/" + String(formNr), form_fx_shim[formNr].pal );
		
		}

} */


void osc_StC_menu_form_fx1_adv_ref()
{
	uint8_t bit = 0;	

		for (uint8_t formNr = 0; formNr < NR_FORM_PARTS; formNr++)
		{
			
				uint8_t real_formNr = formNr;
				 bit = 0;
				while (real_formNr >= 8 )
				{
					bit++;
					real_formNr = real_formNr - 8;	

				}
				

						//case _M_FX_MASK: 					osc_queu_MSG_SEL_VAL( "/ostc/form/fx/mask" , 	formNr,		(bitRead(form_menu[bit][_M_FX_MASK], 		real_formNr)));  	
						//case _M_FX_SUBTRACT: 				osc_queu_MSG_SEL_VAL( "/ostc/form/fx/pm" , 		formNr,		(bitRead(form_menu[bit][_M_FX_SUBTRACT], 	real_formNr)));  		
		
		
		osc_queu_MSG_int( "/ostc/form/fx/fx01/run/" +	String(formNr),		(bitRead(form_menu_fx1[bit][_M_FORM_FX1_RUN], 		real_formNr)));  	
		osc_queu_MSG_int( "/ostc/form/fx/fx01/mir/" +	String(formNr),		(bitRead(form_menu_fx1[bit][_M_FORM_FX1_MIRROR], 		real_formNr)));  	
		osc_queu_MSG_int( "/ostc/form/fx/fx01/rev/" +	String(formNr),		(bitRead(form_menu_fx1[bit][_M_FORM_FX1_REVERSED], 		real_formNr)));  	
 	
		osc_queu_MSG_int("/ostc/form/fx/fx01/lvl/" + String(formNr) , form_fx1[formNr].level );
		//osc_queu_MSG_int("/ostc/form/gv/" + String(formNr), form_fx_glitter[formNr].value );
		osc_queu_MSG_int("/ostc/form/fx/fade/lvl/" +	String(formNr) , form_fx1[formNr].fade );
		osc_queu_MSG_int("/ostc/form/fx/fx01/mix/" + String(formNr), form_fx1[formNr].mix_mode );

		}		

}


/*
void osc_StC_menu_form_leds_ref()
{
		//debugMe("inFormRef ostc");
	
		
		

		for (uint8_t formNr = 0; formNr < NR_FORM_PARTS; formNr++)
		{
			osc_queu_MSG_int("/ostc/form/sl/" + String(formNr), form_cfg[formNr].start_led );
			osc_queu_MSG_int("/ostc/form/nl/" + String(formNr), form_cfg[formNr].nr_leds );
		}

}
*/

void osc_StC_menu_form_pal_adv_ref()
{
		//debugMe("inFormRef ostc");
	
		
		uint8_t bit = 0;

		for (uint8_t formNr = 0; formNr < NR_FORM_PARTS; formNr++)
		{
			
				uint8_t real_formNr = formNr;
				 bit = 0;
				while (real_formNr >= 8 )
				{
					bit++;
					real_formNr = real_formNr - 8;	

				}

		osc_queu_MSG_int( "/ostc/form/pal/run/" + String(formNr), 	(bitRead(form_menu_pal[bit][_M_FORM_PAL_RUN], 	real_formNr)));		
		osc_queu_MSG_int( "/ostc/form/pal/ocl/" + String(formNr), 	(bitRead(form_menu_pal[bit][_M_FORM_PAL_ONECOLOR], 	real_formNr)));
		osc_queu_MSG_int( "/ostc/form/pal/rev/"   + String(formNr), 			(bitRead(form_menu_pal[bit][_M_FORM_PAL_REVERSED], 	real_formNr)));  		
		osc_queu_MSG_int( "/ostc/form/pal/mir/"  + String(formNr), 			(bitRead(form_menu_pal[bit][_M_FORM_PAL_MIRROR], 		real_formNr)));  		
		osc_queu_MSG_int( "/ostc/form/pal/bld/" + String(formNr), 			(bitRead(form_menu_pal[bit][_M_FORM_PAL_BLEND], 		real_formNr)));  	 
		osc_queu_MSG_int( "/ostc/form/pal/ifm/" + String(formNr), 	(bitRead(form_menu_pal[bit][_M_FORM_PAL_SPEED_FROM_FFT], 	real_formNr)));	
		
		osc_queu_MSG_int("/ostc/form/pal/mix/" + String(formNr), form_fx_pal[formNr].mix_mode );
		osc_queu_MSG_int("/ostc/form/pal/pal/" + String(formNr), form_fx_pal[formNr].pal );
		osc_queu_MSG_int("/ostc/form/pal/lvl/" + String(formNr), form_fx_pal[formNr].level );

		
		osc_queu_MSG_int("/ostc/form/pal/ald/" + String(formNr), form_fx_pal[formNr].index_add_led );
		osc_queu_MSG_int("/ostc/form/pal/afm/" + String(formNr), form_fx_pal[formNr].index_add_frame );
		osc_queu_MSG_int("/ostc/form/pal/sid/" + String(formNr), form_fx_pal[formNr].index_start );
		}		
} 
/*
void osc_StC_menu_form_FX_layers_ref()
{
	//debugMe("inFormRef ostc");
	
		
		uint8_t bit = 0;

		for (uint8_t formNr = 0; formNr < NR_FORM_PARTS; formNr++)
		{
				uint8_t real_formNr = formNr;
				 bit = 0;
				while (real_formNr >= 8 )
				{
					bit++;
					real_formNr = real_formNr - 8;	

				}
			
					//	osc_queu_MSG_SEL_VAL( "/ostc/form/fxL/run" , 	formNr,		(bitRead(form_menu_fx1[bit][_M_FORM_FX1_RUN], 		real_formNr)));  		

					osc_queu_MSG_int( "/ostc/form/fx/fire/run/" + formNr,		(bitRead(form_menu_fire[bit][_M_FORM_FIRE_RUN], 			real_formNr)));  	

						osc_queu_MSG_int( "/ostc/form/fx/shim/run/" + formNr,		(bitRead(form_menu_shimmer[bit][_M_FORM_SHIMMER_RUN], 	real_formNr)));  		

						//case _M_FX_3_SIN: 					osc_queu_MSG_SEL_VAL( "/ostc/form/fx/3sin" , 	formNr,		(bitRead(form_menu[bit][_M_FX_3_SIN], 		real_formNr)));  		break;
						//case _M_FX_SIN_PAL: 				osc_queu_MSG_SEL_VAL( "/ostc/form/fx/sinpal" , 	formNr,		(bitRead(form_menu[bit][_M_FX_SIN_PAL], 	real_formNr)));  		break;
						

			



		
			osc_queu_MSG_int("/ostc/form/fx/shim/lvl/" + formNr, form_fx_shim[formNr].level );
			osc_queu_MSG_int("/ostc/form/fx/fire/lvl/" +	formNr , form_fx_fire[formNr].level );
		
		

		}		
		osc_queu_MSG_int("/ostc/master/fireCool", 			led_cfg.fire_cooling );
		osc_queu_MSG_int("/ostc/master/fireSpark",			led_cfg.fire_sparking );
}
*/

void osc_StC_menu_form_level_ref()
{
	//debugMe("inFormRef ostc");
	
		
		uint8_t bit = 0;

		for (uint8_t formNr = 0; formNr < NR_FORM_PARTS; formNr++)
		{
		
				uint8_t real_formNr = formNr;
				 bit = 0;
				while (real_formNr >= 8 )
				{
					bit++;
					real_formNr = real_formNr - 8;	

				}
				osc_queu_MSG_int( "/ostc/form/fx/fx01/mix/" +	String(formNr), form_fx1[formNr].mix_mode );


				osc_queu_MSG_int("/ostc/form/fft/lvl/"+	String(formNr), form_fx_fft[formNr].level );
				osc_queu_MSG_int("/ostc/form/pal/lvl/"+	String(formNr), form_fx_pal[formNr].level );
				
				osc_queu_MSG_int( "/ostc/form/fx/fx01/lvl/" +	String(formNr), form_fx1[formNr].level );
				
				osc_queu_MSG_int( "/ostc/form/fx/fade/lvl/" +	String(formNr), form_fx1[formNr].fade );
				osc_queu_MSG_int( "/ostc/form/fx/shim/lvl/" + String(formNr),	 form_fx_shim[formNr].level);
				osc_queu_MSG_int( "/ostc/form/fx/glit/lvl/" + String(formNr),	 form_fx_glitter[formNr].level);
				osc_queu_MSG_int( "/ostc/form/fx/dott/lvl/" + String(formNr), form_fx_dots[formNr].level );
				osc_queu_MSG_int( "/ostc/form/fx/fire/lvl/" + String(formNr) , form_fx_fire[formNr].level );


		}
}






void osc_StC_menu_audio_ref()
{

		for(uint8_t bin = 0; bin< 7 ; bin++)
		{
			osc_queu_MSG_int("/ostc/audio/fbri/" +String(bin) ,  int(bitRead(fft_data_bri, 6-bin)  ));
			osc_queu_MSG_int("/ostc/audio/ffps/" +String(bin) ,  int(bitRead(fft_data_fps, 6-bin)  ));

			osc_queu_MSG_int("/ostc/audio/redd/"   +String(bin) ,  	int(bitRead(fft_menu[0], 6-bin) ));
			osc_queu_MSG_int("/ostc/audio/gren/" +String(bin) ,  	int(bitRead(fft_menu[1], 6-bin) ));
			osc_queu_MSG_int("/ostc/audio/blue/" +String(bin) ,  	int(bitRead(fft_menu[2], 6-bin) ));

			osc_queu_MSG_int("/ostc/audio/auto/" +String(bin) ,  	int(bitRead(fft_bin_autoTrigger, 6-bin)) );
			
			osc_queu_MSG_int(String("/ostc/audio/fxbin/00/" + String(bin)) ,  	int(bitRead(fft_fxbin[0].menu_select, 6-bin)) );
			osc_queu_MSG_int(String("/ostc/audio/fxbin/01/" + String(bin)) ,  	int(bitRead(fft_fxbin[1].menu_select, 6-bin)) );
			osc_queu_MSG_int(String("/ostc/audio/fxbin/02/" + String(bin)) ,  	int(bitRead(fft_fxbin[2].menu_select, 6-bin)) );
			osc_queu_MSG_int(String("/ostc/audio/fxbin/03/" + String(bin)) ,  	int(bitRead(fft_fxbin[3].menu_select, 6-bin)) );
			osc_queu_MSG_int(String("/ostc/audio/fxbin/04/" + String(bin)) ,  	int(bitRead(fft_fxbin[4].menu_select, 6-bin)) );
			osc_queu_MSG_int(String("/ostc/audio/fxbin/05/" + String(bin)) ,  	int(bitRead(fft_fxbin[5].menu_select, 6-bin)) );
			osc_queu_MSG_int(String("/ostc/audio/fxbin/06/" + String(bin)) ,  	int(bitRead(fft_fxbin[6].menu_select, 6-bin)) );
			osc_queu_MSG_int(String("/ostc/audio/fxbin/07/" + String(bin)) ,  	int(bitRead(fft_fxbin[5].menu_select, 6-bin)) );
			osc_queu_MSG_int(String("/ostc/audio/fxbin/08/" + String(bin)) ,  	int(bitRead(fft_fxbin[4].menu_select, 6-bin)) );
			osc_queu_MSG_int(String("/ostc/audio/fxbin/09/" + String(bin)) ,  	int(bitRead(fft_fxbin[9].menu_select, 6-bin)) );
			osc_queu_MSG_int(String("/ostc/audio/fxbin/10/" + String(bin)) ,  	int(bitRead(fft_fxbin[10].menu_select, 6-bin)) );
			osc_queu_MSG_int(String("/ostc/audio/fxbin/11/" + String(bin)) ,  	int(bitRead(fft_fxbin[11].menu_select, 6-bin)) );
			osc_queu_MSG_int(String("/ostc/audio/fxbin/12/" + String(bin)) ,  	int(bitRead(fft_fxbin[12].menu_select, 6-bin)) );
			osc_queu_MSG_int(String("/ostc/audio/fxbin/13/" + String(bin)) ,  	int(bitRead(fft_fxbin[13].menu_select, 6-bin)) );
			osc_queu_MSG_int(String("/ostc/audio/fxbin/14/" + String(bin)) ,  	int(bitRead(fft_fxbin[14].menu_select, 6-bin)) );
			osc_queu_MSG_int(String("/ostc/audio/fxbin/15/" + String(bin)) ,  	int(bitRead(fft_fxbin[15].menu_select, 6-bin)) );
			osc_queu_MSG_int(String("/ostc/audio/fxbin/16/" + String(bin)) ,  	int(bitRead(fft_fxbin[16].menu_select, 6-bin)) );
			osc_queu_MSG_int(String("/ostc/audio/fxbin/17/" + String(bin)) ,  	int(bitRead(fft_fxbin[17].menu_select, 6-bin)) );
			osc_queu_MSG_int(String("/ostc/audio/fxbin/18/" + String(bin)) ,  	int(bitRead(fft_fxbin[18].menu_select, 6-bin)) );
			osc_queu_MSG_int(String("/ostc/audio/fxbin/19/" + String(bin)) ,  	int(bitRead(fft_fxbin[19].menu_select, 6-bin)) );


		
		}		
			osc_queu_MSG_int("/ostc/audio/minauto" ,  	fft_led_cfg.fftAutoMin );
			osc_queu_MSG_int("/ostc/audio/maxauto" ,  	fft_led_cfg.fftAutoMax );
			osc_queu_MSG_int("/ostc/audio/fftviz" ,  	int(get_bool(FFT_OSTC_VIZ)) );
			osc_queu_MSG_int("/ostc/audio/vizfps" ,  	fft_led_cfg.viz_fps );

			osc_StC_FFT_vizIt();

}	



void osc_oStC_menu_master_wifi_ref()
{
		//debugMe("Wifi conf ref");

		OSCBundle bundle_out;
		IPAddress ip_out(osc_server.remoteIP());



		osc_queu_MSG_int("/ostc/master/wifi/mode", 	get_bool(WIFI_MODE) );
		osc_queu_MSG_int("/ostc/master/wifi/dhcp", 	get_bool(STATIC_IP_ENABLED) );
		osc_queu_MSG_int("/ostc/master/wifi/power", get_bool(WIFI_POWER) );

		osc_queu_MSG_int("/ostc/master/wifi/http", 		get_bool(HTTP_ENABLED)) ;
		osc_queu_MSG_int("/ostc/master/wifi/serial", 	get_bool(DEBUG_OUT)) ;
		osc_queu_MSG_int("/ostc/master/wifi/telnet", 	get_bool(DEBUG_TELNET)) ;


		bundle_out.add("/ostc/master/wifi/name").add(wifi_cfg.APname);
		bundle_out.add("/ostc/master/wifi/appwd").add(wifi_cfg.APpassword);
		bundle_out.add("/ostc/master/wifi/pwd").add(wifi_cfg.pwd);
		bundle_out.add("/ostc/master/wifi/ssid").add(wifi_cfg.ssid);
		bundle_out.add("/ostc/master/wifi/ntp").add(wifi_cfg.ntp_fqdn);


		osc_server.beginPacket(ip_out, OSC_OUTPORT);
		bundle_out.send(osc_server);
		osc_server.endPacket();
		bundle_out.empty();

		

		osc_queu_MSG_int("/ostc/master/wifi/ip/0", 	wifi_cfg.ipStaticLocal[0]) ;
		osc_queu_MSG_int("/ostc/master/wifi/ip/1", 	wifi_cfg.ipStaticLocal[1]) ;
		osc_queu_MSG_int("/ostc/master/wifi/ip/2", 	wifi_cfg.ipStaticLocal[2]) ;
		osc_queu_MSG_int("/ostc/master/wifi/ip/3", 	wifi_cfg.ipStaticLocal[3]) ;

		osc_queu_MSG_int("/ostc/master/wifi/sn/0", 	wifi_cfg.ipSubnet[0]) ;
		osc_queu_MSG_int("/ostc/master/wifi/sn/1", 	wifi_cfg.ipSubnet[1]) ;
		osc_queu_MSG_int("/ostc/master/wifi/sn/2", 	wifi_cfg.ipSubnet[2]) ;
		osc_queu_MSG_int("/ostc/master/wifi/sn/3", 	wifi_cfg.ipSubnet[3]) ;

		osc_queu_MSG_int("/ostc/master/wifi/gw/0", 	wifi_cfg.ipDGW[0]) ;
		osc_queu_MSG_int("/ostc/master/wifi/gw/1", 	wifi_cfg.ipDGW[1]) ;
		osc_queu_MSG_int("/ostc/master/wifi/gw/2", 	wifi_cfg.ipDGW[2]) ;
		osc_queu_MSG_int("/ostc/master/wifi/gw/3", 	wifi_cfg.ipDGW[3]) ;

		osc_queu_MSG_int("/ostc/master/wifi/dns/0", 	wifi_cfg.ipDNS[0]) ;
		osc_queu_MSG_int("/ostc/master/wifi/dns/1", 	wifi_cfg.ipDNS[1]) ;
		osc_queu_MSG_int("/ostc/master/wifi/dns/2", 	wifi_cfg.ipDNS[2]) ;
		osc_queu_MSG_int("/ostc/master/wifi/dns/3", 	wifi_cfg.ipDNS[3]) ;


	

}	


void osc_oStC_menu_master_mqtt_ref()
{
		osc_queu_MSG_int("/ostc/master/mqtt/enable", (get_bool(MQTT_ON)));

		osc_queu_MSG_int("/ostc/master/mqtt/ip/0", 	mqtt_cfg.mqttIP[0]) ;
		osc_queu_MSG_int("/ostc/master/mqtt/ip/1", 	mqtt_cfg.mqttIP[1]) ;
		osc_queu_MSG_int("/ostc/master/mqtt/ip/2", 	mqtt_cfg.mqttIP[2]) ;
		osc_queu_MSG_int("/ostc/master/mqtt/ip/3", 	mqtt_cfg.mqttIP[3]) ;
		osc_queu_MSG_int("/ostc/master/mqtt/ip/4", 	mqtt_cfg.mqttPort );
		osc_queu_MSG_int("/ostc/master/mqtt/ip/5", 	mqtt_cfg.publishSec );
		OSCBundle bundle_out;
		IPAddress ip_out(osc_server.remoteIP());

		bundle_out.add("/ostc/master/mqtt/uname").add(mqtt_cfg.username);
		bundle_out.add("/ostc/master/mqtt/passwd").add(mqtt_cfg.password);
		
		osc_server.beginPacket(ip_out , OSC_OUTPORT);
		bundle_out.send(osc_server);
		osc_server.endPacket();
		bundle_out.empty();

}

void osc_StC_menu_master_artnet_ref()
{
	osc_queu_MSG_int("/ostc/master/artnet/on", 		get_bool(ARTNET_SEND)); 
	osc_queu_MSG_int("/ostc/master/artnet/rc", 		get_bool(ARTNET_RECIVE)); 
	osc_queu_MSG_int("/ostc/master/artnet/su", 		artnet_cfg.startU);
	osc_queu_MSG_int("/ostc/master/artnet/nu", 		artnet_cfg.numU);
	
		uint8_t  universeCounter = 0;
	for (uint8_t node = 0; node < ARTNET_NR_NODES_TPM ; node++)
	{
		//debugMe("in"); debugMe(ARTNET_NR_NODES_TPM); debugMe("in");debugMe(String(node));
		osc_queu_MSG_int("/ostc/master/artnet/node/"+ String(node+1) +  "/0", 	artnetNode[node].startU); 
		osc_queu_MSG_int("/ostc/master/artnet/node/"+ String(node+1) +  "/1", 	artnetNode[node].numU); 
		osc_queu_MSG_int("/ostc/master/artnet/node/"+ String(node+1) +  "/2", 	artnetNode[node].IP[0]); 
		osc_queu_MSG_int("/ostc/master/artnet/node/"+ String(node+1) +  "/3", 	artnetNode[node].IP[1]); 
		osc_queu_MSG_int("/ostc/master/artnet/node/"+ String(node+1) +  "/4", 	artnetNode[node].IP[2]); 
		osc_queu_MSG_int("/ostc/master/artnet/node/"+ String(node+1) +  "/5", 	artnetNode[node].IP[3]); 
		
		osc_queu_MSG_int("/ostc/master/artnet/nodeSL/"+ String(node+1), (universeCounter * 170) 	) ; 
		universeCounter = universeCounter + artnetNode[node].numU;  

	}

}
void osc_StC_menu_master_ledcfg_ref()
{

	osc_queu_MSG_int("/ostc/master/data/sl/1", 		led_cfg.DataStart_leds[0] );
	osc_queu_MSG_int("/ostc/master/data/nl/1", 		led_cfg.DataNR_leds[0]);
	osc_queu_MSG_int("/ostc/master/data/sl/2", 		led_cfg.DataStart_leds[1] );
	osc_queu_MSG_int("/ostc/master/data/nl/2", 		led_cfg.DataNR_leds[1]);
	osc_queu_MSG_int("/ostc/master/data/sl/3", 		led_cfg.DataStart_leds[2] );
	osc_queu_MSG_int("/ostc/master/data/nl/3", 		led_cfg.DataNR_leds[2]);
	osc_queu_MSG_int("/ostc/master/data/sl/4", 		led_cfg.DataStart_leds[3] );
	osc_queu_MSG_int("/ostc/master/data/nl/4", 		led_cfg.DataNR_leds[3]);
	osc_queu_MSG_int("/ostc/master/data/nl/0", 		led_cfg.NrLeds);   		// Mirror mode NR of leds


	osc_queu_MSG_int("/ostc/master/data/mode", 		led_cfg.ledMode);
	osc_queu_MSG_int("/ostc/master/data/aparate", 	led_cfg.apa102data_rate);
	osc_queu_MSG_int("/ostc/master/data/select/1", 	get_bool(DATA1_ENABLE));
	osc_queu_MSG_int("/ostc/master/data/select/2", 	get_bool(DATA2_ENABLE));
	osc_queu_MSG_int("/ostc/master/data/select/3", 	get_bool(DATA3_ENABLE));
	osc_queu_MSG_int("/ostc/master/data/select/4", 	get_bool(DATA4_ENABLE));
	osc_queu_MSG_int("/ostc/master/pots",      		get_bool(POT_DISABLE));









}

void osc_StC_menu_master_ref()
{
	osc_queu_MSG_int("/ostc/master/bri", 		led_cfg.bri) ; //float(led_cfg.bri) / float(led_cfg.max_bri) );
	osc_queu_MSG_int("/ostc/master/r", 			led_cfg.r);
	osc_queu_MSG_int("/ostc/master/g", 			led_cfg.g);
	osc_queu_MSG_int("/ostc/master/b", 			led_cfg.b);
	osc_queu_MSG_int("/ostc/master/palbri", 	led_cfg.pal_bri);
	osc_queu_MSG_int("/ostc/master/fps", 		led_cfg.pal_fps);
	osc_queu_MSG_int("/ostc/blend", 			(get_bool(BLEND_INVERT))); 
	osc_queu_MSG_int("/ostc/master/seq", 		(get_bool(SEQUENCER_ON))); 
	osc_queu_MSG_float("/ostc/heap", float(ESP.getFreeHeap()));
	

	
	


	osc_queu_MSG_int("/ostc/master/data/maxbri",  led_cfg.max_bri );
	osc_queu_MSG_int("/ostc/master/playnr", 	led_cfg.Play_Nr);

	osc_queu_MSG_int("/ostc/audio/rfps", 		LEDS_get_FPS());
	osc_queu_MSG_int("/ostc/audio/rbri", 		LEDS_get_real_bri()); 

	for (uint8_t layer = 0 ; layer < MAX_LAYERS_SELECT ; layer++)
	{
			osc_queu_MSG_int("/ostc/master/laye/" + String(layer) , 	layer_select[layer]	); 
			
	}
	
	//yield();


}


void osc_StC_menu_pal_ref(uint8_t pal) 
{
	// OSC MESSAGE OUT :/pal/?/?/1-3


	//byte outvalue = 0;

	
		for (int i = 0; i < 16; i++) 
		{
				//osc_send_MSG_rgb( String("/ostc/pal/"+ String(pal) + "/" + String(i) ) ,  LEDS_pal_read(pal,i,0), LEDS_pal_read(pal,i,1), LEDS_pal_read(pal,i,2));
				osc_queu_MSG_rgb( String("/ostc/pal/1/" + String(i) ) ,  LEDS_pal_read(pal,i,0), LEDS_pal_read(pal,i,1), LEDS_pal_read(pal,i,2) );	
		}
		
	//}
	osc_queu_MSG_int("/ostc/pal/edit/edit", led_cfg.edit_pal); 
}



void osc_StC_menu_master_loadsave_ref()
{


	for(uint8_t confNr = 0; confNr < MAX_NR_SAVES ; confNr++)  		// update leds to show what confs are saved
	{
		//debugMe(confNr);
		if(FS_check_Conf_Available(confNr) == false)
			{
				osc_queu_MSG_rgb(String("/ostc/master/conf/l/"+String(confNr)  ), uint8_t(255),uint8_t(0),uint8_t(0));
				//debugMe("in red");
				//yield();
			}
			else
			{
				osc_queu_MSG_rgb(String("/ostc/master/conf/l/"+String(confNr)  ), uint8_t(0),uint8_t(255),uint8_t(0));
				//debugMe("ingreen");
			 //	yield();
			}


			osc_queu_MSG_int(String("/ostc/master/tmin/"+String(confNr)  ),  int(play_conf_time_min[confNr])    );
			osc_queu_MSG_int(String("/ostc/master/auto/"+String(confNr) ),    LEDS_get_sequencer(confNr)   );

			
	}
	



}



//////////////////////////////////////////////////////////////////////////////////////////////////
////////////// END REFRESH FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////////////////
/////////////// 







//////////////////////////////////////////////////////////////////////////////////////////////////
///////////////   OSTC IN FUNCTIONS 
// Start  Open Stage Controll "IN" Functions 





/////////// AUDIO  IN 

void osc_StC_audio_trig_in(OSCMessage &msg, int addrOffset)
{
	String bin_no_string;		// form  NR
			
	char address[5];

	uint8_t bin_no = 0;						// form NR in uint8_t


	memset(address, 0, sizeof(address));
	msg.getAddress(address, addrOffset + 1);

	for (byte i = 0; i < sizeof(address); i++) 
	{
			bin_no_string = bin_no_string + address[i];

	}

	 bin_no = 6 - bin_no_string.toInt();  // flipped so 6-bin no.

	fft_data[bin_no].trigger = constrain(uint8_t(msg.getInt(0)), 0,255); 

}

void osc_StC_audio_routing(OSCMessage &msg, int addrOffset)
{
	char address[13] ;
	memset(address, 0, sizeof(address));
	msg.getAddress(address, addrOffset );
	

	 if ( address[6]  == '/' )
	{
			String form_no_string;
			memset(address, 0, sizeof(address));
			msg.getAddress(address, addrOffset +10 );
			 

			for (byte i = 0; i < sizeof(address); i++)  { form_no_string = form_no_string + address[i]; }
			uint8_t i_orig_bin_nr =  form_no_string.toInt();
				
			//debugMe(i_orig_bin_nr);	
			
			if			(msg.match("/fxbin/00",addrOffset))		{ bitWrite(fft_fxbin[0].menu_select, 	6-i_orig_bin_nr, bool(msg.getInt(0))); }
			else if		(msg.match("/fxbin/01",addrOffset))		{ bitWrite(fft_fxbin[1].menu_select, 	6-i_orig_bin_nr, bool(msg.getInt(0))); }
			else if		(msg.match("/fxbin/02",addrOffset))		{ bitWrite(fft_fxbin[2].menu_select, 	6-i_orig_bin_nr, bool(msg.getInt(0))); }
			else if		(msg.match("/fxbin/03",addrOffset))		{ bitWrite(fft_fxbin[3].menu_select, 	6-i_orig_bin_nr, bool(msg.getInt(0))); }
			else if		(msg.match("/fxbin/04",addrOffset))		{ bitWrite(fft_fxbin[4].menu_select, 	6-i_orig_bin_nr, bool(msg.getInt(0))); }
			else if		(msg.match("/fxbin/05",addrOffset))		{ bitWrite(fft_fxbin[5].menu_select, 	6-i_orig_bin_nr, bool(msg.getInt(0))); }	
			else if		(msg.match("/fxbin/06",addrOffset))		{ bitWrite(fft_fxbin[6].menu_select, 	6-i_orig_bin_nr, bool(msg.getInt(0))); }	
			else if		(msg.match("/fxbin/07",addrOffset))		{ bitWrite(fft_fxbin[7].menu_select, 	6-i_orig_bin_nr, bool(msg.getInt(0))); }	
			else if		(msg.match("/fxbin/08",addrOffset))		{ bitWrite(fft_fxbin[8].menu_select, 	6-i_orig_bin_nr, bool(msg.getInt(0))); }	
			else if		(msg.match("/fxbin/09",addrOffset))		{ bitWrite(fft_fxbin[9].menu_select, 	6-i_orig_bin_nr, bool(msg.getInt(0))); }	
			else if		(msg.match("/fxbin/10",addrOffset))		{ bitWrite(fft_fxbin[10].menu_select, 	6-i_orig_bin_nr, bool(msg.getInt(0))); }	
			else if		(msg.match("/fxbin/11",addrOffset))		{ bitWrite(fft_fxbin[11].menu_select, 	6-i_orig_bin_nr, bool(msg.getInt(0))); }	
			else if		(msg.match("/fxbin/12",addrOffset))		{ bitWrite(fft_fxbin[12].menu_select, 	6-i_orig_bin_nr, bool(msg.getInt(0))); }	
			else if		(msg.match("/fxbin/13",addrOffset))		{ bitWrite(fft_fxbin[13].menu_select, 	6-i_orig_bin_nr, bool(msg.getInt(0))); }	
			else if		(msg.match("/fxbin/14",addrOffset))		{ bitWrite(fft_fxbin[14].menu_select, 	6-i_orig_bin_nr, bool(msg.getInt(0))); }	
			else if		(msg.match("/fxbin/15",addrOffset))		{ bitWrite(fft_fxbin[15].menu_select, 	6-i_orig_bin_nr, bool(msg.getInt(0))); }	
			else if		(msg.match("/fxbin/16",addrOffset))		{ bitWrite(fft_fxbin[16].menu_select, 	6-i_orig_bin_nr, bool(msg.getInt(0))); }	
			else if		(msg.match("/fxbin/17",addrOffset))		{ bitWrite(fft_fxbin[17].menu_select, 	6-i_orig_bin_nr, bool(msg.getInt(0))); }	
			else if		(msg.match("/fxbin/18",addrOffset))		{ bitWrite(fft_fxbin[18].menu_select, 	6-i_orig_bin_nr, bool(msg.getInt(0))); }	
			else if		(msg.match("/fxbin/19",addrOffset))		{ bitWrite(fft_fxbin[19].menu_select, 	6-i_orig_bin_nr, bool(msg.getInt(0))); }	
			
			else if 	(msg.match("/fxbin/vl",addrOffset))		fft_fxbin[i_orig_bin_nr].set_val 	=   uint8_t(msg.getInt(0));
			else if 	(msg.match("/fxbin/tg",addrOffset))		fft_fxbin[i_orig_bin_nr].trrig_val	=   uint8_t(msg.getInt(0));
			
	}
	else 
	if 	( address[5]  == '/' )
		
	/*(msg.match("/redd",addrOffset))
	||		(msg.match("/gren",addrOffset))
	||		(msg.match("/blue",addrOffset))
	||		(msg.match("/fbri",addrOffset))
	||		(msg.match("/ffps",addrOffset))
	||		(msg.match("/auto",addrOffset))
	||		(msg.match("/fd/0",addrOffset))
	||		(msg.match("/fd/1",addrOffset))
	||		(msg.match("/fd/2",addrOffset))
	||		(msg.match("/fd/3",addrOffset)) 
	||		(msg.match("/fd/4",addrOffset))
	||		(msg.match("/fd/5",addrOffset))
	||		(msg.match("/f/tr",addrOffset))
	||		(msg.match("/f/vl",addrOffset)) */
	//)
	{
			String form_no_string;
			memset(address, 0, sizeof(address));
			msg.getAddress(address, addrOffset +6 );
			

			for (byte i = 0; i < sizeof(address); i++)  { form_no_string = form_no_string + address[i]; }
			
				uint8_t i_orig_bin_nr =  form_no_string.toInt(); 
				
			
			//debugMe(i_orig_bin_nr);


			if 			(msg.match("/redd",addrOffset))		{ bitWrite(fft_menu[0], 		6-i_orig_bin_nr, bool(msg.getInt(0))); }
			else if		(msg.match("/gren",addrOffset))		{ bitWrite(fft_menu[1], 		6-i_orig_bin_nr, bool(msg.getInt(0))); }
			else if		(msg.match("/blue",addrOffset))		{ bitWrite(fft_menu[2], 		6-i_orig_bin_nr, bool(msg.getInt(0))); }
			else if		(msg.match("/fbri",addrOffset))		{ bitWrite(fft_data_bri, 		6-i_orig_bin_nr, bool(msg.getInt(0))); }
			else if		(msg.match("/ffps",addrOffset))		{ bitWrite(fft_data_fps, 		6-i_orig_bin_nr, bool(msg.getInt(0))); }
			else if		(msg.match("/auto",addrOffset))		{ bitWrite(fft_bin_autoTrigger, 6-i_orig_bin_nr, bool(msg.getInt(0))); }
			else if 	(msg.match("/trig",addrOffset)) 	fft_data[6-i_orig_bin_nr].trigger = msg.getInt(0);
	}
	
	
	else if		(msg.fullMatch("/ref",addrOffset))			{osc_StC_menu_audio_ref();} 
	else if		(msg.fullMatch("/minauto",addrOffset))		{ fft_led_cfg.fftAutoMin = uint8_t(msg.getInt(0));}
	else if		(msg.fullMatch("/maxauto",addrOffset))		{ fft_led_cfg.fftAutoMax = uint8_t(msg.getInt(0));}

	else if		(msg.fullMatch("/fftviz",addrOffset))		{write_bool(FFT_OSTC_VIZ,	bool(msg.getInt(0))); }
	else if		(msg.fullMatch("/vizfps",addrOffset))		{ fft_led_cfg.viz_fps  =  constrain(uint8_t(msg.getInt(0)) , 1 , VIZ_FPS_MAX)  ;}
	

	

	
	//debugMe("in : " +String(bool(msg.getInt(0))));


}





////////////// FORM IN

void osc_StC_form_routing(OSCMessage &msg, int addrOffset)
{

		//msg.route("/pal", osc_StC_form_pal_routing, addrOffset);

	//uint8_t orig_form_nr = uint8_t(msg.getInt(0));
	//uint8_t form_nr =  orig_form_nr;
	//uint8_t bit_int = 0;
	/*
	while (form_nr >=8)
	{
		bit_int++;
		form_nr = form_nr-8;
	}
	*/
	
		if		(msg.fullMatch("/menu/ref/main",addrOffset))							
		{
			switch(msg.getInt(0))
			{
				case 0: osc_StC_menu_form_play_ref(); break;
				case 1: osc_StC_menu_form_level_ref(); break; 
				//case 2: osc_StC_menu_form_FX_layers_ref();break;

			}
			
			
		}
		else if		(msg.fullMatch("/menu/ref/adv0",addrOffset))							
		{
			switch(msg.getInt(0))
			{
				case 0: osc_StC_menu_form_pal_adv_ref();  break;
				case 1: osc_StC_menu_form_fft_adv_ref(); break; 
				case 2: osc_StC_menu_form_fx_fire_adv_ref(); break; 
				case 3: osc_StC_menu_form_shim_adv_ref(); break;
				case 4: {osc_StC_menu_form_fx1_adv_ref(); osc_StC_menu_form_glit_adv_ref(); osc_StC_menu_form_dot_adv_ref(); }break; 
				//case 2: osc_StC_menu_form_FX_layers_ref();break;

			}
			
			
		}

		else if  	(msg.fullMatch("/play/ref",addrOffset)	&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_play_ref();}
		else if  	(msg.fullMatch("/level/ref",addrOffset)	&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_level_ref();}

		else if  	(msg.fullMatch("/pal/adv",addrOffset) 	&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_pal_adv_ref();}
		else if  	(msg.fullMatch("/fft/adv",addrOffset)	&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_fft_adv_ref();}
		else if  	(msg.fullMatch("/leds/adv",addrOffset)	&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_led_adv_ref();}
		else if  	(msg.fullMatch("/fx/fire/adv",addrOffset)	&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_fx_fire_adv_ref();}
		else if  	(msg.fullMatch("/fx/shim/adv",addrOffset)	&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_shim_adv_ref();}
		else if  	(msg.fullMatch("/fx/glit/adv",addrOffset)	&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_glit_adv_ref();}
		else if  	(msg.fullMatch("/fx/dott/adv",addrOffset)	&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_dot_adv_ref();}
	
		else if 	(msg.match("/fx",addrOffset))	
		{
										//debugMe("ohh");

			char address[5] ;
			String form_no_string;
			memset(address, 0, sizeof(address));
			msg.getAddress(address, addrOffset + 13);

			for (byte i = 0; i < sizeof(address); i++)  { form_no_string = form_no_string + address[i]; }
			
				uint8_t orig_form_nr =  form_no_string.toInt(); 
				uint8_t i_form_nr =  orig_form_nr;
				uint8_t i_bit_int = 0;
				while (i_form_nr >=8)
				{
					i_bit_int++;
					i_form_nr = i_form_nr-8;
				}

			//else if  	(msg.match("/mix/fx1",addrOffset))  	form_fx1[sel_form_no].mix_mode = uint8_t(msg.getInt(0))	;

			//if				(msg.match("/fx/dott/saw",addrOffset))			{ bitWrite(form_menu_dot[i_bit_int][_M_FORM_DOT_SAW], 	i_form_nr, 	bool(msg.getInt(0)));  ;}
			//else if		(msg.match("/fx/dott/jug",addrOffset))			{ bitWrite(form_menu_dot[i_bit_int][_M_FORM_DOT_SINE], 		i_form_nr,	bool(msg.getInt(0)));  ;}
			//else if		(msg.match("/fx/dott/fft",addrOffset))			{ bitWrite(form_menu_dot[i_bit_int][_M_FORM_DOT_FFT], 	i_form_nr,	bool(msg.getInt(0)));  ;}
			if  	(msg.match("/fx/dott/num",addrOffset))  				form_fx_dots[orig_form_nr].nr_dots = uint8_t(msg.getInt(0))	;
			else if  	(msg.match("/fx/dott/bpm",addrOffset))  		form_fx_dots[orig_form_nr].speed = uint8_t(msg.getInt(0))	;
			else if  	(msg.match("/fx/dott/pbm",addrOffset))  		form_fx_dots[orig_form_nr].index_add = uint8_t(msg.getInt(0))	;
			else if  	(msg.match("/fx/dott/lvl",addrOffset))  		form_fx_dots[orig_form_nr].level = uint8_t(msg.getInt(0))	;
			//else if  	(msg.match("/fx/dott/mix",addrOffset))  		form_fx_dots[orig_form_nr].mix_mode = uint8_t(msg.getInt(0))	;
			else if  	(msg.match("/fx/dott/pal",addrOffset))  		form_fx_dots[orig_form_nr].pal = uint8_t(msg.getInt(0))	;
			else if  	(msg.match("/fx/dott/typ",addrOffset)) 			{ bitWrite(form_menu_dot[i_bit_int][_M_FORM_DOT_TYPE], 				i_form_nr, bool(msg.getInt(0)));  }
			else if		(msg.match("/fx/dott/run",addrOffset))			{ bitWrite(form_menu_dot[i_bit_int][_M_FORM_DOT_RUN], 				i_form_nr, bool(msg.getInt(0)));  }
			else if		(msg.match("/fx/glit/run",addrOffset))			{ bitWrite(form_menu_glitter[i_bit_int][_M_FORM_GLITTER_RUN], 				i_form_nr, bool(msg.getInt(0)));  }
			else if		(msg.match("/fx/glit/gdb",addrOffset))			{ bitWrite(form_menu_glitter[i_bit_int][_M_FORM_GLITTER_FFT], 				i_form_nr, bool(msg.getInt(0)));  }
			//else if		(msg.match("/fx/glit/fft",addrOffset))			{ bitWrite(form_menu_glitter[i_bit_int][_M_FORM_GLITTER_FFT], 	i_form_nr, bool(msg.getInt(0)));  ;}
			else if		(msg.match("/fx/glit/pal",addrOffset))			{ form_fx_glitter[orig_form_nr].pal  =  uint8_t(msg.getInt(0))  ;}
			else if		(msg.match("/fx/glit/lvl",addrOffset))			{ form_fx_glitter[orig_form_nr].level  =  uint8_t(msg.getInt(0))  ;}
			//else if		(msg.match("/fx/glit/mix",addrOffset))			{ form_fx_glitter[orig_form_nr].mix_mode  =  uint8_t(msg.getInt(0))  ;}
			else if  	(msg.match("/fx/glit/val",addrOffset))  		form_fx_glitter[orig_form_nr].value = uint8_t(msg.getInt(0))	; 


			else if		(msg.match("/fx/fade/lvl",addrOffset))			{  form_fx1[orig_form_nr].fade  =  uint8_t(msg.getInt(0))  ;}
			else if		(msg.match("/fx/fx01/lvl",addrOffset))			{  form_fx1[orig_form_nr].level  =  uint8_t(msg.getInt(0))  ;}
			else if		(msg.match("/fx/fx01/mix",addrOffset))			{  form_fx1[orig_form_nr].mix_mode  =  uint8_t(msg.getInt(0))  ;}
			else if		(msg.match("/fx/fx01/run",addrOffset))			{ bitWrite(form_menu_fx1[i_bit_int][_M_FORM_FX1_RUN], 			i_form_nr, bool(msg.getInt(0)));  }
			else if		(msg.match("/fx/fx01/mir",addrOffset))			{ bitWrite(form_menu_fx1[i_bit_int][_M_FORM_FX1_MIRROR], 			i_form_nr, bool(msg.getInt(0)));  }
			else if		(msg.match("/fx/fx01/rev",addrOffset))			{ bitWrite(form_menu_fx1[i_bit_int][_M_FORM_FX1_REVERSED], 			i_form_nr, bool(msg.getInt(0)));  }


			else if		(msg.match("/fx/fire/run",addrOffset))			{ bitWrite(form_menu_fire[i_bit_int][_M_FORM_FIRE_RUN], 			i_form_nr, 	bool(msg.getInt(0)));  }
			else if		(msg.match("/fx/fire/rev",addrOffset))			{ bitWrite(form_menu_fire[i_bit_int][_M_FORM_FIRE_REVERSED], 		i_form_nr,	bool(msg.getInt(0)));  }
			else if		(msg.match("/fx/fire/mir",addrOffset))			{ bitWrite(form_menu_fire[i_bit_int][_M_FORM_FIRE_MIRROR], 		i_form_nr,	bool(msg.getInt(0)));  }
			else if		(msg.match("/fx/fire/lvl",addrOffset))	  		form_fx_fire[orig_form_nr].level  =  uint8_t(msg.getInt(0))  ;
			else if  	(msg.match("/fx/fire/mix",addrOffset))  		form_fx_fire[orig_form_nr].mix_mode = uint8_t(msg.getInt(0))	;
			else if  	(msg.match("/fx/fire/pal",addrOffset))  		form_fx_fire[orig_form_nr].pal = uint8_t(msg.getInt(0))	;
			else if  	(msg.match("/fx/fire/col",addrOffset))  		form_fx_fire[orig_form_nr].cooling = uint8_t(msg.getInt(0))	;
			else if  	(msg.match("/fx/fire/spk",addrOffset))  		form_fx_fire[orig_form_nr].sparking = uint8_t(msg.getInt(0))	;

			else if		(msg.match("/fx/shim/run",addrOffset))			{ bitWrite(form_menu_shimmer[i_bit_int][_M_FORM_SHIMMER_RUN], i_form_nr,	bool(msg.getInt(0)));  }
			else if		(msg.match("/fx/shim/bld",addrOffset))			{ bitWrite(form_menu_shimmer[i_bit_int][_M_FORM_SHIMMER_BLEND], i_form_nr,	bool(msg.getInt(0)));  }
			else if  	(msg.match("/fx/shim/x_s",addrOffset))  		form_fx_shim[orig_form_nr].xscale = uint8_t(msg.getInt(0))	;
			else if  	(msg.match("/fx/shim/y_s",addrOffset))  		form_fx_shim[orig_form_nr].yscale = uint8_t(msg.getInt(0))	;
			else if  	(msg.match("/fx/shim/bet",addrOffset))  		form_fx_shim[orig_form_nr].beater = uint8_t(msg.getInt(0))	; 
			else if		(msg.match("/fx/shim/lvl",addrOffset))			form_fx_shim[orig_form_nr].level  =  uint8_t(msg.getInt(0))  ;
			else if  	(msg.match("/fx/shim/mix",addrOffset))  		form_fx_shim[orig_form_nr].mix_mode = uint8_t(msg.getInt(0))	;
			else if  	(msg.match("/fx/shim/pal",addrOffset))  		form_fx_shim[orig_form_nr].pal = uint8_t(msg.getInt(0))	;
		}  //   /FX

		//else if		(msg.fullMatch("/fx/3sin",addrOffset))				{ bitWrite(form_menu[bit_int][_M_FX_3_SIN], form_nr,	bool(msg.getInt(0)));  ;}
		//else if		(msg.fullMatch("/fx/2sin",addrOffset))				{ bitWrite(form_menu[bit_int][_M_FX_2_SIN], form_nr,	bool(msg.getInt(0)));  ;}
		//else if		(msg.fullMatch("/fx/sinpal",addrOffset))			{ bitWrite(form_menu[bit_int][_M_FX_SHIM_PAL], form_nr,	bool(msg.getInt(0)));  ;}


		else if		(msg.fullMatch("/fx/tester/0",addrOffset))			{  form_fx_test.val_0 =  uint8_t(msg.getInt(0))  ;}
		else if		(msg.fullMatch("/fx/tester/1",addrOffset))			{  form_fx_test.val_1 =  uint8_t(msg.getInt(0))  ;}
		else if		(msg.fullMatch("/fx/tester/2",addrOffset))			{  form_fx_test.val_2 =  uint8_t(msg.getInt(0))  ;}


		// Palette input 
		else 
		{
			char address[13] ;
			memset(address, 0, sizeof(address));
			msg.getAddress(address, addrOffset);
			if(address[4] == '/')	
			/*
			if		((msg.match("run",addrOffset+5))		
				|| 		(msg.match("ocl",addrOffset+5))	
				|| 	  (msg.match("mir",addrOffset+5))	
				|| 		(msg.match("rev",addrOffset+5))		
				|| 	  (msg.match("bld",addrOffset+5))
				|| 	  (msg.match("lvl",addrOffset+5))	
				|| 	  (msg.match("mix",addrOffset+5))	

				|| 	  (msg.match("sld",addrOffset+5))	
				|| 	  (msg.match("nld",addrOffset+5))	
				|| 	  (msg.match("csd",addrOffset+5))
				|| 	  (msg.match("ald",addrOffset+5))	
				|| 	  (msg.match("afm",addrOffset+5))	
				|| 	  (msg.match("sid",addrOffset+5))	
				|| 	  (msg.match("val",addrOffset+5))	
				|| 	  (msg.match("ofs",addrOffset+5))	
				|| 	  (msg.match("pal",addrOffset+5))	
				|| 	  (msg.match("ifm",addrOffset+5))	
				)*/
				{
						//char address[5] ;
						String form_no_string;
						memset(address, 0, sizeof(address));
						msg.getAddress(address, addrOffset + 9);

						for (byte i = 0; i < sizeof(address); i++)  { form_no_string = form_no_string + address[i]; }
						
							uint8_t orig_form_nr =  form_no_string.toInt(); 
							uint8_t i_form_nr =  orig_form_nr;
							uint8_t i_bit_int = 0;
							while (i_form_nr >=8)
							{
								i_bit_int++;
								i_form_nr = i_form_nr-8;
							}

						
						uint8_t result = msg.getInt(0);
						//debugMe(i_form_nr);
						//debugMe(i_bit_int);
						//debugMe(result);
						

						

						//Form Pallete input
						if  			(msg.match("/sys/sld",addrOffset))  	form_cfg[orig_form_nr].start_led = uint16_t(msg.getInt(0))	;
						else if  	(msg.match("/sys/nld",addrOffset))  	form_cfg[orig_form_nr].nr_leds = uint16_t(msg.getInt(0))	;
						else if  	(msg.match("/sys/csd",addrOffset) && orig_form_nr > 0 )  	{form_cfg[orig_form_nr].start_led = form_cfg[orig_form_nr-1 ].start_led + form_cfg[orig_form_nr-1 ].nr_leds;   osc_queu_MSG_int("/ostc/form/sys/sld/" + String(orig_form_nr), form_cfg[orig_form_nr].start_led ); } 	

						else if		(msg.match("/pal/run",addrOffset))			{ bitWrite(form_menu_pal[i_bit_int][_M_FORM_PAL_RUN], i_form_nr, 				bool(result));  ;}
						else if		(msg.match("/pal/ocl",addrOffset))			{ bitWrite(form_menu_pal[i_bit_int][_M_FORM_PAL_ONECOLOR], i_form_nr, 	bool(result));  ;}
						else if		(msg.match("/pal/mir",addrOffset))			{ bitWrite(form_menu_pal[i_bit_int][_M_FORM_PAL_MIRROR], i_form_nr, 		bool(result));  ;}
						else if		(msg.match("/pal/rev",addrOffset))			{ bitWrite(form_menu_pal[i_bit_int][_M_FORM_PAL_REVERSED], i_form_nr, 	bool(result));  ;}
						else if		(msg.match("/pal/bld",addrOffset))			{ bitWrite(form_menu_pal[i_bit_int][_M_FORM_PAL_BLEND], i_form_nr, 			bool(result));  ;}
						else if  	(msg.match("/pal/ifm",addrOffset))  		{ bitWrite(form_menu_pal[i_bit_int][_M_FORM_PAL_SPEED_FROM_FFT], i_form_nr, 			bool(result));  ;}

						else if  	(msg.match("/pal/ald",addrOffset))  	form_fx_pal[orig_form_nr].index_add_led = uint16_t(msg.getInt(0))	;
						else if  	(msg.match("/pal/afm",addrOffset))  	form_fx_pal[orig_form_nr].index_add_frame = uint16_t(msg.getInt(0))	;   
						else if  	(msg.match("/pal/sid",addrOffset))  	form_fx_pal[orig_form_nr].index_start = uint16_t(msg.getInt(0))	; 
						
						else if		(msg.match("/pal/mix",addrOffset))  	form_fx_pal[orig_form_nr].mix_mode = uint8_t(msg.getInt(0))	; 
						else if  	(msg.match("/pal/pal",addrOffset))  	form_fx_pal[orig_form_nr].pal = uint8_t(msg.getInt(0))	; 
						else if		(msg.match("/pal/lvl",addrOffset))		{  form_fx_pal[orig_form_nr].level  =  uint8_t(result)  ;}

			   

						//else if		(msg.match("/fx/sys/lvl",addrOffset))			{  form_fx1[orig_form_nr].level  =  uint8_t(result)  ;}
						
						

						
						

						else if		(msg.match("/fft/run",addrOffset))			{ bitWrite(form_menu_fft[i_bit_int][_M_FORM_FFT_RUN], 			i_form_nr, 	bool(result));  ;}
						else if		(msg.match("/fft/rev",addrOffset))			{ bitWrite(form_menu_fft[i_bit_int][_M_FORM_FFT_REVERSED], 	i_form_nr, 	bool(result));  ;}
						else if		(msg.match("/fft/mir",addrOffset))			{ bitWrite(form_menu_fft[i_bit_int][_M_FORM_FFT_MIRROR], 		i_form_nr, 	bool(result));  ;}
						else if		(msg.match("/fft/ocl",addrOffset))		{ bitWrite(form_menu_fft[i_bit_int][_M_FORM_FFT_ONECOLOR], 		i_form_nr, 	bool(result));  ;}
						else if  	(msg.match("/fft/ofs",addrOffset))  	form_fx_fft[orig_form_nr].offset = uint8_t(msg.getInt(0))	; 
						else if  	(msg.match("/fft/exd",addrOffset))  	form_fx_fft[orig_form_nr].extend = uint8_t(msg.getInt(0))	; 
						else if		(msg.match("/fft/lvl",addrOffset))		{  form_fx_fft[orig_form_nr].level  =  uint8_t(result)  ;}
						else if  	(msg.match("/fft/mix",addrOffset))  	form_fx_fft[orig_form_nr].mix_mode = uint8_t(msg.getInt(0))	;
			}
		}

	  


//else if (msg.fullMatch("/palbri",addrOffset))	{ led_cfg.pal_bri	= uint8_t(msg.getInt(0)); }

}




////////////// MASTER IN


void osc_StC_master_mqtt_ip_input(OSCMessage &msg, int addrOffset) 
{
		if(!msg.isString(0) )
		{
			if			(msg.fullMatch("/ip/1",addrOffset))		{ mqtt_cfg.mqttIP[0] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/ip/2",addrOffset))		{ mqtt_cfg.mqttIP[1] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/ip/3",addrOffset))		{ mqtt_cfg.mqttIP[2] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/ip/4",addrOffset))		{ mqtt_cfg.mqttIP[3] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/ip/5",addrOffset))		{ mqtt_cfg.mqttPort = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/ip/6",addrOffset))		{ mqtt_cfg.publishSec = uint8_t(msg.getInt(0));}


		}
}


void osc_StC_master_artnet_routing(OSCMessage &msg, int addrOffset) 
{			// 

		char address[6] ;
		String node_string;
		uint8_t node  = 0;
		uint8_t setting = 0;

		memset(address, 0, sizeof(address));
		msg.getAddress(address, addrOffset + 1, 1);

		node = constrain( (address[0] -48) , 0, (ARTNET_NR_NODES_TPM - 1 ) );
	
		memset(address, 0, sizeof(address));
		msg.getAddress(address, addrOffset + 3, 1);

		setting =   constrain(address[0] -48, 0, (ARTNET_NR_NODE_SETTINGS -1 ));


		//debugMe("node : ",false) ; debugMe(node);
		//debugMe("set : ",false) ; debugMe(setting);

		//for (byte i = 0; i < sizeof(address); i++)  { save_no_string = save_no_string + address[i]; }

		//sel_save_no = save_no_string.toInt();  
		switch(setting)
		{
			case 0:
				artnetNode[node-1].startU = uint8_t(msg.getInt(0));
			break;
				
			case 1:
				artnetNode[node-1].numU = uint8_t(msg.getInt(0));
			break;
			case 2:
			case 3:
			case 4:
			case 5:
				artnetNode[node-1].IP[setting -2] = uint8_t(msg.getInt(0));
			break;

		}


}



void osc_StC_master_wifi_routing(OSCMessage &msg, int addrOffset) 
{
		if(!msg.isString(0) )
		{
			if 			(msg.fullMatch("/save",addrOffset))		{ FS_wifi_write(0); }
			else if		(msg.fullMatch("/ip/1",addrOffset))		{ wifi_cfg.ipStaticLocal[0] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/ip/2",addrOffset))		{ wifi_cfg.ipStaticLocal[1] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/ip/3",addrOffset))		{ wifi_cfg.ipStaticLocal[2] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/ip/4",addrOffset))		{ wifi_cfg.ipStaticLocal[3] = uint8_t(msg.getInt(0));    }
			
			
			else if 	(msg.fullMatch("/sn/1",addrOffset))		{ wifi_cfg.ipSubnet[0] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/sn/2",addrOffset))		{ wifi_cfg.ipSubnet[1] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/sn/3",addrOffset))		{ wifi_cfg.ipSubnet[2] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/sn/4",addrOffset))		{ wifi_cfg.ipSubnet[3] = uint8_t(msg.getInt(0));}

			else if 	(msg.fullMatch("/gw/1",addrOffset))		{ wifi_cfg.ipDGW[0] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/gw/2",addrOffset))		{ wifi_cfg.ipDGW[1] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/gw/3",addrOffset))		{ wifi_cfg.ipDGW[2] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/gw/4",addrOffset))		{ wifi_cfg.ipDGW[3] = uint8_t(msg.getInt(0));}

			else if 	(msg.fullMatch("/dns/1",addrOffset))		{ wifi_cfg.ipDNS[0] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/dns/2",addrOffset))		{ wifi_cfg.ipDNS[1] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/dns/3",addrOffset))		{ wifi_cfg.ipDNS[2] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/dns/4",addrOffset))		{ wifi_cfg.ipDNS[3] = uint8_t(msg.getInt(0));}


			else if 	(msg.fullMatch("/http",addrOffset))		{ write_bool(HTTP_ENABLED, bool(msg.getInt(0) )) ; }
			else if 	(msg.fullMatch("/serial",addrOffset))	{ write_bool(DEBUG_OUT, bool(msg.getInt(0) )) ; }
			else if 	(msg.fullMatch("/telnet",addrOffset))	{ write_bool(DEBUG_TELNET, bool(msg.getInt(0) )) ; }

			else if 	(msg.fullMatch("/power",addrOffset))  { write_bool(WIFI_POWER, 			bool(msg.getInt(0) )) ; }
			else if 	(msg.fullMatch("/dhcp",addrOffset))   { write_bool(STATIC_IP_ENABLED, 	bool(msg.getInt(0) )) ; }
			else if 	(msg.fullMatch("/mode",addrOffset))   { write_bool(WIFI_MODE,			bool(msg.getInt(0) )) ; }

			

			
		}
		
		else	// it is a string
		{ 
		int length=msg.getDataLength(0);

		if   	(msg.fullMatch("/name",addrOffset))		{ memset(wifi_cfg.APname, 0, 	 	sizeof(wifi_cfg.APname)		);    	msg.getString( 0,	wifi_cfg.APname,  		length ) ;  }
		else if (msg.fullMatch("/appwd",addrOffset))	{ memset(wifi_cfg.APpassword, 0, 	sizeof(wifi_cfg.APpassword)	);  	msg.getString( 0,	wifi_cfg.APpassword,	length ) ;  }
		else if (msg.fullMatch("/pwd",addrOffset))		{ memset(wifi_cfg.pwd, 0, 			sizeof(wifi_cfg.pwd)		); 		msg.getString( 0,	wifi_cfg.pwd, 		 	length ) ;  }
		else if (msg.fullMatch("/ssid",addrOffset))		{ memset(wifi_cfg.ssid, 0, 			sizeof(wifi_cfg.ssid)		); 		msg.getString( 0,	wifi_cfg.ssid,  		length ) ;  }
		else if (msg.fullMatch("/ntp",addrOffset))		{ memset(wifi_cfg.ntp_fqdn, 0, 		sizeof(wifi_cfg.ntp_fqdn)	);   	msg.getString( 0,	wifi_cfg.ntp_fqdn,  	length ) ;  }

		} // end String


}



void osc_StC_master_routing(OSCMessage &msg, int addrOffset) 
{
		//debugMe("in master routing");
		
			if 		(msg.fullMatch("/bri",addrOffset))				{ led_cfg.bri		= map(uint8_t(msg.getInt(0)), 0 , 255 , 0 , led_cfg.max_bri) ;  osc_queu_MSG_int("/ostc/audio/rbri", LEDS_get_real_bri());    } 
			else if (msg.fullMatch("/conn",addrOffset))				{ osc_StC_menu_master_ref();   osc_StC_menu_master_loadsave_ref();   }
			else if (msg.fullMatch("/ref/wifi",addrOffset))			{ osc_oStC_menu_master_wifi_ref();   }
			else if (msg.fullMatch("/ref/leds",addrOffset))			{ osc_StC_menu_master_ledcfg_ref(); }
			else if (msg.fullMatch("/ref/mqtt",addrOffset))			{ osc_oStC_menu_master_mqtt_ref(); }
			else if (msg.fullMatch("/ref/artnet",addrOffset))		{ osc_StC_menu_master_artnet_ref(); }

			else if (msg.fullMatch("/fps",addrOffset))				{ led_cfg.pal_fps		= constrain(uint8_t(msg.getInt(0) ) , 1 , MAX_PAL_FPS);  	osc_queu_MSG_int("/ostc/audio/rfps", LEDS_get_FPS());    }
			//else if (msg.fullMatch("/palbri",addrOffset))			{ led_cfg.pal_bri		= constrain(uint8_t(msg.getInt(0)), 0, 255); 				osc_queu_MSG_int("/ostc/audio/rbri", LEDS_get_real_bri());  }
			else if (msg.fullMatch("/r",addrOffset))				{ led_cfg.r				= constrain(uint8_t(msg.getInt(0)), 0 , 255); }
			else if (msg.fullMatch("/g",addrOffset))				{ led_cfg.g				= constrain(uint8_t(msg.getInt(0)), 0, 255); }
			else if (msg.fullMatch("/b",addrOffset))				{ led_cfg.b				= constrain(uint8_t(msg.getInt(0)), 0 , 255); }
			else if (msg.fullMatch("/fireCool",addrOffset))		   	{ led_cfg.fire_cooling  = constrain(uint8_t(msg.getInt(0)), FIRE_COOLING_MIN,  FIRE_COOLING_MAX)   ;}
			else if (msg.fullMatch("/fireSpark",addrOffset))		{ led_cfg.fire_sparking = constrain(uint8_t(msg.getInt(0)), FIRE_SPARKING_MIN, FIRE_SPARKING_MAX)   ;}

			else if (msg.fullMatch("/data/sl/1",addrOffset))		{ led_cfg.DataStart_leds[0]  = constrain(uint16_t(msg.getInt(0) ) , 0 , MAX_NUM_LEDS - led_cfg.DataNR_leds[0]); }
			else if (msg.fullMatch("/data/sl/2",addrOffset))		{ led_cfg.DataStart_leds[1]  = constrain(uint16_t(msg.getInt(0) ) , 0 , MAX_NUM_LEDS - led_cfg.DataNR_leds[1]); }
			else if (msg.fullMatch("/data/sl/3",addrOffset))		{ led_cfg.DataStart_leds[2]  = constrain(uint16_t(msg.getInt(0) ) , 0 , MAX_NUM_LEDS - led_cfg.DataNR_leds[2]); }
			else if (msg.fullMatch("/data/sl/4",addrOffset))		{ led_cfg.DataStart_leds[3]  = constrain(uint16_t(msg.getInt(0) ) , 0 , MAX_NUM_LEDS - led_cfg.DataNR_leds[3]); }

			else if (msg.fullMatch("/data/nl/0",addrOffset))		{ led_cfg.NrLeds 	  = constrain(uint16_t(msg.getInt(0) ) , 0 , MAX_NUM_LEDS ); }
			else if (msg.fullMatch("/data/nl/1",addrOffset))		{ led_cfg.DataNR_leds[0] = constrain(uint16_t(msg.getInt(0) ) , 0 , MAX_NUM_LEDS - led_cfg.DataStart_leds[0] ); }
			else if (msg.fullMatch("/data/nl/2",addrOffset))		{ led_cfg.DataNR_leds[1] = constrain(uint16_t(msg.getInt(0) ) , 0 , MAX_NUM_LEDS - led_cfg.DataStart_leds[1]  ); }
			else if (msg.fullMatch("/data/nl/3",addrOffset))		{ led_cfg.DataNR_leds[2] = constrain(uint16_t(msg.getInt(0) ) , 0 , MAX_NUM_LEDS - led_cfg.DataStart_leds[2]  ); }
			else if (msg.fullMatch("/data/nl/4",addrOffset))		{ led_cfg.DataNR_leds[3] = constrain(uint16_t(msg.getInt(0) ) , 0 , MAX_NUM_LEDS - led_cfg.DataStart_leds[3] ); }

			else if (msg.fullMatch("/data/select/1",addrOffset))	{ write_bool(DATA1_ENABLE, bool(msg.getInt(0) )) ; }
			else if (msg.fullMatch("/data/select/2",addrOffset))	{ write_bool(DATA2_ENABLE, bool(msg.getInt(0) )) ; }
			else if (msg.fullMatch("/data/select/3",addrOffset))	{ write_bool(DATA3_ENABLE, bool(msg.getInt(0) )) ; }
			else if (msg.fullMatch("/data/select/4",addrOffset))	{ write_bool(DATA4_ENABLE, bool(msg.getInt(0) )) ; }
			else if (msg.fullMatch("/pots",addrOffset))		{ write_bool(POT_DISABLE,  bool(msg.getInt(0) )) ; }

			
			else if (msg.fullMatch("/data/csl/2",addrOffset))		{ led_cfg.DataStart_leds[1]  =  led_cfg.DataNR_leds[0] ;  osc_queu_MSG_int("/ostc/master/data/sl/2", 	led_cfg.DataStart_leds[1] );}   
			else if (msg.fullMatch("/data/csl/3",addrOffset))		{ led_cfg.DataStart_leds[2]  =  constrain(  led_cfg.DataNR_leds[1] + led_cfg.DataStart_leds[1] ,0, MAX_NUM_LEDS )  ;  osc_queu_MSG_int("/ostc/master/data/sl/3", 	led_cfg.DataStart_leds[2] ); }
			else if (msg.fullMatch("/data/csl/4",addrOffset))		{ led_cfg.DataStart_leds[3]  =  constrain(  led_cfg.DataNR_leds[2] + led_cfg.DataStart_leds[2] ,0, MAX_NUM_LEDS ) ;  osc_queu_MSG_int("/ostc/master/data/sl/4", 	led_cfg.DataStart_leds[3] );}

			else if (msg.fullMatch("/data/mode",addrOffset))		{ led_cfg.ledMode = uint8_t(msg.getInt(0) )  ;}
			else if (msg.fullMatch("/data/aparate",addrOffset))		{ led_cfg.apa102data_rate = uint8_t(msg.getInt(0) )  ;}

			
			else if (msg.fullMatch("/data/save",addrOffset) 	&& boolean(msg.getInt(0)) == true)			{ FS_Bools_write(0) ;}
			else if (msg.fullMatch("/artnet/save",addrOffset) 	&& boolean(msg.getInt(0)) == true ) 		{ FS_artnet_write(); }
			else if (msg.fullMatch("/mqtt/save",addrOffset) 	&& boolean(msg.getInt(0)) == true )      	{ FS_mqtt_write();}

			else if (msg.fullMatch("/data/boot",addrOffset))		{ ESP.restart(); }

			else if (msg.fullMatch("/data/maxbri",addrOffset))		{ led_cfg.max_bri = constrain(uint8_t(msg.getInt(0)) , 0 , 255) ; }
			
			else if (msg.fullMatch("/artnet/rc",addrOffset))		{ osc_toggle_artnet(boolean(msg.getInt(0)) );  }
			else if (msg.fullMatch("/artnet/on",addrOffset))		{ write_bool(ARTNET_SEND, bool(msg.getInt(0) )) ; }
			else if (msg.fullMatch("/artnet/su",addrOffset))		{ artnet_cfg.startU = constrain(uint8_t(msg.getInt(0)) , 0 , 255) ; }
			else if (msg.fullMatch("/artnet/nu",addrOffset))		{ artnet_cfg.numU  = constrain(uint8_t(msg.getInt(0)) , 0 , 4) ; }
			
			
			else if (msg.fullMatch("/mqtt/enable",addrOffset))		{ write_bool(MQTT_ON, bool(msg.getInt(0) )) ; }
			
			
		
			
			else if (msg.fullMatch("/seq",addrOffset))    		{ write_bool(SEQUENCER_ON,		bool(msg.getInt(0) )) ;  led_cfg.confSwitch_time = ( micros() +  play_conf_time_min[led_cfg.Play_Nr] * MICROS_TO_MIN )  ;  }
			else if (msg.fullMatch("/playnr",addrOffset))   	{  FS_play_conf_read(uint8_t(msg.getInt(0) )); }

			else if (	(msg.match("/tmin",addrOffset))
					|| (msg.match("/laye",addrOffset))
					|| (msg.match("/auto",addrOffset))
					|| (msg.match("/save",addrOffset))
					|| (msg.match("/load",addrOffset))
					|| (msg.match("/cler",addrOffset))
			) 
			{
							char address[5] ;
							String save_no_string;
							uint8_t sel_save_no = 0;
							memset(address, 0, sizeof(address));
							msg.getAddress(address, addrOffset + 6);

							for (byte i = 0; i < sizeof(address); i++)  { save_no_string = save_no_string + address[i]; }

							sel_save_no = save_no_string.toInt();  

							if  			(msg.match("/tmin",addrOffset))  	play_conf_time_min[sel_save_no] = uint16_t(msg.getInt(0))	;
							else if  	(msg.match("/laye",addrOffset))  	layer_select[sel_save_no] = uint16_t(msg.getInt(0))	;
							else if		(msg.match("/auto",addrOffset))		{ LEDS_write_sequencer( uint8_t(sel_save_no), boolean(msg.getInt(0)) ); } 
							else if (boolean(msg.getInt(0)))  // if pushdown only
							{
								
								//debugMe(conf_NR);
								if 			(msg.match("/save",addrOffset))		{ FS_play_conf_write(sel_save_no);  osc_queu_MSG_rgb(String("/ostc/master/conf/l/"+String(sel_save_no)), 0,255,0); }
								else if 	(msg.match("/load",addrOffset))		{ FS_play_conf_read(sel_save_no);   LEDS_pal_reset_index();  }
								else if 	(msg.match("/cler",addrOffset))	{ FS_play_conf_clear(sel_save_no);  osc_queu_MSG_rgb(String("/ostc/master/conf/l/"+String(sel_save_no)), 255,0,0); }
							
							}

			}

		/*
			else if (msg.fullMatch("/conf/ref" ,addrOffset) )
			{ 
				switch(	bool(msg.getInt(0) ))
				{
						//case 0: osc_StC_menu_master_loadsave(); break;
						case 1: osc_oStC_menu_master_wifi_ref(); break ;
			 
			 }

			}  */

			
			else   // route it or its a string
			{
				int length=msg.getDataLength(0);
				if 			(msg.fullMatch("/mqtt/uname",addrOffset))		{ memset(mqtt_cfg.username, 0, 	 	sizeof(mqtt_cfg.username)		);    	msg.getString( 0,	mqtt_cfg.username,  length ) ;  }
				else if 	(msg.fullMatch("/mqtt/passwd",addrOffset))    	{ memset(mqtt_cfg.password, 0, 	 	sizeof(mqtt_cfg.password)		);    	msg.getString( 0,	mqtt_cfg.password,  length ) ;  } 


				else  // not a string
				{
				//msg.route("/conf", osc_StC_master_conf_routing, addrOffset);
					msg.route("/wifi", osc_StC_master_wifi_routing, addrOffset);
					msg.route("/artnet/node", osc_StC_master_artnet_routing, addrOffset);
					//msg.route("/mqtt/ip",,  osc_StC_master_mqtt_ip_input, addrOffset);
				}


			}

}





//////////////////// PAL In

void osc_StC_pal_rec(OSCMessage &msg, int addrOffset)
 {
	// OSC MESSAGE :/pal/?/?/1-3   

	String color_no_string;		// color  NR
	String pal_no_string;		// Pallete NO
	char address[5];
	
	
	//uint8_t pal_no = 0;						// form NR in uint8_t
	//String outbuffer = "/pal/0/x/1-3";
	//float outvalue = 0;

	//msg.getAddress(address, addrOffset - 1, 1);
	//pal_no_string = pal_no_string + address[0];
	//pal_no = pal_no_string.toInt();

	//memset(address, 0, sizeof(address));
	msg.getAddress(address, addrOffset + 1);

	for (byte i = 0; i < sizeof(address); i++) 
	{
			color_no_string = color_no_string + address[i];

	}

	
	int color_no = color_no_string.toInt();  // What CRGB value in the pallete

	//debugMe(pal_no);
	//debugMe(color_no);

	LEDS_pal_write(led_cfg.edit_pal, color_no, 0, uint8_t(msg.getInt(0)));
	LEDS_pal_write(led_cfg.edit_pal, color_no, 1, uint8_t(msg.getInt(1)));
	LEDS_pal_write(led_cfg.edit_pal, color_no, 2, uint8_t(msg.getInt(2)));

}







void osc_StC_pal_routing(OSCMessage &msg, int addrOffset) 
{
	
	if 		(msg.fullMatch("/edit/edit",addrOffset))			{ led_cfg.edit_pal = uint8_t(msg.getInt(0));  osc_StC_menu_pal_ref(led_cfg.edit_pal) ;  }
	else if (msg.fullMatch("/edit/load",addrOffset))			{ LEDS_pal_load(  led_cfg.edit_pal	, uint8_t(msg.getInt(0)) ); osc_StC_menu_pal_ref(led_cfg.edit_pal) ;  }
	else if (msg.fullMatch("/fs/load",addrOffset))				{FS_pal_load(uint8_t(msg.getInt(0)), led_cfg.edit_pal); osc_StC_menu_pal_ref(led_cfg.edit_pal);}
	else if (msg.fullMatch("/fs/save",addrOffset)) 				{FS_pal_save(uint8_t(msg.getInt(0)),  led_cfg.edit_pal);}
	else
	{
		msg.route("/1", osc_StC_pal_rec, addrOffset) ;
	}

	//debugMe("in par routing");
	//msg.route("/0", osc_StC_pal_rec, addrOffset) ; 
	
	//msg.route("/load", ostc_sct_pal_load,  addrOffset)  ;    

}



//////////////////// MENU 


void osc_StC_menu_routing(OSCMessage &msg, int addrOffset) 
{
	uint8_t select =  uint8_t(msg.getInt(0));
	//debugMe("Menu ostc : " + String(select));
	switch(select)
	{
		case 0:  osc_StC_menu_master_ref(); break; //	osc_StC_menu_pal_ref(0) ; osc_StC_menu_pal_ref(1) ; break;	
	//	case 1:  osc_StC_menu_form_ref() ; 	break;	
		//case 2: osc_StC_menu_strip_ref(0) ; break; 
		case 3:  osc_StC_menu_pal_ref(led_cfg.edit_pal) ;   break;
		case 4:  osc_StC_menu_audio_ref() ; break;


	}

	//debugMe("in menu routing");
	//osc_send_MSG_rgb( String("/ostc/pal/1/00") ,  LEDS_pal_read(0,5,0), LEDS_pal_read(0,5,1), LEDS_pal_read(0,5,2));
}




void osc_StC_routing(OSCMessage &msg, int addrOffset) 
{
	/*debugMe("in ostc");
		if(msg.isBlob(0)) debugMe("BLOB");
		if(msg.isBoolean(0)) debugMe("bool");
		if(msg.isChar(0)) debugMe("char");
		if(msg.isInt(0)) debugMe("int");
		if(msg.isFloat(0)) debugMe("float");
		if(msg.isDouble(0)) debugMe("double");
		if(msg.isString(0)) debugMe("string");

		if(msg.isTime(0)) debugMe("string");
		if(msg.hasError()) debugMe("ERROR");
	*/ 
	
	msg.route("/menu", 		osc_StC_menu_routing , 	addrOffset);   // Routing for PALLETE TAB -  Open Stage Controll
	msg.route("/master", 	osc_StC_master_routing,	addrOffset);   // Routing for MASTER TAB  -  Open Stage Controll
	msg.route("/pal", 		osc_StC_pal_routing , 	addrOffset);   // Routing for PALLETE TAB -  Open Stage Controll
	msg.route("/form", 		osc_StC_form_routing , 	addrOffset);   // Routing for FORM TAB    -  Open Stage Controll
	msg.route("/audio", 	osc_StC_audio_routing , 	addrOffset);
	//msg.route("/strip", 	osc_StC_strip_routing , 	addrOffset);

	//if (msg.fullMatch("/blend",addrOffset))			{ write_bool(BLEND_INVERT, bool(msg.getInt(0)));     }
	if (msg.fullMatch("/index_reset",addrOffset))	{ LEDS_pal_reset_index(); }

	osc_queu_MSG_rgb( String("/ostc/master/connled" ) ,  	getrand8() ,getrand8() ,getrand8( )  );	

}



///////////////////////////////////////////////////////////////////-------------- END OPEN STage Controll
///////////////////////////////////////////////////////////////////  Start TouchOSC Iphone

void osc_tosc_refresh()
{
	osc_queu_MSG_float("/tosc/bri", byte_tofloat(led_cfg.bri, 255)) ;
	osc_queu_MSG_float("/tosc/bril", float(led_cfg.bri));
	osc_queu_MSG_float("/tosc/ups", byte_tofloat(led_cfg.pal_fps, MAX_PAL_FPS));
	osc_queu_MSG_float("/tosc/upsl", float(led_cfg.pal_fps));
	osc_queu_MSG_float("/tosc/FPSL", LEDS_get_FPS());
	osc_queu_MSG_float("/tosc/r", byte_tofloat(led_cfg.r,255));
	osc_queu_MSG_float("/tosc/g", byte_tofloat(led_cfg.g,255));
	osc_queu_MSG_float("/tosc/b", byte_tofloat(led_cfg.b,255));
	
	osc_send_MSG_String("/tosc/SSID", String(wifi_cfg.ssid));
	osc_send_MSG_String("/tosc/WPW", String(wifi_cfg.pwd));
	osc_send_MSG_String("/tosc/WAPNL", String(wifi_cfg.APname));
	for (uint8_t i = 0; i < 4; i++)
		{
			// IP Config
			osc_queu_MSG_float(String("/tosc/L/SIP/" + String(i)), float(wifi_cfg.ipStaticLocal[i]));
			osc_queu_MSG_float(String("/tosc/L/SNM/" + String(i)), float(wifi_cfg.ipSubnet[i]));
			osc_queu_MSG_float(String("/tosc/L/DGW/" + String(i)), float(wifi_cfg.ipDGW[i]));
			osc_queu_MSG_float(String("/tosc/L/DNS/" + String(i)), float(wifi_cfg.ipDNS[i]));
		}

	
	osc_queu_MSG_float("/tosc/L/LNL/4", float(led_cfg.NrLeds));
	osc_queu_MSG_float("/tosc/L/LNL/0", float(led_cfg.DataNR_leds[0]));
	osc_queu_MSG_float("/tosc/L/LNL/1", float(led_cfg.DataNR_leds[1]));
	osc_queu_MSG_float("/tosc/L/LNL/2", float(led_cfg.DataNR_leds[2]));
	osc_queu_MSG_float("/tosc/L/LNL/3", float(led_cfg.DataNR_leds[3]));

	osc_queu_MSG_float("/tosc/L/LSL/0", float(led_cfg.DataStart_leds[0] ));
	osc_queu_MSG_float("/tosc/L/LSL/1", float(led_cfg.DataStart_leds[1] ));
	osc_queu_MSG_float("/tosc/L/LSL/2", float(led_cfg.DataStart_leds[2] ));
	osc_queu_MSG_float("/tosc/L/LSL/3", float(led_cfg.DataStart_leds[3] ));
	

	osc_queu_MSG_float("/tosc/LTP/1/1", 0);
	osc_queu_MSG_float("/tosc/LTP/2/1", 0);
	osc_queu_MSG_float("/tosc/LTP/3/1", 0);
	osc_queu_MSG_float("/tosc/LTP/4/1", 0);
	osc_queu_MSG_float("/tosc/LTP/5/1", 0);
	osc_queu_MSG_float("/tosc/LTP/6/1", 0);

	switch (led_cfg.ledMode)
	{
	case 0:
		osc_queu_MSG_float("/tosc/LTP/1/1", 1);
		break;
	case 1:
		osc_queu_MSG_float("/tosc/LTP/2/1", 1);
		break;
	case 2:
		osc_queu_MSG_float("/tosc/LTP/3/1", 1);
		break;
	case 3:;
		osc_queu_MSG_float("/tosc/LTP/4/1", 1);
	break;	
	case 4:
		osc_queu_MSG_float("/tosc/LTP/5/1", 1);
	break;
	case 5:
		osc_queu_MSG_float("/tosc/LTP/6/1", 1);
	break;
	
	}

	osc_queu_MSG_float("/tosc/DON/1/1", get_bool(DATA1_ENABLE));
	osc_queu_MSG_float("/tosc/DON/2/1", get_bool(DATA2_ENABLE));
	osc_queu_MSG_float("/tosc/DON/3/1", get_bool(DATA3_ENABLE));
	osc_queu_MSG_float("/tosc/DON/4/1", get_bool(DATA4_ENABLE));

	osc_queu_MSG_float("/tosc/WP", get_bool(WIFI_POWER));
	osc_queu_MSG_float("/tosc/WAP", get_bool(WIFI_ACCESSPOINT));


	osc_queu_MSG_float("/tosc/ASUL", float(artnet_cfg.startU));
	osc_queu_MSG_float("/tosc/ANUL", float(artnet_cfg.numU));
	osc_queu_MSG_float("/tosc/ANE", float(get_bool(ARTNET_SEND)));

	osc_queu_MSG_float(String("/tosc/ESIP"), float(get_bool(STATIC_IP_ENABLED)));
	osc_queu_MSG_float(String("/tosc/httpd"), float(get_bool(HTTP_ENABLED)));
	osc_queu_MSG_float(String("/tosc/debug"), float(get_bool(DEBUG_OUT)));
	osc_queu_MSG_float(String("/tosc/TNdebug"), float(get_bool(DEBUG_TELNET)));




	
	

	


}


void osc_tosc_routing(OSCMessage &msg, int addrOffset) 
{
	if 		(msg.fullMatch("/bri",addrOffset))										{ led_cfg.bri	= byte(msg.getFloat(0)	* 255); }
	else if (msg.fullMatch("/ups",addrOffset))										{ led_cfg.pal_fps = constrain(byte(msg.getFloat(0) * MAX_PAL_FPS), 1, MAX_PAL_FPS); osc_queu_MSG_float("/tosc/upsl", float(led_cfg.pal_fps)); }
 	else if (msg.fullMatch("/r",addrOffset))				{ led_cfg.r		= byte(msg.getFloat(0)	* 255); }
    else if (msg.fullMatch("/g",addrOffset))				{ led_cfg.g		= byte(msg.getFloat(0)	* 255); }
    else if (msg.fullMatch("/b",addrOffset))				{ led_cfg.b		= byte(msg.getFloat(0)	* 255); }
	else if (msg.fullMatch("/FPS", addrOffset))										osc_queu_MSG_float("/tosc/FPSL", LEDS_get_FPS());
	
	else if (msg.fullMatch("/ref", addrOffset) && bool(msg.getFloat(0)) == true)			{ osc_tosc_refresh(); }
	else if (msg.fullMatch("/RESET", addrOffset) && bool(msg.getFloat(0)) == true)			{ESP.restart(); }
	else if (msg.fullMatch("/IPSAVE", addrOffset) && bool(msg.getFloat(0)) == true) 		{FS_wifi_write(0); FS_Bools_write(0); }
	else if (msg.fullMatch("/ARTNETSAVE", addrOffset) && bool(msg.getFloat(0)) == true) 		{FS_artnet_write(); }
		
	else if (msg.fullMatch("/WAP", addrOffset))												{ write_bool(WIFI_MODE, bool(msg.getFloat(0))); }//debugMe("BLAH!!!");debugMe(get_bool(WIFI_MODE)); }
	else if (msg.fullMatch("/WP", addrOffset))												{ write_bool(WIFI_POWER, bool(msg.getFloat(0)));}
	else if (msg.fullMatch("/ESIP", addrOffset))												{ write_bool(STATIC_IP_ENABLED, bool(msg.getFloat(0)));}
	
	
	// Last page
	else if (msg.fullMatch("/httpd", addrOffset))											httpd_toggle_webserver();
	else if (msg.fullMatch("/debug", addrOffset))											{ write_bool(DEBUG_OUT, bool(msg.getFloat(0)));}
	else if (msg.fullMatch("/TNdebug", addrOffset))											{ write_bool(DEBUG_TELNET, bool(msg.getFloat(0))); }
	else if (msg.fullMatch("/ANE", addrOffset)) 											osc_toggle_artnet(bool(msg.getFloat(0)));



	else
	{
			char address[13] ;
			
			memset(address, 0, sizeof(address));
			msg.getAddress(address, addrOffset );
			//debugMe(address);

			if ( address[4]  == '/' )
			{		// SIP SNM DGW DNS
				float outval = 0;
				String string_addr;
				String string_addr2;
				memset(address, 0, sizeof(address));
				msg.getAddress(address, addrOffset +5 , 1);

				for (byte i = 0; i < sizeof(address); i++)  { string_addr = string_addr + address[i]; }
					uint8_t addr1 =  string_addr.toInt();
				
				memset(address, 0, sizeof(address));
				
				msg.getAddress(address, addrOffset +7 );
				
				for (byte i = 0; i < sizeof(address); i++)  { string_addr2 = string_addr2 + address[i]; }
					uint8_t addr2 =  string_addr2.toInt();
				addr1--;
				addr2--;
				//debugMe(addr1);
				//debugMe(addr2);
				if (msg.match("/LD1", addrOffset)  &&  (bool(msg.getFloat(0)) == true)) 
				{
					FS_play_conf_read(addr1);

				
				}
				else if (msg.match("/LD2", addrOffset)  &&  (bool(msg.getFloat(0)) == true)) 
				{
					FS_play_conf_read(addr1+8);

				
				}
				else if (msg.match("/SIP", addrOffset)  &&  (bool(msg.getFloat(0)) == true)) 
				{
					switch (addr1) 
					{
						case 0:
							wifi_cfg.ipStaticLocal[addr2]--;
							break;
						case 2:
							wifi_cfg.ipStaticLocal[addr2]++;
							break;
					}

					outval = wifi_cfg.ipStaticLocal[addr2];
					//debugMe(outval);
				}
				else if (msg.match("/SNM", addrOffset)  &&  (bool(msg.getFloat(0)) == true)) 
				{
					switch (addr1) 
					{
						case 0:
							wifi_cfg.ipSubnet[addr2]--;
						
							break;
						case 2:
							wifi_cfg.ipSubnet[addr2]++;
							break;
					}

					outval = wifi_cfg.ipSubnet[addr2];
				}
				else if (msg.match("/DGW", addrOffset)  &&  (bool(msg.getFloat(0)) == true)) 
				{
					switch (addr1) 
					{
						case 0:
							wifi_cfg.ipDGW[addr2]--;
						
							break;
						case 2:
							wifi_cfg.ipDGW[addr2]++;
							break;
					}
					outval = wifi_cfg.ipDGW[addr2];


				}
				else if (msg.match("/DNS", addrOffset)  &&  (bool(msg.getFloat(0)) == true)) 
				{
					switch (addr1) 
					{
						case 0:
							wifi_cfg.ipDNS[addr2]--;
						
							break;
						case 2:
							wifi_cfg.ipDNS[addr2]++;
							break;
					}
					outval = wifi_cfg.ipDNS[addr2];


				}


				else if (msg.match("/LSL", addrOffset) &&  (bool(msg.getFloat(0)) == true)) 
				{
					
						switch (addr1) 
						{
							case 0:
							
								led_cfg.DataStart_leds[addr2]  = constrain(led_cfg.DataStart_leds[addr2] -  100, 0, MAX_NUM_LEDS - led_cfg.DataNR_leds[addr2] );
								break;
							case 1:
								led_cfg.DataStart_leds[addr2]  = constrain(led_cfg.DataStart_leds[addr2] -  10, 0, MAX_NUM_LEDS - led_cfg.DataNR_leds[addr2] );
								break;
							case 2:
								led_cfg.DataStart_leds[addr2]  = constrain(led_cfg.DataStart_leds[addr2] -  1, 0, MAX_NUM_LEDS - led_cfg.DataNR_leds[addr2] );
								break;
							case 3:
								if (addr2 == 0)
									led_cfg.DataStart_leds[addr2]  = 0;
								else 
									led_cfg.DataStart_leds[addr2]  = led_cfg.DataNR_leds[addr2-1] + led_cfg.DataStart_leds[addr2-1] ;
							break;
							case 4:
								led_cfg.DataStart_leds[addr2]  = constrain(led_cfg.DataStart_leds[addr2]  +  1, 0, MAX_NUM_LEDS - led_cfg.DataNR_leds[addr2] );
							break;
							case 5:
								led_cfg.DataStart_leds[addr2]  = constrain(led_cfg.DataStart_leds[addr2]  +  10, 0, MAX_NUM_LEDS - led_cfg.DataNR_leds[addr2] );
							break;
							case 6:
								led_cfg.DataStart_leds[addr2]  = constrain(led_cfg.DataStart_leds[addr2]  +  100, 0, MAX_NUM_LEDS - led_cfg.DataNR_leds[addr2] );
							break;
						}
						outval = led_cfg.DataStart_leds[addr2] ;
					
				
					
				
				}
				else if (msg.match("/LNL", addrOffset) &&  (bool(msg.getFloat(0)) == true)) 
				{
					if (addr2 != 4)
					{
						switch (addr1) 
						{
							case 0:
							
								led_cfg.DataNR_leds[addr2]  = constrain(led_cfg.DataNR_leds[addr2] -  100, 0, MAX_NUM_LEDS - led_cfg.DataStart_leds[addr2] );
								break;
							case 1:
								led_cfg.DataNR_leds[addr2]  = constrain(led_cfg.DataNR_leds[addr2] -  10, 0, MAX_NUM_LEDS - led_cfg.DataStart_leds[addr2] );
								break;
							case 2:
								led_cfg.DataNR_leds[addr2]  = constrain(led_cfg.DataNR_leds[addr2] -  1, 0, MAX_NUM_LEDS - led_cfg.DataStart_leds[addr2] );
								break;
							case 3:
								//led_cfg.DataNR_leds[addr2]  = 0;
							break;
							case 4:
								led_cfg.DataNR_leds[addr2]  = constrain(led_cfg.DataNR_leds[addr2]  +  1, 0, MAX_NUM_LEDS - led_cfg.DataStart_leds[addr2] );
							break;
							case 5:
								led_cfg.DataNR_leds[addr2]  = constrain(led_cfg.DataNR_leds[addr2]  +  10, 0, MAX_NUM_LEDS - led_cfg.DataStart_leds[addr2] );
							break;
							case 6:
								led_cfg.DataNR_leds[addr2]  = constrain(led_cfg.DataNR_leds[addr2]  +  100, 0, MAX_NUM_LEDS - led_cfg.DataStart_leds[addr2] );
							break;
						}
						outval = led_cfg.DataNR_leds[addr2] ;
					}
					else
					{
						//debugMe("in max");
						switch (addr1) 
						{
							case 0:
							
								led_cfg.NrLeds  = constrain(led_cfg.NrLeds -  100, 0, MAX_NUM_LEDS  );
								break;
							case 1:
								led_cfg.NrLeds  = constrain(led_cfg.NrLeds -  10, 0, MAX_NUM_LEDS  );
								break;
							case 2:
								led_cfg.NrLeds  = constrain(led_cfg.NrLeds -  1, 0, MAX_NUM_LEDS  );
								break;
							case 3:
								led_cfg.NrLeds  = 0;
							break;
							case 4:
								led_cfg.NrLeds  = constrain(led_cfg.NrLeds  +  1, 0, MAX_NUM_LEDS );
							break;
							case 5:
								led_cfg.NrLeds  = constrain(led_cfg.NrLeds  +  10, 0, MAX_NUM_LEDS  );
							break;
							case 6:
								led_cfg.NrLeds  = constrain(led_cfg.NrLeds  +  100, 0, MAX_NUM_LEDS  );
							break;
						}
						outval = led_cfg.NrLeds ;
						//debugMe(outval);
					}
					
				
				}
				else if (msg.match("/DON", addrOffset)) 
				{
					//debugMe(addr1);
					
					bool result = bool(msg.getFloat(0));
					//debugMe(result);

					switch (addr1) 
					{
						case 0:
							write_bool(DATA1_ENABLE,result);
							
							//debugMe("Data1-toggle");
							break;
						case 1:
							write_bool(DATA2_ENABLE,result);
							//debugMe("Data2-toggle");
							break;
						case 2:
							write_bool(DATA3_ENABLE,result);
							//debugMe("Data3-toggle");
							break;
						case 3:
							//debugMe("Data4-toggle");
							write_bool(DATA4_ENABLE,result);
							break;	
						default:
				
						break;
					}
				
					outval = result;


				}
				else if (msg.match("/LTP", addrOffset)) 
				{
					//debugMe(addr1);
					osc_queu_MSG_float("/tosc/LTP/1/1"   , 0);
					osc_queu_MSG_float("/tosc/LTP/2/1"   , 0);
					osc_queu_MSG_float("/tosc/LTP/3/1"   , 0);
					osc_queu_MSG_float("/tosc/LTP/4/1"   , 0);
					osc_queu_MSG_float("/tosc/LTP/5/1"   , 0);
					osc_queu_MSG_float("/tosc/LTP/6/1"   , 0);

					switch (addr1) 
					{
						case 0:
							led_cfg.ledMode = 0;
							

							break;
						case 1:
							led_cfg.ledMode = 1;
						
							break;
						case 2:
							led_cfg.ledMode = 2;
							
							break;
						case 3:
							led_cfg.ledMode = 3;
							
							break;
						case 4:
							led_cfg.ledMode =4 ;
							
							break;	
						case 5:
							led_cfg.ledMode =5 ;
							
							break;
					}
				
					outval = 1;


				}


			char outaddr[15] ;
			memset(outaddr, 0, sizeof(outaddr));
			msg.getAddress(outaddr, addrOffset, 4);
			String osc_addr;
			if ((msg.match("/DON", addrOffset)) ||  (msg.match("/LTP", addrOffset)))
			{
				osc_addr = "/tosc"+String(outaddr) + "/" + String(addr1+1) + "/1" ;
				osc_queu_MSG_float(osc_addr , outval);
				//debugMe(osc_addr);
			}
			else if (bool(msg.getFloat(0) == true))
			{
				osc_addr = "/tosc/L"+String(outaddr) + "/" + String(addr2) ;
				osc_queu_MSG_float(osc_addr , outval);
				//debugMe(osc_addr);
			}
			
		}
			

	

	}

}












// Main OSC loop
void OSC_loop() 
{	// the main osc loop
	


	OSCMessage oscMSG;
	OSCMessage oscMSG_MC;


	int size = osc_server.parsePacket();

	if (size > 0) {
		// debugMe(size);
		while (size--) {
			oscMSG.fill(osc_server.read());
		}
		if (!oscMSG.hasError())
		{
		//	debugMe("osc: rem , locel");
			//debugMe(osc_mc_server.remoteIP());
			//debugMe(WiFi.localIP());
			char address[30];
			memset(address, 0, sizeof(address));

			oscMSG.getAddress(address);
			debugMe(address);


			oscMSG.route("/ostc", osc_StC_routing);   // Routing for Open Stage Control

			oscMSG.route("/tosc", osc_tosc_routing);	// Routing for touchosc

			if (oscMSG.fullMatch("/reset-index", 0) && bool(oscMSG.getFloat(0)) == true) LEDS_pal_reset_index();
		}
		else {
			//error = bundle.getError();
			Serial.print("OSC error: ");
			//Serial.println( bundle.getError());
		}
	}   //else debugMe("XXXXX");
	osc_send_out_float_MSG_buffer();

}


#endif // OAC
//
