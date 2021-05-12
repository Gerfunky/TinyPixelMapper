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


//#define OSC_MULTIPLY_OPTIONS 11					// how many multiply options to add to input from touchosc
//#define NO_OF_PALLETS 2

struct osc_cfg_struct				// OSC configuration structure
{
	IPAddress		ipMulti;		// holder of the Multicast dest IP address for OSC brodcasts
	unsigned int	portMulti;		// the multicast port to send to 
	unsigned int	outPort;		// the OSC port we sent to on responsed
	unsigned int	inPort;			// The input osc port
	uint8_t return_ip_LB;			// the last byte of the return address no routing! 
	uint8_t conf_multiply;			// when using osc so that we can increment by 1,10 and 100
};



void OSC_setup();		// wifi-ota.h
void OSC_loop();		// wifi-ota.h
//void osc_StC_FFT_vizIt(); 






#endif

