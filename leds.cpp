// 
// 
// 

//#define FASTLED_DISABELED

// ******** IDEAS
// FFT adjuster +-1,10,50 on trigger value
// FFT Delay py frame, would need another buffer  
// FFT auto mode 2... lock ratio of channels to each other and just move it + or minus automatically
// FFT Booster on each color to brighten it up by x
// make a new color creator from fft! 
// add fire animation link it to FFT if possible!  

#include "config_TPM.h"    // Load the main config
#include "leds.h"
#include "tools.h"
#include "wifi-ota.h"
#include "config_fs.h"
#include "msgeq7_fft.h"

#ifdef _MSC_VER
	#include <FastLED\FastLED.h>
	#include <RunningAverage\RunningAverage.h>
	//#include <QueueArray\QueueArray.h>
#else
	#include <FastLED.h>
	#include <RunningAverage.h>			// For Auto FFT
	// <QueueArray.h>				// For buffering incoming FFT packets
#endif




// *************** External Functions
// from wifi-ota.cpp

extern artnet_struct artnet_cfg;





// ************** FFT Variables
// FFT Average Buffers for Auto FFT 
	uint8_t FFT_stage1_sample_count = 0;			// used to count the samples in FFT Stage 1  for pulling into Stage 2
	#define FFT_AVERAGE_SAMPLES 30 //60					// How many samples to take for the FFT average = Stage 1
	RunningAverage fft_bin0(FFT_AVERAGE_SAMPLES);	// Buffers for the FFT values
	RunningAverage fft_bin1(FFT_AVERAGE_SAMPLES);
	RunningAverage fft_bin2(FFT_AVERAGE_SAMPLES);
	RunningAverage fft_bin3(FFT_AVERAGE_SAMPLES);
	RunningAverage fft_bin4(FFT_AVERAGE_SAMPLES);
	RunningAverage fft_bin5(FFT_AVERAGE_SAMPLES);
	RunningAverage fft_bin6(FFT_AVERAGE_SAMPLES);

	#define FFT_AVERAGE_SAMPLES_STAGE2 10						// How many  samples to take in Stage 2 auto FFT average
	RunningAverage fft_bin0stage2(FFT_AVERAGE_SAMPLES_STAGE2);	// Buffers for auto FFT Stage 2
	RunningAverage fft_bin1stage2(FFT_AVERAGE_SAMPLES_STAGE2);
	RunningAverage fft_bin2stage2(FFT_AVERAGE_SAMPLES_STAGE2);
	RunningAverage fft_bin3stage2(FFT_AVERAGE_SAMPLES_STAGE2);
	RunningAverage fft_bin4stage2(FFT_AVERAGE_SAMPLES_STAGE2);
	RunningAverage fft_bin5stage2(FFT_AVERAGE_SAMPLES_STAGE2);
	RunningAverage fft_bin6stage2(FFT_AVERAGE_SAMPLES_STAGE2);

	// FFT

	//QueueArray <uint8_t> FFT_fifo;
	//uint8_t	fft_fps;
	fft_led_cfg_struct fft_led_cfg = { 0,1,25,240,11,1 };
	byte fft_menu[3] = { 3,7,200 };

//#define FFT_FIFO_COUNT_0_8_NR_PACKETS 35 //28 //35
//#define FFT_FIFO_COUNT_0_9_NR_PACKETS 28 //21 //28




fft_data_struct fft_data[7] =   // FFT data Sructure 
{ 
	 { 0,0,0,100,0,0,0 }
	,{ 0,0,0,100,0,0,0 }
	,{ 0,0,0,100,0,0,0 }
	,{ 0,0,0,100,0,0,0 }
	,{ 0,0,0,100,0,0,0 }
	,{ 0,0,0,100,0,0,0 }
	,{ 0,0,0,100,0,0,0 }
};   





// ********************* LED Setup  FastLed
	CRGBArray<NUM_LEDS> leds;			// The Led array!
	//CRGB leds[NUM_LEDS];
	//CRGBSet leds_p(leds, NUM_LEDS);





	byte  copy_leds_mode[NR_COPY_LED_BYTES] = { 0,0 };
	led_Copy_Struct copy_leds[NR_COPY_STRIPS] = 
	{
		{ 0,0,0 }
		,{ 0,0,0 }
		,{ 0,0,0 }
		,{ 0,0,0 }
		,{ 0,0,0 }
		,{ 0,0,0 }
		,{ 0,0,0 }
		,{ 0,0,0 }
		,{ 0,0,0 }
		,{ 0,0,0 }
		,{ 0,0,0 }
		,{ 0,0,0 }
		,{ 0,0,0 }
		,{ 0,0,0 }
		,{ 0,0,0 }
		,{ 0,0,0 }
	};



// ******** LED Pallete
	CRGBPalette16 *LEDS_pal_work[NR_PALETTS];			// Make 2 pallets pointers
	CRGBPalette16 LEDS_pal_cur[NR_PALETTS];				//	Make 2 real current pallets to hold the data
	//CRGBPalette16 LEDS_pal_target[NR_PALETTS];


	led_controls_struct led_cnt = { 150,30,POT_SENSE_DEF };

led_cfg_struct led_cfg = { DEF_MAX_BRI , DEF_BRI,DEF_MAX_BRI, 255,255,255,0, 0,30, 200, 1,1,1 , 0,50,50 };			// The basic led config

Strip_FL_Struct part[NR_STRIPS] = {						// Holds the  Strip settings
	{ 0,  0,  0,  1,  0 , 1 ,  0}  //0
	,{ 0,  0,  0 , 1,  0 , 1 ,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0}	//9
	,{ 0,  0,  0 , 1,  0 , 1 ,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0}	//19
	,{ 0,  0,  0 , 1,  0 , 1 ,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0}	//29
	,{ 0,  0,  0 , 1,  0 , 1 ,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0}
};

struct form_Part_FL_Struct form_part[NR_FORM_PARTS] =					// Holds the Form settings
{
	{ 0, 1, 0, NUM_LEDS, 0, 0, 0, 0, 0, 0,0, 1 ,  0}
	,{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,0,1 ,  0}
	,{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,0,1 ,  0}
	,{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,0,1 ,  0}
	,{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,0,1 ,  0}
	,{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,0,1 ,  0}
	,{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,0,1 ,  0}
	,{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,0,1 ,  0} //7
	,{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,0,1 ,  0}
	,{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,0 ,1 ,  0}
	,{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,0 ,1 ,  0}
	,{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,0 ,1 ,  0}
	,{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,0 ,1 ,  0}
	,{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,0 ,1 ,  0}
	,{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,0 ,1 ,  0}
	,{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,0 ,1 ,  0} //15 
};

byte strip_menu[_M_NR_STRIP_BYTES_][_M_NR_OPTIONS_] =				// Strip Selection menu what efferct on/off/fft ....
{
	{ 0,0,0,0,0,0,0,0 }
	,{ 0,0,0,0,0,0,0,0 }
	,{ 0,0,0,0,0,0,0,0 }
	,{ 0,0,0,0,0,0,0,0 }
};


uint8_t global_strip_opt[_M_NR_STRIP_BYTES_][_M_NR_GLOBAL_OPTIONS_] = { { 0,0 } ,{ 0,0 } };			// Test for global mirruring and reversing even in artnet


byte form_menu[_M_NR_FORM_BYTES_][_M_NR_FORM_OPTIONS_] =				// Form selection menu
{
	 { 0,0,0,0,0,0,0,0,0,0,0,0,0,0 }
	,{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0 }
};


