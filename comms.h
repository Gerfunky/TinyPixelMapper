// comms.h


// TODO implemnt on off over osc or serial buffer dequeu to avoid memory problems


#ifndef _COMMS_h
#define _COMMS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


enum coms_msg_types
{
  // Commands
  kAcknowledge         , //0 Command to acknowledge that cmd was received
  kError               , //1 Command to report errors
  Teensy_send_FFT      , //2 switch play mode
  ESP_rec_FFT          , //3 what effects = byte
  ESP_send_FPS				// 4 send the FPS value to the Teensy
};


#define CMD_MESSEGER_PORT Serial
//#define CMD_MESSGER_SPEED 9600 //57600





#endif

