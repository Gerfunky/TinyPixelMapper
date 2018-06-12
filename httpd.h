// httpd.h

#ifndef _HTTPD_h
#define _HTTPD_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

	
	// fror wifi-ota.cpp
	void httpd_setup();
	void http_loop();

	void httpd_toggle_webserver();		//osc.cpp

#endif

