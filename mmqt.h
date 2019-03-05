#ifndef _MMQT_h
#define _MMQT_h


#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#endif

#include "config_TPM.h"	

#define MMQT_BROKER "172.16.222.19"

#define MMQT_DEF_BRI_TOPIC_RECIVE "light/wz/bri/set"
#define MMQT_DEF_BRI_TOPIC_SEND "light/wz/bri/status"

#define MMQT_DEF_STATUS_TOPIC_RECIVE "light/wz/set"
#define MMQT_DEF_STATUS_TOPIC_SEND "light/wz/status"


#define MMQT_DEF_PLAY_TOPIC_RECIVE "light/wz/play/set"
#define MMQT_DEF_PLAY_TOPIC_SEND "light/wz/play/status"


// MMQT Topics
// 
// The topics are automatically generated
// deviceName/Function/action
//
// device:  APname / deviceName
// Action: /set = subsribe topic to set something in the TPM
//         /status = publish the status of the Function 
// Functions : /device = Is it on or off
//              /bri   = Master fader
//              /play  = Play nummer / config
//              /
//



// try 2 

#define MMQT_TOPIC_SEND "/status"
#define MMQT_TOPIC_RECIVE "/set"

#define MMQT_TOPIC_FUNCTION_PLAY    "/play"
#define MMQT_TOPIC_FUNCTION_BRI     "/bri"
#define MMQT_TOPIC_FUNCTION_DEVICE  "/device"

#define MMQT_DEF_NAME           "TPM-WZ"
#define MMQT_DEF_USER          "lights"
#define MMQT_DEF_PASSWORD      "lights123"

#define MMQT_ENABLED  false


void MMQT_setup();
void MMQT_loop()  ;

struct MMQT_config_struct
{
    boolean   enabled;

};







#endif 