// 
// 
// 

#include "msgeq7_fft.h"


int Msgeq7_spectrumValue[7];


void MSGEQ7_setup() {

	pinMode(MSGEQ7_INPUT_PIN, INPUT);
	pinMode(MSGEQ7_STROBE_PIN, OUTPUT);
	pinMode(MSGEQ7_RESET_PIN, OUTPUT);
	digitalWrite(MSGEQ7_RESET_PIN, LOW);
	digitalWrite(MSGEQ7_STROBE_PIN, HIGH);

}




void MSGEQ7_get()
{

	digitalWrite(MSGEQ7_RESET_PIN, HIGH);
	digitalWrite(MSGEQ7_RESET_PIN, LOW);
	delayMicroseconds(75);

	for (int i = 0; i<7; i++)
	{
		digitalWrite(MSGEQ7_STROBE_PIN, LOW);
		delayMicroseconds(40);
		Msgeq7_spectrumValue[i] = analogRead(MSGEQ7_INPUT_PIN) / 4;   // 
		digitalWrite(MSGEQ7_STROBE_PIN, HIGH);
		delayMicroseconds(40);
	}

	
}

