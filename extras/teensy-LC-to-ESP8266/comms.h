// comms.h

#ifndef _COMMS_h
#define _COMMS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


enum
{
  // Commands
  kAcknowledge         , //0 Command to acknowledge that cmd was received
  kError               , //1 Command to report errors
  Teensy_send_FFT      , //2 send the FPS and fft data to ESP from Teensy
  ESP_rec_FFT          ,  //3 
  ESP_send_FPS				// 4 send the FPS value to the Teensy
  
};


#define CMD_MESSEGER_PORT Serial2
#define CMD_MESSGER_SPEED 57600





#endif

