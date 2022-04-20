// osc.h

#ifndef _OSC_h
#define _OSC_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "IPAddress.h"


#define OSC_BUNDLE_SEND_COUNT 18				// how many OSC messages to send in one bundle.
#define OSC_FX_BUNDLE_SEND_COUNT 6				// how many OSC messages to send in one bundle.
#define OSC_CONF_MAX_SAVES 	16					// what is the max amount of saves
#define VIZ_FPS_MAX 15
#define OSC_QEUE_ADD_LEN 30
//#define OSC_MC_SERVER_DISABLED


struct osc_cfg_struct				// OSC configuration structure
{
	IPAddress		ipMulti;		// holder of the Multicast dest IP address for OSC brodcasts
	unsigned int	portMulti;		// the multicast port to send to 
	unsigned int	outPort;		// the OSC port we sent to on responsed
	unsigned int	inPort;			// The input osc port
	//uint8_t return_ip_LB;			// the last byte of the return address no routing! 
	uint8_t conf_multiply;			// when using osc so that we can increment by 1,10 and 100
	
};



void OSC_setup();		// wifi-ota.h
void OSC_loop();		// wifi-ota.h
void osc_StC_Load_confname_Refresh(uint8_t sel_save_no);
void osc_ostc_Start_refreshAll();
bool osc_Isconnected();
//void osc_StC_FFT_vizIt(); 
void osc_StC_menu_master_ref();
//void osc_StC_Send_Confname(uint8_t SaveNo, char ConfName[]);
bool  osc_send_out_float_MSG_buffer() ;
void osc_StC_Send_CharArray(String Address, char ConfName[]);
void osc_queu_MSG_rgb(String addr_string, uint8_t red,uint8_t green,uint8_t blue) ;
void osc_queu_MSG_float(String addr_string, float value) ;
void osc_queu_MSG_int(String addr_string, int value) ;
void osc_queu_MSG_VAL_STRING(String addr_string, String StringValue);
void osc_Send_String(String Address, String StringName);
void osc_StC_ref_lampConfig();


#endif