void LEDS_setLED_show(uint8_t ledNr, uint8_t color[3])
{	
	leds[ledNr].r = color[0];
	leds[ledNr].g = color[1];
	leds[ledNr].b = color[2];
	FastLED.show();
}



// ************* FUNCTIONS


// Fire2012 by Mark Kriegsman, July 2012
// as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
////   Modded to acept vaiabled for strip / form selection
////	
// This basic one-dimensional 'fire' simulation works roughly as follows:
// There's a underlying array of 'heat' cells, that model the temperature
// at each point along the line.  Every cycle through the simulation, 
// four steps are performed:
//  1) All cells cool down a little bit, losing heat to the air
//  2) The heat from each cell drifts 'up' and diffuses a little
//  3) Sometimes randomly new 'sparks' of heat are added at the bottom
//  4) The heat from each cell is rendered as a color into the leds array
//     The heat-to-color mapping uses a black-body radiation approximation.
//
// Temperature is in arbitrary units from 0 (cold black) to 255 (white hot).
//
// This simulation scales it self a bit depending on NUM_LEDS; it should look
// "OK" on anywhere from 20 to 100 LEDs without too much tweaking. 
//
// I recommend running this simulation at anywhere from 30-100 frames per second,
// meaning an interframe delay of about 10-35 milliseconds.
//
// Looks best on a high-density LED setup (60+ pixels/meter).
//
//
// There are two main parameters you can play with to control the look and
// feel of your fire: COOLING (used in step 1 above), and SPARKING (used
// in step 3 above).
//
// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 55, suggested range 20-100 
#define COOLING  55

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 120
// Array of temperature readings at each simulation cell
//static 
	byte heat[NUM_LEDS];


	uint8_t LEDS_FFT_get_fire_cooling()
	{
		return COOLING * 2;
	}
	uint8_t LEDS_FFT_get_fire_sparking()
	{
		return SPARKING  *2 ;

	}


void Fire2012WithPalette(uint16_t start_led, uint16_t Nr_leds, bool reversed, bool pal, bool mirror) //, bool mirrored)
{
	uint8_t cooling = led_cfg.fire_cooling;
	uint8_t sparking = led_cfg.fire_sparking;

	/*if (true == get_bool(FFT_ENABLE))
	{
		cooling = LEDS_FFT_get_fire_cooling();
		sparking = LEDS_FFT_get_fire_sparking();

	} */

	if (true == mirror)
	{
		uint16_t NR_leds_M = Nr_leds / 2;
		if (isODDnumber(Nr_leds) == true) 
		{
			Nr_leds = Nr_leds / 2 +1;								// for the outer pass were just doing the mirror here	
			
		}
		else
		{
			Nr_leds = NR_leds_M;
			
		}

		uint16_t start_led_M = (start_led + Nr_leds);

		//uint16_t start_led_M = (start_led + Nr_leds /2  );
		//uint16_t NR_leds_M = Nr_leds / 2; 
		

		// Step 1.  Cool down every cell a little
		for (int i = start_led_M; i < start_led_M + NR_leds_M; i++) {
			heat[i] = qsub8(heat[i], random8(0, ((cooling * 10) / NR_leds_M) + 2));
		}

		// Step 2.  Heat from each cell drifts 'up' and diffuses a little
		if (true == reversed)
		{
			for (int k = (start_led_M + NR_leds_M - 1); k >= (start_led_M + 2); k--) {
				heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
			}
		}
		else
		{
			for (int k = (start_led_M); k < (start_led_M + NR_leds_M - 3); k++) {
				heat[k] = (heat[k + 1] + heat[k + 2] + heat[k + 2]) / 3;
			}
		}
		// Step 3.  Randomly ignite new 'sparks' of heat near the bottom
		// needs to spark at every strip / form start if selected
		if (true == reversed)
		{
			if (random8() < sparking) {
				int y = random8(7) + start_led_M;
				heat[y] = qadd8(heat[y], random8(160, 255));
			}
		}
		else
		{
			if (random8() < sparking) {
				int y = -(random8(7)) + start_led_M + NR_leds_M - 1;
				heat[y] = qadd8(heat[y], random8(160, 255));
			}
		}
		// Step 4.  Map from heat cells to LED colors
		for (int j = start_led_M; j < NR_leds_M + start_led_M; j++) {
			// Scale the heat value from 0-255 down to 0-240
			// for best results with color palettes.
			byte colorindex = scale8(heat[j], 240);
			//CRGB color = HeatColor(heat[j]);
			CRGB color = ColorFromPalette(LEDS_pal_cur[pal], colorindex);
			int pixelnumber;
			/*if (reversed) {
			pixelnumber = (Nr_leds + start_led - 1) - j;
			}
			else */
			{
				pixelnumber = j;
			}
			leds[pixelnumber] = color;
		}

	}
	//else
	{	
		// Step 1.  Cool down every cell a little
		for (int i = start_led; i < start_led + Nr_leds; i++) {
			heat[i] = qsub8(heat[i], random8(0, ((cooling * 10) / Nr_leds) + 2));
		}



		// Step 2.  Heat from each cell drifts 'up' and diffuses a little
		if (false == reversed)
		{
			for (int k = (start_led + Nr_leds - 1); k >= (start_led + 2) ; k--) {
				heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
			}
		}
		else 
		{
			for (int k = (start_led ); k < (start_led + Nr_leds -3); k++) {
				heat[k] = (heat[k + 1] + heat[k + 2] + heat[k + 2]) / 3;
			}
		}
		// Step 3.  Randomly ignite new 'sparks' of heat near the bottom
		// needs to spark at every strip / form start if selected
		if (false == reversed)
		{
			if (random8() < sparking) {
				int y = random8(7) + start_led;
				heat[y] = qadd8(heat[y], random8(160, 255));
			}
		}
		else
		{
				if (random8() < sparking) {
					int y = -(random8(7)) + start_led + Nr_leds - 1;
					heat[y] = qadd8(heat[y], random8(160, 255));
				}
		}
		// Step 4.  Map from heat cells to LED colors
		for (int j = start_led; j < Nr_leds + start_led; j++) {
			// Scale the heat value from 0-255 down to 0-240
			// for best results with color palettes.
			//CRGB color = HeatColor(heat[j]);
			byte colorindex = scale8(heat[j], 240);
			CRGB color = ColorFromPalette(LEDS_pal_cur[pal], colorindex);
			int pixelnumber;
			/*if (reversed) {
				pixelnumber = (Nr_leds + start_led - 1) - j;
			}
			else */ 
			{
				pixelnumber = j;
			} 
			leds[pixelnumber] = color;
		}

	}
}






// END Fire

void  LEDS_setall_color() {

	//
	fill_solid(&(leds[0]), NUM_LEDS, CRGB(180, 180, 180));
	

}

void LEDS_fadeout()
{
	// make a fadout loop goddamit!
	leds.fadeToBlackBy(255);
	yield();
	FastLED.show();
	yield();
}

float LEDS_get_FPS()
{	// return the FPS value
	return float(FastLED.getFPS());
}


void LEDS_Copy_strip(uint16_t start_LED, int nr_LED, uint16_t ref_LED)
{
	// copy a strip to somewhere else 
	if (nr_LED != 0 && (nr_LED + start_LED <= NUM_LEDS))
	{
		if (nr_LED < 0)	leds((start_LED - nr_LED - 1), (start_LED)) = leds((ref_LED), (ref_LED - nr_LED - 1));
		else			leds((start_LED), (start_LED + nr_LED - 1)) = leds((ref_LED), (ref_LED + nr_LED - 1));
	}
}







