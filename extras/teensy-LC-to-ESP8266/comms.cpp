// 
// 
// 

#include "comms.h"


#include <CmdMessenger.h>  // CmdMessenger
CmdMessenger cmdMessenger = CmdMessenger(CMD_MESSEGER_PORT);



// EXTERNAL's

// from mseq7.cpp
extern int Msgeq7_spectrumValue[7];		// the fft bins
extern uint8_t fps;



void Com_master_Send_FFT_bins() {


	cmdMessenger.sendCmdStart(Teensy_send_FFT);
	cmdMessenger.sendCmdBinArg<byte>(fps);
	for (int i = 0; i < 7; i++)
		cmdMessenger.sendCmdBinArg<byte>(Msgeq7_spectrumValue[i]);

	cmdMessenger.sendCmdEnd();

}

void comms_S_FPS()
{
	cmdMessenger.sendCmdStart(ESP_send_FPS);
	cmdMessenger.sendCmdBinArg<byte>(fps);
	cmdMessenger.sendCmdEnd();

}



// ------------------  C A L L B A C K S -----------------------


void comms_R_master_FFT()
{
	//Reicve the FFT from the Teensy 
	fps = cmdMessenger.readBinArg<byte>();
	for (int i = 0; i < 7; i++)
		Msgeq7_spectrumValue[i] = cmdMessenger.readBinArg<byte>();


	// send the data to the Slave ESP's
};


void comms_R_FPS()
{
	fps = cmdMessenger.readBinArg<byte>();
	Serial.print("in FPS=");
	Serial.println(fps);
}


// Called when a received command has no attached function
void OnUnknownCommand()
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
	cmdMessenger.attach(ESP_send_FPS, comms_R_FPS);     // recive  the FPS from the ESP
	//cmdMessenger.attach(slave_recive_FFT, R_slave_FFT);
	cmdMessenger.attach(Teensy_send_FFT, comms_R_master_FFT);     // Send the FFT from teensy to ESP
	
}


// ------------------ M A I N  ----------------------

// Setup function
void setup_comms()
{
	// Listen on serial connection for messages from the pc
	CMD_MESSEGER_PORT.begin(CMD_MESSGER_SPEED);

	// Adds newline to every command
	cmdMessenger.printLfCr();

	// Attach my application's user-defined callback methods
	attachCommandCallbacks();


}



// Loop function
void comms_loop()
{
	// Process incoming serial data, and perform callbacks
	cmdMessenger.feedinSerialData();

	// DBG_OUTPUT_PORT.println("hello");
	// do something after the Check ? Set something ?

}


