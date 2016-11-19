// 
// 
// 

#include "msgeq7.h"
#include "comms.h"

int Msgeq7_spectrumValue[7];

unsigned long update_time = 0;	// when to get the new data
uint8_t fps = 25;
boolean fft_get = true;

unsigned long  currenTime = 0;
#define SEND_SERIAL Serial2



// External 
// from comms
extern void Com_master_Send_FFT_bins();



void setup_MSGEQ7() {

	//Serial2.begin(115200);
	pinMode(MSGEQ7_INPUT_PIN, INPUT);
	pinMode(MSGEQ7_STROBE_PIN, OUTPUT);
	pinMode(MSGEQ7_RESET_PIN, OUTPUT);
	digitalWrite(MSGEQ7_RESET_PIN, LOW);
	digitalWrite(MSGEQ7_STROBE_PIN, HIGH);

	//SSerial.begin(115200);

	//pinMode(SS_RX_PIN, INPUT);
	//pinMode(SS_TX_PIN, OUTPUT);
	
	//SEND_SERIAL.begin(CMD_MESSGER_SPEED);
	//SEND_SERIAL.println("Starting FFT DATA stream");


}

void get_MSGEQ7() {

	digitalWrite(MSGEQ7_RESET_PIN, HIGH);
	digitalWrite(MSGEQ7_RESET_PIN, LOW);
	delayMicroseconds(75);

	for (int i = 0; i<7; i++) {
		digitalWrite(MSGEQ7_STROBE_PIN, LOW);
		delayMicroseconds(40);
		Msgeq7_spectrumValue[i] = analogRead(MSGEQ7_INPUT_PIN) / 4;   // 
		digitalWrite(MSGEQ7_STROBE_PIN, HIGH);
		delayMicroseconds(40);
	}

	/*
	for (int i=0;i<7;i++)
	{
	Serial2.print(Msgeq7_spectrumValue[i]);
	Serial2.print(" ");
	}*/

	//  return Msgeq7_spectrumValue;

}


void send_MSGEQ7()
{
	//Com_master_Send_FFT_bins();
	SEND_SERIAL.print(fps);
	SEND_SERIAL.print("-");
	for (int i = 0; i<7; i++)
	{
		SEND_SERIAL.print(Msgeq7_spectrumValue[i]);
		SEND_SERIAL.print("-");

	}
	SEND_SERIAL.println("*");
}

void run_MSGEQ7()
{
	 currenTime = millis();
	//SEND_SERIAL.println(currenTime);
	if ((fft_get == true) && (update_time <= currenTime))
	{
		//SEND_SERIAL.println(currenTime);
		//SEND_SERIAL.println(update_time);
		update_time = currenTime + (1000 / fps);
		//SEND_SERIAL.println(update_time);
		get_MSGEQ7();


		Com_master_Send_FFT_bins();
		//send_MSGEQ7();

		if (update_time <= millis()) SEND_SERIAL.println("NOOOO");
		//SSerial.println("HELLO");
		// send the data out of the serial interface
	}


	//if (SEND_SERIAL.available())
		//SEND_SERIAL.write(SEND_SERIAL.read());


	//SSerial.print("*");

}