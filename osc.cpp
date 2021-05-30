// 
// collapse all ctr+k+0  vis studoip code
// expand all ctl+k+j
// 

#include "config_TPM.h"   // include the main Defines
#include "osc.h"

#include <WiFiUdp.h>

#ifdef ESP32
	#include <WiFi.h>
#endif

#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>
#include <QueueArray.h>

#include "tools.h"								// include the Tools enums for reading and writing bools

#include "config_fs.h"
#include "httpd.h"
#include "tpm_artnet.h"
	


// External Variables/ Structures


	#include "wifi-ota.h"
	extern wifi_Struct wifi_cfg;
	extern artnet_struct artnet_cfg;

	#include "leds.h"
	extern deck_struct deck[2] ;
	

	extern led_cfg_struct led_cfg;
	//extern led_Copy_Struct copy_leds[NR_COPY_STRIPS];
	//extern byte  copy_leds_mode[NR_COPY_LED_BYTES];

	//extern fft_led_cfg_struct fft_led_cfg;
	extern uint8_t fft_bin_results[7];


	extern void LEDS_load_default_play_conf();
	//extern byte fft_menu[3];
	//extern fft_data_struct fft_data[7];
	//extern byte fft_data_bri;
	//extern byte fft_data_fps;
	//extern uint8_t fft_color_result_bri ;

	//extern uint8_t fft_bin_autoTrigger;
	//extern uint8_t fft_color_fps;

	extern uint8_t LEDS_fft_get_fxbin_result(uint8_t fxbin, uint8_t deckNo);

	//extern fft_fxbin_struct  fft_fxbin[FFT_FX_NR_OF_BINS];
	
	extern uint16_t play_conf_time_min[MAX_NR_SAVES];
	//extern uint8_t layer_select[MAX_LAYERS_SELECT] ;

	extern artnet_node_struct artnetNode[ARTNET_NR_NODES_TPM] ;
	
	//extern uint8_t global_strip_opt[_M_NR_STRIP_BYTES_][_M_NR_GLOBAL_OPTIONS_];

// from mmqt.cpp
	#include "mmqt.h"
	extern mqtt_Struct mqtt_cfg;



// ************* Local Variables


	// Queues for outgoing OSC messages 
	QueueArray <char> osc_out_float_addr;
	QueueArray <float> osc_out_float_value;

	QueueArray <char> osc_out_rgb_addr;
	QueueArray <uint8_t> osc_out_rgb_value;
	QueueArray <char> osc_out_SelVal_addr;
	QueueArray <int> osc_out_SelVal_value;
	QueueArray <char> osc_out_int_addr;
	QueueArray <int> osc_out_int_value;


    QueueArray <char>    osc_out_fx_addr;
	QueueArray <int> osc_out_fx_intvalue;
	//QueueArray <int> osc_out_fx_strip;
	//QueueArray <int> osc_out_fx_deck;


	// OSC configuration
	osc_cfg_struct osc_cfg = { OSC_IPMULTI_ ,OSC_PORT_MULTI_,OSC_OUTPORT, OSC_INPORT, 0,1 };    // 

	// The OSC Server
	WiFiUDP osc_server;				// the normal osc server

	uint8_t Refreshloop = 255;


void OSC_setup()
{	// OSC Setup function 
	osc_server.begin(osc_cfg.inPort);
	debugMe("OSC Setup Done In Port: ", false ); debugMe(String(osc_cfg.inPort), true  );
	debugMe(" - Out Port: ", false );debugMe(String(osc_cfg.outPort), true  );    // osc_server.remotePort()   // OSC_OUTPORT
}




/////////////////////////////////////////// OSC  SEND / gerneral functions
//
//



float osc_byte_tofloat(byte value, uint8_t max_value = 255)
 {	// convert int 0-255 to float 0-1 value

	float float_out = float(value) / max_value;

	return float_out;
}


