// 
// 
// 

#include "config_TPM.h"


#ifndef DISABLED_COMS

#include "comms.h"
#include "leds.h"
#include "tools.h"

#ifdef _MSC_VER
	#include <CmdMessenger\CmdMessenger.h>/ CmdMessenger
#else
	#include <CmdMessenger.h>  // CmdMessenger
#endif

CmdMessenger cmdMessenger = CmdMessenger(CMD_MESSEGER_PORT);



// from mseq7.cpp
//extern int Msgeq7_spectrumValue[7];		// the fft bins
//extern uint8_t fps;
extern led_cfg_struct led_cfg;
extern fft_data_struct fft_data[7];
extern fft_led_cfg_struct fft_led_cfg;



void Com_master_Send_FFT_bins() {


	cmdMessenger.sendCmdStart(Teensy_send_FFT);
	cmdMessenger.sendCmdBinArg<byte>(fft_led_cfg.fps);
	for (int i = 0; i < 7; i++)
		cmdMessenger.sendCmdBinArg<byte>(fft_data[i].value);

	cmdMessenger.sendCmdEnd();

}
void comms_S_FPS(uint8_t fps)
{
	cmdMessenger.sendCmdStart(ESP_send_FPS);
	cmdMessenger.sendCmdBinArg<byte>(fps);
	cmdMessenger.sendCmdEnd();
	//debugMe("send-FPS-to teensy");
}

// ------------------  C A L L B A C K S -----------------------


void comms_R_master_FFT()
{
	//Reicve the FFT from the Teensy 
	fft_led_cfg.fps = cmdMessenger.readBinArg<byte>();
	for (int i = 0; i < 7; i++)
		fft_data[i].value = cmdMessenger.readBinArg<byte>();

	

	//debugMe("FPS-in = ", false);
	//debugMe(fft_led_cfg.fps, true);
	WIFI_FFT_master_send();
	// send the data to the Slave ESP's
};

void comms_R_FPS()
{
	fft_led_cfg.fps = cmdMessenger.readBinArg<byte>();

}


// Called when a received command has no attached function
void comms_OnUnknownCommand()
{
	cmdMessenger.sendCmd(kError, "Command without attached callback");
	//  DBG_OUTPUT_PORT.println("UKNOWCommand");
}


// ------ CALLBACK Setup ----

// Commands we send from the PC and want to receive on the Arduino.
// We must define a callback function in our Arduino program for each entry in the list below.

void attachCommandCallbacks()
{
	// Attach callback methods
	//cmdMessenger.attach(OnUnknownCommand);

	//cmdMessenger.attach(slave_recive_FFT, R_slave_FFT);
	cmdMessenger.attach(Teensy_send_FFT, comms_R_master_FFT);     // Send the FFT from teensy to ESP
	cmdMessenger.attach(ESP_send_FPS, comms_R_FPS);     // recive  the FPS from the ESP
}


// ------------------ M A I N  ----------------------

void startSerial(int Speed)
{
	CMD_MESSEGER_PORT.begin(Speed);

}



// Setup function
void setup_comms(boolean bootDebug, int Speed)
{

	if (false == bootDebug)		startSerial(Speed);   // Start the serial if it was not started alredy.

	// Adds newline to every command
	//cmdMessenger.printLfCr();

	// Attach user-defined callback methods
	attachCommandCallbacks();


}



// Loop function
void comms_loop()
{
	// Process incoming serial data, and perform callbacks
	cmdMessenger.feedinSerialData();


}


#endif
