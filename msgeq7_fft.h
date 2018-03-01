// msgeq7_fft.h

#ifndef _MSGEQ7_FFT_h
#define _MSGEQ7_FFT_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif






#define MSGEQ7_INPUT_PIN  A4 // (=18) //A1    // input from mic
#define MSGEQ7_STROBE_PIN 16 //3		// stobe pin
#define MSGEQ7_RESET_PIN  15 //4		// reset pin

/*

											MSGEQ7
									 --------\_/-------
				   3v && GND-0.1uF- |1-VDDA 	 CKIN-8|	-- 200k - VVC &&  -- 33pF - GND 
								GND	|2-VSSA		RESET-7|	- Reset PIN (17)
							to A4	|3-OUT		  GND-6|	- 0.1uF - GND
					Strobe pin (19) |4-STROBE	   IN-5|	- 0.01uF -22k - Mic (??33pF to input
									 ------------------

									 ADAfruit MAX9814
									 |GND
									 |V+
									 |Gain
									 |out
									 |AR

									 Teensy-LC      
									 9 = RX2
									 10 = TX2




*/


// Functions
void MSGEQ7_setup();
void MSGEQ7_get();


#endif

