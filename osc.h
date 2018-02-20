// osc.h

#ifndef _OSC_h
#define _OSC_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "IPAddress.h"

/*
#define OSC_IPMULTI_	{239,0,0,58}			// the multicast IP we sent to to brodcast the FFT data
#define OSC_PORT_MULTI_	420						// the multicast port to send to
#define OSC_OUTPORT		9000					// the OSC port we sent to on responsed
#define OSC_INPORT		8000					// The input osc port

#define OSC_BUNDLE_SEND_COUNT 16				// how many OSC messages to send in one bundle.

#define OSC_MULTIPLY_OPTIONS 11 

*/


struct osc_cfg_struct				// OSC configuration structure
{
	IPAddress		ipMulti;		// holder of the Multicast dest IP address for OSC brodcasts
	unsigned int	portMulti;		// the multicast port to send to 
	unsigned int	outPort;		// the OSC port we sent to on responsed
	unsigned int	inPort;			// The input osc port
	uint8_t return_ip_LB;			// the last byte of the return address no routing! 
	uint8_t conf_multiply;			// when using osc so that we can increment by 1,10 and 100
};




#endif