// global effects
void LEDS_G_flipstrip(uint16_t start_LED, uint16_t nr_leds)
{
	// a function to reverse a strip
	CRGB buffer_strip[nr_leds];

	for (int i = 0; i < nr_leds; i++)
	{
		//leds[start_LED + i] = buffer_strip[nr_leds - i - 1];
		buffer_strip[nr_leds - i - 1] = leds[start_LED + i];
	}
	memcpy8(&leds[start_LED], &buffer_strip, nr_leds * 3);
	//memmove8
	//memcpy8(dest, src, bytecount)
}


void LED_G_bit_run()
{	// A TEST function 
	// trying flipping globally so that we can also map artnet abit
	
	for (byte i = 0; i < 8; i++)
	{
		for (byte z = 0; z < _M_NR_STRIP_BYTES_; z++)
		{
			if (bitRead(strip_menu[z][_M_REVERSED_], i)  == true)
			{
				LEDS_G_flipstrip(part[i + (z * 8)].start_led, part[i + (z * 8)].nr_leds);
			}
		}
	}


}

// pre show precessing


//void rotate_form(uint16_t start_led, uint16_t nr_ledss, int rotate_by)
//{
//	if (rotate_by < nr_ledss && rotate_by > -nr_ledss && nr_ledss != 0)
//	{
//		CRGBArray<ROTATE_FORM_BUFFER_SIZE> leds_copy;
//		leds_copy(0, nr_ledss - 1) = leds(start_led, start_led + nr_ledss - 1);
//
//		if (rotate_by > 0)
//		{
//			leds(start_led + rotate_by, start_led + nr_ledss - 1) = leds_copy(0, nr_ledss - rotate_by - 1);
//			leds(start_led, start_led + rotate_by - 1) = leds_copy(nr_ledss - rotate_by, nr_ledss - 1);
//		}
//
//		if (rotate_by < 0)
//		{
//			rotate_by = -rotate_by;  // since its negative invert it so that the code below is better readable
//			leds(start_led, start_led + nr_ledss - rotate_by - 1) = leds_copy(rotate_by, nr_ledss - 1);
//			leds(start_led + nr_ledss - rotate_by, start_led + nr_ledss - 1) = leds_copy(0, rotate_by - 1);
//		}
//
//
//		/*if (rotate_by > 0){
//		leds(start_led, start_led + nr_ledss - rotate_by - 1) = leds_copy(rotate_by, nr_ledss -1);
//		leds(start_led + nr_ledss - rotate_by - 1,  start_led + nr_ledss - 1) = leds_copy(0, rotate_by - 1);
//		} */
//		/*else if (rotate_by < 0) {
//		leds(start_led+ nr_ledss -1 , start_led - rotate_by) = leds_copy(nr_ledss -1, -rotate_by);
//		leds(start_led- rotate_by-1 , start_led) = leds_copy(-rotate_by, 0); */
//
//
//		//leds[start_led, start_led + nr_leds - rotate_by - 1] = leds_copy[rotate_by, nr_leds - rotate_by - 1];
//		//leds[start_led + nr_leds - rotate_by - 1, start_led + nr_leds - 1] = leds_copy[0, rotate_by - 1];
//
//		//}
//	}
//}

void LEDS_G_E_addGlitter(fract8 chanceOfGlitter, uint16_t *start_led, uint16_t *nr_leds)
{	// Glitter effect origional code from  FastLed library examples DemoReel100
	if (*nr_leds != 0 && (*nr_leds + *start_led <= NUM_LEDS))
	{
		// leds(*start_led,*start_led+*nr_leds).fadeToBlackBy(chanceOfGlitter/2);
		//leds(*start_led,*start_led+*nr_leds) =(CRGB::Black);
		if (random8() < chanceOfGlitter)
		{
			leds[*start_led + (random16(*nr_leds))] += CRGB::White;
		}

		// Serial.print("G");	
	}
}


void LEDS_G_E_addGlitterRainbow(fract8 chanceOfGlitter, uint16_t *start_led, uint16_t *nr_leds)
{	// Glitter effect origional code from  FastLed library examples DemoReel100
	if (*nr_leds != 0 && (*nr_leds + *start_led <= NUM_LEDS))
	{
		//leds(start_led,start_led+nr_leds).fadeToBlackBy(chanceOfGlitter);
		//leds(start_led,start_led+nr_leds) =(CRGB::Black);
		/*if (random8() < chanceOfGlitter / 3) leds[*start_led + (random16(*nr_leds))] += CRGB::Red;
		if (random8() < chanceOfGlitter / 3) leds[*start_led + (random16(*nr_leds))] += CRGB::Blue;
		if (random8() < chanceOfGlitter / 3) leds[*start_led + (random16(*nr_leds))] += CRGB::Green; */

		if (random8() < chanceOfGlitter) leds[*start_led + (random16(*nr_leds))] += CHSV(random8(), 255, random8());
		
		//if (random8() < chanceOfGlitter / 4) leds[*start_led + (random16(*nr_leds))] += CRGB::White;
	}
}

void LEDS_G_E_juggle(uint8_t nr_dots, uint16_t *start_led, uint16_t *nr_leds, uint8_t *jd_speed, boolean reversed)		// sine dots speed = BPM
{	// Make a dot  run  a sine wave over the leds normal speed = bpm additional leds = bpm +1
	// origional code from  FastLed library examples DemoReel100
	if (*nr_leds != 0 && (*nr_leds + *start_led <= NUM_LEDS))
	{
		byte dothue = 0;
		for (int i = 0; i < nr_dots; i++)
		{
			if (reversed == true)	leds[beatsin16(i + *jd_speed, *start_led + *nr_leds - 1, *start_led)] |= CHSV(led_cfg.hue + dothue, 255, 255);
			else					leds[beatsin16(i + *jd_speed, *start_led, *start_led + *nr_leds - 1)] |= CHSV(led_cfg.hue + dothue, 255, 255);
			dothue += (255 / nr_dots);
		}
		
	}
}

void LEDS_G_E_juggle2(uint8_t nr_dots, uint16_t *start_led, uint16_t *nr_leds, uint8_t *jd_speed, boolean reversed)  // Saw Dots that run in cirles in the form
{	// Make a dot  run  a SAW wave over the leds normal speed = bpm additional leds = bpm +1
	if (*nr_leds != 0 && (*nr_leds + *start_led <= NUM_LEDS))
	{
		byte dothue = 0;
		for (int i = 0; i < nr_dots; i++)
		{
			//leds[beatsin16(i + *jd_speed, *start_led, *start_led + *nr_leds - 1)] |= CHSV(led_cfg.hue + dothue, 255, 255);
			if (reversed == true) 	leds[map(beat16(i + *jd_speed),0 , 65535, *start_led + *nr_leds - 1, *start_led)] |= CHSV(led_cfg.hue + dothue, 255, 255);
			else					leds[map(beat16(i + *jd_speed), 0, 65535, *start_led , *start_led + *nr_leds - 1)] |= CHSV(led_cfg.hue + dothue, 255, 255);
			dothue += (255 / nr_dots);
		}

	}
}



void LEDS_G_E_Form_Fade_it(uint8_t fadyBy, uint16_t *Start_led, uint16_t *nr_leds)				// fade effect for form
{	// Fade effect 

	if (*nr_leds != 0 && (*nr_leds + *Start_led <= NUM_LEDS))
	{
		leds(*Start_led, *Start_led + *nr_leds - 1).fadeToBlackBy(fadyBy);
	}
}


