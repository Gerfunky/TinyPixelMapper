// osc.h

#ifndef _OSC_h
#define _OSC_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "IPAddress.h"


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