void osc_send_MSG_rgb(String addr_string , uint8_t  red = 128, uint8_t  green = 128, uint8_t  blue = 128  ) 
{	// Send out a color Directly to the OCS target not bufferd for bundle

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


void osc_send_MSG_led(String addr_string , uint8_t  state  )    
{	// Send out a predifined color Directly to the OCS target not bufferd for bundle
	// 0 = black
	// 1 = red
	// 2 = green
	// 3 = blue
	// 4 = white

	char address_out[30];
	addr_string.toCharArray(address_out, addr_string.length() + 1); //address_out 

	//IPAddress ip_out(osc_server.remoteIP());
	OSCMessage msg_out(address_out);
	
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
{	// Queue a Message to bundle output with 2 options
	// 
	for (int i = 0; i < addr_string.length(); i++)
		osc_out_SelVal_addr.enqueue(addr_string.charAt(i));

	osc_out_SelVal_addr.enqueue(0);	// add a null on the end 
	osc_out_SelVal_value.enqueue(select);
	osc_out_SelVal_value.enqueue(value);

}


void osc_queu_MSG_float(String addr_string, float value) 
{	// Queue a float value message to the bundle output 

	IPAddress ip_out(osc_server.remoteIP());
	osc_cfg.return_ip_LB = ip_out[3];

	for (int i = 0; i < addr_string.length(); i++)
		osc_out_float_addr.enqueue(addr_string.charAt(i));

	osc_out_float_addr.enqueue(0);	// add a null on the end 
	osc_out_float_value.enqueue(value);
}


void osc_queu_MSG_int(String addr_string, int value) 
{  // Queue a int value message to the bundle output 
	IPAddress ip_out(osc_server.remoteIP());
	osc_cfg.return_ip_LB = ip_out[3];

	for (int i = 0; i < addr_string.length(); i++)
		osc_out_int_addr.enqueue(addr_string.charAt(i));

	osc_out_int_addr.enqueue(0);	// add a null on the end 
	osc_out_int_value.enqueue(value);

}


void osc_queu_MSG_fx_Bool(String addr_string, int deck, byte inarray[_M_NR_FORM_BYTES_][_M_NR_FORM_PAL_OPTIONS_] , uint8_t valSelect) 
{  // Queue a int value message to the bundle output 
	IPAddress ip_out(osc_server.remoteIP());
	osc_cfg.return_ip_LB = ip_out[3];

	for (int i = 0; i < addr_string.length(); i++)
		osc_out_fx_addr.enqueue(addr_string.charAt(i));

	osc_out_fx_addr.enqueue(0);	// add a null on the end 
	osc_out_fx_intvalue.enqueue(deck);


	for (uint8_t bit = 0; bit < _M_NR_FORM_BYTES_ ; bit++)
	{
		for (uint8_t bit_formNr = 0; bit_formNr < 8 ; bit_formNr++)
		{
				//if ( bit_formNr < 3 ) debugMe( bitRead(inarray[bit][valSelect], 	bit_formNr));
				osc_out_fx_intvalue.enqueue(uint8_t( bitRead(inarray[bit][valSelect], 	bit_formNr)));
				
		}

	}
	


}

void osc_queu_MSG_fx_Int(String addr_string, int deck, byte inarray[NR_FORM_PARTS] ) 
{  // Queue a int value message to the bundle output 
	IPAddress ip_out(osc_server.remoteIP());
	osc_cfg.return_ip_LB = ip_out[3];

	for (int i = 0; i < addr_string.length(); i++)
		osc_out_fx_addr.enqueue(addr_string.charAt(i));

	osc_out_fx_addr.enqueue(0);	// add a null on the end 
	osc_out_fx_intvalue.enqueue(deck);

	for (uint8_t bit = 0; bit < NR_FORM_PARTS ; bit++)
	{
		
				//if ( bit_formNr < 3 ) debugMe( bitRead(inarray[bit][valSelect], 	bit_formNr));
				osc_out_fx_intvalue.enqueue(inarray[bit]);
				
	}
	


}




void osc_queu_MSG_rgb(String addr_string, uint8_t red,uint8_t green,uint8_t blue) 
{  // Queue a RGB value message to the bundle output 
	IPAddress ip_out(osc_server.remoteIP());
	osc_cfg.return_ip_LB = ip_out[3];

	for (int i = 0; i < addr_string.length(); i++)
		osc_out_rgb_addr.enqueue(addr_string.charAt(i));

	osc_out_rgb_addr.enqueue(0);	// add a null on the end 
	osc_out_rgb_value.enqueue(red);
	osc_out_rgb_value.enqueue(green);
	osc_out_rgb_value.enqueue(blue);


}




void osc_send_MSG_String(String addr_string, String value) 
{ // Send out a String directly to osc target

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
bool  osc_send_out_float_MSG_buffer() 
{ 	// Send out the Queues as Bundles
	// Max bundle size = OSC_BUNDLE_SEND_COUNT
	// return true if the buffer still has contents.

	


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

		if (osc_out_float_value.count() != 0 ||  (osc_out_rgb_value.count()) != 0 ||  (osc_out_SelVal_value.count())   != 0 ||  (osc_out_int_value.count())   != 0)
		{
		
			OSCBundle bundle_out;
			//IPAddress ip_out(WiFi.localIP());
			IPAddress ip_out(osc_server.remoteIP());
		
			uint8_t bundlecount = 0;

			while ( bundlecount <= OSC_BUNDLE_SEND_COUNT)
			{
				for (uint8_t z = 0; z < osc_out_float_value.count(); z++)
				{
					if(osc_out_float_value.isEmpty() != true)
					{
						i = 0;
						while (osc_out_float_addr.peek() != 0 && i < OSC_QEUE_ADD_LEN)
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

				if	( bundlecount >= OSC_BUNDLE_SEND_COUNT) { break; }
				




			if (osc_out_float_value.isEmpty() &&  (osc_out_rgb_value.isEmpty()) &&  (osc_out_SelVal_value.isEmpty())   &&  (osc_out_int_value.isEmpty())) break;
			}	// end while

			osc_server.beginPacket(ip_out,OSC_OUTPORT);// OSC_OUTPORT); //{172,16,222,104}, 8001) ; //  osc_server.remotePort()
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
	//if (osc_out_float_value.isEmpty()  &&  osc_out_rgb_value .isEmpty() &&  osc_out_SelVal_value.isEmpty() && osc_out_int_value.isEmpty() != true) 
	if (osc_out_float_value.count() == 0 &&  (osc_out_rgb_value.count()) == 0 &&  (osc_out_SelVal_value.count())   == 0 &&  (osc_out_int_value.count())   == 0)
		return false;
	else return true;
}




bool  osc_send_out_API_FX_MSG_buffer() 
{ 	// Send out the Queues as Bundles
	// Max bundle size = OSC_FX_BUNDLE_SEND_COUNT
	// return true if the buffer still has contents.


	if (osc_out_fx_intvalue.isEmpty() != true ) 
	{

		char address_out[OSC_QEUE_ADD_LEN] ;
		memset(address_out, 0, OSC_QEUE_ADD_LEN);
		uint8_t i = 0;
		
		int deckSel ;

		uint8_t send_val = 0;

		if (osc_out_fx_intvalue.count() != 0 )
		{
		
			OSCBundle bundle_out;
			//IPAddress ip_out(WiFi.localIP());
			IPAddress ip_out(osc_server.remoteIP());
		
			uint8_t bundlecount = 0;

			while ( bundlecount <= OSC_FX_BUNDLE_SEND_COUNT)
			{
				
				for (uint8_t z = 0; z < (osc_out_fx_intvalue.count() ); z++)
				{
					if(osc_out_fx_intvalue.isEmpty() != true)
					{
						i = 0;
						while (osc_out_fx_addr.peek() != 0 && i <OSC_QEUE_ADD_LEN) {
							address_out[i] = osc_out_fx_addr.dequeue();
							i++;
						}
						address_out[i] = osc_out_fx_addr.dequeue(); // get the null char for end of string as well. so that we can fetch the next msg next time
						deckSel = osc_out_fx_intvalue.dequeue();

						OSCMessage msg_out(address_out); 
						msg_out.add(deckSel);

						

						for (uint8_t foromi = 0; foromi < NR_FORM_PARTS ; foromi++)
						{

							int sendVal =  osc_out_fx_intvalue.dequeue() ;
							if ( foromi < 5 ) debugMe(sendVal);
							msg_out.add(sendVal);		
						}

						bundle_out.add(msg_out);

						bundlecount++;
						yield();
						memset(address_out, 0, OSC_QEUE_ADD_LEN);
					}
					if	( bundlecount >= OSC_FX_BUNDLE_SEND_COUNT) { break; }
				}




			if (osc_out_fx_intvalue.isEmpty() ) break;
			}	// end while

			osc_server.beginPacket(ip_out,OSC_OUTPORT);// OSC_OUTPORT); //{172,16,222,104}, 8001) ; //  osc_server.remotePort()
			bundle_out.send(osc_server);
			osc_server.endPacket();
			bundle_out.empty();
			yield();
			//delay(1);

		}
		
	}
	
	if (osc_out_fx_intvalue.count() == 0 )
		return false;
	else return true;
}


void osc_StC_Send_Confname(uint8_t SaveNo, char ConfName[])
{
		OSCBundle bundle_out;
		IPAddress ip_out(osc_server.remoteIP());		
		//osc_queu_MSG_rgb(String("/ostc/master/conf/l/"+String(sel_save_no)), 0,255,0);
		char ConfOutAddress[25] ;
		String CounfOutString = "/ostc/master/savename/" + String(SaveNo);
		
		//char ConfOutName[32] ;
		//ConfName.toCharArray(ConfOutName, CounfOutString.length() + 1);


		CounfOutString.toCharArray(ConfOutAddress, CounfOutString.length() + 1);
		bundle_out.add(ConfOutAddress ).add(ConfName);
		osc_server.beginPacket(ip_out , OSC_OUTPORT);   //osc_server.remotePort());//
		bundle_out.send(osc_server);
		osc_server.endPacket();
		bundle_out.empty();



}

void osc_StC_Load_confname_Refresh(uint8_t sel_save_no)
{

	OSCBundle bundle_out;
	IPAddress ip_out(osc_server.remoteIP());
	bundle_out.add("/ostc/master/confname").add(deck[0].cfg.confname);
	
	
	//char ConfOutAddress[25] ;
	//String CounfOutString = "/ostc/master/savename/" + String(sel_save_no);
	
	//CounfOutString.toCharArray(ConfOutAddress, CounfOutString.length() + 1);
	//bundle_out.add(ConfOutAddress ).add(deck[0].cfg.confname);
	osc_server.beginPacket(ip_out , OSC_OUTPORT);   //osc_server.remotePort());//
	bundle_out.send(osc_server);
	osc_server.endPacket();
	bundle_out.empty();

	osc_StC_Send_Confname(sel_save_no, deck[0].cfg.confname);
	osc_queu_MSG_rgb(String("/ostc/master/conf/l/"+String(sel_save_no)), 0,255,0);



}


// OSC Settings MSG:/s




void osc_multiply_send()
{	// TouchOSC M
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
		deck[0].cfg.led_master_cfg.bri = led_cfg.max_bri;
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
{	// Send out the values x times a second  for visualisation

	osc_queu_MSG_int("/ostc/audio/rfps", 		LEDS_get_FPS());
	osc_queu_MSG_int("/ostc/audio/rbri", 		LEDS_get_real_bri()); 
	osc_queu_MSG_int("/ostc/audio/sum/bri", 	deck[0].run.fft.fft_color_result_bri);
	osc_queu_MSG_int("/ostc/audio/sum/fps", 	deck[0].run.fft.fft_color_fps);

	osc_queu_MSG_int("/ostc/audio/sum/red", 	LEDS_FFT_get_color_result(0,0));
	osc_queu_MSG_int("/ostc/audio/sum/green", 	LEDS_FFT_get_color_result(0,1));
	osc_queu_MSG_int("/ostc/audio/sum/blue",	LEDS_FFT_get_color_result(0,2));


	for(uint8_t fxbin = 0; fxbin < FFT_FX_NR_OF_BINS; fxbin++)
	{
		osc_queu_MSG_int("/ostc/audio/fxb/sum/" + String(fxbin) , 		deck[0].run.fft.fft_fxbin[fxbin].sum);
		osc_queu_MSG_int("/ostc/audio/fxb/res/" + String(fxbin) , 		LEDS_fft_get_fxbin_result(fxbin,0));
		osc_queu_MSG_int("/ostc/audio/fxbin/tg/" + String(fxbin) , 		deck[0].cfg.fft_config.fft_fxbin[fxbin].trrig_val);
		osc_queu_MSG_int("/ostc/audio/fxbin/vl/" + String(fxbin) , 		deck[0].cfg.fft_config.fft_fxbin[fxbin].set_val);
		
	}


	for(uint8_t bin = 0 ; bin < 7 ; bin++)
	{
		osc_queu_MSG_int("/ostc/audio/abin/"+ String(bin) ,  	deck[0].run.fft_data[6-bin].avarage );
		osc_queu_MSG_int("/ostc/audio/mbin/"+ String(bin) ,  	deck[0].run.fft_data[6-bin].max );
		osc_queu_MSG_int("/ostc/audio/trig/"+ String(bin) ,  	deck[0].cfg.fft_config.trigger[6-bin]  );
		osc_queu_MSG_int("/ostc/audio/lbin/"+ String(bin) ,  	fft_bin_results[6-bin] );
		osc_queu_MSG_int("/ostc/audio/rbin/"+ String(bin) ,  	constrain((fft_bin_results[6-bin] - deck[0].cfg.fft_config.trigger[6-bin] ) , 0 ,255  ) );		
	}
	
 
//debugMe("invizz");
}





////////////////////////////////////////////-------------- OPEN STage Controll



// REFRESH 
void osc_StC_menu_form_led_adv_ref()
{


		for (uint8_t formNr = 0; formNr < NR_FORM_PARTS; formNr++)
		{
			
		osc_queu_MSG_int("/ostc/form/sys/sld/" + String(formNr), deck[0].cfg.form_cfg[formNr].start_led );
		osc_queu_MSG_int("/ostc/form/sys/nld/" + String(formNr), deck[0].cfg.form_cfg[formNr].nr_leds );

		}	
}

void osc_StC_menu_form_modify_adv_ref(uint8_t bit)
{
	
		for (uint8_t bit_formNr = 0; bit_formNr < 8; bit_formNr++)
		{
			
			uint8_t formNr = bit *8 + bit_formNr ;

			//osc_queu_MSG_int( "/ostc/form/mod/rev/" + String(formNr),	(bitRead(deck[0].form_menu_modify[bit][_M_FORM_FFT_REVERSED], 		bit_formNr)));  		
			osc_queu_MSG_int( "/ostc/form/rot/run/" + String(formNr),	(bitRead(deck[0].cfg.form_menu_modify[bit][_M_FORM_MODIFY_ROTATE], 		bit_formNr)));  	
			osc_queu_MSG_int( "/ostc/form/rot/rev/" + String(formNr),	(bitRead(deck[0].cfg.form_menu_modify[bit][_M_FORM_MODIFY_ROTATE_REVERSED], 		bit_formNr)));  				
			osc_queu_MSG_int("/ostc/form/rot/rot/"  +	String(formNr), deck[0].cfg.form_fx_modify[formNr].RotateFixed );
			
			
			//osc_queu_MSG_int( "/ostc/form/fx/miro/run/" + String(formNr),	(bitRead(deck[0].fx1_cfg.form_menu_modify[bit][_M_FORM_MODIFY_MIRROR], 		bit_formNr)));  

		}	

		osc_queu_MSG_int("/ostc/form/rot/rff/"  +	String(bit), deck[0].cfg.form_fx_modify_bytes[bit].RotateFullFrames );
		osc_queu_MSG_int("/ostc/form/rot/tgp/"  +	String(bit), deck[0].cfg.form_fx_modify_bytes[bit].RotateTriggerBin );	
}




void osc_StC_menu_form_fft_adv_ref(uint8_t bit)
{
	
		//uint8_t bit = 0;

		for (uint8_t bit_formNr = 0; bit_formNr < 8; bit_formNr++)
		{
			
			uint8_t formNr = bit *8 + bit_formNr ;

			osc_queu_MSG_int( "/ostc/form/fft/rev/" + String(formNr),	(bitRead(deck[0].cfg.form_menu_fft[bit][_M_FORM_FFT_REVERSED], 		bit_formNr)));  		
			osc_queu_MSG_int( "/ostc/form/fft/run/" + String(formNr),	(bitRead(deck[0].cfg.form_menu_fft[bit][_M_FORM_FFT_RUN], 			bit_formNr)));  		
			osc_queu_MSG_int( "/ostc/form/fft/ocl/" + String(formNr),	(bitRead(deck[0].cfg.form_menu_fft[bit][_M_FORM_FFT_ONECOLOR], 		bit_formNr)));  		
			osc_queu_MSG_int( "/ostc/form/fft/mir/" + String(formNr),	(bitRead(deck[0].cfg.form_menu_fft[bit][_M_FORM_FFT_MIRROR], 		bit_formNr)));  

			osc_queu_MSG_int("/ostc/form/fft/ofs/" + String(formNr), deck[0].cfg.form_fx_fft[formNr].offset );
			osc_queu_MSG_int("/ostc/form/fft/exd/" + String(formNr), deck[0].cfg.form_fx_fft[formNr].extend );
			osc_queu_MSG_int("/ostc/form/fft/lvl/"+	String(formNr), deck[0].cfg.form_fx_fft[formNr].level );
			
		}
		
		
			osc_queu_MSG_int("/ostc/form/fft/mix/" + String(bit),deck[0].cfg.form_fx_fft_signles[bit].mix_mode );
			osc_queu_MSG_int("/ostc/form/fft/tgp/"+	String(bit), deck[0].cfg.form_fx_fft_signles[bit].triggerBin );
			osc_queu_MSG_int("/ostc/form/fft/lvb/"+	String(bit), deck[0].cfg.form_fx_fft_signles[bit].lvl_bin );
			osc_queu_MSG_int("/ostc/form/fft/clr/"+	String(bit), deck[0].cfg.form_fx_fft_signles[bit].color );
			osc_queu_MSG_int("/ostc/form/fft/mlv/"+	String(bit), deck[0].cfg.form_fx_fft_signles[bit].master_lvl );

				
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

		osc_queu_MSG_int( "/ostc/form/pal/run/" + String(formNr), 			(bitRead(deck[0].cfg.form_menu_pal[bit][_M_FORM_PAL_RUN], 	real_formNr)));		
		osc_queu_MSG_int( "/ostc/form/fft/run/" + 	String(formNr),			(bitRead(deck[0].cfg.form_menu_fft[bit][_M_FORM_FFT_RUN], 	real_formNr))); 

		osc_queu_MSG_int( "/ostc/form/fx/fire/run/" + String(formNr),		(bitRead(deck[0].fx1_cfg.form_menu_fire[bit][_M_FORM_FIRE_RUN], 			real_formNr)));  		
		osc_queu_MSG_int( "/ostc/form/fx/shim/run/" +	String(formNr),		(bitRead(deck[0].fx1_cfg.form_menu_shimmer[bit][_M_FORM_SHIMMER_RUN], 	real_formNr)));  		

		osc_queu_MSG_int( "/ostc/form/fx/fx01/run/" +	String(formNr),		(bitRead(deck[0].fx1_cfg.form_menu_fx1[bit][_M_FORM_FX1_RUN], 		real_formNr)));  	
		osc_queu_MSG_int("/ostc/form/fx/dott/run/" + String(formNr), 		(bitRead(deck[0].fx1_cfg.form_menu_dot[bit][_M_FORM_DOT_RUN], 		real_formNr)) );
		osc_queu_MSG_int( "/ostc/form/fx/glit/run/" +	String(formNr),		(bitRead(deck[0].fx1_cfg.form_menu_glitter[bit][_M_FORM_GLITTER_RUN], 	real_formNr)));  		

		
		}

}


void osc_StC_menu_form_fx_strobe_adv_ref(uint8_t bit)
{


		for (uint8_t bit_formNr = 0; bit_formNr < 8; bit_formNr++)
		{
		
			uint8_t formNr = bit *8 + bit_formNr ;
					
			osc_queu_MSG_int("/ostc/form/fx/strb/run/" + String(formNr), (bitRead(deck[0].fx1_cfg.form_menu_strobe[bit][_M_FORM_STROBE_RUN], 			bit_formNr)));  		

			
			
		}
			osc_queu_MSG_int("/ostc/form/fx/strb/lvb/" + String(bit), deck[0].fx1_cfg.form_fx_strobe_bytes[bit].lvl_bin );
			osc_queu_MSG_int("/ostc/form/fx/strb/lvl/" + String(bit), deck[0].fx1_cfg.form_fx_strobe_bytes[bit].level );
			osc_queu_MSG_int("/ostc/form/fx/strb/mix/" + String(bit), deck[0].fx1_cfg.form_fx_strobe_bytes[bit].mix_mode );
			osc_queu_MSG_int("/ostc/form/fx/strb/pal/" + String(bit), deck[0].fx1_cfg.form_fx_strobe_bytes[bit].pal );
			osc_queu_MSG_int("/ostc/form/fx/strb/ofF/" + String(bit), deck[0].fx1_cfg.form_fx_strobe_bytes[bit].off_frames  );
			osc_queu_MSG_int("/ostc/form/fx/strb/onF/" + String(bit), deck[0].fx1_cfg.form_fx_strobe_bytes[bit].on_frames );
			osc_queu_MSG_int("/ostc/form/fx/strb/tgp/" + String(bit), deck[0].fx1_cfg.form_fx_strobe_bytes[bit].triggerBin );
			osc_queu_MSG_int("/ostc/form/fx/strb/mlv/" + String(bit), deck[0].fx1_cfg.form_fx_strobe_bytes[bit].master_lvl );


}

void osc_StC_menu_form_fx_eyes_adv_ref(uint8_t bit)
{


		for (uint8_t bit_formNr = 0; bit_formNr < 8; bit_formNr++)
		{
		
			uint8_t formNr = bit *8 + bit_formNr ;
					
			osc_queu_MSG_int("/ostc/form/fx/eyes/run/" + String(formNr), (bitRead(deck[0].fx1_cfg.form_menu_eyes[bit][_M_FORM_EYES_RUN], 			bit_formNr)));  		

		}
			osc_queu_MSG_int("/ostc/form/fx/eyes/lvl/" + String(bit), deck[0].fx1_cfg.form_fx_eyes_bytes[bit].level );
			osc_queu_MSG_int("/ostc/form/fx/eyes/mix/" + String(bit), deck[0].fx1_cfg.form_fx_eyes_bytes[bit].mix_mode );
			osc_queu_MSG_int("/ostc/form/fx/eyes/pal/" + String(bit), deck[0].fx1_cfg.form_fx_eyes_bytes[bit].color );
			osc_queu_MSG_int("/ostc/form/fx/eyes/eyW/" + String(bit), deck[0].fx1_cfg.form_fx_eyes_bytes[bit].EyeWidth  );
			osc_queu_MSG_int("/ostc/form/fx/eyes/eyS/" + String(bit), deck[0].fx1_cfg.form_fx_eyes_bytes[bit].EyeSpace  );
			osc_queu_MSG_int("/ostc/form/fx/eyes/onF/" + String(bit), deck[0].fx1_cfg.form_fx_eyes_bytes[bit].on_frames );
			osc_queu_MSG_int("/ostc/form/fx/eyes/paF/" + String(bit), deck[0].fx1_cfg.form_fx_eyes_bytes[bit].pause_frames );
			osc_queu_MSG_int("/ostc/form/fx/eyes/tgp/" + String(bit), deck[0].fx1_cfg.form_fx_eyes_bytes[bit].triggerBin );
			osc_queu_MSG_int("/ostc/form/fx/eyes/lvb/" + String(bit), deck[0].fx1_cfg.form_fx_eyes_bytes[bit].lvl_bin );
			osc_queu_MSG_int("/ostc/form/fx/eyes/fad/" + String(bit), deck[0].fx1_cfg.form_fx_eyes_bytes[bit].fadeval );
			osc_queu_MSG_int("/ostc/form/fx/eyes/mlv/" + String(bit), deck[0].fx1_cfg.form_fx_eyes_bytes[bit].master_lvl );


}

void osc_StC_menu_form_fx_meteor_adv_ref(uint8_t bit)
{


		for (uint8_t bit_formNr = 0; bit_formNr < 8; bit_formNr++)
		{
		
			uint8_t formNr = bit *8 + bit_formNr ;
					
			osc_queu_MSG_int("/ostc/form/fx/meto/run/" + String(formNr), (bitRead(deck[0].fx1_cfg.form_menu_meteor[bit][_M_FORM_METEOR_RUN], 			bit_formNr)));  		
			osc_queu_MSG_int("/ostc/form/fx/meto/rdd/" + String(formNr), (bitRead(deck[0].fx1_cfg.form_menu_meteor[bit][_M_FORM_METEOR_RANDOMDECAY], 	bit_formNr)));  		
			
			osc_queu_MSG_int("/ostc/form/fx/meto/mSZ/" + String(formNr), deck[0].fx1_cfg.form_fx_meteor[formNr].meteorSize  );
			osc_queu_MSG_int("/ostc/form/fx/meto/mTR/" + String(formNr), deck[0].fx1_cfg.form_fx_meteor[formNr].meteorTrailDecay  );
			

		}
			osc_queu_MSG_int("/ostc/form/fx/meto/lvl/" + String(bit), deck[0].fx1_cfg.form_fx_meteor_bytes[bit].level );
			osc_queu_MSG_int("/ostc/form/fx/meto/pal/" + String(bit), deck[0].fx1_cfg.form_fx_meteor_bytes[bit].color );
			osc_queu_MSG_int("/ostc/form/fx/meto/tgp/" + String(bit), deck[0].fx1_cfg.form_fx_meteor_bytes[bit].triggerBin );
			osc_queu_MSG_int("/ostc/form/fx/meto/lvb/" + String(bit), deck[0].fx1_cfg.form_fx_meteor_bytes[bit].lvl_bin );
			osc_queu_MSG_int("/ostc/form/fx/meto/mlv/" + String(bit), deck[0].fx1_cfg.form_fx_meteor_bytes[bit].master_lvl );


}


void osc_StC_menu_form_fx_fire_adv_ref(uint8_t bit)
{


		for (uint8_t bit_formNr = 0; bit_formNr < 8; bit_formNr++)
		{
		
			uint8_t formNr = bit *8 + bit_formNr ;
					
			osc_queu_MSG_int( "/ostc/form/fx/fire/run/" + String(formNr),		(bitRead(deck[0].fx1_cfg.form_menu_fire[bit][_M_FORM_FIRE_RUN], 			bit_formNr)));  		
			osc_queu_MSG_int( "/ostc/form/fx/fire/mir/" + String(formNr),	(bitRead(deck[0].fx1_cfg.form_menu_fire[bit][_M_FORM_FIRE_MIRROR], 	bit_formNr)));  		
			osc_queu_MSG_int( "/ostc/form/fx/fire/rev/" + String(formNr),	(bitRead(deck[0].fx1_cfg.form_menu_fire[bit][_M_FORM_FIRE_REVERSED], 		bit_formNr)));  		
		}
			osc_queu_MSG_int("/ostc/form/fx/fire/lvl/" + String(bit) , deck[0].fx1_cfg.form_fx_fire_bytes[bit].level );
			osc_queu_MSG_int("/ostc/form/fx/fire/mix/" + String(bit), deck[0].fx1_cfg.form_fx_fire_bytes[bit].mix_mode );
			osc_queu_MSG_int("/ostc/form/fx/fire/pal/" + String(bit), deck[0].fx1_cfg.form_fx_fire_bytes[bit].pal );
			osc_queu_MSG_int("/ostc/form/fx/fire/col/" + String(bit), deck[0].fx1_cfg.form_fx_fire_bytes[bit].cooling );
			osc_queu_MSG_int("/ostc/form/fx/fire/spk/" + String(bit), deck[0].fx1_cfg.form_fx_fire_bytes[bit].sparking );
			osc_queu_MSG_int("/ostc/form/fx/fire/tgp/" + String(bit), deck[0].fx1_cfg.form_fx_fire_bytes[bit].triggerBin );
			osc_queu_MSG_int("/ostc/form/fx/fire/lvb/" + String(bit), deck[0].fx1_cfg.form_fx_fire_bytes[bit].lvl_bin );
			osc_queu_MSG_int("/ostc/form/fx/fire/mlv/" + String(bit), deck[0].fx1_cfg.form_fx_fire_bytes[bit].master_lvl );


}


void osc_StC_menu_form_dot_adv_ref(uint8_t bit)
{


		for (uint8_t bit_formNr = 0; bit_formNr < 8; bit_formNr++)
		{
			
			uint8_t formNr = bit *8 + bit_formNr ;

			osc_queu_MSG_int("/ostc/form/fx/dott/num/" + String(formNr), deck[0].fx1_cfg.form_fx_dots[formNr].nr_dots );
			osc_queu_MSG_int("/ostc/form/fx/dott/bpm/" + String(formNr), deck[0].fx1_cfg.form_fx_dots[formNr].speed );
			osc_queu_MSG_int("/ostc/form/fx/dott/pbm/" + String(formNr), deck[0].fx1_cfg.form_fx_dots[formNr].index_add );

			osc_queu_MSG_int("/ostc/form/fx/dott/run/" + String(formNr), 	(bitRead(deck[0].fx1_cfg.form_menu_dot[bit][_M_FORM_DOT_RUN], 		bit_formNr)) );
			osc_queu_MSG_int("/ostc/form/fx/dott/typ/" + String(formNr), 	(bitRead(deck[0].fx1_cfg.form_menu_dot[bit][_M_FORM_DOT_TYPE], 		bit_formNr)));

		}
			osc_queu_MSG_int("/ostc/form/fx/dott/pal/" + String(bit), deck[0].fx1_cfg.form_fx_dots_bytes[bit].pal );
			osc_queu_MSG_int("/ostc/form/fx/dott/lvl/" + String(bit), deck[0].fx1_cfg.form_fx_dots_bytes[bit].level );		
}

void osc_StC_menu_form_glit_adv_ref(uint8_t bit)
{
	

		for (uint8_t bit_formNr = 0; bit_formNr < 8; bit_formNr++)
		{
			
			uint8_t formNr = bit *8 + bit_formNr ;

			osc_queu_MSG_int( "/ostc/form/fx/glit/run/" +	String(formNr),		(bitRead(deck[0].fx1_cfg.form_menu_glitter[bit][_M_FORM_GLITTER_RUN], 	bit_formNr)));  		
			osc_queu_MSG_int( "/ostc/form/fx/glit/gdb/" +	String(formNr),		(bitRead(deck[0].fx1_cfg.form_menu_glitter[bit][_M_FORM_GLITTER_FFT], 	bit_formNr)));  		
			osc_queu_MSG_int( "/ostc/form/fx/glit/val/" + String(formNr),	 	deck[0].fx1_cfg.form_fx_glitter[formNr].value);
		}
			osc_queu_MSG_int( "/ostc/form/fx/glit/lvl/" + String(bit),	 deck[0].fx1_cfg.form_fx_glitter_bytes[bit].level);
			osc_queu_MSG_int( "/ostc/form/fx/glit/pal/" + String(bit),	 deck[0].fx1_cfg.form_fx_glitter_bytes[bit].pal);
			
			osc_queu_MSG_int( "/ostc/form/fx/glit/gvb/" + String(bit),	 deck[0].fx1_cfg.form_fx_glitter_bytes[bit].glit_bin);
				
}

void osc_StC_menu_form_shim_adv_ref(uint8_t bit)
{
	

		for (uint8_t bit_formNr = 0; bit_formNr < 8; bit_formNr++)
		{
			
			uint8_t formNr = bit *8 + bit_formNr ;

			osc_queu_MSG_int( "/ostc/form/fx/shim/run/" +	String(formNr),		(bitRead(deck[0].fx1_cfg.form_menu_shimmer[bit][_M_FORM_SHIMMER_RUN], 	bit_formNr)));  		
			osc_queu_MSG_int( "/ostc/form/fx/shim/bld/" + String(formNr),		(bitRead(deck[0].fx1_cfg.form_menu_shimmer[bit][_M_FORM_SHIMMER_BLEND], bit_formNr)));  		

			osc_queu_MSG_int( "/ostc/form/fx/shim/x_s/" + String(formNr),	 deck[0].fx1_cfg.form_fx_shim[formNr].xscale);
			osc_queu_MSG_int( "/ostc/form/fx/shim/y_s/" + String(formNr),	 deck[0].fx1_cfg.form_fx_shim[formNr].yscale);
			osc_queu_MSG_int( "/ostc/form/fx/shim/bet/" + String(formNr),	 deck[0].fx1_cfg.form_fx_shim[formNr].beater);
		}
			osc_queu_MSG_int( "/ostc/form/fx/shim/lvl/" + String(bit),	 deck[0].fx1_cfg.form_fx_shim_bytes[bit].level);
			osc_queu_MSG_int( "/ostc/form/fx/shim/mix/" + String(bit),	 deck[0].fx1_cfg.form_fx_shim_bytes[bit].mix_mode);
			osc_queu_MSG_int( "/ostc/form/fx/shim/pal/" + String(bit),	 deck[0].fx1_cfg.form_fx_shim_bytes[bit].pal);
			osc_queu_MSG_int( "/ostc/form/fx/shim/tgp/" + String(bit),	 deck[0].fx1_cfg.form_fx_shim_bytes[bit].triggerBin);
			osc_queu_MSG_int( "/ostc/form/fx/shim/lvb/" + String(bit),	 deck[0].fx1_cfg.form_fx_shim_bytes[bit].lvl_bin);
			osc_queu_MSG_int( "/ostc/form/fx/shim/mlv/" + String(bit),	 deck[0].fx1_cfg.form_fx_shim_bytes[bit].master_lvl);
				
}



void osc_StC_menu_form_fx1_adv_ref(uint8_t bit)
{

		for (uint8_t bit_formNr = 0; bit_formNr < 8; bit_formNr++)
		{
			
			uint8_t formNr = bit *8 + bit_formNr ;	
	
			osc_queu_MSG_int( "/ostc/form/fx/fx01/run/" +	String(formNr),		(bitRead(deck[0].fx1_cfg.form_menu_fx1[bit][_M_FORM_FX1_RUN], 		bit_formNr)));  	
			osc_queu_MSG_int( "/ostc/form/fx/fx01/mir/" +	String(formNr),		(bitRead(deck[0].fx1_cfg.form_menu_fx1[bit][_M_FORM_FX1_MIRROR], 		bit_formNr)));  	
			osc_queu_MSG_int( "/ostc/form/fx/fx01/rev/" +	String(formNr),		(bitRead(deck[0].fx1_cfg.form_menu_fx1[bit][_M_FORM_FX1_REVERSED], 		bit_formNr)));  	
		}


			osc_queu_MSG_int("/ostc/form/fx/fx01/lvl/" + String(bit) , deck[0].fx1_cfg.form_fx1[bit].level );
			//osc_queu_MSG_int("/ostc/form/gv/" + String(formNr), deck[0].cfg.form_fx_glitter[formNr].value );
			osc_queu_MSG_int("/ostc/form/fx/fade/lvl/" +	String(bit) , deck[0].fx1_cfg.form_fx1[bit].fade );
			osc_queu_MSG_int("/ostc/form/fx/fx01/mix/" + String(bit), deck[0].fx1_cfg.form_fx1[bit].mix_mode );
			osc_queu_MSG_int("/ostc/form/fx/fx01/tgp/" + String(bit), deck[0].fx1_cfg.form_fx1[bit].triggerBin );
			osc_queu_MSG_int("/ostc/form/fx/fx01/lvb/" + String(bit), deck[0].fx1_cfg.form_fx1[bit].lvl_bin );
			osc_queu_MSG_int("/ostc/form/fx/fx01/mlv/" + String(bit), deck[0].fx1_cfg.form_fx1[bit].master_lvl );

				

}

void osc_StC_menu_form_pal_adv_ref(uint8_t bit)
{
		//debugMe("inFormRef ostc");

		for (uint8_t bit_formNr = 0; bit_formNr < 8 ; bit_formNr++)
		{
			
			uint8_t formNr = bit *8 + bit_formNr ;		


			osc_queu_MSG_int( "/ostc/form/pal/run/" + String(formNr), 	(bitRead(deck[0].cfg.form_menu_pal[bit][_M_FORM_PAL_RUN], 		bit_formNr)));		
			osc_queu_MSG_int( "/ostc/form/pal/ocl/" + String(formNr), 	(bitRead(deck[0].cfg.form_menu_pal[bit][_M_FORM_PAL_ONECOLOR], 	bit_formNr)));
			osc_queu_MSG_int( "/ostc/form/pal/rev/" + String(formNr),   (bitRead(deck[0].cfg.form_menu_pal[bit][_M_FORM_PAL_REVERSED], 	bit_formNr)));  		
			osc_queu_MSG_int( "/ostc/form/pal/mir/" + String(formNr), 	(bitRead(deck[0].cfg.form_menu_pal[bit][_M_FORM_PAL_MIRROR], 	bit_formNr)));  		
			osc_queu_MSG_int( "/ostc/form/pal/bld/" + String(formNr), 	(bitRead(deck[0].cfg.form_menu_pal[bit][_M_FORM_PAL_BLEND], 	bit_formNr)));  	 
			osc_queu_MSG_int( "/ostc/form/pal/ifm/" + String(formNr), 	(bitRead(deck[0].cfg.form_menu_pal[bit][_M_FORM_PAL_SPEED_FROM_FFT], 	bit_formNr)));	
				
			osc_queu_MSG_int("/ostc/form/pal/lvl/" + String(formNr), deck[0].cfg.form_fx_pal[formNr].level );

			osc_queu_MSG_int("/ostc/form/pal/ald/" + String(formNr), deck[0].cfg.form_fx_pal[formNr].index_add_led );
			osc_queu_MSG_int("/ostc/form/pal/afm/" + String(formNr), deck[0].cfg.form_fx_pal[formNr].index_add_frame );
			osc_queu_MSG_int("/ostc/form/pal/sid/" + String(formNr), deck[0].cfg.form_fx_pal[formNr].index_start );
		}	

			osc_queu_MSG_int("/ostc/form/pal/mix/" + String(bit), deck[0].cfg.form_fx_pal_singles[bit].mix_mode );
			osc_queu_MSG_int("/ostc/form/pal/pal/" + String(bit), deck[0].cfg.form_fx_pal_singles[bit].pal );
			osc_queu_MSG_int("/ostc/form/pal/tgp/" + String(bit), deck[0].cfg.form_fx_pal_singles[bit].triggerBin );
			osc_queu_MSG_int("/ostc/form/pal/stg/" + String(bit), deck[0].cfg.form_fx_pal_singles[bit].palSpeedBin );
			osc_queu_MSG_int("/ostc/form/pal/lvb/" + String(bit), deck[0].cfg.form_fx_pal_singles[bit].lvl_bin );
			osc_queu_MSG_int("/ostc/form/pal/mlv/" + String(bit), deck[0].cfg.form_fx_pal_singles[bit].master_lvl );



} 

/*
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
				osc_queu_MSG_int( "/ostc/form/fx/fx01/mix/" +	String(formNr), deck[0].cfg.form_fx1[formNr].mix_mode );


				osc_queu_MSG_int("/ostc/form/fft/lvl/"+	String(formNr), deck[0].cfg.form_fx_fft[formNr].level );
				osc_queu_MSG_int("/ostc/form/pal/lvl/"+	String(formNr), deck[0].cfg.form_fx_pal[formNr].level );
				
				osc_queu_MSG_int( "/ostc/form/fx/fx01/lvl/" + String(formNr), deck[0].cfg.form_fx1[formNr].level );
				
				osc_queu_MSG_int( "/ostc/form/fx/fade/lvl/" + String(formNr), deck[0].cfg.form_fx1[formNr].fade );
				osc_queu_MSG_int( "/ostc/form/fx/shim/lvl/" + String(formNr),	 deck[0].cfg.form_fx_shim[formNr].level);
				osc_queu_MSG_int( "/ostc/form/fx/glit/lvl/" + String(formNr),	 deck[0].cfg.form_fx_glitter[formNr].level);
				osc_queu_MSG_int( "/ostc/form/fx/dott/lvl/" + String(formNr), deck[0].cfg.form_fx_dots[formNr].level );
				osc_queu_MSG_int( "/ostc/form/fx/fire/lvl/" + String(formNr) , deck[0].cfg.form_fx_fire[formNr].level );


		}
}
*/





void osc_StC_menu_audio_ref()
{

		for(uint8_t bin = 0; bin< 7 ; bin++)
		{
			osc_queu_MSG_int("/ostc/audio/fbri/" +String(bin) ,  int(bitRead(deck[0].cfg.fft_config.fft_menu_bri, 6-bin)  ));
			osc_queu_MSG_int("/ostc/audio/ffps/" +String(bin) ,  int(bitRead(deck[0].cfg.fft_config.fft_menu_fps, 6-bin)  ));

			osc_queu_MSG_int("/ostc/audio/redd/"   +String(bin) ,  	int(bitRead(deck[0].cfg.fft_config.fft_menu[0], 6-bin) ));
			osc_queu_MSG_int("/ostc/audio/gren/" +String(bin) ,  	int(bitRead(deck[0].cfg.fft_config.fft_menu[1], 6-bin) ));
			osc_queu_MSG_int("/ostc/audio/blue/" +String(bin) ,  	int(bitRead(deck[0].cfg.fft_config.fft_menu[2], 6-bin) ));

			osc_queu_MSG_int("/ostc/audio/auto/" +String(bin) ,  	int(bitRead(deck[0].cfg.fft_config.fft_bin_autoTrigger, 6-bin)) );
			
			for (uint8_t z = 0; z < 10 ; z ++) 					osc_queu_MSG_int(String("/ostc/audio/fxbin/0"+ String(z) + "/" + String(bin)) , int(bitRead(deck[0].cfg.fft_config.fft_fxbin[z].menu_select, 6-bin)) );
			for (uint8_t z = 10; z < FFT_FX_NR_OF_BINS ; z ++) 	osc_queu_MSG_int(String("/ostc/audio/fxbin/" + String(z) + "/" + String(bin)) , int(bitRead(deck[0].cfg.fft_config.fft_fxbin[z].menu_select, 6-bin)) );
		
		}		
			osc_queu_MSG_int("/ostc/audio/minauto" ,  	deck[0].cfg.fft_config.fftAutoMin );
			osc_queu_MSG_int("/ostc/audio/maxauto" ,  	deck[0].cfg.fft_config.fftAutoMax );
			osc_queu_MSG_int("/ostc/audio/fftviz" ,  	int(get_bool(FFT_OSTC_VIZ)) );
			osc_queu_MSG_int("/ostc/audio/vizfps" ,  	deck[0].cfg.fft_config.viz_fps );

			osc_StC_FFT_vizIt();

}	



void osc_oStC_menu_master_wifi_ref()
{
		//debugMe("Wifi conf ref");

		OSCBundle bundle_out;
		IPAddress ip_out(osc_server.remoteIP());



		osc_queu_MSG_int("/ostc/master/wifi/mode", 	get_bool(WIFI_MODE_TPM) );
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

		

		osc_server.beginPacket(ip_out, OSC_OUTPORT);    // osc_server.remotePort());//
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
		OSCBundle bundle_out;
		IPAddress ip_out(osc_server.remoteIP());

		osc_queu_MSG_int("/ostc/master/mqtt/enable", (get_bool(MQTT_ON)));

		osc_queu_MSG_int("/ostc/master/mqtt/ip/0", 	mqtt_cfg.mqttIP[0]) ;
		osc_queu_MSG_int("/ostc/master/mqtt/ip/1", 	mqtt_cfg.mqttIP[1]) ;
		osc_queu_MSG_int("/ostc/master/mqtt/ip/2", 	mqtt_cfg.mqttIP[2]) ;
		osc_queu_MSG_int("/ostc/master/mqtt/ip/3", 	mqtt_cfg.mqttIP[3]) ;
		osc_queu_MSG_int("/ostc/master/mqtt/ip/4", 	mqtt_cfg.mqttPort );
		osc_queu_MSG_int("/ostc/master/mqtt/ip/5", 	mqtt_cfg.publishSec );


		bundle_out.add("/ostc/master/mqtt/uname").add(mqtt_cfg.username);
		bundle_out.add("/ostc/master/mqtt/passwd").add(mqtt_cfg.password);
		
		osc_server.beginPacket(ip_out , OSC_OUTPORT);   //osc_server.remotePort());//
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
	osc_queu_MSG_int("/ostc/master/pots/enable",      		get_bool(POT_DISABLE));
	osc_queu_MSG_int("/ostc/master/pots/lvlmaster",      	get_bool(POTS_LVL_MASTER));
	osc_queu_MSG_int("/ostc/master/bootconf", 		led_cfg.bootCFG);









}

void osc_StC_menu_master_ref()
{
	osc_queu_MSG_int("/ostc/master/bri", 		deck[0].cfg.led_master_cfg.bri) ; //float(led_cfg.bri) / float(led_cfg.max_bri) );
	osc_queu_MSG_int("/ostc/master/r", 			deck[0].cfg.led_master_cfg.r);
	osc_queu_MSG_int("/ostc/master/g", 			deck[0].cfg.led_master_cfg.g);
	osc_queu_MSG_int("/ostc/master/b", 			deck[0].cfg.led_master_cfg.b);
	osc_queu_MSG_int("/ostc/master/palbri", 	deck[0].cfg.led_master_cfg.pal_bri);
	osc_queu_MSG_int("/ostc/master/fps", 		deck[0].cfg.led_master_cfg.pal_fps);
	osc_queu_MSG_int("/ostc/blend", 			(get_bool(BLEND_INVERT))); 
	osc_queu_MSG_int("/ostc/master/seq", 		(get_bool(SEQUENCER_ON))); 
	osc_queu_MSG_int("/ostc/master/pasue", 		(get_bool(PAUSE_DISPLAY))); 
	osc_queu_MSG_float("/ostc/heap", float(ESP.getFreeHeap()));
	
	osc_queu_MSG_int("/ostc/master/usedBytes",   FS_get_UsedBytes()  ); 
	osc_queu_MSG_int("/ostc/master/totalBytes",  FS_get_TotalBytes()  );
	osc_queu_MSG_int("/ostc/master/leftBytes",  FS_get_leftBytes()  );
	
	OSCBundle bundle_out;
	IPAddress ip_out(osc_server.remoteIP());
	bundle_out.add("/ostc/master/confname").add(deck[0].cfg.confname);
	osc_server.beginPacket(ip_out , OSC_OUTPORT);   //osc_server.remotePort());//
	bundle_out.send(osc_server);
	osc_server.endPacket();
	bundle_out.empty();



	osc_queu_MSG_int("/ostc/master/data/maxbri",  led_cfg.max_bri );
	osc_queu_MSG_int("/ostc/master/playnr", 	led_cfg.Play_Nr);

	osc_queu_MSG_int("/ostc/audio/rfps", 		LEDS_get_FPS());
	osc_queu_MSG_int("/ostc/audio/rbri", 		LEDS_get_real_bri()); 

	for (uint8_t layer = 0 ; layer < MAX_LAYERS_SELECT ; layer++)
	{
			osc_queu_MSG_int("/ostc/master/laye/" + String(layer) , 	deck[0].cfg.layer.select[layer]	); 
			
	}
	for (uint8_t layersv = 0 ; layersv < NO_OF_SAVE_LAYERS ; layersv++)
	{
			osc_queu_MSG_int("/ostc/master/lymx/" + String(layersv) , 	deck[0].cfg.layer.save_mix[layersv]	); 
			osc_queu_MSG_int("/ostc/master/lylv/" + String(layersv) , 	deck[0].cfg.layer.save_lvl[layersv]	); 
			osc_queu_MSG_int("/ostc/master/lynl/" + String(layersv) , 	deck[0].cfg.layer.save_NrLeds[layersv]	); 
			osc_queu_MSG_int("/ostc/master/lysl/" + String(layersv) , 	deck[0].cfg.layer.save_startLed[layersv]	); 
	}
	osc_queu_MSG_int("/ostc/master/lycs"  , 	deck[0].cfg.layer.clear_start_led	);
	osc_queu_MSG_int("/ostc/master/lycn"  , 	deck[0].cfg.layer.clear_Nr_leds	);

	
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

	deck[0].cfg.fft_config.trigger[bin_no] = constrain(uint8_t(msg.getInt(0)), 0,255); 

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

			if		 	(msg.match("/fxbin/vl",addrOffset))		deck[0].cfg.fft_config.fft_fxbin[i_orig_bin_nr].set_val 	=   uint8_t(msg.getInt(0));
			else if 	(msg.match("/fxbin/tg",addrOffset))		deck[0].cfg.fft_config.fft_fxbin[i_orig_bin_nr].trrig_val	=   uint8_t(msg.getInt(0));
			else if		(msg.match("/fxbin",addrOffset))  // with fxbin number
			{

				String binAddr_string;
				memset(address, 0, sizeof(address));
				msg.getAddress(address, addrOffset +7 );

				for (byte i = 0; i < 2 ; i++)  { binAddr_string = binAddr_string + address[i]; }
					uint8_t binAddr =  constrain(binAddr_string.toInt(), 0 , FFT_FX_NR_OF_BINS-1) ;
				debugMe("bin :",false);debugMe(binAddr);

				bitWrite(deck[0].cfg.fft_config.fft_fxbin[binAddr].menu_select, 	6-i_orig_bin_nr, bool(msg.getInt(0)));
			

			}
			
			
			
			
			
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


			if 			(msg.match("/redd",addrOffset))		{ bitWrite(deck[0].cfg.fft_config.fft_menu[0], 		6-i_orig_bin_nr, bool(msg.getInt(0))); }
			else if		(msg.match("/gren",addrOffset))		{ bitWrite(deck[0].cfg.fft_config.fft_menu[1], 		6-i_orig_bin_nr, bool(msg.getInt(0))); }
			else if		(msg.match("/blue",addrOffset))		{ bitWrite(deck[0].cfg.fft_config.fft_menu[2], 		6-i_orig_bin_nr, bool(msg.getInt(0))); }
			else if		(msg.match("/fbri",addrOffset))		{ bitWrite(deck[0].cfg.fft_config.fft_menu_bri, 		6-i_orig_bin_nr, bool(msg.getInt(0))); }
			else if		(msg.match("/ffps",addrOffset))		{ bitWrite(deck[0].cfg.fft_config.fft_menu_fps, 		6-i_orig_bin_nr, bool(msg.getInt(0))); }
			else if		(msg.match("/auto",addrOffset))		{ bitWrite(deck[0].cfg.fft_config.fft_bin_autoTrigger, 6-i_orig_bin_nr, bool(msg.getInt(0))); }
			else if 	(msg.match("/trig",addrOffset)) 	deck[0].cfg.fft_config.trigger[6-i_orig_bin_nr]  = msg.getInt(0);
	}
	
	
	else if		(msg.fullMatch("/ref",addrOffset))			{osc_StC_menu_audio_ref();} 
	else if		(msg.fullMatch("/minauto",addrOffset))		{ deck[0].cfg.fft_config.fftAutoMin = uint8_t(msg.getInt(0));}
	else if		(msg.fullMatch("/maxauto",addrOffset))		{ deck[0].cfg.fft_config.fftAutoMax = uint8_t(msg.getInt(0));}

	else if		(msg.fullMatch("/fftviz",addrOffset))		{write_bool(FFT_OSTC_VIZ,	bool(msg.getInt(0))); }
	else if		(msg.fullMatch("/vizfps",addrOffset))		{ deck[0].cfg.fft_config.viz_fps  =  constrain(uint8_t(msg.getInt(0)) , 1 , VIZ_FPS_MAX)  ;}
	

	

	
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
				//case 1: osc_StC_menu_form_level_ref(); break; 
				//case 2: osc_StC_menu_form_FX_layers_ref();break;

			}
			
			
		}
	//	else if		(msg.fullMatch("/menu/ref/adv0",addrOffset))							
		//{
	//		switch(msg.getInt(0))
		//	{
				//case 0: osc_StC_menu_form_pal_adv_ref();  break;
				//case 1: osc_StC_menu_form_fft_adv_ref(); break; 
				//case 2: osc_StC_menu_form_fx_fire_adv_ref(); break; 
				//case 3: osc_StC_menu_form_shim_adv_ref(); break;
				//case 4: {osc_StC_menu_form_fx1_adv_ref(); osc_StC_menu_form_glit_adv_ref(); osc_StC_menu_form_dot_adv_ref(); }break; 
				//case 2: osc_StC_menu_form_FX_layers_ref();break;

		//	}
			
			
		//}

		else if  	(msg.fullMatch("/play/ref",addrOffset)	&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_play_ref();}
		//else if  	(msg.fullMatch("/level/ref",addrOffset)	&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_level_ref();}

		else if  	(msg.fullMatch("/pal/adv/0",addrOffset) 	&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_pal_adv_ref(0);osc_StC_menu_form_pal_adv_ref(1);}
		else if  	(msg.fullMatch("/pal/adv/1",addrOffset) 	&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_pal_adv_ref(2);osc_StC_menu_form_pal_adv_ref(3);}
		else if  	(msg.fullMatch("/pal/adv/2",addrOffset) 	&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_pal_adv_ref(4);osc_StC_menu_form_pal_adv_ref(5);}
		else if  	(msg.fullMatch("/pal/adv/3",addrOffset) 	&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_pal_adv_ref(6);osc_StC_menu_form_pal_adv_ref(7);}
		
		else if  	(msg.fullMatch("/fft/adv/0",addrOffset)			&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_fft_adv_ref(0);osc_StC_menu_form_fft_adv_ref(1);}
		else if  	(msg.fullMatch("/fft/adv/1",addrOffset)			&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_fft_adv_ref(2);osc_StC_menu_form_fft_adv_ref(3);}
		else if  	(msg.fullMatch("/fft/adv/2",addrOffset)			&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_fft_adv_ref(4);osc_StC_menu_form_fft_adv_ref(5);}
		else if  	(msg.fullMatch("/fft/adv/3",addrOffset)			&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_fft_adv_ref(6);osc_StC_menu_form_fft_adv_ref(7);}

		else if  	(msg.fullMatch("/fx/modify/adv/0",addrOffset)	&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_modify_adv_ref(0);osc_StC_menu_form_modify_adv_ref(1);}
		else if  	(msg.fullMatch("/fx/modify/adv/1",addrOffset)	&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_modify_adv_ref(2);osc_StC_menu_form_modify_adv_ref(3);}
		else if  	(msg.fullMatch("/fx/modify/adv/2",addrOffset)	&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_modify_adv_ref(4);osc_StC_menu_form_modify_adv_ref(5);}
		else if  	(msg.fullMatch("/fx/modify/adv/3",addrOffset)	&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_modify_adv_ref(6);osc_StC_menu_form_modify_adv_ref(7);}


		else if  	(msg.fullMatch("/leds/adv",addrOffset)			&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_led_adv_ref();}


		else if  	(msg.fullMatch("/fx/fire/adv/0",addrOffset)		&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_fx_fire_adv_ref(0);osc_StC_menu_form_fx_fire_adv_ref(1);}
		else if  	(msg.fullMatch("/fx/fire/adv/1",addrOffset)		&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_fx_fire_adv_ref(2);osc_StC_menu_form_fx_fire_adv_ref(3);}
		

		else if  	(msg.fullMatch("/fx/shim/adv/0",addrOffset)		&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_shim_adv_ref(0);osc_StC_menu_form_shim_adv_ref(1);}
		else if  	(msg.fullMatch("/fx/shim/adv/1",addrOffset)		&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_shim_adv_ref(2);osc_StC_menu_form_shim_adv_ref(3);}
		

		else if  	(msg.fullMatch("/fx/glit/adv/0",addrOffset)		&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_glit_adv_ref(0);osc_StC_menu_form_glit_adv_ref(1);}
		else if  	(msg.fullMatch("/fx/glit/adv/1",addrOffset)		&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_glit_adv_ref(2);osc_StC_menu_form_glit_adv_ref(3);}
		

		else if  	(msg.fullMatch("/fx/dott/adv/0",addrOffset)		&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_dot_adv_ref(0);osc_StC_menu_form_dot_adv_ref(1);}
		else if  	(msg.fullMatch("/fx/dott/adv/1",addrOffset)		&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_dot_adv_ref(2);osc_StC_menu_form_dot_adv_ref(3);}
		

		else if  	(msg.fullMatch("/fx/fx01/adv/0",addrOffset)		&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_fx1_adv_ref(0);osc_StC_menu_form_fx1_adv_ref(1);}
		else if  	(msg.fullMatch("/fx/fx01/adv/1",addrOffset)		&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_fx1_adv_ref(2);osc_StC_menu_form_fx1_adv_ref(3);}
		

		else if  	(msg.fullMatch("/fx/strobe/adv/0",addrOffset)	&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_fx_strobe_adv_ref(0);osc_StC_menu_form_fx_strobe_adv_ref(1);}
		else if  	(msg.fullMatch("/fx/strobe/adv/1",addrOffset)	&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_fx_strobe_adv_ref(2);osc_StC_menu_form_fx_strobe_adv_ref(3);}
		

		else if  	(msg.fullMatch("/fx/meto/adv/0",addrOffset)	&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_fx_meteor_adv_ref(0);osc_StC_menu_form_fx_meteor_adv_ref(1);}
		else if  	(msg.fullMatch("/fx/meto/adv/1",addrOffset)	&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_fx_meteor_adv_ref(2);osc_StC_menu_form_fx_meteor_adv_ref(3);}
		

		else if  	(msg.fullMatch("/fx/eyes/adv/0",addrOffset)	&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_fx_eyes_adv_ref(0);osc_StC_menu_form_fx_eyes_adv_ref(1);}
		else if  	(msg.fullMatch("/fx/eyes/adv/1",addrOffset)	&& bool(msg.getInt(0)) == true)	{osc_StC_menu_form_fx_eyes_adv_ref(2);osc_StC_menu_form_fx_eyes_adv_ref(3);}
		


		else if 	(msg.match("/fx",addrOffset))	
		{
										//debugMe("ohh");

			char address[5] ;
			String form_no_string;
			memset(address, 0, sizeof(address));
			msg.getAddress(address, addrOffset + 13);

			for (byte i = 0; i < sizeof(address); i++)  { form_no_string = form_no_string + address[i]; }
			
				uint8_t orig_form_nr =  form_no_string.toInt(); 
				orig_form_nr = constrain(orig_form_nr, 0, NR_FX_PARTS);
				uint8_t i_form_nr =  orig_form_nr;
				uint8_t i_bit_int = 0;
				while (i_form_nr >=8)
				{
					i_bit_int++;
					i_form_nr = i_form_nr-8;
				}
			
			//else if  	(msg.match("/mix/fx1",addrOffset))  	deck[0].cfg.form_fx1[sel_form_no].mix_mode = uint8_t(msg.getInt(0))	;

			//if				(msg.match("/fx/dott/saw",addrOffset))			{ bitWrite(deck[0].cfg.form_menu_dot[i_bit_int][_M_FORM_DOT_SAW], 	i_form_nr, 	bool(msg.getInt(0)));  ;}
			//else if		(msg.match("/fx/dott/jug",addrOffset))			{ bitWrite(deck[0].cfg.form_menu_dot[i_bit_int][_M_FORM_DOT_SINE], 		i_form_nr,	bool(msg.getInt(0)));  ;}
			//else if		(msg.match("/fx/dott/fft",addrOffset))			{ bitWrite(deck[0].cfg.form_menu_dot[i_bit_int][_M_FORM_DOT_FFT], 	i_form_nr,	bool(msg.getInt(0)));  ;}
			if  	 	(msg.match("/fx/dott/num",addrOffset))  		deck[0].fx1_cfg.form_fx_dots[orig_form_nr].nr_dots = uint8_t(msg.getInt(0))	;
			else if  	(msg.match("/fx/dott/bpm",addrOffset))  		deck[0].fx1_cfg.form_fx_dots[orig_form_nr].speed = uint8_t(msg.getInt(0))	;
			else if  	(msg.match("/fx/dott/pbm",addrOffset))  		deck[0].fx1_cfg.form_fx_dots[orig_form_nr].index_add = uint8_t(msg.getInt(0))	;

			else if  	(msg.match("/fx/glit/val",addrOffset))  		deck[0].fx1_cfg.form_fx_glitter[orig_form_nr].value = uint8_t(msg.getInt(0))	; 
			
			else if  	(msg.match("/fx/meto/mSZ",addrOffset))  		deck[0].fx1_cfg.form_fx_meteor[orig_form_nr].meteorSize = uint8_t(msg.getInt(0))	;
			else if  	(msg.match("/fx/meto/mTR",addrOffset))  		deck[0].fx1_cfg.form_fx_meteor[orig_form_nr].meteorTrailDecay = uint8_t(msg.getInt(0))	;

			

			
			else if  	(msg.match("/fx/shim/x_s",addrOffset))  		deck[0].fx1_cfg.form_fx_shim[orig_form_nr].xscale = uint8_t(msg.getInt(0))	;
			else if  	(msg.match("/fx/shim/y_s",addrOffset))  		deck[0].fx1_cfg.form_fx_shim[orig_form_nr].yscale = uint8_t(msg.getInt(0))	;
			else if  	(msg.match("/fx/shim/bet",addrOffset))  		deck[0].fx1_cfg.form_fx_shim[orig_form_nr].beater = uint8_t(msg.getInt(0))	; 



			
			

			else if		(msg.match("/fx/fx01/run",addrOffset))			{ bitWrite(deck[0].fx1_cfg.form_menu_fx1[i_bit_int][_M_FORM_FX1_RUN], 			i_form_nr, bool(msg.getInt(0)));  }
			else if		(msg.match("/fx/fx01/mir",addrOffset))			{ bitWrite(deck[0].fx1_cfg.form_menu_fx1[i_bit_int][_M_FORM_FX1_MIRROR], 			i_form_nr, bool(msg.getInt(0)));  }
			else if		(msg.match("/fx/fx01/rev",addrOffset))			{ bitWrite(deck[0].fx1_cfg.form_menu_fx1[i_bit_int][_M_FORM_FX1_REVERSED], 			i_form_nr, bool(msg.getInt(0)));  }

			else if  	(msg.match("/fx/dott/typ",addrOffset)) 			{ bitWrite(deck[0].fx1_cfg.form_menu_dot[i_bit_int][_M_FORM_DOT_TYPE], 				i_form_nr, bool(msg.getInt(0)));  }
			else if		(msg.match("/fx/dott/run",addrOffset))			{ bitWrite(deck[0].fx1_cfg.form_menu_dot[i_bit_int][_M_FORM_DOT_RUN], 				i_form_nr, bool(msg.getInt(0)));  }
			else if		(msg.match("/fx/glit/run",addrOffset))			{ bitWrite(deck[0].fx1_cfg.form_menu_glitter[i_bit_int][_M_FORM_GLITTER_RUN], 				i_form_nr, bool(msg.getInt(0)));  }
			else if		(msg.match("/fx/glit/gdb",addrOffset))			{ bitWrite(deck[0].fx1_cfg.form_menu_glitter[i_bit_int][_M_FORM_GLITTER_FFT], 				i_form_nr, bool(msg.getInt(0)));  }
			


			else if		(msg.match("/fx/fire/run",addrOffset))			{ bitWrite(deck[0].fx1_cfg.form_menu_fire[i_bit_int][_M_FORM_FIRE_RUN], 			i_form_nr, 	bool(msg.getInt(0)));  }
			else if		(msg.match("/fx/fire/rev",addrOffset))			{ bitWrite(deck[0].fx1_cfg.form_menu_fire[i_bit_int][_M_FORM_FIRE_REVERSED], 		i_form_nr,	bool(msg.getInt(0)));  }
			else if		(msg.match("/fx/fire/mir",addrOffset))			{ bitWrite(deck[0].fx1_cfg.form_menu_fire[i_bit_int][_M_FORM_FIRE_MIRROR], 		i_form_nr,	bool(msg.getInt(0)));  }
					
			else if		(msg.match("/fx/strb/run",addrOffset))			{ bitWrite(deck[0].fx1_cfg.form_menu_strobe[i_bit_int][_M_FORM_STROBE_RUN], 			i_form_nr, 	bool(msg.getInt(0)));  }
			
			else if		(msg.match("/fx/eyes/run",addrOffset))			{ bitWrite(deck[0].fx1_cfg.form_menu_eyes[i_bit_int][_M_FORM_EYES_RUN], 			i_form_nr, 	bool(msg.getInt(0)));  }

			else if		(msg.match("/fx/meto/run",addrOffset))			{ bitWrite(deck[0].fx1_cfg.form_menu_meteor[i_bit_int][_M_FORM_METEOR_RUN], 			i_form_nr, 	bool(msg.getInt(0)));  }
			else if		(msg.match("/fx/meto/rdd",addrOffset))			{ bitWrite(deck[0].fx1_cfg.form_menu_meteor[i_bit_int][_M_FORM_METEOR_RANDOMDECAY], 	i_form_nr, 	bool(msg.getInt(0)));  }

			
			else if		(msg.match("/fx/shim/bld",addrOffset))			{ bitWrite(deck[0].fx1_cfg.form_menu_shimmer[i_bit_int][_M_FORM_SHIMMER_BLEND], i_form_nr,	bool(msg.getInt(0)));  }

			//else if		(msg.match("/fx/miro/run",addrOffset))			{ bitWrite(deck[0].fx1_cfg.form_menu_modify[i_bit_int][_M_FORM_MODIFY_MIRROR], 			i_form_nr, 	bool(msg.getInt(0)));  }
			
			else if		(msg.match("/fx/shim/run",addrOffset))			{ bitWrite(deck[0].fx1_cfg.form_menu_shimmer[i_bit_int][_M_FORM_SHIMMER_RUN], i_form_nr,	bool(msg.getInt(0)));  }
			
			else 
			{
				orig_form_nr = constrain(orig_form_nr, 0, NR_FX_BYTES);

					if		(msg.match("/fx/fire/lvl",addrOffset))	  		deck[0].fx1_cfg.form_fx_fire_bytes[orig_form_nr].level  =  uint8_t(msg.getInt(0))  ;
				else if  	(msg.match("/fx/fire/mix",addrOffset))  		deck[0].fx1_cfg.form_fx_fire_bytes[orig_form_nr].mix_mode = uint8_t(msg.getInt(0))	;
				else if  	(msg.match("/fx/fire/pal",addrOffset))  		deck[0].fx1_cfg.form_fx_fire_bytes[orig_form_nr].pal = uint8_t(msg.getInt(0))	;
				else if  	(msg.match("/fx/fire/col",addrOffset))  		deck[0].fx1_cfg.form_fx_fire_bytes[orig_form_nr].cooling = uint8_t(msg.getInt(0))	;
				else if  	(msg.match("/fx/fire/spk",addrOffset))  		deck[0].fx1_cfg.form_fx_fire_bytes[orig_form_nr].sparking = uint8_t(msg.getInt(0))	;
				else if  	(msg.match("/fx/fire/tgp",addrOffset))  		deck[0].fx1_cfg.form_fx_fire_bytes[orig_form_nr].triggerBin = uint8_t(msg.getInt(0))	;
				else if  	(msg.match("/fx/fire/lvb",addrOffset))  		deck[0].fx1_cfg.form_fx_fire_bytes[orig_form_nr].lvl_bin = uint8_t(msg.getInt(0))	;
				else if  	(msg.match("/fx/fire/mlv",addrOffset))  		{deck[0].fx1_cfg.form_fx_fire_bytes[orig_form_nr].master_lvl = uint8_t(msg.getInt(0))	; deck[0].fx1_cfg.form_fx_fire_bytes[orig_form_nr+1].master_lvl  = deck[0].fx1_cfg.form_fx_fire_bytes[orig_form_nr].master_lvl ; }
				
				else if		(msg.match("/fx/strb/lvl",addrOffset))	  	   	deck[0].fx1_cfg.form_fx_strobe_bytes[orig_form_nr].level  =  uint8_t(msg.getInt(0))  ;
				else if  	(msg.match("/fx/strb/mix",addrOffset))  		deck[0].fx1_cfg.form_fx_strobe_bytes[orig_form_nr].mix_mode = uint8_t(msg.getInt(0))	;
				else if  	(msg.match("/fx/strb/pal",addrOffset))  		deck[0].fx1_cfg.form_fx_strobe_bytes[orig_form_nr].pal = uint8_t(msg.getInt(0))	;
				else if  	(msg.match("/fx/strb/onF",addrOffset))  		deck[0].fx1_cfg.form_fx_strobe_bytes[orig_form_nr].on_frames = uint8_t(msg.getInt(0))	;
				else if  	(msg.match("/fx/strb/ofF",addrOffset))  		deck[0].fx1_cfg.form_fx_strobe_bytes[orig_form_nr].off_frames = uint8_t(msg.getInt(0))	;
				else if  	(msg.match("/fx/strb/tgp",addrOffset))  		deck[0].fx1_cfg.form_fx_strobe_bytes[orig_form_nr].triggerBin = uint8_t(msg.getInt(0))	;
				else if  	(msg.match("/fx/strb/lvb",addrOffset))  		deck[0].fx1_cfg.form_fx_strobe_bytes[orig_form_nr].lvl_bin = uint8_t(msg.getInt(0))	;
				else if  	(msg.match("/fx/strb/mlv",addrOffset))  	{	deck[0].fx1_cfg.form_fx_strobe_bytes[orig_form_nr].master_lvl = uint8_t(msg.getInt(0))	; deck[0].fx1_cfg.form_fx_strobe_bytes[orig_form_nr+1].master_lvl = deck[0].fx1_cfg.form_fx_strobe_bytes[orig_form_nr].master_lvl; }

				else if		(msg.match("/fx/eyes/lvl",addrOffset))	  	   	deck[0].fx1_cfg.form_fx_eyes_bytes[orig_form_nr].level  =  uint8_t(msg.getInt(0))  ;
				else if  	(msg.match("/fx/eyes/mix",addrOffset))  		deck[0].fx1_cfg.form_fx_eyes_bytes[orig_form_nr].mix_mode = uint8_t(msg.getInt(0))	;
				else if  	(msg.match("/fx/eyes/pal",addrOffset))  		deck[0].fx1_cfg.form_fx_eyes_bytes[orig_form_nr].color  = uint8_t(msg.getInt(0))	;
				else if  	(msg.match("/fx/eyes/eyW",addrOffset))  		deck[0].fx1_cfg.form_fx_eyes_bytes[orig_form_nr].EyeWidth = uint8_t(msg.getInt(0))	;
				else if  	(msg.match("/fx/eyes/eyS",addrOffset))  		deck[0].fx1_cfg.form_fx_eyes_bytes[orig_form_nr].EyeSpace = uint8_t(msg.getInt(0))	;
				else if  	(msg.match("/fx/eyes/onF",addrOffset))  		deck[0].fx1_cfg.form_fx_eyes_bytes[orig_form_nr].on_frames = uint8_t(msg.getInt(0))	;
				else if  	(msg.match("/fx/eyes/paF",addrOffset))  		deck[0].fx1_cfg.form_fx_eyes_bytes[orig_form_nr].pause_frames = uint8_t(msg.getInt(0))	;
				else if  	(msg.match("/fx/eyes/fad",addrOffset))  		deck[0].fx1_cfg.form_fx_eyes_bytes[orig_form_nr].fadeval = uint8_t(msg.getInt(0))	;
				else if  	(msg.match("/fx/eyes/tgp",addrOffset))  		deck[0].fx1_cfg.form_fx_eyes_bytes[orig_form_nr].triggerBin = uint8_t(msg.getInt(0))	;
				else if  	(msg.match("/fx/eyes/lvb",addrOffset))  		deck[0].fx1_cfg.form_fx_eyes_bytes[orig_form_nr].lvl_bin = uint8_t(msg.getInt(0))	;
				else if  	(msg.match("/fx/eyes/mlv",addrOffset))  	{	deck[0].fx1_cfg.form_fx_eyes_bytes[orig_form_nr].master_lvl = uint8_t(msg.getInt(0))	; deck[0].fx1_cfg.form_fx_eyes_bytes[orig_form_nr+1].master_lvl = deck[0].fx1_cfg.form_fx_eyes_bytes[orig_form_nr].master_lvl;}

				else if		(msg.match("/fx/meto/lvl",addrOffset))	  	   	deck[0].fx1_cfg.form_fx_meteor_bytes[orig_form_nr].level  =  uint8_t(msg.getInt(0))  ;
				else if  	(msg.match("/fx/meto/pal",addrOffset))  		deck[0].fx1_cfg.form_fx_meteor_bytes[orig_form_nr].color  = uint8_t(msg.getInt(0))	;
				else if  	(msg.match("/fx/meto/tgp",addrOffset))  		deck[0].fx1_cfg.form_fx_meteor_bytes[orig_form_nr].triggerBin = uint8_t(msg.getInt(0))	;
				else if  	(msg.match("/fx/meto/lvb",addrOffset))  		deck[0].fx1_cfg.form_fx_meteor_bytes[orig_form_nr].lvl_bin = uint8_t(msg.getInt(0))	;
				else if  	(msg.match("/fx/meto/mvl",addrOffset))  	{	deck[0].fx1_cfg.form_fx_meteor_bytes[orig_form_nr].master_lvl = uint8_t(msg.getInt(0))	; deck[0].fx1_cfg.form_fx_meteor_bytes[orig_form_nr+1].master_lvl = deck[0].fx1_cfg.form_fx_meteor_bytes[orig_form_nr].master_lvl ;}

				else if		(msg.match("/fx/fade/lvl",addrOffset))		{	deck[0].fx1_cfg.form_fx1[orig_form_nr].fade  =  uint8_t(msg.getInt(0))  ;}
				else if		(msg.match("/fx/fx01/lvl",addrOffset))		{	deck[0].fx1_cfg.form_fx1[orig_form_nr].level  =  uint8_t(msg.getInt(0))  ;}
				else if		(msg.match("/fx/fx01/mix",addrOffset))		{	deck[0].fx1_cfg.form_fx1[orig_form_nr].mix_mode  =  uint8_t(msg.getInt(0))  ;}
				else if		(msg.match("/fx/fx01/tgp",addrOffset))		{	deck[0].fx1_cfg.form_fx1[orig_form_nr].triggerBin  =  uint8_t(msg.getInt(0))  ;}
				else if		(msg.match("/fx/fx01/mlv",addrOffset))		{	deck[0].fx1_cfg.form_fx1[orig_form_nr].master_lvl  =  uint8_t(msg.getInt(0))  ;  deck[0].fx1_cfg.form_fx1[orig_form_nr+1].master_lvl =  deck[0].fx1_cfg.form_fx1[orig_form_nr].master_lvl;}

				else if  	(msg.match("/fx/dott/lvl",addrOffset))  		deck[0].fx1_cfg.form_fx_dots_bytes[orig_form_nr].level = uint8_t(msg.getInt(0))	;
				else if  	(msg.match("/fx/dott/pal",addrOffset))  		deck[0].fx1_cfg.form_fx_dots_bytes[orig_form_nr].pal = uint8_t(msg.getInt(0))	;
				
				else if		(msg.match("/fx/glit/pal",addrOffset))		{	deck[0].fx1_cfg.form_fx_glitter_bytes[orig_form_nr].pal  =  uint8_t(msg.getInt(0))  ;}
				else if		(msg.match("/fx/glit/lvl",addrOffset))		{	deck[0].fx1_cfg.form_fx_glitter_bytes[orig_form_nr].level  =  uint8_t(msg.getInt(0))  ;}
				else if  	(msg.match("/fx/glit/gvb",addrOffset))  		deck[0].fx1_cfg.form_fx_glitter_bytes[orig_form_nr].glit_bin = uint8_t(msg.getInt(0))	; 



				else if  	(msg.match("/fx/shim/mix",addrOffset))  		deck[0].fx1_cfg.form_fx_shim_bytes[orig_form_nr].mix_mode = uint8_t(msg.getInt(0))	;
				else if  	(msg.match("/fx/shim/pal",addrOffset))  		deck[0].fx1_cfg.form_fx_shim_bytes[orig_form_nr].pal = uint8_t(msg.getInt(0))	;
				else if  	(msg.match("/fx/shim/tgp",addrOffset))  		deck[0].fx1_cfg.form_fx_shim_bytes[orig_form_nr].triggerBin = uint8_t(msg.getInt(0))	;
				else if  	(msg.match("/fx/shim/lvb",addrOffset))  		deck[0].fx1_cfg.form_fx_shim_bytes[orig_form_nr].lvl_bin = uint8_t(msg.getInt(0))	;
				else if		(msg.match("/fx/shim/lvl",addrOffset))			deck[0].fx1_cfg.form_fx_shim_bytes[orig_form_nr].level  =  uint8_t(msg.getInt(0))  ;
				else if		(msg.match("/fx/shim/mlv",addrOffset))		{	deck[0].fx1_cfg.form_fx_shim_bytes[orig_form_nr].master_lvl  =  uint8_t(msg.getInt(0))  ; deck[0].fx1_cfg.form_fx_shim_bytes[orig_form_nr+1].master_lvl = deck[0].fx1_cfg.form_fx_shim_bytes[orig_form_nr].master_lvl; }


			}

		}  //   /FX

		//else if		(msg.fullMatch("/fx/3sin",addrOffset))				{ bitWrite(form_menu[bit_int][_M_FX_3_SIN], form_nr,	bool(msg.getInt(0)));  ;}
		//else if		(msg.fullMatch("/fx/2sin",addrOffset))				{ bitWrite(form_menu[bit_int][_M_FX_2_SIN], form_nr,	bool(msg.getInt(0)));  ;}
		//else if		(msg.fullMatch("/fx/sinpal",addrOffset))			{ bitWrite(form_menu[bit_int][_M_FX_SHIM_PAL], form_nr,	bool(msg.getInt(0)));  ;}


		//else if		(msg.fullMatch("/fx/tester/0",addrOffset))			{  form_fx_test.val_0 =  uint8_t(msg.getInt(0))  ;}
		//else if		(msg.fullMatch("/fx/tester/1",addrOffset))			{  form_fx_test.val_1 =  uint8_t(msg.getInt(0))  ;}
		//else if		(msg.fullMatch("/fx/tester/2",addrOffset))			{  form_fx_test.val_2 =  uint8_t(msg.getInt(0))  ;}


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
							orig_form_nr = constrain(orig_form_nr, 0, NR_FORM_PARTS);
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
						if  		(msg.match("/sys/sld",addrOffset))  						{deck[0].cfg.form_cfg[orig_form_nr].start_led 	= constrain(uint16_t(msg.getInt(0)), 0 , (led_cfg.NrLeds - deck[0].cfg.form_cfg[orig_form_nr].nr_leds ));  osc_queu_MSG_int("/ostc/form/sys/sld/" + String(orig_form_nr), deck[0].cfg.form_cfg[orig_form_nr].start_led );  }
						else if  	(msg.match("/sys/nld",addrOffset))  						{deck[0].cfg.form_cfg[orig_form_nr].nr_leds 	= constrain(uint16_t(msg.getInt(0)), 0,  (led_cfg.NrLeds - deck[0].cfg.form_cfg[orig_form_nr].start_led )  );   osc_queu_MSG_int("/ostc/form/sys/nld/" + String(orig_form_nr), deck[0].cfg.form_cfg[orig_form_nr].nr_leds ); }
						else if  	(msg.match("/sys/csd",addrOffset) && orig_form_nr > 0 )  	{deck[0].cfg.form_cfg[orig_form_nr].start_led 	= deck[0].cfg.form_cfg[orig_form_nr-1 ].start_led + deck[0].cfg.form_cfg[orig_form_nr-1 ].nr_leds;   osc_queu_MSG_int("/ostc/form/sys/sld/" + String(orig_form_nr), deck[0].cfg.form_cfg[orig_form_nr].start_led ); } 	

						else if  	(msg.match("/pal/ald",addrOffset))  	deck[0].cfg.form_fx_pal[orig_form_nr].index_add_led = uint16_t(msg.getInt(0))	;
						else if  	(msg.match("/pal/afm",addrOffset))  	deck[0].cfg.form_fx_pal[orig_form_nr].index_add_frame = uint16_t(msg.getInt(0))	;   
						else if  	(msg.match("/pal/sid",addrOffset))  	deck[0].cfg.form_fx_pal[orig_form_nr].index_start = uint16_t(msg.getInt(0))	; 
						else if		(msg.match("/pal/lvl",addrOffset))		deck[0].cfg.form_fx_pal[orig_form_nr].level  	=  uint8_t(result)  ;

						else if  	(msg.match("/fft/ofs",addrOffset))  	deck[0].cfg.form_fx_fft[orig_form_nr].offset = uint8_t(msg.getInt(0))	; 
						else if  	(msg.match("/fft/exd",addrOffset))  	deck[0].cfg.form_fx_fft[orig_form_nr].extend = uint8_t(msg.getInt(0))	; 
						else if		(msg.match("/fft/lvl",addrOffset))		deck[0].cfg.form_fx_fft[orig_form_nr].level  =  uint8_t(result)  ;



						else if		(msg.match("/pal/run",addrOffset))			{ bitWrite(deck[0].cfg.form_menu_pal[i_bit_int][_M_FORM_PAL_RUN], i_form_nr, 				bool(result));  ;}
						else if		(msg.match("/pal/ocl",addrOffset))			{ bitWrite(deck[0].cfg.form_menu_pal[i_bit_int][_M_FORM_PAL_ONECOLOR], i_form_nr, 	bool(result));  ;}
						else if		(msg.match("/pal/mir",addrOffset))			{ bitWrite(deck[0].cfg.form_menu_pal[i_bit_int][_M_FORM_PAL_MIRROR], i_form_nr, 		bool(result));  ;}
						else if		(msg.match("/pal/rev",addrOffset))			{ bitWrite(deck[0].cfg.form_menu_pal[i_bit_int][_M_FORM_PAL_REVERSED], i_form_nr, 	bool(result));  ;}
						else if		(msg.match("/pal/bld",addrOffset))			{ bitWrite(deck[0].cfg.form_menu_pal[i_bit_int][_M_FORM_PAL_BLEND], i_form_nr, 			bool(result));  ;}
						else if  	(msg.match("/pal/ifm",addrOffset))  		{ bitWrite(deck[0].cfg.form_menu_pal[i_bit_int][_M_FORM_PAL_SPEED_FROM_FFT], i_form_nr, 			bool(result));  ;}

						else if		(msg.match("/fft/run",addrOffset))		{ bitWrite(deck[0].cfg.form_menu_fft[i_bit_int][_M_FORM_FFT_RUN], 			i_form_nr, 	bool(result));  ;}
						else if		(msg.match("/fft/rev",addrOffset))		{ bitWrite(deck[0].cfg.form_menu_fft[i_bit_int][_M_FORM_FFT_REVERSED], 	i_form_nr, 	bool(result));  ;}
						else if		(msg.match("/fft/mir",addrOffset))		{ bitWrite(deck[0].cfg.form_menu_fft[i_bit_int][_M_FORM_FFT_MIRROR], 		i_form_nr, 	bool(result));  ;}
						else if		(msg.match("/fft/ocl",addrOffset))		{ bitWrite(deck[0].cfg.form_menu_fft[i_bit_int][_M_FORM_FFT_ONECOLOR], 		i_form_nr, 	bool(result));  ;}

						else if		(msg.match("/rot/run",addrOffset))			{ bitWrite(deck[0].cfg.form_menu_modify[i_bit_int][_M_FORM_MODIFY_ROTATE], 						i_form_nr, 	bool(msg.getInt(0)));  }
						else if		(msg.match("/rot/rev",addrOffset))			{ bitWrite(deck[0].cfg.form_menu_modify[i_bit_int][_M_FORM_MODIFY_ROTATE_REVERSED], 			i_form_nr, 	bool(msg.getInt(0)));  }
						else if		(msg.match("/rot/rff",addrOffset))	  	   	deck[0].cfg.form_fx_modify_bytes[orig_form_nr].RotateFullFrames  =  uint16_t(msg.getInt(0))  ;
						else if		(msg.match("/rot/tgp",addrOffset))	  	   	deck[0].cfg.form_fx_modify_bytes[orig_form_nr].RotateTriggerBin  =  uint8_t(msg.getInt(0))  ;
						else if		(msg.match("/rot/rot",addrOffset))	  	   	deck[0].cfg.form_fx_modify[orig_form_nr].RotateFixed  =  uint16_t(msg.getInt(0))  ;


						else
						{
							orig_form_nr = constrain(orig_form_nr, 0, _M_NR_FORM_BYTES_);

								 if		(msg.match("/pal/mix",addrOffset))  	deck[0].cfg.form_fx_pal_singles[orig_form_nr].mix_mode 		= uint8_t(msg.getInt(0))	; 
							else if  	(msg.match("/pal/pal",addrOffset))  	deck[0].cfg.form_fx_pal_singles[orig_form_nr].pal 			= uint8_t(msg.getInt(0))	; 
							else if  	(msg.match("/pal/tgp",addrOffset))  	deck[0].cfg.form_fx_pal_singles[orig_form_nr].triggerBin 	= uint8_t(msg.getInt(0))	; 
							else if  	(msg.match("/pal/lvb",addrOffset))  	deck[0].cfg.form_fx_pal_singles[orig_form_nr].lvl_bin 		= uint8_t(msg.getInt(0))	; 
							else if  	(msg.match("/pal/stg",addrOffset))  	deck[0].cfg.form_fx_pal_singles[orig_form_nr].palSpeedBin 	= uint8_t(msg.getInt(0))	; 
							else if  	(msg.match("/pal/mlv",addrOffset))  {	deck[0].cfg.form_fx_pal_singles[orig_form_nr].master_lvl 	= uint8_t(msg.getInt(0))	; deck[0].cfg.form_fx_pal_singles[orig_form_nr +1 ].master_lvl  = deck[0].cfg.form_fx_pal_singles[orig_form_nr].master_lvl ; }
							
							else if  	(msg.match("/fft/mix",addrOffset))  	deck[0].cfg.form_fx_fft_signles[orig_form_nr].mix_mode = uint8_t(msg.getInt(0))	;
							else if  	(msg.match("/fft/tgp",addrOffset))  	deck[0].cfg.form_fx_fft_signles[orig_form_nr].triggerBin 	= uint8_t(msg.getInt(0))	;
							else if  	(msg.match("/fft/lvb",addrOffset))  	deck[0].cfg.form_fx_fft_signles[orig_form_nr].lvl_bin 	= uint8_t(msg.getInt(0))	;
							else if  	(msg.match("/fft/clr",addrOffset))  	deck[0].cfg.form_fx_fft_signles[orig_form_nr].color 	= uint8_t(msg.getInt(0))	;
							else if  	(msg.match("/fft/mlv",addrOffset))  { 	deck[0].cfg.form_fx_fft_signles[orig_form_nr].master_lvl 	= uint8_t(msg.getInt(0))	; deck[0].cfg.form_fx_fft_signles[orig_form_nr+1].master_lvl = deck[0].cfg.form_fx_fft_signles[orig_form_nr].master_lvl;}

			
						}

			
						
			}
		}

	  


//else if (msg.fullMatch("/palbri",addrOffset))	{ led_cfg.pal_bri	= uint8_t(msg.getInt(0)); }

}




////////////// MASTER IN


void osc_StC_master_mqtt_ip_input(OSCMessage &msg, int addrOffset) 
{
		if(!msg.isString(0) )
		{
			if			(msg.fullMatch("/ip/0",addrOffset))		{ mqtt_cfg.mqttIP[0] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/ip/1",addrOffset))		{ mqtt_cfg.mqttIP[1] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/ip/2",addrOffset))		{ mqtt_cfg.mqttIP[2] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/ip/3",addrOffset))		{ mqtt_cfg.mqttIP[3] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/ip/4",addrOffset))		{ mqtt_cfg.mqttPort = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/ip/5",addrOffset))		{ mqtt_cfg.publishSec = uint8_t(msg.getInt(0));}


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
			if 			(msg.fullMatch("/save",addrOffset))		{ FS_wifi_write(); }
			else if		(msg.fullMatch("/ip/0",addrOffset))		{ wifi_cfg.ipStaticLocal[0] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/ip/1",addrOffset))		{ wifi_cfg.ipStaticLocal[1] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/ip/2",addrOffset))		{ wifi_cfg.ipStaticLocal[2] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/ip/3",addrOffset))		{ wifi_cfg.ipStaticLocal[3] = uint8_t(msg.getInt(0));    }
			
			
			else if 	(msg.fullMatch("/sn/0",addrOffset))		{ wifi_cfg.ipSubnet[0] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/sn/1",addrOffset))		{ wifi_cfg.ipSubnet[1] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/sn/2",addrOffset))		{ wifi_cfg.ipSubnet[2] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/sn/3",addrOffset))		{ wifi_cfg.ipSubnet[3] = uint8_t(msg.getInt(0));}

			else if 	(msg.fullMatch("/gw/0",addrOffset))		{ wifi_cfg.ipDGW[0] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/gw/1",addrOffset))		{ wifi_cfg.ipDGW[1] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/gw/2",addrOffset))		{ wifi_cfg.ipDGW[2] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/gw/3",addrOffset))		{ wifi_cfg.ipDGW[3] = uint8_t(msg.getInt(0));}

			else if 	(msg.fullMatch("/dns/0",addrOffset))		{ wifi_cfg.ipDNS[0] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/dns/1",addrOffset))		{ wifi_cfg.ipDNS[1] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/dns/2",addrOffset))		{ wifi_cfg.ipDNS[2] = uint8_t(msg.getInt(0));}
			else if 	(msg.fullMatch("/dns/3",addrOffset))		{ wifi_cfg.ipDNS[3] = uint8_t(msg.getInt(0));}


			else if 	(msg.fullMatch("/http",addrOffset))		{ write_bool(HTTP_ENABLED, bool(msg.getInt(0) )) ; }
			else if 	(msg.fullMatch("/serial",addrOffset))	{ write_bool(DEBUG_OUT, bool(msg.getInt(0) )) ; }
			else if 	(msg.fullMatch("/telnet",addrOffset))	{ write_bool(DEBUG_TELNET, bool(msg.getInt(0) )) ; }

			else if 	(msg.fullMatch("/power",addrOffset))  { write_bool(WIFI_POWER, 			bool(msg.getInt(0) )) ; }
			else if 	(msg.fullMatch("/dhcp",addrOffset))   { write_bool(STATIC_IP_ENABLED, 	bool(msg.getInt(0) )) ; }
			else if 	(msg.fullMatch("/mode",addrOffset))   { write_bool(WIFI_MODE_TPM,			bool(msg.getInt(0) )) ; }

			

			
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
		
			if 		(msg.fullMatch("/bri",addrOffset))				{ deck[0].cfg.led_master_cfg.bri		= map(uint8_t(msg.getInt(0)), 0 , 255 , 0 , led_cfg.max_bri) ;  osc_queu_MSG_int("/ostc/audio/rbri", LEDS_get_real_bri());    } 
			else if (msg.fullMatch("/conn",addrOffset))				{ osc_StC_menu_master_ref();   osc_StC_menu_master_loadsave_ref();   }
			else if (msg.fullMatch("/ref/wifi",addrOffset))			{ osc_oStC_menu_master_wifi_ref();   }
			else if (msg.fullMatch("/ref/leds",addrOffset))			{ osc_StC_menu_master_ledcfg_ref(); }
			else if (msg.fullMatch("/ref/mqtt",addrOffset))			{ osc_oStC_menu_master_mqtt_ref(); }
			else if (msg.fullMatch("/ref/artnet",addrOffset))		{ osc_StC_menu_master_artnet_ref(); }

			else if (msg.fullMatch("/fps",addrOffset))				{ deck[0].cfg.led_master_cfg.pal_fps		= constrain(uint8_t(msg.getInt(0) ) , 1 , MAX_PAL_FPS);  	osc_queu_MSG_int("/ostc/audio/rfps", LEDS_get_FPS());    }
			//else if (msg.fullMatch("/palbri",addrOffset))			{ led_cfg.pal_bri		= constrain(uint8_t(msg.getInt(0)), 0, 255); 				osc_queu_MSG_int("/ostc/audio/rbri", LEDS_get_real_bri());  }
			else if (msg.fullMatch("/r",addrOffset))				{ deck[0].cfg.led_master_cfg.r				= constrain(uint8_t(msg.getInt(0)), 0 , 255); }
			else if (msg.fullMatch("/g",addrOffset))				{ deck[0].cfg.led_master_cfg.g				= constrain(uint8_t(msg.getInt(0)), 0, 255); }
			else if (msg.fullMatch("/b",addrOffset))				{ deck[0].cfg.led_master_cfg.b				= constrain(uint8_t(msg.getInt(0)), 0 , 255); }
			//else if (msg.fullMatch("/fireCool",addrOffset))		   	{ deck[0].cfg.led_master_cfg.fire_cooling  = constrain(uint8_t(msg.getInt(0)), FIRE_COOLING_MIN,  FIRE_COOLING_MAX)   ;}
			//else if (msg.fullMatch("/fireSpark",addrOffset))		{ deck[0].cfg.led_master_cfg.fire_sparking = constrain(uint8_t(msg.getInt(0)), FIRE_SPARKING_MIN, FIRE_SPARKING_MAX)   ;}
			
			else if (msg.fullMatch("/bootconf",addrOffset))			{ led_cfg.bootCFG = uint8_t(msg.getInt(0) )  ;}
			else if (msg.fullMatch("/LoadNames",addrOffset) 		&& boolean(msg.getInt(0)) == true)			{ FS_play_conf_readSendSavenames( ) ;}
			
			else if (msg.fullMatch("/data/sl/1",addrOffset))		{ led_cfg.DataStart_leds[0]  = constrain(uint16_t(msg.getInt(0) ) , 0 , led_cfg.NrLeds - led_cfg.DataNR_leds[0]); }
			else if (msg.fullMatch("/data/sl/2",addrOffset))		{ led_cfg.DataStart_leds[1]  = constrain(uint16_t(msg.getInt(0) ) , 0 , led_cfg.NrLeds - led_cfg.DataNR_leds[1]); }
			else if (msg.fullMatch("/data/sl/3",addrOffset))		{ led_cfg.DataStart_leds[2]  = constrain(uint16_t(msg.getInt(0) ) , 0 , led_cfg.NrLeds - led_cfg.DataNR_leds[2]); }
			else if (msg.fullMatch("/data/sl/4",addrOffset))		{ led_cfg.DataStart_leds[3]  = constrain(uint16_t(msg.getInt(0) ) , 0 , led_cfg.NrLeds - led_cfg.DataNR_leds[3]); }

			
			else if (msg.fullMatch("/data/nl/0",addrOffset))		{ led_cfg.NrLeds 	  	 = constrain(uint16_t(msg.getInt(0) ) , 0 , MAX_NUM_LEDS ); }
			else if (msg.fullMatch("/data/nl/1",addrOffset))		{ led_cfg.DataNR_leds[0] = constrain(uint16_t(msg.getInt(0) ) , 0 , led_cfg.NrLeds - led_cfg.DataStart_leds[0] ); }
			else if (msg.fullMatch("/data/nl/2",addrOffset))		{ led_cfg.DataNR_leds[1] = constrain(uint16_t(msg.getInt(0) ) , 0 , led_cfg.NrLeds - led_cfg.DataStart_leds[1]  ); }
			else if (msg.fullMatch("/data/nl/3",addrOffset))		{ led_cfg.DataNR_leds[2] = constrain(uint16_t(msg.getInt(0) ) , 0 , led_cfg.NrLeds - led_cfg.DataStart_leds[2]  ); }
			else if (msg.fullMatch("/data/nl/4",addrOffset))		{ led_cfg.DataNR_leds[3] = constrain(uint16_t(msg.getInt(0) ) , 0 , led_cfg.NrLeds - led_cfg.DataStart_leds[3] ); }

			else if (msg.fullMatch("/data/select/1",addrOffset))	{ write_bool(DATA1_ENABLE, bool(msg.getInt(0) )) ; }
			else if (msg.fullMatch("/data/select/2",addrOffset))	{ write_bool(DATA2_ENABLE, bool(msg.getInt(0) )) ; }
			else if (msg.fullMatch("/data/select/3",addrOffset))	{ write_bool(DATA3_ENABLE, bool(msg.getInt(0) )) ; }
			else if (msg.fullMatch("/data/select/4",addrOffset))	{ write_bool(DATA4_ENABLE, bool(msg.getInt(0) )) ; }
			else if (msg.fullMatch("/pots/enable",addrOffset))				{ write_bool(POT_DISABLE,  bool(msg.getInt(0) )) ; }
			else if (msg.fullMatch("/pots/lvlmaster",addrOffset))			{ write_bool(POTS_LVL_MASTER,  bool(msg.getInt(0) )) ; }

			
			else if (msg.fullMatch("/data/csl/2",addrOffset))		{ led_cfg.DataStart_leds[1]  =  led_cfg.DataNR_leds[0] ;  osc_queu_MSG_int("/ostc/master/data/sl/2", 	led_cfg.DataStart_leds[1] );}   
			else if (msg.fullMatch("/data/csl/3",addrOffset))		{ led_cfg.DataStart_leds[2]  =  constrain(  led_cfg.DataNR_leds[1] + led_cfg.DataStart_leds[1] ,0, led_cfg.NrLeds )  ;  osc_queu_MSG_int("/ostc/master/data/sl/3", 	led_cfg.DataStart_leds[2] ); }
			else if (msg.fullMatch("/data/csl/4",addrOffset))		{ led_cfg.DataStart_leds[3]  =  constrain(  led_cfg.DataNR_leds[2] + led_cfg.DataStart_leds[2] ,0, led_cfg.NrLeds ) ;  osc_queu_MSG_int("/ostc/master/data/sl/4", 	led_cfg.DataStart_leds[3] );}

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
			
			else if  	(msg.fullMatch("/lycs",addrOffset))  	 	deck[0].cfg.layer.clear_start_led = constrain(uint16_t(msg.getInt(0)),0,led_cfg.NrLeds)	;
			else if  	(msg.fullMatch("/lycn",addrOffset))  		deck[0].cfg.layer.clear_Nr_leds =  constrain(uint16_t(msg.getInt(0)),1,led_cfg.NrLeds)	;
		
			else if (msg.fullMatch("/pause",addrOffset))    	{ write_bool(PAUSE_DISPLAY,		bool(msg.getInt(0) )) ;   }
			else if (msg.fullMatch("/seq",addrOffset))    		{ write_bool(SEQUENCER_ON,		bool(msg.getInt(0) )) ;  led_cfg.confSwitch_time = ( micros() +  play_conf_time_min[led_cfg.Play_Nr] * MICROS_TO_MIN )  ;  }
			else if (msg.fullMatch("/playnr",addrOffset))   	{ FS_play_conf_read(uint8_t(msg.getInt(0) ) ,&deck[0].cfg ,&deck[0].fx1_cfg    )   ; }
			else if (msg.fullMatch("/layreset",addrOffset))   	{ LEDS_clear_all_layers(0) ; for (uint8_t layer = 0 ; layer < MAX_LAYERS_SELECT ; layer++) 	osc_queu_MSG_int("/ostc/master/laye/" + String(layer) , 	deck[0].cfg.layer.select[layer]	); }
			else if (msg.fullMatch("/layall",addrOffset))   	{ LEDS_default_layers(0) ;   for (uint8_t layer = 0 ; layer < MAX_LAYERS_SELECT ; layer++) 	osc_queu_MSG_int("/ostc/master/laye/" + String(layer) , 	deck[0].cfg.layer.select[layer]	); }

			else if (msg.fullMatch("/confname",addrOffset))		{ int length=msg.getDataLength(0); memset(deck[0].cfg.confname, 0, 	 	sizeof(deck[0].cfg.confname)		);    	msg.getString( 0,	deck[0].cfg.confname,  		length ) ;  debugMe(String(deck[0].cfg.confname));  }

			else if (	(msg.match("/tmin",addrOffset))
					|| (msg.match("/laye",addrOffset))
					|| (msg.match("/auto",addrOffset))
					|| (msg.match("/save",addrOffset))
					|| (msg.match("/load",addrOffset))
					|| (msg.match("/cler",addrOffset))
					|| (msg.match("/lymx",addrOffset))
					|| (msg.match("/lylv",addrOffset))
					|| (msg.match("/lynl",addrOffset)) 
					|| (msg.match("/lysl",addrOffset))
			) 
			{
							char address[5] ;
							String save_no_string;
							uint8_t sel_save_no = 0;
							memset(address, 0, sizeof(address));
							msg.getAddress(address, addrOffset + 6);

							for (byte i = 0; i < sizeof(address); i++)  { save_no_string = save_no_string + address[i]; }

							sel_save_no = save_no_string.toInt();  

							if  		(msg.match("/tmin",addrOffset))  	play_conf_time_min[sel_save_no] = uint16_t(msg.getInt(0))	;
							else if  	(msg.match("/laye",addrOffset))  	deck[0].cfg.layer.select[sel_save_no] = uint8_t(msg.getInt(0))	;
							else if  	(msg.match("/lymx",addrOffset))  	deck[0].cfg.layer.save_mix[sel_save_no] = uint8_t(msg.getInt(0))	;
							else if  	(msg.match("/lylv",addrOffset))  	deck[0].cfg.layer.save_lvl[sel_save_no] = uint8_t(msg.getInt(0))	;
							else if  	(msg.match("/lynl",addrOffset))  	deck[0].cfg.layer.save_NrLeds[sel_save_no] = uint16_t(msg.getInt(0))	;
							else if  	(msg.match("/lysl",addrOffset))  	deck[0].cfg.layer.save_startLed[sel_save_no] = uint16_t(msg.getInt(0))	;
							else if		(msg.match("/auto",addrOffset))		{ LEDS_write_sequencer( uint8_t(sel_save_no), boolean(msg.getInt(0)) ); } 
							else if (boolean(msg.getInt(0)))  // if pushdown only
							{
								
								//debugMe(conf_NR);
								if 			(msg.match("/save",addrOffset))		
								{
									LEDS_G_LoadSAveFade(true ,sel_save_no) ;
											/*
											
									    FS_play_conf_write(sel_save_no) ;
									 	{  
											
									 		osc_queu_MSG_rgb(String("/ostc/master/conf/l/"+String(sel_save_no)), 0,255,0); 
									 		OSCBundle bundle_out;
											IPAddress ip_out(osc_server.remoteIP());
																						
											char ConfOutAddress[25] ;
											String CounfOutString = "/ostc/master/savename/" + String(sel_save_no);
											
											CounfOutString.toCharArray(ConfOutAddress, CounfOutString.length() + 1);
											bundle_out.add(ConfOutAddress ).add(deck[0].cfg.confname);
											osc_server.beginPacket(ip_out , OSC_OUTPORT);   //osc_server.remotePort());//
											bundle_out.send(osc_server);
											osc_server.endPacket();
											bundle_out.empty(); 
									 	}*/
								}
								else if 	(msg.match("/load",addrOffset))		
									{ 
										LEDS_G_LoadSAveFade(false,sel_save_no) ;


/*										FS_play_conf_read(sel_save_no,&deck[0].cfg, &deck[0].fx1_cfg);   
										LEDS_pal_reset_index();  
											OSCBundle bundle_out;
											IPAddress ip_out(osc_server.remoteIP());
											bundle_out.add("/ostc/master/confname").add(deck[0].cfg.confname);
											
											char ConfOutAddress[25] ;
											String CounfOutString = "/ostc/master/savename/" + String(sel_save_no);
											
											CounfOutString.toCharArray(ConfOutAddress, CounfOutString.length() + 1);
											bundle_out.add(ConfOutAddress ).add(deck[0].cfg.confname);
											osc_server.beginPacket(ip_out , OSC_OUTPORT);   //osc_server.remotePort());//
											bundle_out.send(osc_server);
											osc_server.endPacket();
											bundle_out.empty();*/
									}
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

	LEDS_pal_write(&deck[0].cfg.LEDS_pal_cur[led_cfg.edit_pal] ,led_cfg.edit_pal, color_no, 0, uint8_t(msg.getInt(0)));
	LEDS_pal_write(&deck[0].cfg.LEDS_pal_cur[led_cfg.edit_pal] ,led_cfg.edit_pal, color_no, 1, uint8_t(msg.getInt(1)));
	LEDS_pal_write(&deck[0].cfg.LEDS_pal_cur[led_cfg.edit_pal] ,led_cfg.edit_pal, color_no, 2, uint8_t(msg.getInt(2)));

}







void osc_StC_pal_routing(OSCMessage &msg, int addrOffset) 
{
	
	if 		(msg.fullMatch("/edit/edit",addrOffset))			{ led_cfg.edit_pal = uint8_t(msg.getInt(0));  osc_StC_menu_pal_ref(led_cfg.edit_pal) ;  }
	else if (msg.fullMatch("/edit/load",addrOffset))			{ LEDS_pal_load( &deck[0].cfg.LEDS_pal_cur[led_cfg.edit_pal] , led_cfg.edit_pal	, uint8_t(msg.getInt(0)) ); osc_StC_menu_pal_ref(led_cfg.edit_pal) ;  }
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


//////////////////////////////////// API 
///


// API Queu messages
	
/// ///////////////////////
/// API Sys

void osc_api_sys_dataOn(OSCMessage &msg, int addrOffset) 
{
	switch(msg.getInt(0))
	{
		case 1:
			write_bool(DATA1_ENABLE, bool(msg.getInt(1) )) ;
		break;
		case 2:
			write_bool(DATA2_ENABLE, bool(msg.getInt(1) )) ;
		break;
		case 3:
			write_bool(DATA3_ENABLE, bool(msg.getInt(1) )) ;
		break;
		case 4:
			write_bool(DATA4_ENABLE, bool(msg.getInt(1) )) ;
		break;
	}


}

void osc_api_sys_led_ref()
{
	
	for (int i = 0 ; i < 4 ; i++)
	{
		osc_queu_MSG_int("/api/sys/DataSL/" + String(i), 		led_cfg.DataStart_leds[i] );
		osc_queu_MSG_int("/api/sys/DataNL/" + String(i), 		led_cfg.DataNR_leds[i] );
		osc_queu_MSG_int("/api/sys/DataOn/" + String(i), 		get_bool(DATA1_ENABLE+i) );  

	}
	osc_queu_MSG_int("/api/sys/pots/enable",      		get_bool(POT_DISABLE));
	osc_queu_MSG_int("/api/sys/pots/lvlmaster",      	get_bool(POTS_LVL_MASTER));
	osc_queu_MSG_int("/api/sys/MirrorMNrLeds", 		led_cfg.NrLeds);   		// Mirror mode NR of leds
	osc_queu_MSG_int("/api/sys/LedMode", 		led_cfg.ledMode);
	osc_queu_MSG_int("/api/sys/APADatarate", 	led_cfg.apa102data_rate);
	osc_queu_MSG_int("/api/sys/MaxBri", 	  	led_cfg.max_bri);

}

void osc_api_sys(OSCMessage &msg, int addrOffset) 
{
	
	//msg.route("/asd", 		osc_api_pal_rec , 	addrOffset);   // Routing for PALLETE TAB -  API 
	if (msg.fullMatch("/LedMode",addrOffset))  	{ led_cfg.ledMode = uint8_t(msg.getInt(0) )  ;}   //
	else if (msg.fullMatch("/APADatarate",addrOffset)) 	{ led_cfg.apa102data_rate = uint8_t(msg.getInt(0) )  ;}
	else if (msg.fullMatch("/MirrorMNrLeds",addrOffset)){ led_cfg.NrLeds 	  = constrain(uint16_t(msg.getInt(0) ) , 0 , led_cfg.NrLeds ); }
	else if (msg.fullMatch("/MaxBri",addrOffset)) 		{ led_cfg.max_bri = constrain(uint8_t(msg.getInt(0)) , 0 , 255) ; }
	else if (msg.fullMatch("/Pots",addrOffset)) 		{ write_bool(POT_DISABLE,  bool(msg.getInt(0) )) ;}
	else if (msg.fullMatch("/DataSL",addrOffset))  		{ uint8_t DataNr = constrain(msg.getInt(0), 0 , 3 )  ;    led_cfg.DataStart_leds[DataNr] = constrain(uint16_t(msg.getInt(1) ) , 0 , led_cfg.NrLeds - led_cfg.DataNR_leds[DataNr] );}
	else if (msg.fullMatch("/DataNL",addrOffset))  		{ uint8_t DataNr = constrain(msg.getInt(0), 0 , 3 )  ;    led_cfg.DataNR_leds[DataNr] 	 = constrain(uint16_t(msg.getInt(1) ) , 0 , led_cfg.NrLeds - led_cfg.DataStart_leds[DataNr] );}
	else if (msg.fullMatch("/CalcSL",addrOffset))  		{ uint8_t CalcSLNr = constrain(msg.getInt(0), 1 , 3 )  ;   led_cfg.DataStart_leds[CalcSLNr ]  =  constrain(  led_cfg.DataNR_leds[CalcSLNr -1 ] + led_cfg.DataStart_leds[CalcSLNr-1] ,0, led_cfg.NrLeds )  ;  osc_queu_MSG_int("/api/sys/CalcSL/" + String(CalcSLNr), 	led_cfg.DataStart_leds[CalcSLNr] ); } 
	else if (msg.fullMatch("/leds/save",addrOffset))    { FS_Bools_write(0) ;  }
	else if (msg.fullMatch("/leds/ref",addrOffset))     { osc_api_sys_led_ref(); }
	//else if (msg.fullMatch("/DataOn" ,addrOffset)) { write_bool(DATA1_ENABLE, bool(msg.getInt(0) )) ;   }
	else if (msg.fullMatch("/DataOn",addrOffset))  		{write_bool(DATA1_ENABLE + constrain(msg.getInt(0), 0 , 3 ) , bool(msg.getInt(1) )) ; }
	//msg.route("/DataOn", 		osc_api_sys_dataOn , 	addrOffset); {write_bool(DATA1_ENABLE + msg.getInt(0) , bool(msg.getInt(1) )) ; }
}





/////////////////////////////////
////////// API  Pal 



void osc_api_pal_refall()        // Palletes
{
		for (int pal = 0; pal < NR_PALETTS; pal++) 
		
			for (int i = 0; i < 16; i++) 
			{
					osc_queu_MSG_rgb( String("/api/pal/set/"+ String(pal) +"/" + String(i) ) ,  LEDS_pal_read(pal,i,0), LEDS_pal_read(pal,i,1), LEDS_pal_read(pal,i,2) );	
			}		
		for (int pal = 16; pal <= 28; pal++) 	
			for (int i = 0; i < 16; i++) 
			{
					
					osc_queu_MSG_rgb( String("/api/pal/set/"+ String(pal) +"/" + String(i) ) ,  LEDS_pal_read(pal,i,0), LEDS_pal_read(pal,i,1), LEDS_pal_read(pal,i,2) );	
			}

} 

void osc_api_pal_ref(uint8_t palin)   // Palletes
{

	for (int i = 0; i < 16; i++) 
		{
				osc_queu_MSG_rgb( String("/api/pal/set/"+ String(palin) +"/" + String(i) ) ,  LEDS_pal_read(palin,i,0), LEDS_pal_read(palin,i,1), LEDS_pal_read(palin,i,2) );	
		}

		
	//}
	//osc_queu_MSG_int("/ostc/pal/edit/edit", led_cfg.edit_pal); 
}








void osc_api_fx_pal_ref(uint8_t InDeck = 0)
{
		//debugMe("inFormRef api");


		osc_queu_MSG_fx_Bool("/ref/fx/pal/run", InDeck , deck[InDeck].cfg.form_menu_pal, _M_FORM_PAL_RUN);

		osc_queu_MSG_fx_Bool("/ref/fx/pal/ocl", InDeck , deck[InDeck].cfg.form_menu_pal, _M_FORM_PAL_ONECOLOR);
		osc_queu_MSG_fx_Bool("/ref/fx/pal/rev", InDeck , deck[InDeck].cfg.form_menu_pal, _M_FORM_PAL_REVERSED);
		osc_queu_MSG_fx_Bool("/ref/fx/pal/mir", InDeck , deck[InDeck].cfg.form_menu_pal, _M_FORM_PAL_MIRROR);
		osc_queu_MSG_fx_Bool("/ref/fx/pal/bld", InDeck , deck[InDeck].cfg.form_menu_pal, _M_FORM_PAL_BLEND);
		//osc_queu_MSG_fx_Bool("/ref/fx/pal/sff", InDeck , deck[InDeck].form_menu_pal, _M_FORM_PAL_SPEED_FROM_FFT);

		uint8_t outArray[NR_FORM_PARTS];

		for (uint8_t part = 0; part < NR_FORM_PARTS ; part++)  		outArray[part] = deck[InDeck].cfg.form_fx_pal[part].level; 				osc_queu_MSG_fx_Int("/ref/fx/pal/lvl", InDeck , outArray);
		for (uint8_t part = 0; part < NR_FORM_PARTS ; part++)  		outArray[part] = deck[InDeck].cfg.form_fx_pal[part].index_add_led; 		osc_queu_MSG_fx_Int("/ref/fx/pal/ald", InDeck , outArray);
		for (uint8_t part = 0; part < NR_FORM_PARTS ; part++)  		outArray[part] = deck[InDeck].cfg.form_fx_pal[part].index_add_frame; 	osc_queu_MSG_fx_Int("/ref/fx/pal/afm", InDeck , outArray);
		for (uint8_t part = 0; part < NR_FORM_PARTS ; part++)  		outArray[part] = deck[InDeck].cfg.form_fx_pal[part].index_start; 		osc_queu_MSG_fx_Int("/ref/fx/pal/sid", InDeck , outArray);

		for (uint8_t part = 0; part < _M_NR_FORM_BYTES_ ; part++)  	outArray[part] = deck[InDeck].cfg.form_fx_pal_singles[part].mix_mode; 			osc_queu_MSG_fx_Int("/ref/fx/pal/mix", InDeck , outArray);
		for (uint8_t part = 0; part < _M_NR_FORM_BYTES_ ; part++)  	outArray[part] = deck[InDeck].cfg.form_fx_pal_singles[part].pal; 				osc_queu_MSG_fx_Int("/ref/fx/pal/pal", InDeck , outArray);
		for (uint8_t part = 0; part < _M_NR_FORM_BYTES_ ; part++)  	outArray[part] = deck[InDeck].cfg.form_fx_pal_singles[part].triggerBin; 		osc_queu_MSG_fx_Int("/ref/fx/pal/tgp", InDeck , outArray);
		for (uint8_t part = 0; part < _M_NR_FORM_BYTES_ ; part++)  	outArray[part] = deck[InDeck].cfg.form_fx_pal_singles[part].palSpeedBin; 		osc_queu_MSG_fx_Int("/ref/fx/pal/stg", InDeck , outArray);
		for (uint8_t part = 0; part < _M_NR_FORM_BYTES_ ; part++)  	outArray[part] = deck[InDeck].cfg.form_fx_pal_singles[part].lvl_bin; 			osc_queu_MSG_fx_Int("/ref/fx/pal/lvb", InDeck , outArray);
		for (uint8_t part = 0; part < _M_NR_FORM_BYTES_ ; part++)  	outArray[part] = deck[InDeck].cfg.form_fx_pal_singles[part].master_lvl; 		osc_queu_MSG_fx_Int("/ref/fx/pal/mlv", InDeck , outArray);
} 


// OSC MESSAGE :    Int PalNr , int ColorNr, int Red , int Green , intBlue
// Recive a Palette Color.
void osc_api_pal_rec(OSCMessage &msg, int addrOffset)
 {
	
	int PalNr = msg.getInt(0);
	int ColorNr = msg.getInt(1);

	if(PalNr>= 0 && PalNr < NR_PALETTS && ColorNr>= 0 && ColorNr < 16   ) 
	{
		LEDS_pal_write(&deck[0].cfg.LEDS_pal_cur[PalNr] ,PalNr, ColorNr, 0, uint8_t(msg.getInt(2)));
		LEDS_pal_write(&deck[0].cfg.LEDS_pal_cur[PalNr] ,PalNr, ColorNr, 1, uint8_t(msg.getInt(3)));
		LEDS_pal_write(&deck[0].cfg.LEDS_pal_cur[PalNr] ,PalNr, ColorNr, 2, uint8_t(msg.getInt(4)));
	}
}






void osc_api_pal(OSCMessage &msg, int addrOffset) 
{
	
	msg.route("/set", 		osc_api_pal_rec , 	addrOffset);   // Routing for PALLETE TAB -  API 
	if (msg.fullMatch("/ref",addrOffset))	   	osc_api_pal_ref(constrain(msg.getInt(0),0,255) ) ;   //
	if (msg.fullMatch("/refall",addrOffset))	osc_api_pal_refall() ;   //
	if (msg.fullMatch("/loadin",addrOffset)) 	{ LEDS_load_default_play_conf(); LEDS_pal_load( &deck[0], constrain(msg.getInt(0),0,NR_PALETTS-1 ), constrain(msg.getInt(1),0,NR_PALETTS_SELECT-1)  );  osc_api_pal_ref(constrain(msg.getInt(0),0,NR_PALETTS -1) ) ;   }
	//if (msg.fullMatch("/copyPal",addrOffset))	osc_api_pal_copy();  LEDS_pal_load(  constrain(msg.getInt(0),0,NR_PALETTS)	, uint8_t(msg.getInt(1)) ); osc_StC_menu_pal_ref(led_cfg.edit_pal) ;  }  //
	
}

void osc_api_refreshAll()
{
	Refreshloop = 0;
	debugMe(Refreshloop);
	

}
void osc_api_refreshAllLoop()
{
	
	switch(Refreshloop)
	{
		case 0:
			osc_api_pal_refall() ;
		break;
		case 1:
			osc_StC_menu_audio_ref() ;
		break;
		case 2:
			osc_StC_menu_master_ref();
		break;
		case 3:
			 osc_StC_menu_audio_ref() ;
		break;
		case 4:
			osc_StC_menu_form_led_adv_ref();
		break;
		case 5:
			osc_oStC_menu_master_wifi_ref(); 
		break;
		case 6:
			osc_StC_menu_master_ledcfg_ref();
		break;
		case 7:
			osc_oStC_menu_master_mqtt_ref();
		break;
		case 8:
			osc_StC_menu_master_artnet_ref();
		break;
		case 9:
			osc_StC_menu_form_pal_adv_ref(0);
		break;
		case 10:
			osc_StC_menu_form_pal_adv_ref(1);
		break;
		case 11:
			osc_StC_menu_form_pal_adv_ref(2);
		break;
		case 12:
			osc_StC_menu_form_pal_adv_ref(3);
		break;
		case 13:
			osc_StC_menu_form_fft_adv_ref(0);
		break;
		case 14:
			osc_StC_menu_form_fft_adv_ref(1);
		break;
		case 15:
			osc_StC_menu_form_fft_adv_ref(2);
		break;
		case 16:
			osc_StC_menu_form_fft_adv_ref(3);
		break;
		case 17:
			osc_StC_menu_form_fx_fire_adv_ref(0);
		break;
		case 18:
			osc_StC_menu_form_fx_fire_adv_ref(1);
		break;
		case 19:
			osc_StC_menu_form_fx_fire_adv_ref(2);
		break;
		case 20:
			osc_StC_menu_form_fx_fire_adv_ref(3);
		break;
		case 21:
			osc_StC_menu_form_shim_adv_ref(0);
		break;
		case 22:
			osc_StC_menu_form_shim_adv_ref(1);
		break;
		case 23:
			osc_StC_menu_form_shim_adv_ref(2);
		break;
		case 24:
			osc_StC_menu_form_shim_adv_ref(3);
		break;
		case 25:
			osc_StC_menu_form_glit_adv_ref(0);
		break;
		case 26:
			osc_StC_menu_form_glit_adv_ref(1);
		break;
		case 27:
			osc_StC_menu_form_glit_adv_ref(2);
		break;
		case 28:
			osc_StC_menu_form_glit_adv_ref(3);
		break;
		case 29:
			osc_StC_menu_form_dot_adv_ref(0);
		break;
		case 30:
			osc_StC_menu_form_dot_adv_ref(1);
		break;
		case 31:
			osc_StC_menu_form_dot_adv_ref(2);
		break;
		case 32:
			osc_StC_menu_form_dot_adv_ref(3);
		break;
		case 33:
			osc_StC_menu_form_fx1_adv_ref(0);
		break;
		case 34:
			osc_StC_menu_form_fx1_adv_ref(1);
		break;
		case 35:
			osc_StC_menu_form_fx1_adv_ref(2);
		break;
		case 36:
			osc_StC_menu_form_fx1_adv_ref(3);
		break;
		case 37:
			osc_StC_menu_form_fx_strobe_adv_ref(0);
		break;
		case 38:
			osc_StC_menu_form_fx_strobe_adv_ref(1);
		break;
		case 39:
			osc_StC_menu_form_fx_strobe_adv_ref(2);
		break;
		case 40:
			osc_StC_menu_form_fx_strobe_adv_ref(3);
		break;
		case 41:
			osc_StC_menu_form_fx_meteor_adv_ref(0);
		break;
		case 42:
			osc_StC_menu_form_fx_meteor_adv_ref(1);
		break;
		case 43:
			osc_StC_menu_form_fx_meteor_adv_ref(2);
		break;
		case 44:
			osc_StC_menu_form_fx_meteor_adv_ref(3);
		break;
		case 45:
			osc_StC_menu_form_fx_eyes_adv_ref(0);
		break;
		case 46:
			osc_StC_menu_form_fx_eyes_adv_ref(1);
		break;
		case 47:
			osc_StC_menu_form_fx_eyes_adv_ref(2);
		break;
		case 48:
			osc_StC_menu_form_fx_eyes_adv_ref(3);
		break;
		case 49:
			osc_StC_menu_form_modify_adv_ref(0);
		break;
		case 50:
			osc_StC_menu_form_modify_adv_ref(1);
		break;
		case 51:
			osc_StC_menu_form_modify_adv_ref(2);
		break;
		case 52:
			osc_StC_menu_form_modify_adv_ref(3);
		break;
		default :
		Refreshloop = 254;
		break;






	}
	Refreshloop++;




}



// OSC MESSAGE :    Int PalNr , int ColorNr, int Red , int Green , intBlue
// Recive a Palette Color.
void osc_api_fx(OSCMessage &msg, int addrOffset)
 {
	
	 int DeckNr     = msg.getInt(0);
	 int OrigFormNr = msg.getInt(1);
	 int result     = msg.getInt(2);

	uint8_t i_form_nr =  OrigFormNr;
	uint8_t i_bit_int = 0;
	while (i_form_nr >=8)
	{
		i_bit_int++;
		i_form_nr = i_form_nr-8;
	}
	debugMe(DeckNr);
	debugMe(OrigFormNr);
	debugMe(result);
    if (msg.fullMatch("/pal/run",addrOffset))  { bitWrite(deck[DeckNr].cfg.form_menu_pal[i_bit_int][_M_FORM_PAL_RUN], OrigFormNr, 	bool(result));  }
	if (msg.fullMatch("/pal/ocl",addrOffset))  { bitWrite(deck[DeckNr].cfg.form_menu_pal[i_bit_int][_M_FORM_PAL_ONECOLOR], OrigFormNr, 	bool(result));  }
	if (msg.fullMatch("/pal/rev",addrOffset))  { bitWrite(deck[DeckNr].cfg.form_menu_pal[i_bit_int][_M_FORM_PAL_REVERSED], OrigFormNr, 	bool(result));  }
	if (msg.fullMatch("/pal/mir",addrOffset))  { bitWrite(deck[DeckNr].cfg.form_menu_pal[i_bit_int][_M_FORM_PAL_MIRROR], OrigFormNr, 	bool(result));  }
	if (msg.fullMatch("/pal/bld",addrOffset))  { bitWrite(deck[DeckNr].cfg.form_menu_pal[i_bit_int][_M_FORM_PAL_BLEND], OrigFormNr, 	bool(result));  }
	//if (msg.fullMatch("/pal/sff",addrOffset))  { bitWrite(deck[DeckNr].form_menu_pal[i_bit_int][_M_FORM_PAL_SPEED_FROM_FFT], OrigFormNr, 	bool(result));  }

	
	else if	(msg.fullMatch("/pal/lvl",addrOffset))	 {deck[DeckNr].cfg.form_fx_pal[OrigFormNr].level  				=  uint8_t(result)  ;}
	else if	(msg.fullMatch("/pal/mix",addrOffset))	 {deck[DeckNr].cfg.form_fx_pal_singles[OrigFormNr].mix_mode 		 	=  uint8_t(result)  ;}
	else if	(msg.fullMatch("/pal/pal",addrOffset))	 {deck[DeckNr].cfg.form_fx_pal_singles[OrigFormNr].pal 				 	=  uint8_t(result)  ;}
	else if	(msg.fullMatch("/pal/tgp",addrOffset))	 {deck[DeckNr].cfg.form_fx_pal_singles[OrigFormNr].triggerBin  			=  uint8_t(result)  ;}
	else if	(msg.fullMatch("/pal/stg",addrOffset))	 {deck[DeckNr].cfg.form_fx_pal_singles[OrigFormNr].palSpeedBin  		=  uint8_t(result)  ;}
	else if	(msg.fullMatch("/pal/lvb",addrOffset))	 {deck[DeckNr].cfg.form_fx_pal_singles[OrigFormNr].lvl_bin  			=  uint8_t(result)  ;}
	else if	(msg.fullMatch("/pal/mlv",addrOffset))	 {deck[DeckNr].cfg.form_fx_pal_singles[OrigFormNr].master_lvl  			=  uint8_t(result)  ;}
	else if	(msg.fullMatch("/pal/ald",addrOffset))	 {deck[DeckNr].cfg.form_fx_pal[OrigFormNr].index_add_led  		=  uint8_t(result)  ;}
	else if	(msg.fullMatch("/pal/afm",addrOffset))	 {deck[DeckNr].cfg.form_fx_pal[OrigFormNr].index_add_frame  	=  uint8_t(result)  ;}
	else if	(msg.fullMatch("/pal/sid",addrOffset))	 {deck[DeckNr].cfg.form_fx_pal[OrigFormNr].index_start  		=  uint8_t(result)  ;}
	



	
	

 }




/////////////////////////////////
////////// API  Routing 



void osc_api_routing(OSCMessage &msg, int addrOffset) 
{
	//osc_queu_MSG_rgb( String("/ostc/master/connled" ) ,  	getrand8() ,getrand8() ,getrand8( )  );	

	msg.route("/pal", 		osc_api_pal , 	addrOffset);   // Routing for PALLETE TAB -  API
	msg.route("/sys", 		osc_api_sys , 	addrOffset); 
	msg.route("/fx", 		osc_api_fx  ,   addrOffset); 
	if (msg.fullMatch("/refreshAll",addrOffset))   osc_api_fx_pal_ref(); //osc_api_refreshAll();



}

///////////////////////////////////////////////////////////////////-------------- END OPEN STage Controll
///////////////////////////////////////////////////////////////////  Start TouchOSC Iphone

void osc_tosc_refresh()
{
	osc_queu_MSG_float("/tosc/bri", byte_tofloat(deck[0].cfg.led_master_cfg.bri, 255)) ;
	osc_queu_MSG_float("/tosc/bril", float(deck[0].cfg.led_master_cfg.bri));
	osc_queu_MSG_float("/tosc/ups", byte_tofloat(deck[0].cfg.led_master_cfg.pal_fps, MAX_PAL_FPS));
	osc_queu_MSG_float("/tosc/upsl", float(deck[0].cfg.led_master_cfg.pal_fps));
	osc_queu_MSG_float("/tosc/FPSL", LEDS_get_FPS());
	osc_queu_MSG_float("/tosc/r", byte_tofloat(deck[0].cfg.led_master_cfg.r,255));
	osc_queu_MSG_float("/tosc/g", byte_tofloat(deck[0].cfg.led_master_cfg.g,255));
	osc_queu_MSG_float("/tosc/b", byte_tofloat(deck[0].cfg.led_master_cfg.b,255));
	
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
	if 		(msg.fullMatch("/bri",addrOffset))										{ deck[0].cfg.led_master_cfg.bri	= byte(msg.getFloat(0)	* 255); }
	else if (msg.fullMatch("/ups",addrOffset))										{ deck[0].cfg.led_master_cfg.pal_fps = constrain(byte(msg.getFloat(0) * MAX_PAL_FPS), 1, MAX_PAL_FPS); osc_queu_MSG_float("/tosc/upsl", float(deck[0].cfg.led_master_cfg.pal_fps)); }
 	else if (msg.fullMatch("/r",addrOffset))				{ deck[0].cfg.led_master_cfg.r		= byte(msg.getFloat(0)	* 255); }
    else if (msg.fullMatch("/g",addrOffset))				{ deck[0].cfg.led_master_cfg.g		= byte(msg.getFloat(0)	* 255); }
    else if (msg.fullMatch("/b",addrOffset))				{ deck[0].cfg.led_master_cfg.b		= byte(msg.getFloat(0)	* 255); }
	else if (msg.fullMatch("/FPS", addrOffset))										osc_queu_MSG_float("/tosc/FPSL", LEDS_get_FPS());
	
	else if (msg.fullMatch("/ref", addrOffset) && bool(msg.getFloat(0)) == true)			{ osc_tosc_refresh(); }
	else if (msg.fullMatch("/RESET", addrOffset) && bool(msg.getFloat(0)) == true)			{ESP.restart(); }
	else if (msg.fullMatch("/IPSAVE", addrOffset) && bool(msg.getFloat(0)) == true) 		{FS_wifi_write(); FS_Bools_write(0); }
	else if (msg.fullMatch("/ARTNETSAVE", addrOffset) && bool(msg.getFloat(0)) == true) 		{FS_artnet_write(); }
		
	else if (msg.fullMatch("/WAP", addrOffset))												{ write_bool(WIFI_MODE_TPM, bool(msg.getFloat(0))); }//debugMe("BLAH!!!");debugMe(get_bool(WIFI_MODE_TPM)); }
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
					FS_play_conf_read(addr1,&deck[0].cfg, &deck[0].fx1_cfg);

				
				}
				else if (msg.match("/LD2", addrOffset)  &&  (bool(msg.getFloat(0)) == true)) 
				{
					FS_play_conf_read(addr1+8,&deck[0].cfg, &deck[0].fx1_cfg);

				
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
							
								led_cfg.DataStart_leds[addr2]  = constrain(led_cfg.DataStart_leds[addr2] -  100, 0, led_cfg.NrLeds - led_cfg.DataNR_leds[addr2] );
								break;
							case 1:
								led_cfg.DataStart_leds[addr2]  = constrain(led_cfg.DataStart_leds[addr2] -  10, 0, led_cfg.NrLeds - led_cfg.DataNR_leds[addr2] );
								break;
							case 2:
								led_cfg.DataStart_leds[addr2]  = constrain(led_cfg.DataStart_leds[addr2] -  1, 0, led_cfg.NrLeds - led_cfg.DataNR_leds[addr2] );
								break;
							case 3:
								if (addr2 == 0)
									led_cfg.DataStart_leds[addr2]  = 0;
								else 
									led_cfg.DataStart_leds[addr2]  = led_cfg.DataNR_leds[addr2-1] + led_cfg.DataStart_leds[addr2-1] ;
							break;
							case 4:
								led_cfg.DataStart_leds[addr2]  = constrain(led_cfg.DataStart_leds[addr2]  +  1, 0, led_cfg.NrLeds - led_cfg.DataNR_leds[addr2] );
							break;
							case 5:
								led_cfg.DataStart_leds[addr2]  = constrain(led_cfg.DataStart_leds[addr2]  +  10, 0, led_cfg.NrLeds - led_cfg.DataNR_leds[addr2] );
							break;
							case 6:
								led_cfg.DataStart_leds[addr2]  = constrain(led_cfg.DataStart_leds[addr2]  +  100, 0, led_cfg.NrLeds - led_cfg.DataNR_leds[addr2] );
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
							
								led_cfg.DataNR_leds[addr2]  = constrain(led_cfg.DataNR_leds[addr2] -  100, 0, led_cfg.NrLeds - led_cfg.DataStart_leds[addr2] );
								break;
							case 1:
								led_cfg.DataNR_leds[addr2]  = constrain(led_cfg.DataNR_leds[addr2] -  10, 0, led_cfg.NrLeds - led_cfg.DataStart_leds[addr2] );
								break;
							case 2:
								led_cfg.DataNR_leds[addr2]  = constrain(led_cfg.DataNR_leds[addr2] -  1, 0, led_cfg.NrLeds - led_cfg.DataStart_leds[addr2] );
								break;
							case 3:
								//led_cfg.DataNR_leds[addr2]  = 0;
							break;
							case 4:
								led_cfg.DataNR_leds[addr2]  = constrain(led_cfg.DataNR_leds[addr2]  +  1, 0, led_cfg.NrLeds - led_cfg.DataStart_leds[addr2] );
							break;
							case 5:
								led_cfg.DataNR_leds[addr2]  = constrain(led_cfg.DataNR_leds[addr2]  +  10, 0, led_cfg.NrLeds - led_cfg.DataStart_leds[addr2] );
							break;
							case 6:
								led_cfg.DataNR_leds[addr2]  = constrain(led_cfg.DataNR_leds[addr2]  +  100, 0, led_cfg.NrLeds - led_cfg.DataStart_leds[addr2] );
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
			debugMe(address,false);
			debugMe(" : ",false);
			debugMe(oscMSG.getInt(0));
			//debugMe(oscMSG.getFloat(0));
			//debugMe(oscMSG.getInt(0));

			oscMSG.route("/ostc", osc_StC_routing);   // Routing for Open Stage Control
			oscMSG.route("/api", osc_api_routing);   // Routing for Open Stage Control

			oscMSG.route("/tosc", osc_tosc_routing);	// Routing for touchosc

			if (oscMSG.fullMatch("/reset-index", 0) && bool(oscMSG.getFloat(0)) == true) LEDS_pal_reset_index();
		}
		else {
			//error = bundle.getError();
			debugMe("OSC error: ");	
			debugMe( oscMSG.getError());
		}
	}   //else debugMe("XXXXX");


	if (osc_send_out_float_MSG_buffer() == false && Refreshloop < 255  )  osc_api_refreshAllLoop();

	osc_send_out_API_FX_MSG_buffer() ;
}