void LEDS_G_form_effectsRouting()				// Chcek wwhat effect bits are set and do it
{	// main routing function for effects
	// read the bit from the menu and run if active

	for (byte z = 0; z < _M_NR_FORM_BYTES_; z++)
	{

		for (byte i = 0; i < 8; i++)
		{
			if (form_part[i + (z * 8)].nr_leds != 0)  // only run if we actualy have leds to do 
			{										 // fade first so that we only fade the new effects on next go
				if (bitRead(form_menu[z][_M_FADE_], i) == true)           LEDS_G_E_Form_Fade_it(form_part[i + (z * 8)].fade_value, &form_part[i + (z * 8)].start_led, &form_part[i + (z * 8)].nr_leds);

				if (bitRead(form_menu[z][_M_GLITTER_], i) == true)        LEDS_G_E_addGlitter(form_part[i + (z * 8)].glitter_value, &form_part[i + (z * 8)].start_led, &form_part[i + (z * 8)].nr_leds);
				if (bitRead(form_menu[z][_M_RBOW_GLITTER_], i) == true)   LEDS_G_E_addGlitterRainbow(form_part[i + (z * 8)].glitter_value, &form_part[i + (z * 8)].start_led, &form_part[i + (z * 8)].nr_leds);
				
				if (bitRead(form_menu[z][_M_JUGGLE_], i) == true)         LEDS_G_E_juggle(form_part[i + (z * 8)].juggle_nr_dots, &form_part[i + (z * 8)].start_led, &form_part[i + (z * 8)].nr_leds, &form_part[i + (z * 8)].juggle_speed, bitRead(form_menu[z][_M_REVERSED_], i));
				// TODO check above was rebooting for no reason on some selection
				if (bitRead(form_menu[z][_M_SAW_DOT_], i) == true)        LEDS_G_E_juggle2(form_part[i + (z * 8)].juggle_nr_dots, &form_part[i + (z * 8)].start_led, &form_part[i + (z * 8)].nr_leds, &form_part[i + (z * 8)].juggle_speed, bitRead(form_menu[z][_M_REVERSED_], i));
				
				
			}

		}
	}
	for (byte i = 0; i < 8; i++)
	{
		if (bitRead(copy_leds_mode[0], i) == true) LEDS_Copy_strip(copy_leds[i].start_led, copy_leds[i].nr_leds, copy_leds[i].Ref_LED);
		if (bitRead(copy_leds_mode[1], i) == true) LEDS_Copy_strip(copy_leds[i + 8].start_led, copy_leds[i + 8].nr_leds, copy_leds[i + 8].Ref_LED);

	} 
	//for (byte i = 0; i < NR_FORM_PARTS; i++) 
	//	if (form_part[i].rotate != 0) rotate_form(form_part[i].start_led  , form_part[i].nr_leds , form_part[i].rotate);


	led_cfg.hue++;
}


void LEDS_G_pre_show_processing()
{	// the leds pre show prcessing 
	// run the effects and set the brightness.

	//LEDS_G_master_rgb_faders();
	LEDS_G_form_effectsRouting();
	//LED_G_bit_run();

	
	//uint8_t bri = led_cfg.max_bri * led_cfg.bri / 255;
	uint8_t bri = analogRead(POTI_BRI_PIN) / ANALOG_IN_DEVIDER;
	if (bri > led_cnt.PotBriLast + led_cnt.PotSens || bri < led_cnt.PotBriLast - led_cnt.PotSens)
	{
		led_cfg.bri = map(bri, 0, 255, 0, led_cfg.max_bri);
		led_cnt.PotBriLast = bri;
	}

	FastLED.setBrightness(led_cfg.bri);
	
	//debugMe(led_cfg.bri);
	
	

	uint8_t fps = analogRead(POTI_FPS_PIN) / ANALOG_IN_DEVIDER;
	
	//led_cfg.pal_fps = fps /4;
	///*
	if (fps > led_cnt.PotFPSLast + led_cnt.PotSens || fps < led_cnt.PotFPSLast - led_cnt.PotSens)
	{
		led_cfg.pal_fps = map(fps, 0, 255, 1, MAX_PAL_FPS);   //*/
		led_cnt.PotFPSLast = fps;
	}
	//Serial.println(fps);  

	
	//LED_G_bit_run();
		//= led_cfg.max_br * led_cfg.bri / 255
}



// 


boolean LEDS_checkIfAudioSelected()
{	// check if there are audi strips if so return true
	for (byte zp = 0; zp < _M_NR_STRIP_BYTES_; zp++) if (strip_menu[zp][_M_AUDIO_] != 0)   return true;
	for (byte zf = 0; zf < _M_NR_FORM_BYTES_; zf++)  if ((form_menu[zf][_M_AUDIO_] != 0) || (form_menu[zf][_M_AUDIO_DOT_] != 0)) return true;
	return false;

}



// palletes

const TProgmemPalette16 myRedAndGreenPalette_p PROGMEM =
{
	CRGB::Red,
	CRGB::Red, 
	CRGB::Green,
	CRGB::Red,

	CRGB::Red,
	CRGB::Green,
	CRGB::Green,
	CRGB::Red,

	CRGB::Red,
	CRGB::Red,
	CRGB::Black,
	CRGB::Red,

	CRGB::Red,
	CRGB::Green,
	CRGB::Green,
	CRGB::Red
};


void LEDS_pal_load(uint8_t pal_no, uint8_t pal_menu)
{
	// load a pallete from the default (FastLed)

	switch (pal_menu)
	{
	case 1: LEDS_pal_cur[pal_no] = RainbowColors_p; break;
	case 2: LEDS_pal_cur[pal_no] = RainbowStripeColors_p; break;
	case 3: LEDS_pal_cur[pal_no] = CloudColors_p; break;
	case 4: LEDS_pal_cur[pal_no] = PartyColors_p; break;
	case 5: LEDS_pal_cur[pal_no] = OceanColors_p; break;
	case 6: LEDS_pal_cur[pal_no] = ForestColors_p; break;
	case 7: LEDS_pal_cur[pal_no] = HeatColors_p; break;
	case 8: for (int i = 0; i < 16; i++) { LEDS_pal_cur[pal_no][i] = CHSV(random8(), 255, random8());} break;
	case 9: for (int i = 0; i < 16; i++) { LEDS_pal_cur[pal_no][i] = CHSV(0, 20, 255); } break;
	default: LEDS_pal_cur[pal_no] = RainbowColors_p; break;
		

	}

}


void LEDS_pal_advance() 
{
	// advance the pallete for each strip/form

/*#ifdef BLEND_PATTERN
	nblendPaletteTowardPalette(currentPalette_P0, targetPalette_P0, paletteMaxChanges_P0);   // blend to Target Palette_0
	nblendPaletteTowardPalette(currentPalette_P1, targetPalette_P1, paletteMaxChanges_P1);   // blend to Target Palette
#endif // BLEND_PATTERN
*/

	for (int i = 0; i < NR_STRIPS; i++) {

		part[i].index = part[i].index + part[i].index_add_pal;
		part[i].index_long = part[i].index_long + part[i].index_add_pal;
		if (MAX_INDEX_LONG <= part[i].index_long)
			part[i].index_long = part[i].index_long - MAX_INDEX_LONG;
	}


	for (int i = 0; i < NR_FORM_PARTS; i++) {

		form_part[i].index = form_part[i].index + form_part[i].index_add_pal;
		form_part[i].indexLong = form_part[i].indexLong + form_part[i].index_add_pal;
		if (MAX_INDEX_LONG <= form_part[i].indexLong)
			form_part[i].indexLong = form_part[i].indexLong - MAX_INDEX_LONG;
	}

	
}

void LEDS_pal_reset_index() 
{	// reset all the pallete indexes

	for (int z = 0; z < _M_NR_STRIP_BYTES_; z++) {
		for (int i = 0; i < 8; i++) {

			part[i + (z * 8)].index = part[i + (z * 8)].index_start;
			part[i + (z * 8)].index_long = part[i + (z * 8)].index_start;

		}
	}

	for (int z = 0; z < _M_NR_FORM_BYTES_; z++) {
		for (int i = 0; i < 8; i++) {

			form_part[i+(z * 8)].index = form_part[i+ (z * 8)].index_start;
			form_part[i + (z * 8)].indexLong = form_part[i + (z * 8)].index_start;
		}
		}
}


CRGB ColorFrom_LONG_Palette(   // made a new fuction to spread out the 255 index/color  pallet to 16*255 = 4080 colors
	boolean pal,
	uint16_t longIndex,
	//uint8_t index,
	uint8_t brightness = 255,
	TBlendType blendType = LINEARBLEND) 
{
	uint8_t indexC1 = 0;
	uint8_t indexC2 = 0;
	//uint8_t shortIndex = longIndex;
	//debugMe(longIndex,false);
	//debugMe("..", false);
	if (255 < longIndex)
	while (255 < longIndex)
	{
		longIndex = longIndex - 256;
		indexC1++;
	}

	if (indexC1 != 15)
		indexC2 = indexC1 + 1;
	//else if (indexC1 != 15)
		
	//	debugMe(longIndex,false);
	//debugMe("..", false);
	//debugMe(indexC1);
	//delay(100);
	//debugMe(indexC1);

	CRGB color1 = ColorFromPalette(*LEDS_pal_work[pal], indexC1 * 16, brightness , blendType);
	CRGB color2 = ColorFromPalette(*LEDS_pal_work[pal], indexC2 * 16, brightness , blendType);
	//nblend(CRGB& existing, const CRGB& overlay, fract8 amountOfOverlay)
	CRGB outcolor = blend(color1, color2, longIndex);
	//debugMe(String(String(color1.red) + "." + String(color1.green) + "." + String(color1.blue)));
	if (blendType == NOBLEND)
	//return (CRGB::Red);
	return color1;
	else 
	return outcolor;


}

void LEDS_long_pal_fill(boolean targetPaletteX, boolean currentBlending, uint16_t colorIndex, int index_add, uint16_t Start_led, uint16_t number_of_leds, boolean reversed, boolean one_color, boolean mirror)
{
	// fill the pallete with colors

	TBlendType currentBlendingTB;
	byte mirror_div = 1;
	byte mirror_add = 0;

	if ((number_of_leds != 0) && (number_of_leds + Start_led <= NUM_LEDS))
	{

		if (get_bool(BLEND_INVERT) == true)
			currentBlending = !currentBlending;
		if (currentBlending == true)
			currentBlendingTB = LINEARBLEND;
		else
			currentBlendingTB = NOBLEND;

		if (mirror == true) {
			mirror_div = 2;

			if (isODDnumber(number_of_leds) == true) {
				mirror_add = 1; // dosmething
			}


		}

		if (one_color == true) {
			leds(Start_led, Start_led + number_of_leds - 1) = ColorFrom_LONG_Palette(targetPaletteX, colorIndex, led_cfg.pal_bri * led_cfg.max_bri / 255, currentBlendingTB);

		}
		else {


			if (reversed == true) {
				//colorIndex = (colorIndex-(number_of_leds) * index_add);

				for (int i = (Start_led + number_of_leds / mirror_div - 1 + mirror_add); i >Start_led - 1; i--) {
					leds[i] = ColorFrom_LONG_Palette(targetPaletteX, colorIndex, led_cfg.pal_bri * led_cfg.max_bri / 255, currentBlendingTB);
					colorIndex = colorIndex + index_add;
				}

				//index_add = -index_add;
			}
			else
				for (int i = Start_led; i < (Start_led + number_of_leds / mirror_div + mirror_add); i++) {
					leds[i] = ColorFrom_LONG_Palette(targetPaletteX, colorIndex, led_cfg.pal_bri * led_cfg.max_bri / 255, currentBlendingTB);
					colorIndex = colorIndex + index_add;
				}

			if (mirror == true) {
				LEDS_Copy_strip(Start_led + number_of_leds / 2 + mirror_add, -number_of_leds / 2, Start_led);
				if (number_of_leds == 1) leds[Start_led] = ColorFrom_LONG_Palette(targetPaletteX, colorIndex, led_cfg.pal_bri * led_cfg.max_bri / 255, currentBlendingTB);

			}

		}


		// fade RGB if we are not on full
		if (led_cfg.r != 255)
			for (int i = Start_led; i < Start_led + number_of_leds; i++)
				leds[i].r = leds[i].r  * led_cfg.r / 255;
		if (led_cfg.g != 255)
			for (int i = Start_led; i < Start_led + number_of_leds; i++)
				leds[i].g = leds[i].g * led_cfg.g / 255;
		if (led_cfg.b != 255)
			for (int i = Start_led; i < Start_led + number_of_leds; i++)
				leds[i].b = leds[i].b * led_cfg.b / 255;


	}
	else debugMe("LEDS_long_pal_fill-NOT NUNNING");
}





void LEDS_pal_fill(boolean targetPaletteX, boolean currentBlending, uint8_t colorIndex, int index_add, uint16_t Start_led, uint16_t number_of_leds, boolean reversed, boolean one_color, boolean mirror)
{
	// fill the pallete with colors

	TBlendType currentBlendingTB;
	byte mirror_div = 1;
	byte mirror_add = 0;

	if ((number_of_leds != 0) && (number_of_leds + Start_led <= NUM_LEDS))
	{

		if (get_bool(BLEND_INVERT) == true)
			currentBlending = !currentBlending;
		if (currentBlending == true)
			currentBlendingTB = LINEARBLEND;
		else
			currentBlendingTB = NOBLEND;

		if (mirror == true) {
			mirror_div = 2;

			if (isODDnumber(number_of_leds) == true) {
				mirror_add = 1; // dosmething
			}


		}

		if (one_color == true) {
			leds(Start_led, Start_led + number_of_leds - 1) = ColorFromPalette(*LEDS_pal_work[targetPaletteX], colorIndex, led_cfg.pal_bri * led_cfg.max_bri / 255, currentBlendingTB);

		}
		else {


			if (reversed == true) {
				//colorIndex = (colorIndex-(number_of_leds) * index_add);

				for (int i = (Start_led + number_of_leds / mirror_div - 1 + mirror_add); i >Start_led - 1; i--) {
					leds[i] = ColorFromPalette(*LEDS_pal_work[targetPaletteX], colorIndex, led_cfg.pal_bri * led_cfg.max_bri /255, currentBlendingTB);
					colorIndex = colorIndex + index_add;
				}

				//index_add = -index_add;
			}
			else
				for (int i = Start_led; i < (Start_led + number_of_leds / mirror_div + mirror_add); i++) {
					leds[i] = ColorFromPalette(*LEDS_pal_work[targetPaletteX], colorIndex, led_cfg.pal_bri * led_cfg.max_bri /255, currentBlendingTB);
					colorIndex = colorIndex + index_add;
				}

			if (mirror == true) {
				LEDS_Copy_strip(Start_led + number_of_leds / 2 + mirror_add, -number_of_leds / 2, Start_led);
				if (number_of_leds == 1) leds[Start_led] = ColorFromPalette(*LEDS_pal_work[targetPaletteX], colorIndex, led_cfg.pal_bri * led_cfg.max_bri /255, currentBlendingTB);

			}

		}

			
			// fade RGB if we are not on full
		if (led_cfg.r != 255 ) 
			for (int i = Start_led ; i < Start_led + number_of_leds; i++)
				leds[i].r = leds[i].r  * led_cfg.r / 255;
		if (led_cfg.g != 255) 
			for (int i = Start_led; i < Start_led + number_of_leds; i++)
				leds[i].g = leds[i].g * led_cfg.g / 255;
		if (led_cfg.b != 255) 
			for (int i = Start_led; i < Start_led + number_of_leds; i++)
				leds[i].b = leds[i].b * led_cfg.b / 255;


	}
}


void LEDS_pal_routing() 
{	// main routing functions for palletes
	// if the bit is set then run the pallete 
	// on that strip/form
	
	for (byte i = 0; i < 8; i++)
	{

		for (byte zp = 0; zp < _M_NR_STRIP_BYTES_; zp++)
		{
			//if ((part[i + (zp * 8)].nr_leds != 0) && (bitRead(strip_menu[zp][_M_STRIP_], i) == true)) 		LEDS_pal_fill(bitRead(strip_menu[zp][_M_PALETTE_], i), bitRead(strip_menu[zp][_M_BLEND_], i), part[i + (zp * 8)].index, part[i + (zp * 8)].index_add, part[i + (zp * 8)].start_led, part[i + (zp * 8)].nr_leds, bitRead(strip_menu[zp][_M_REVERSED_], i), bitRead(strip_menu[zp][_M_ONE_COLOR_], i), bitRead(strip_menu[zp][_M_MIRROR_OUT_], i));
			if ((part[i + (zp * 8)].nr_leds != 0) && (bitRead(strip_menu[zp][_M_STRIP_], i) == true)) 		LEDS_long_pal_fill(bitRead(strip_menu[zp][_M_PALETTE_], i), bitRead(strip_menu[zp][_M_BLEND_], i), part[i + (zp * 8)].index_long, part[i + (zp * 8)].index_add, part[i + (zp * 8)].start_led, part[i + (zp * 8)].nr_leds, bitRead(strip_menu[zp][_M_REVERSED_], i), bitRead(strip_menu[zp][_M_ONE_COLOR_], i), bitRead(strip_menu[zp][_M_MIRROR_OUT_], i));
			if ((part[i + (zp * 8)].nr_leds != 0) && (bitRead(strip_menu[zp][_M_FIRE_], i) == true))		Fire2012WithPalette(part[i + (zp * 8)].start_led, part[i + (zp * 8)].nr_leds, bitRead(strip_menu[zp][_M_REVERSED_], i), bitRead(strip_menu[zp][_M_PALETTE_], i), bitRead(strip_menu[zp][_M_MIRROR_OUT_], i));
		}
	

		for (byte zf = 0; zf < _M_NR_FORM_BYTES_; zf++)
		{
			//if ((form_part[i + (zf * 8)].nr_leds != 0) && (bitRead(form_menu[zf][_M_STRIP_], i) == true)) LEDS_pal_fill(bitRead(form_menu[zf][_M_PALETTE_], i), bitRead(form_menu[zf][_M_BLEND_], i), form_part[i + (zf * 8)].index, form_part[i + (zf * 8)].index_add, form_part[i + (zf * 8)].start_led, form_part[i + (zf * 8)].nr_leds, bitRead(form_menu[zf][_M_REVERSED_], i), bitRead(form_menu[zf][_M_ONE_COLOR_], i), bitRead(form_menu[zf][_M_MIRROR_OUT_], i));
			if ((form_part[i + (zf * 8)].nr_leds != 0) && (bitRead(form_menu[zf][_M_STRIP_], i) == true)) LEDS_long_pal_fill(bitRead(form_menu[zf][_M_PALETTE_], i), bitRead(form_menu[zf][_M_BLEND_], i), form_part[i + (zf * 8)].indexLong, form_part[i + (zf * 8)].index_add, form_part[i + (zf * 8)].start_led, form_part[i + (zf * 8)].nr_leds, bitRead(form_menu[zf][_M_REVERSED_], i), bitRead(form_menu[zf][_M_ONE_COLOR_], i), bitRead(form_menu[zf][_M_MIRROR_OUT_], i));
			if ((form_part[i + (zf * 8)].nr_leds != 0) && (bitRead(form_menu[zf][_M_FIRE_], i) == true)	)   Fire2012WithPalette(form_part[i + (zf * 8)].start_led, form_part[i + (zf * 8)].nr_leds, bitRead(form_menu[zf][_M_REVERSED_], i), bitRead(form_menu[zf][_M_PALETTE_], i), bitRead(form_menu[zf][_M_MIRROR_OUT_], i));
		}
	}


}

void LEDS_pal_write(uint8_t pal, uint8_t no, uint8_t color , uint8_t value)
{
	// write incoming color information into a pallete entry
	switch (color)
	{
		case 0:
			LEDS_pal_cur[pal][no].r = value;
		break;
		case 1:
			LEDS_pal_cur[pal][no].g = value;
		break;
		case 2:
			LEDS_pal_cur[pal][no].b = value;
		break;


	}

}

uint8_t LEDS_pal_read(uint8_t pal, uint8_t no, uint8_t color)
{	// read the color info for 1 color in a pallete
	switch(color)
	{
		case 0:
			return LEDS_pal_cur[pal][no].r;
		break;
		case 1:
			return LEDS_pal_cur[pal][no].g;
		break;
		case 2:
			return LEDS_pal_cur[pal][no].b;
		break;

	
	}
	return 0;
	

}

boolean LEDS_pal_check_bit()
{	// check if we have any pallete bits set if so return true
	

	/*
	for (byte i = 0; i < 8; i++)
	{

		for (byte z = 0; z < _M_NR_STRIP_BYTES_; z++) if ( bitRead(strip_menu[z][_M_STRIP_], i) == true) return true;
		for (byte z = 0; z < _M_NR_FORM_BYTES_; z++) if ( bitRead(form_menu[z][_M_STRIP_], i) == true) return true;
		

	}

	*/

	for (byte z = 0; z < _M_NR_STRIP_BYTES_; z++) if ((strip_menu[z][_M_STRIP_]) > 0) return true;
	for (byte z = 0; z < _M_NR_FORM_BYTES_; z++) if ((form_menu[z][_M_STRIP_]) > 0  ) return true;
	for (byte z = 0; z < _M_NR_STRIP_BYTES_; z++) if ((strip_menu[z][_M_FIRE_]) > 0) return true;
	for (byte z = 0; z < _M_NR_FORM_BYTES_; z++) if ((form_menu[z][_M_FIRE_]) > 0) return true;

	return false;
}



//void LEDS_pal_run()
//{
//
//
//
//}

// Artnet
void LEDS_artnet_in(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{	// process the ARTNET information and send it to the leds

	if ((universe >= artnet_cfg.startU) && (universe < artnet_cfg.startU + artnet_cfg.numU))
	{
		//FastLED.show();
		byte internal_universe = universe - artnet_cfg.startU;

		// read universe and put into the right part of the display buffer
		for (int i = 0; i < length / 3; i++)
		{
			int led = i + (internal_universe * 170);

			if (led < NUM_LEDS) {
				// Do something
				//DBG_OUTPUT_PORT.print("fetch DMX frame led : ");
				//DBG_OUTPUT_PORT.println(led);
				leds[led].r = data[i * 3];
				leds[led].g = data[i * 3 + 1];
				leds[led].b = data[i * 3 + 2];

			}
		}
		yield();
		//LED_G_bit_run();
		
	}
	yield();
	FastLED.show();
	yield();
}




// ********************* FFT Functions

/*
void LEDS_FFT_enqueue(uint8_t invalue)
{	// put the invalue into the FFT buffer
	
	FFT_fifo.enqueue(invalue);

}
*/


uint8_t LEDS_FFT_get_value(uint8_t bit)
{
	// return the FFT value for the specified bit
	return fft_data[bit].value;
}

void LEDS_FFT_auto()
{	// automatically calculate the trigger value and set it
	if (FFT_stage1_sample_count >= led_cfg.pal_fps)				// trigger on the FPS so that we get one stage 2 sammple a second
	{
		fft_bin0stage2.addValue(fft_data[0].avarage);
		fft_bin1stage2.addValue(fft_data[1].avarage);
		fft_bin2stage2.addValue(fft_data[2].avarage);
		fft_bin3stage2.addValue(fft_data[3].avarage);
		fft_bin4stage2.addValue(fft_data[4].avarage);
		fft_bin5stage2.addValue(fft_data[5].avarage);
		fft_bin6stage2.addValue(fft_data[6].avarage);
		
		
		fft_data[0].trigger = constrain((fft_bin0stage2.getFastAverage() + fft_bin0stage2.GetMaxInBuffer()) / 2, fft_led_cfg.fftAutoMin, fft_led_cfg.fftAutoMax);
		fft_data[1].trigger = constrain((fft_bin1stage2.getFastAverage() + fft_bin1stage2.GetMaxInBuffer()) / 2, fft_led_cfg.fftAutoMin, fft_led_cfg.fftAutoMax);
		fft_data[2].trigger = constrain((fft_bin2stage2.getFastAverage() + fft_bin2stage2.GetMaxInBuffer()) / 2, fft_led_cfg.fftAutoMin, fft_led_cfg.fftAutoMax);
		fft_data[3].trigger = constrain((fft_bin3stage2.getFastAverage() + fft_bin3stage2.GetMaxInBuffer()) / 2, fft_led_cfg.fftAutoMin, fft_led_cfg.fftAutoMax);
		fft_data[4].trigger = constrain((fft_bin4stage2.getFastAverage() + fft_bin4stage2.GetMaxInBuffer()) / 2, fft_led_cfg.fftAutoMin, fft_led_cfg.fftAutoMax);
		fft_data[5].trigger = constrain((fft_bin5stage2.getFastAverage() + fft_bin5stage2.GetMaxInBuffer()) / 2, fft_led_cfg.fftAutoMin, fft_led_cfg.fftAutoMax);
		fft_data[6].trigger = constrain((fft_bin6stage2.getFastAverage() + fft_bin6stage2.GetMaxInBuffer()) / 2, fft_led_cfg.fftAutoMin, fft_led_cfg.fftAutoMax);
		//fft_data[7].trigger = fft_bin7stage2.getFastAverage();
		// debugMe("max bin 0" + String(fft_bin0stage2.GetMaxInBuffer()));

		FFT_stage1_sample_count = 0;
	}


}
void LEDS_FFT_calc_avarage()
{	// automatically calculate the average fft values
	fft_bin0.addValue(fft_data[0].value);
	fft_bin1.addValue(fft_data[1].value);
	fft_bin2.addValue(fft_data[2].value);
	fft_bin3.addValue(fft_data[3].value);
	fft_bin4.addValue(fft_data[4].value);
	fft_bin5.addValue(fft_data[5].value);
	fft_bin6.addValue(fft_data[6].value);



	fft_data[0].avarage = fft_bin0.getFastAverage();
	fft_data[1].avarage = fft_bin1.getFastAverage();
	fft_data[2].avarage = fft_bin2.getFastAverage();
	fft_data[3].avarage = fft_bin3.getFastAverage();
	fft_data[4].avarage = fft_bin4.getFastAverage();
	fft_data[5].avarage = fft_bin5.getFastAverage();
	fft_data[6].avarage = fft_bin6.getFastAverage();
	

	if (get_bool(FFT_AUTO))
	{
	 FFT_stage1_sample_count++;
	 LEDS_FFT_auto();
	 }


}



void LEDS_MSGEQ7_setup() {

	pinMode(MSGEQ7_INPUT_PIN, INPUT);
	pinMode(MSGEQ7_STROBE_PIN, OUTPUT);
	pinMode(MSGEQ7_RESET_PIN, OUTPUT);
	digitalWrite(MSGEQ7_RESET_PIN, LOW);
	digitalWrite(MSGEQ7_STROBE_PIN, HIGH);

}




void LEDS_MSGEQ7_get() // get the FFT data and put it in fft_data[i].value
{
	//noInterrupts();
	digitalWrite(MSGEQ7_STROBE_PIN, LOW);
	digitalWrite(MSGEQ7_RESET_PIN, HIGH);
	digitalWrite(MSGEQ7_RESET_PIN, LOW);
	
	//delayMicroseconds(36);

	for (int i = 0; i<7; i++)
	{
		digitalWrite(MSGEQ7_STROBE_PIN, HIGH);
		digitalWrite(MSGEQ7_STROBE_PIN, LOW);
		delayMicroseconds(36);
		fft_data[i].value = analogRead(MSGEQ7_INPUT_PIN) / ANALOG_IN_DEVIDER;   // 
		//digitalWrite(MSGEQ7_STROBE_PIN, HIGH);
		//delayMicroseconds(40);
		
	}
	//interrupts();
	/*
	for (int i = 0; i < 7; i++)
	{
		debugMe(fft_data[i].value, false);
		debugMe(" - ", false);
	}
	debugMe(" x ", true);
	//*/
}







CRGB LEDS_FFT_process()
{	// process the fft data and genereat a color

	CRGB color_result = (CRGB::Black);
	int bins[7] = {0,0,0,0,0,0,0};

	LEDS_MSGEQ7_get();  // get the FFT data and put it in fft_data[i].value
	LEDS_FFT_calc_avarage(); // update the avarages for autofft.

	// debugMe("FFT fill bins");
	for (byte i = 0; i < 7; i++) 
	{
		bins[i] = constrain((fft_data[i].value) - fft_data[i].trigger, 0, 255);
		if (bitRead(fft_menu[0], i) == true) color_result.r = constrain((color_result.r + bins[i]), 0, 255);
		if (bitRead(fft_menu[1], i) == true) color_result.g = constrain((color_result.g + bins[i]), 0, 255);
		if (bitRead(fft_menu[2], i) == true) color_result.b = constrain((color_result.b + bins[i]), 0, 255);
	}

	
	// fade the RGB 
	if (led_cfg.r != 255) color_result.r = color_result.r * led_cfg.r / 255 ;
	if (led_cfg.g != 255) color_result.g = color_result.g * led_cfg.g / 255 ;
	if (led_cfg.b != 255) color_result.b = color_result.b * led_cfg.b / 255 ;

	if (0 != fft_led_cfg.Scale)
	{
		color_result.r = constrain((color_result.r + (fft_led_cfg.Scale * color_result.r / 100)),0,255);
		color_result.g = constrain((color_result.g + (fft_led_cfg.Scale * color_result.g / 100)),0,255);
		color_result.b = constrain((color_result.b + (fft_led_cfg.Scale * color_result.b / 100)),0,255);
	}

	// debugMe("FFT pre return color result from bins");
	//color_result = constrain((color_result + (fft_led_cfg.Scale * color_result / 100)),0,255);
	return color_result;

}


void LEDS_FFT_fill_leds(CRGB color_result, uint16_t *Start_led, uint16_t *number_of_leds, boolean dir, boolean onecolor, boolean mirror) //, uint8_t ref_led)
{
	// fill the Strips and form with FFT data
	// we scoll dowm the strip and then put the new fft color at the start

	if (*number_of_leds != 0) 
	{
		if (onecolor == false)
		{
			byte mirror_div = 1;
			byte mirror_add = 0;
			if (mirror == true) 
			{
				mirror_div = 2;
				if (isODDnumber(*number_of_leds) == true) 
				{
					mirror_add = 1; // dosmething
				}
			}
			if (dir == true) 
			{  // reversed = true

				for (int i = (*Start_led); i < (*Start_led) + ((*number_of_leds / mirror_div) - 1 + mirror_add); i++) 
				{
					leds[i] = leds[i + 1];
				}
				if (mirror == true) leds[(*Start_led) + ((*number_of_leds / mirror_div) - 1 + mirror_add)] = color_result;
				else				leds[(*Start_led) + ((*number_of_leds) - 1)] = color_result;

			}
			else  // dir = false = forward
			{

				for (int i = *Start_led + (*number_of_leds / mirror_div) - 1 + mirror_add; i > (*Start_led); i--) 
				{
					leds[i] = leds[i - 1];
				}
				leds[(*Start_led)] = color_result;
			}
			if (mirror == true) LEDS_Copy_strip(*Start_led + *number_of_leds / 2 + mirror_add, -*number_of_leds / 2, *Start_led);
		}
		else
		{
			leds(*Start_led, *Start_led + *number_of_leds - 1) = color_result;

		}


	}
}

void LEDS_FFT_running_dot(CRGB color_result, uint16_t *Start_led, uint16_t *number_of_leds, boolean dir, uint8_t jd_speed, uint8_t nr_dots)
{
	if (0 != *number_of_leds && (*number_of_leds + *Start_led <= NUM_LEDS) )
	{
		
		for (int i = 0; i < nr_dots; i++)
		{
			//leds[beatsin16(i + *jd_speed, *start_led, *start_led + *nr_leds - 1)] |= CHSV(led_cfg.hue + dothue, 255, 255);
			if (dir == true) 	leds[map(beat16(i + jd_speed),0 , 65535, *Start_led + *number_of_leds - 1, *Start_led)] = color_result;
			else				leds[map(beat16(i + jd_speed), 0, 65535, *Start_led, *Start_led + *number_of_leds - 1)] = color_result;
			
		}

	}

}


void LEDS_FFT_check_leds(CRGB color_result)
{	// check if FFT is selected and then send it to the leds

	if (LEDS_checkIfAudioSelected())
	{
		for (byte z = 0; z < _M_NR_STRIP_BYTES_; z++) 
		{
			for (byte i = 0; i < 8; i++) 
			{ 	
			if (bitRead(strip_menu[z][_M_AUDIO_], i) == true)		
				LEDS_FFT_fill_leds(color_result, &part[i + (z * 8)].start_led, &part[i + (z * 8)].nr_leds, bitRead(strip_menu[z][_M_REVERSED_], i), bitRead(strip_menu[z][_M_ONE_COLOR_], i), bitRead(strip_menu[z][_M_MIRROR_OUT_], i));
			
			}	
		}
		for (byte z = 0; z < _M_NR_FORM_BYTES_; z++) 
		{
			for (byte i = 0; i < 8; i++) 
			{
				if (bitRead(form_menu[z][_M_AUDIO_], i) == true)		LEDS_FFT_fill_leds(color_result, &form_part[i + (z * 8)].start_led, &form_part[i + (z * 8)].nr_leds, bitRead(form_menu[z][_M_REVERSED_], i), bitRead(form_menu[z][_M_ONE_COLOR_], i), bitRead(form_menu[z][_M_MIRROR_OUT_], i));   //, form_part[i + ( z * 8 ) ].ref_led) ;
				//if (bitRead(form_menu[z][_M_AUDIO_DOT_], i) == true) ramdom_audio_dot(color_result, &form_part[i + (z * 8)].start_led, &form_part[i + (z * 8)].nr_leds);
				if (bitRead(form_menu[z][_M_AUDIO_DOT_], i) == true)    LEDS_FFT_running_dot(color_result, &form_part[i + (z * 8)].start_led, &form_part[i + (z * 8)].nr_leds, bitRead(form_menu[z][_M_REVERSED_], i), form_part[i + (z * 8)].juggle_speed, form_part[i + (z * 8)].juggle_nr_dots);
			}
		}
	}

}


void LEDS_setup()
{	// the main led setup function
	// add the correct type of led
	 debugMe("in LED Setup");
	 LEDS_MSGEQ7_setup();
	 


	switch(led_cfg.ledType)
	{
		case 0:
			FastLED.addLeds<APA102, LED_DATA_PIN, LED_CLK_PIN, BGR>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip); //, DATA_RATE_MHZ(6) //, DATA_RATE_MHZ(12)
			 debugMe("APA102 leds added");
		break;
		
		case 1:
			FastLED.addLeds<WS2812, LED_DATA_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
			 debugMe("WS2812B leds added");
		break;
		case 2:
			FastLED.addLeds<SK6822, LED_DATA_PIN>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
			 debugMe("SK6822 leds added");
		break;
		default:
			FastLED.addLeds<WS2812, LED_DATA_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
			 debugMe("WS2812B leds added");
		break;

	}

	
	for (int i = 0; i < NR_PALETTS; i++) 
	{
#ifdef BLEND_PATTERN
		//for ( int i = 0 ; i < NR_STRIPS ; i++)
		LEDS_pal_work[i] = &LEDS_pal_cur[i];
#else
		LEDS_pal_work[i] = &LEDS_pal_cur[i];
#endif
	}
	LEDS_pal_cur[0] = myRedAndGreenPalette_p;
	LEDS_pal_cur[1] = myRedAndGreenPalette_p;


	led_cfg.bri = led_cfg.startup_bri;				// set the bri to the startup bri

	if (FS_play_conf_read(0) == false)				
	{
		form_menu[0][_M_STRIP_] = 1;

	}
	LEDS_pal_reset_index();
	
		//led_cfg.r = 255;
		//led_cfg.g = 255;
		//led_cfg.b = 255;
		//led_cfg.bri = 255;
		//led_cfg.max_bri = 255;


	debugMe("end LEDS setup");
}


void LEDS_loop()
{	// the main led loop

	unsigned long currentT = micros();

	#ifndef ARTNET_DISABLED
		WiFi_artnet_loop();  //  fetshing data 
	#endif


	if (currentT > led_cfg.update_time  && get_bool(ARTNET_ENABLE) == false )
	{
			
			CRGB color_result = LEDS_FFT_process();  // Get the color from the FFT data
			
			LEDS_FFT_check_leds(color_result);      // send the color to the leds.
			yield();

				
			 

		
		{
			//debugMe("IN LED LOOP - disabled fft");
			led_cfg.update_time = currentT + (1000000 / led_cfg.pal_fps);
			//write_bool(UPDATE_LEDS, true);

			if (LEDS_pal_check_bit() == true)
			{
				yield();
				//debugMe("pre pal advance");
				LEDS_pal_advance();
				yield();
				//debugMe("pre leds routing");
				LEDS_pal_routing();
			}

			/*
			uint8_t buffer;
			while (FFT_fifo.count() >= 7)		// sanity check to keep the queue down if disabled free up memory
			{
				 debugMe("dequing overflow");
				buffer = FFT_fifo.dequeue();
			} */

		}

	
		//if (get_bool(UPDATE_LEDS) == true)
		//{
		//debugMe("pre show processing");
			LEDS_G_pre_show_processing();
			yield();
			//debugMe("pre leds SHOW");
			FastLED.show();
			yield();
		//	write_bool(UPDATE_LEDS, false);
		//}
	}


	//debugMe("leds loop end ", false);
	//debugMe(String(xPortGetCoreID()));
}
