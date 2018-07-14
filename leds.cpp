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


	#include <FastLED.h>
	#include "led_fx.h"
	#include <RunningAverage.h>			// For Auto FFT
	#include <QueueArray.h>	   			// For buffering incoming FFT packets


	#define ANALOG_IN_DEVIDER 16 // devide analog in by this value to get into a 0-255 range 


// -- The core to run FastLED.show()
#define FASTLED_SHOW_CORE 0


	extern  void osc_StC_FFT_vizIt();


// -- Task handles for use in the notifications
static TaskHandle_t FastLEDshowTaskHandle = 0;
static TaskHandle_t userTaskHandle = 0;


/** show() for ESP32
 *  Call this function instead of FastLED.show(). It signals core 0 to issue a show, 
 *  then waits for a notification that it is done.
 */
void FastLEDshowESP32()
{
    if (userTaskHandle == 0) {
        // -- Store the handle of the current task, so that the show task can
        //    notify it when it's done
        userTaskHandle = xTaskGetCurrentTaskHandle();

        // -- Trigger the show task
        xTaskNotifyGive(FastLEDshowTaskHandle);

        // -- Wait to be notified that it's done
        const TickType_t xMaxBlockTime = pdMS_TO_TICKS( 200 );
        ulTaskNotifyTake(pdTRUE, xMaxBlockTime);
        userTaskHandle = 0;
    }
}

/** show Task
 *  This function runs on core 0 and just waits for requests to call FastLED.show()
 */
void FastLEDshowTask(void *pvParameters)
{
    // -- Run forever...
    for(;;) {
        // -- Wait for the trigger
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // -- Do the show (synchronously)
        FastLED.show();

        // -- Notify the calling task
        xTaskNotifyGive(userTaskHandle);
    }
}









// *************** External Functions
// from wifi-ota.cpp

extern artnet_struct artnet_cfg;



CRGB GlobalColor_result;

// ************** FFT Variables
// FFT Average Buffers for Auto FFT 
	uint8_t FFT_stage1_sample_count = 0;		    	// used to count the samples in FFT Stage 1  for pulling into Stage 2
	#define FFT_AVERAGE_SAMPLES 60 //30 //60					// How many samples to take for the FFT average = Stage 1
	RunningAverage fft_bin0(FFT_AVERAGE_SAMPLES);	   // Buffers for the FFT values
	RunningAverage fft_bin1(FFT_AVERAGE_SAMPLES);
	RunningAverage fft_bin2(FFT_AVERAGE_SAMPLES);
	RunningAverage fft_bin3(FFT_AVERAGE_SAMPLES);
	RunningAverage fft_bin4(FFT_AVERAGE_SAMPLES);
	RunningAverage fft_bin5(FFT_AVERAGE_SAMPLES);
	RunningAverage fft_bin6(FFT_AVERAGE_SAMPLES);

	#define FFT_AVERAGE_SAMPLES_STAGE2 10						// How many  samples to take in Stage 2 auto FFT average
	RunningAverage fft_bin0stage2(FFT_AVERAGE_SAMPLES_STAGE2);	// Buffers for auto FFT Stage 2
	RunningAverage fft_bin1stage2(FFT_AVERAGE_SAMPLES_STAGE2);	// one stage to is keppt every second. so with 10 samples we have an average+max of the last 10 seconds.
	RunningAverage fft_bin2stage2(FFT_AVERAGE_SAMPLES_STAGE2);
	RunningAverage fft_bin3stage2(FFT_AVERAGE_SAMPLES_STAGE2);
	RunningAverage fft_bin4stage2(FFT_AVERAGE_SAMPLES_STAGE2);
	RunningAverage fft_bin5stage2(FFT_AVERAGE_SAMPLES_STAGE2);
	RunningAverage fft_bin6stage2(FFT_AVERAGE_SAMPLES_STAGE2);

	// FFT

	QueueArray <uint8_t> FFT_fifo;
	uint8_t	fft_fps;
	fft_led_cfg_struct fft_led_cfg = { 0,1,25,240,11,1};
	byte fft_menu[3] = { 3,7,200 };			// 3 fft data bins for RGB 
	byte fft_data_menu[3] = { 3,7,200 };   // 3 fft data bins for effects
	byte fft_data_bri = 0;	// howmuch to add to bri based on fft data 	
	byte fft_data_fps = 0;   // howmuch to add to the FPS based on FFT data selected.

//#define FFT_FIFO_COUNT_0_8_NR_PACKETS 35 //28 //35
//#define FFT_FIFO_COUNT_0_9_NR_PACKETS 28 //21 //28




fft_data_struct fft_data[7] =   // FFT data Sructure 
{ 
	 {100,0,0,0,0 }
	,{100,0,0,0,0 }
	,{100,0,0,0,0 }
	,{100,0,0,0,0 }
	,{100,0,0,0,0 }
	,{100,0,0,0,0 }
	,{100,0,0,0,0 }
};   


	
	uint8_t fft_color_result_data[3] = {0,0,0};
	uint8_t fft_color_result_bri = 0;
	uint8_t fft_bin_autoTrigger = 0;
	uint8_t fft_color_fps = 0;

// ********************* LED Setup  FastLed
	CRGBArray<MAX_NUM_LEDS> leds;			// The Led array!    CRGBArray<NUM_LEDS> leds;
	//CRGB leds[NUM_LEDS];
	//CRGBSet leds_p(leds, NUM_LEDS); led_cfg.NrLeds
	CRGBArray<MAX_NUM_LEDS> leds_FFT_history;
	CRGBArray<MAX_NUM_LEDS> led_FX_out;    // make a FX output array. 
	


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

led_cfg_struct led_cfg = { DEF_MAX_BRI , DEF_BRI,DEF_MAX_BRI, 255,255,255,0, 0,30, 200, 1,1,1 ,DEF_LED_MODE, NUM_LEDS ,DEF_FIRE_SPARKING,DEF_FIRE_COOLING,DEF_PLAY_MODE,DEF_DATA1_START_NR,DEF_DATA2_NR_LEDS,DEF_DATA2_START_NR,DEF_DATA3_NR_LEDS,DEF_DATA3_START_NR,DEF_DATA4_NR_LEDS,DEF_DATA4_START_NR, DEF_VIZ_UPDATE_TIME_FPS };			// The basic led config

Strip_FL_Struct part[NR_STRIPS] = {						// Holds the  Strip settings
	{ 0,  0,  0,  1,  0 , 1 ,  0,  0}  //0
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0}	//9
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0}	//19
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0}	//29
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0}
};


struct form_fx_test_val form_fx_test = {0,0,0};

struct form_Part_FL_Struct form_part[NR_FORM_PARTS] =					// Holds the Form settings
{
	{ 0, 1, 0, NUM_LEDS, 0, 0, 0, 0, 0, 0,0, 1 ,  0,255,0 , 0, 0, 0}
	,{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,0,1 ,  0,255,0, 0, 0, 0}
	,{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,0,1 ,  0,255,0, 0, 0, 0}
	,{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,0,1 ,  0,255,0, 0, 0, 0}
	,{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,0,1 ,  0,255,0, 0, 0, 0}
	,{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,0,1 ,  0,255,0, 0, 0, 0}
	,{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,0,1 ,  0,255,0, 0, 0, 0}
	,{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,0,1 ,  0,255,0, 0, 0, 0} //7
	,{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,0,1 ,  0,255,0, 0, 0, 0}
	,{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,0 ,1 ,  0,255,0, 0, 0, 0}
	,{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,0 ,1 ,  0,255,0, 0, 0, 0}
	,{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,0 ,1 ,  0,255,0, 0, 0, 0}
	,{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,0 ,1 ,  0,255,0, 0, 0, 0}
	,{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,0 ,1 ,  0,255,0, 0, 0, 0}
	,{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,0 ,1 ,  0,255,0, 0, 0, 0}
	,{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,0 ,1 ,  0,255,0, 0, 0, 0} //15 
};

byte strip_menu[_M_NR_STRIP_BYTES_][_M_NR_OPTIONS_] =				// Strip Selection menu what efferct on/off/fft ....
{
	{ 0,0,0,0,0,0,0,0,0,0 }
	,{ 0,0,0,0,0,0,0,0,0,0 }
	,{ 0,0,0,0,0,0,0,0,0,0 }
	,{ 0,0,0,0,0,0,0,0,0,0 }
};


uint8_t global_strip_opt[_M_NR_STRIP_BYTES_][_M_NR_GLOBAL_OPTIONS_] = { { 0,0 } ,{ 0,0 } };			// Test for global mirruring and reversing even in artnet


byte form_menu[_M_NR_FORM_BYTES_][_M_NR_FORM_OPTIONS_] =				// Form selection menu
{
	 { 0,0,1,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0 }
	,{ 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0 }
};



uint8_t LEDS_get_real_bri()
{

	return qadd8(led_cfg.bri,fft_color_result_bri ); 
}

void LEDS_show()
{	
			if(fft_data_bri != 0)
				FastLED.setBrightness(LEDS_get_real_bri() );
			else
				FastLED.setBrightness(led_cfg.bri);
			 FastLEDshowESP32();
			//FastLED.show();
			//FastLED[0].showLeds(led_cfg.bri);
			//FastLED[1].showLeds(led_cfg.bri);
			//FastLED[2].showLeds(led_cfg.bri);
}

void LEDS_setLED_show(uint8_t ledNr, uint8_t color[3])
{	
	leds[ledNr].r = color[0];
	leds[ledNr].g = color[1];
	leds[ledNr].b = color[2];
	LEDS_show();
}



// ************* FUNCTIONS



// END Fire

void  LEDS_setall_color(uint8_t color = 0) {

	// set all leds to a color
	// 0 = white 50%
	// 1 = green 50%
	// 2 = black
	// 3 = red 50%

	switch(color) {

		case 0: fill_solid(&(leds[0]), MAX_NUM_LEDS, 	CRGB(180, 	180, 	180));	break;
		case 1: fill_solid(&(leds[0]), MAX_NUM_LEDS, 	CRGB(0,		127, 	0));	break;
		case 2: fill_solid(&(leds[0]), MAX_NUM_LEDS,	CRGB(0,		0, 		0));	break;
		case 3: fill_solid(&(leds[0]), MAX_NUM_LEDS, 	CRGB(127, 	0, 		0));	break;
	   default: fill_solid(&(leds[0]), MAX_NUM_LEDS, 	CRGB(180,	180, 	180)); break;	
	}
	debugMe("Setall Leds to : " + String(color));	
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
	if (nr_LED != 0 && (nr_LED + start_LED <= MAX_NUM_LEDS))
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

void LEDS_FX_run_mix(uint16_t start_led, uint16_t nr_leds, boolean reversed, boolean mirror , boolean StripFormOn, boolean subrtact_mode = false , boolean mask= true, uint8_t fx_level = 255 )

{
	StripFormOn = true;


	if(nr_leds != 0)
	{
		//if(!reversed)  // = forward
		{	
			if(StripFormOn)
				for(uint16_t led_num = start_led; led_num < start_led + nr_leds  ; led_num ++ )
				{

					if(!mask)
					{
						if(subrtact_mode)
						{
							leds[led_num].red 	= qsub8(leds[led_num  ].red	,	map(led_FX_out[led_num].red   ,0,255,0,fx_level ));	
							leds[led_num].green = qsub8(leds[led_num  ].green,	map(led_FX_out[led_num].green ,	0,255,0,fx_level ));
							leds[led_num].blue 	= qsub8(leds[led_num  ].blue,	map(led_FX_out[led_num].blue  ,	0,255,0,fx_level ));
						}
						else
						{
							leds[led_num].red 	= qadd8(leds[led_num ].red,		map(led_FX_out[led_num].red  ,	0,255,0,fx_level )	);	
							leds[led_num].green = qadd8(leds[led_num ].green, 	map(led_FX_out[led_num].green ,	0,255,0,fx_level )	);
							leds[led_num].blue 	= qadd8(leds[led_num ].blue,	map(led_FX_out[led_num].blue,	0,255,0,fx_level )  );
						}
					}
					else
					{
							leds[led_num].red 	= map(map(leds[led_num  ].red,	0, 255, 0,	led_FX_out[led_num].red   ),	0,255,0,fx_level );	
							leds[led_num].green = map(map(leds[led_num  ].green,0, 255, 0,	led_FX_out[led_num].green ),	0,255,0,fx_level );
							leds[led_num].blue 	= map(map(leds[led_num  ].blue,	0, 255, 0, 	led_FX_out[led_num].blue  ),	0,255,0,fx_level );
					}


				}
				
			else
				for(uint16_t led_num = start_led; led_num < start_led + nr_leds  ; led_num ++ )
				{

						leds[led_num].red 	= map(led_FX_out[led_num].red  ,	0,255,0,fx_level )	;	
						leds[led_num].green = map(led_FX_out[led_num].green,	0,255,0,fx_level ) ;
						leds[led_num].blue 	= map(led_FX_out[led_num].blue ,	0,255,0,fx_level ) ;
					

				//if (mirror == true) LEDS_Copy_strip(start_led + nr_leds / 2 + mirror_add , -nr_leds / 2 , start_led);		
				}

			
		}
		/*else
		{
			if(StripFormOn)
				for(uint16_t led_num = start_led   ; led_num < start_led + (nr_leds/mirror_div)  + mirror_add  ; led_num++ ) 
				{

					

					if(subrtact_mode)
					{
						leds[led_num].red =       qsub8(leds[ nr_leds/mirror_div + mirror_add - (led_num - start_led) ].red,	led_FX_out[led_num].red   ) ;
						leds[led_num].green  =    qsub8(leds[ nr_leds/mirror_div + mirror_add - (led_num - start_led) ].green,	led_FX_out[led_num].green   ) ;
						leds[led_num].blue  =     qsub8(leds[ nr_leds/mirror_div + mirror_add - (led_num - start_led) ].blue,	led_FX_out[led_num].blue  ) ;

					}
					else
					{
						leds[led_num].red =       qadd8(leds[ nr_leds/mirror_div + mirror_add + (led_num - start_led) ].red,	led_FX_out[led_num].red   ) ;
						leds[led_num].green  =    qadd8(leds[ nr_leds/mirror_div + mirror_add + (led_num - start_led) ].green,	led_FX_out[led_num].green   ) ;
						leds[led_num].blue  =     qadd8(leds[ nr_leds/mirror_div + mirror_add + (led_num - start_led) ].blue,	led_FX_out[led_num].blue  ) ;

					}



					if (mirror == true) LEDS_Copy_strip(start_led + nr_leds / 2  , -(nr_leds + mirror_add) / 2, start_led);	
				}
			else
				for(uint16_t led_num = start_led; led_num < start_led + nr_leds/ mirror_div  + mirror_add  ; led_num ++ )
				{

						leds[led_num].red 	= led_FX_out[led_num].red  	;	
						leds[led_num].green = led_FX_out[led_num].green ;
						leds[led_num].blue 	= led_FX_out[led_num].blue  ;
					

					if (mirror == true) LEDS_Copy_strip(start_led + nr_leds / 2  , -(nr_leds + mirror_add) / 2, start_led);	
				}	


		}
		*/
	
	}
}




void LEDS_G_form_effectsRouting()				// Chcek wwhat effect bits are set and do it
{	// main routing function for effects
	// read the bit from the menu and run if active

	for (byte z = 0; z < _M_NR_FORM_BYTES_; z++)
	{

		for (byte i = 0; i < 8; i++)
		{
			if(bitRead(form_menu[z][_M_FX1_ON], i ))
			{
				if (form_part[i + (z * 8)].nr_leds != 0)  // only run if we actualy have leds to do 
				{										 // fade first so that we only fade the new effects on next go
					boolean trigger = false;
				
					if (form_part[i + (z * 8)].fade_value != 0 )         	  { LEDS_G_E_Form_Fade_it(form_part[i + (z * 8)].fade_value, &form_part[i + (z * 8)].start_led, &form_part[i + (z * 8)].nr_leds); }

														
					if (bitRead(form_menu[z][_M_FX_3_SIN], i) == true)         { FX_three_sin(form_part[i + (z * 8)].start_led, 	form_part[i + (z * 8)].nr_leds,  bitRead(form_menu[z][_M_PALETTE_], i), bitRead(form_menu[z][_M_MIRROR_OUT_], i), bitRead(form_menu[z][_M_FX_SIN_PAL], i), form_fx_test.val_0) ; trigger = true ;  }
					if (bitRead(form_menu[z][_M_FX_SHIMMER], i) == true)		{ LEDS_G_E_shimmer(form_part[i + (z * 8)].start_led, 	form_part[i + (z * 8)].nr_leds,  bitRead(form_menu[z][_M_PALETTE_], i), bitRead(form_menu[z][_M_MIRROR_OUT_], i), bitRead(form_menu[z][_M_BLEND_], i) , form_fx_test.val_0 , form_fx_test.val_1, form_fx_test.val_2  ); trigger = true ;  }

					if (bitRead(form_menu[z][_M_AUDIO_FX4], i) == true)         { noise16_2_pallete(form_part[i + (z * 8)].start_led, 	form_part[i + (z * 8)].nr_leds,  bitRead(form_menu[z][_M_PALETTE_], i), bitRead(form_menu[z][_M_MIRROR_OUT_], i), bitRead(form_menu[z][_M_BLEND_], i)); trigger = true ;  }
					if (bitRead(form_menu[z][_M_AUDIO_FX3], i) == true)         { noise16_2(form_part[i + (z * 8)].start_led, 			form_part[i + (z * 8)].nr_leds,  bitRead(form_menu[z][_M_PALETTE_], i), bitRead(form_menu[z][_M_MIRROR_OUT_], i), bitRead(form_menu[z][_M_BLEND_], i)); trigger = true ;  }
					if (bitRead(form_menu[z][_M_AUDIO_FX5], i) == true) 		{ FX_noise_fill(form_part[i + (z * 8)].start_led, 		form_part[i + (z * 8)].nr_leds) ; trigger = true ;  }

					if (bitRead(form_menu[z][_M_GLITTER_], i) == true)        { if(!bitRead(form_menu[z][_M_GLITTER_FROM_FFT_DATA1], i)) LEDS_G_E_addGlitter(form_part[i + (z * 8)].glitter_value    , 		&form_part[i + (z * 8)].start_led, &form_part[i + (z * 8)].nr_leds); else LEDS_G_E_addGlitter( fft_color_result_data[0]    , 		&form_part[i + (z * 8)].start_led, &form_part[i + (z * 8)].nr_leds);  trigger = true;     }
					if (bitRead(form_menu[z][_M_RBOW_GLITTER_], i) == true)   { if(!bitRead(form_menu[z][_M_GLITTER_FROM_FFT_DATA1], i)) LEDS_G_E_addGlitterRainbow(form_part[i + (z * 8)].glitter_value, 	&form_part[i + (z * 8)].start_led, &form_part[i + (z * 8)].nr_leds); else LEDS_G_E_addGlitterRainbow( fft_color_result_data[0] , 	&form_part[i + (z * 8)].start_led, &form_part[i + (z * 8)].nr_leds); trigger = true;}
					
					if (bitRead(form_menu[z][_M_JUGGLE_], i) == true)         { LEDS_G_E_juggle(form_part[i + (z * 8)].juggle_nr_dots, 	&form_part[i + (z * 8)].start_led, &form_part[i + (z * 8)].nr_leds, &form_part[i + (z * 8)].juggle_speed, bitRead(form_menu[z][_M_REVERSED_], i)); trigger = true;}
					if (bitRead(form_menu[z][_M_SAW_DOT_], i) == true)        { LEDS_G_E_juggle2(form_part[i + (z * 8)].juggle_nr_dots, &form_part[i + (z * 8)].start_led, &form_part[i + (z * 8)].nr_leds, &form_part[i + (z * 8)].juggle_speed, bitRead(form_menu[z][_M_REVERSED_], i));trigger = true; }
					
					if (bitRead(form_menu[z][_M_AUDIO_DOT_], i) == true)     { LEDS_FFT_running_dot(GlobalColor_result, &form_part[i + (z * 8)].start_led, &form_part[i + (z * 8)].nr_leds, bitRead(form_menu[z][_M_AUDIO_REVERSED], i), form_part[i + (z * 8)].juggle_speed, form_part[i + (z * 8)].juggle_nr_dots); trigger = true; }

					if ((form_part[i + (z * 8)].nr_leds != 0) && (bitRead(form_menu[z][_M_FIRE_], i) == true)	)   { Fire2012WithPalette(form_part[i + (z * 8)].start_led, form_part[i + (z * 8)].nr_leds, bitRead(form_menu[z][_M_REVERSED_], i), bitRead(form_menu[z][_M_FIRE_PAL], i), bitRead(form_menu[z][_M_MIRROR_OUT_], i)); trigger = true;   }
					
					if(trigger)
					{
							
							LEDS_FX_run_mix(form_part[i + (z * 8)].start_led, form_part[i + (z * 8)].nr_leds, bitRead(form_menu[z][_M_REVERSED_], i), bitRead(form_menu[z][_M_MIRROR_OUT_], i),  (bitRead(form_menu[z][_M_STRIP_], i) == true || bitRead(form_menu[z][_M_AUDIO_], i) == true ),bitRead(form_menu[z][_M_FX_SUBTRACT], i), bitRead(form_menu[z][_M_FX_MASK], i),form_part[i + (z * 8)].FX_level);

						if (bitRead(form_menu[z][_M_MIRROR_OUT_], i) == true) 	LEDS_Copy_strip((form_part[i + (z * 8)].start_led  + form_part[i + (z * 8)].nr_leds / 2) , -(form_part[i + (z * 8)].nr_leds ) / 2, form_part[i + (z * 8)].start_led);  
					}
					
				

				}
			}
			else
			{
					for (byte i = 0; i < 8; i++)
						if (form_part[i + (z * 8)].nr_leds != 0)  // only run if we actualy have leds to do 
							if (form_part[i + (z * 8)].fade_value != 0 )         	 
								 { LEDS_G_E_Form_Fade_it(form_part[i + (z * 8)].fade_value, &form_part[i + (z * 8)].start_led, &form_part[i + (z * 8)].nr_leds); }

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

	if(!get_bool(POT_DISABLE))
	{
		//uint8_t bri = led_cfg.max_bri * led_cfg.bri / 255;
		uint8_t bri = analogRead(POTI_BRI_PIN) / ANALOG_IN_DEVIDER;
		if (bri > led_cnt.PotBriLast + led_cnt.PotSens || bri < led_cnt.PotBriLast - led_cnt.PotSens)
		{
			led_cfg.bri = map(bri, 0, 255, 0, led_cfg.max_bri);
			led_cnt.PotBriLast = bri;
		}

		//FastLED.setBrightness(led_cfg.bri);  moved to show
		
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
	}
	
	//LED_G_bit_run();
		//= led_cfg.max_br * led_cfg.bri / 255
}






boolean LEDS_checkIfAudioSelected()
{	// check if there are audi strips if so return true
	for (byte zp = 0; zp < _M_NR_STRIP_BYTES_; zp++) if (strip_menu[zp][_M_AUDIO_] != 0)   return true;
	for (byte zf = 0; zf < _M_NR_FORM_BYTES_; zf++)  if ((form_menu[zf][_M_AUDIO_] != 0) || (form_menu[zf][_M_AUDIO_DOT_] != 0)) return true;
	if(fft_data_bri != 0) return true;
	if(fft_data_menu[0] != 0) return true;
	if(fft_data_menu[1] != 0) return true;
	if(fft_data_menu[2] != 0) return true;
	if(fft_data_fps != 0) return true;
	return false;

}


uint8_t getrand8()
{
return random8();

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
	debugMe("Load pal" + String(pal_menu));
	switch (pal_menu)
	{
	case 0: LEDS_pal_cur[pal_no] = RainbowStripeColors_p; break;	
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

CRGB myColorFromPalette(boolean pallete, uint8_t index , uint8_t bri , boolean blend)
{
    TBlendType currentBlendingTB;
    if (get_bool(BLEND_INVERT) == true)
			blend = !blend;
		if (blend == true)
			currentBlendingTB = LINEARBLEND;
		else
			currentBlendingTB = NOBLEND;


	CRGB color = ColorFromPalette(*LEDS_pal_work[pallete], index , bri , currentBlendingTB);
	return color;

}




void LEDS_long_pal_fill(boolean targetPaletteX, boolean currentBlending, uint16_t colorIndex, int index_add, uint16_t Start_led, uint16_t number_of_leds, boolean reversed, boolean one_color, boolean mirror)
{
	// fill the pallete with colors

	TBlendType currentBlendingTB;
	byte mirror_div = 1;
	byte mirror_add = 0;

	if ((number_of_leds != 0) && (number_of_leds + Start_led <= MAX_NUM_LEDS))
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

	if ((number_of_leds != 0) && (number_of_leds + Start_led <= MAX_NUM_LEDS))
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
			//if ((part[i + (zp * 8)].nr_leds != 0) && (bitRead(strip_menu[zp][_M_FIRE_], i) == true))		Fire2012WithPalette(part[i + (zp * 8)].start_led, part[i + (zp * 8)].nr_leds, bitRead(strip_menu[zp][_M_REVERSED_], i), bitRead(strip_menu[zp][_M_PALETTE_], i), bitRead(strip_menu[zp][_M_MIRROR_OUT_], i));
		}
	

		for (byte zf = 0; zf < _M_NR_FORM_BYTES_; zf++)
		{
			//if ((form_part[i + (zf * 8)].nr_leds != 0) && (bitRead(form_menu[zf][_M_STRIP_], i) == true)) LEDS_pal_fill(bitRead(form_menu[zf][_M_PALETTE_], i), bitRead(form_menu[zf][_M_BLEND_], i), form_part[i + (zf * 8)].index, form_part[i + (zf * 8)].index_add, form_part[i + (zf * 8)].start_led, form_part[i + (zf * 8)].nr_leds, bitRead(form_menu[zf][_M_REVERSED_], i), bitRead(form_menu[zf][_M_ONE_COLOR_], i), bitRead(form_menu[zf][_M_MIRROR_OUT_], i));
			if ((form_part[i + (zf * 8)].nr_leds != 0) && (bitRead(form_menu[zf][_M_STRIP_], i) == true)) LEDS_long_pal_fill(bitRead(form_menu[zf][_M_PALETTE_], i), bitRead(form_menu[zf][_M_BLEND_], i), form_part[i + (zf * 8)].indexLong, form_part[i + (zf * 8)].index_add, form_part[i + (zf * 8)].start_led, form_part[i + (zf * 8)].nr_leds, bitRead(form_menu[zf][_M_REVERSED_], i), bitRead(form_menu[zf][_M_ONE_COLOR_], i), bitRead(form_menu[zf][_M_MIRROR_OUT_], i));
			//if ((form_part[i + (zf * 8)].nr_leds != 0) && (bitRead(form_menu[zf][_M_FIRE_], i) == true)	)   Fire2012WithPalette(form_part[i + (zf * 8)].start_led, form_part[i + (zf * 8)].nr_leds, bitRead(form_menu[zf][_M_REVERSED_], i), bitRead(form_menu[zf][_M_PALETTE_], i), bitRead(form_menu[zf][_M_MIRROR_OUT_], i));
		}
	}


}



void LEDS_PAL_invert(uint8_t pal = 0)
{

		for(int pal_pos = 0; pal_pos < 16; pal_pos++)
		{
		LEDS_pal_cur[pal][pal_pos].r = qsub8(255, LEDS_pal_cur[pal][pal_pos].r );
		LEDS_pal_cur[pal][pal_pos].g = qsub8(255, LEDS_pal_cur[pal][pal_pos].g );
		LEDS_pal_cur[pal][pal_pos].b = qsub8(255, LEDS_pal_cur[pal][pal_pos].b );
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

			if (led < MAX_NUM_LEDS) {
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
	FastLEDshowESP32();
	//FastLED.show();
	yield();
}




// ********************* FFT Functions


void LEDS_FFT_enqueue(uint8_t invalue)
{	// put the invalue into the FFT buffer
	
	FFT_fifo.enqueue(invalue);

}

uint8_t LEDS_FFT_get_MAX_value(uint8_t bit)
{
	// return the FFT value for the specified bit
	return fft_data[bit].max;
}

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
		
		
		if (bitRead(fft_bin_autoTrigger, 0)) fft_data[0].trigger = constrain((fft_bin0stage2.getFastAverage() + fft_bin0stage2.GetMaxInBuffer()) / 2, fft_led_cfg.fftAutoMin, fft_led_cfg.fftAutoMax);
		if (bitRead(fft_bin_autoTrigger, 1)) fft_data[1].trigger = constrain((fft_bin1stage2.getFastAverage() + fft_bin1stage2.GetMaxInBuffer()) / 2, fft_led_cfg.fftAutoMin, fft_led_cfg.fftAutoMax);
		if (bitRead(fft_bin_autoTrigger, 2)) fft_data[2].trigger = constrain((fft_bin2stage2.getFastAverage() + fft_bin2stage2.GetMaxInBuffer()) / 2, fft_led_cfg.fftAutoMin, fft_led_cfg.fftAutoMax);
		if (bitRead(fft_bin_autoTrigger, 3)) fft_data[3].trigger = constrain((fft_bin3stage2.getFastAverage() + fft_bin3stage2.GetMaxInBuffer()) / 2, fft_led_cfg.fftAutoMin, fft_led_cfg.fftAutoMax);
		if (bitRead(fft_bin_autoTrigger, 4)) fft_data[4].trigger = constrain((fft_bin4stage2.getFastAverage() + fft_bin4stage2.GetMaxInBuffer()) / 2, fft_led_cfg.fftAutoMin, fft_led_cfg.fftAutoMax);
		if (bitRead(fft_bin_autoTrigger, 5)) fft_data[5].trigger = constrain((fft_bin5stage2.getFastAverage() + fft_bin5stage2.GetMaxInBuffer()) / 2, fft_led_cfg.fftAutoMin, fft_led_cfg.fftAutoMax);
		if (bitRead(fft_bin_autoTrigger, 6)) fft_data[6].trigger = constrain((fft_bin6stage2.getFastAverage() + fft_bin6stage2.GetMaxInBuffer()) / 2, fft_led_cfg.fftAutoMin, fft_led_cfg.fftAutoMax);
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


	fft_data[0].max = fft_bin0.GetMaxInBuffer();
	fft_data[1].max = fft_bin1.GetMaxInBuffer();
	fft_data[2].max = fft_bin2.GetMaxInBuffer();
	fft_data[3].max = fft_bin3.GetMaxInBuffer();
	fft_data[4].max = fft_bin4.GetMaxInBuffer();
	fft_data[5].max = fft_bin5.GetMaxInBuffer();
	fft_data[6].max = fft_bin6.GetMaxInBuffer();
	
/*
	fft_data[0].max = fft_bin0stage2.GetMaxInBuffer();
	fft_data[1].max = fft_bin1stage2.GetMaxInBuffer();
	fft_data[2].max = fft_bin2stage2.GetMaxInBuffer();
	fft_data[3].max = fft_bin3stage2.GetMaxInBuffer();
	fft_data[4].max = fft_bin4stage2.GetMaxInBuffer();
	fft_data[5].max = fft_bin5stage2.GetMaxInBuffer();
	fft_data[6].max = fft_bin6stage2.GetMaxInBuffer();
*/

	//if (get_bool(FFT_AUTO))
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
	fft_color_result_data[0] = 0;
	fft_color_result_data[1] = 0;
	fft_color_result_data[2] = 0;
	fft_color_result_bri = 0;
	fft_color_fps = 0;
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

		if (bitRead(fft_data_menu[0], i) == true) fft_color_result_data[0] = constrain((fft_color_result_data[0] + bins[i]), 0, 255);
		if (bitRead(fft_data_menu[1], i) == true) fft_color_result_data[1] = constrain((fft_color_result_data[1] + bins[i]), 0, 255);
		if (bitRead(fft_data_menu[2], i) == true) fft_color_result_data[2] = constrain((fft_color_result_data[2] + bins[i]), 0, 255);

		if (bitRead(fft_data_bri, i) == true) fft_color_result_bri = constrain((fft_color_result_bri + bins[i]), 0, 255);
		if (bitRead(fft_data_fps, i) == true) fft_color_fps = constrain((fft_color_fps + bins[i]), 0, 255);
	}
	//debugMe(fft_color_result_data[1]);	
	//debugMe(fft_data_menu[0], false);
	//debugMe("..");
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
	
	 GlobalColor_result = color_result;

	return color_result;

}



//void LEDS_FFT_fill_leds( uint16_t *Start_led, uint16_t *number_of_leds, boolean reversed, boolean onecolor, boolean mirror) //, uint8_t ref_led)
void LEDS_FFT_fill_leds(uint16_t start_led, uint16_t nr_leds, boolean reversed, boolean onecolor, boolean mirror, uint8_t fft_offset = 0 )
{
		// form_part[i + (z * 8)].start_led, &form_part[i + (z * 8)].nr_leds, bitRead(form_menu[z][_M_REVERSED_], i), bitRead(form_menu[z][_M_ONE_COLOR_], i), bitRead(form_menu[z][_M_MIRROR_OUT_], i));

	//debugMe("s:" + String(leds[0].red) + " : "  + String(leds[0].green) + " : " + String(leds[0].blue) );
	
	byte mirror_div = 1;
	byte mirror_add = 0;
			if (mirror == true) 
			{
				mirror_div = 2;
				if (isODDnumber(nr_leds) == true) 
				{
					mirror_add = 1; // dosmething
				}
			}
	
	if(nr_leds != 0)
	{
		if(!reversed)  // = forward
		{	
			
			for(uint16_t led_num = start_led; led_num < start_led + nr_leds/ mirror_div  + mirror_add  ; led_num ++ )
			{
				
				if(!onecolor)
				{
					leds[led_num].red 	= leds_FFT_history[led_num - start_led + fft_offset ].red;	
					leds[led_num].green = leds_FFT_history[led_num - start_led + fft_offset ].green;
					leds[led_num].blue 	= leds_FFT_history[led_num - start_led + fft_offset ].blue;
				}
				else 
				{
					leds[led_num].red 	= leds_FFT_history[0 + fft_offset ].red;	
					leds[led_num].green = leds_FFT_history[0 + fft_offset ].green;
					leds[led_num].blue 	= leds_FFT_history[0 + fft_offset ].blue;
				}	
				
				//leds[led_num].red 	= constrain(leds_FFT_history[led_num - start_led  ].red - leds[led_num].red , 0, 255) ;
				//leds[led_num].blue 	= constrain(leds_FFT_history[led_num - start_led].blue	- leds[led_num].blue , 0, 255) ;
				//leds[led_num].green = constrain(leds_FFT_history[led_num - start_led].green - leds[led_num].green , 0, 255) ;

				//debugMe("nr:");
					
			}

			if (mirror == true) LEDS_Copy_strip(start_led + nr_leds / 2 + mirror_add , -nr_leds / 2 , start_led);

		}
		else // wer are reversed
		{
			if(onecolor) for(uint16_t led_num = start_led   ; led_num < start_led + (nr_leds)    ; led_num++ ) 
			{
					leds[led_num].red 	= leds_FFT_history[0 + fft_offset ].red;	
					leds[led_num].green = leds_FFT_history[0 + fft_offset ].green;
					leds[led_num].blue 	= leds_FFT_history[0 + fft_offset ].blue;

			}
			else if(mirror)
			{

				for(uint16_t led_num = start_led  ; led_num < start_led  + nr_leds  ; led_num++ ) 
				{

					if(led_num <= start_led + nr_leds/2)
						{
							uint16_t hist_led_num =  nr_leds/2 - (led_num - start_led);
							leds[led_num].red =       leds_FFT_history[ hist_led_num].red  ;
							leds[led_num].green  =    leds_FFT_history[ hist_led_num].green ;
							leds[led_num].blue  =     leds_FFT_history[ hist_led_num].blue ;
						}
						else
						{
							uint16_t hist_led_num = nr_leds/2 -(nr_leds - (led_num - start_led))   ;
							leds[led_num].red =       leds_FFT_history[ hist_led_num].red  ;
							leds[led_num].green  =    leds_FFT_history[  hist_led_num].green ;
							leds[led_num].blue  =     leds_FFT_history[  hist_led_num].blue ;
						}
						//leds[led_num].red =       leds_FFT_history[ nr_leds/mirror_div + mirror_add - (led_num - start_led) ].red  ;
						//leds[led_num].green  =    leds_FFT_history[ nr_leds/mirror_div + mirror_add - (led_num - start_led) ].green ;
						//leds[led_num].blue  =     leds_FFT_history[ nr_leds/mirror_div + mirror_add - (led_num - start_led) ].blue ;
					//leds[led_num] =   constrain(leds_FFT_history[ nr_leds/mirror_div + mirror_add - (led_num - start_led) ] - leds[led_num] , 0, 255) ;
					//leds[led_num].red =   constrain(leds_FFT_history[ nr_leds/mirror_div + mirror_add - (led_num - start_led) ].red - leds[led_num].red , 0, 255) ;
					//leds[led_num].blue =  constrain(leds_FFT_history[ nr_leds/mirror_div + mirror_add - (led_num - start_led) ].blue - leds[led_num].blue , 0, 255) ;
					//leds[led_num].green = constrain(leds_FFT_history[ nr_leds/mirror_div + mirror_add - (led_num - start_led) ].green - leds[led_num].green, 0, 255) ;



				}

			//LEDS_Copy_strip(start_led   , -(nr_leds + mirror_add) / 2, start_led + (nr_leds/2) );			
			}
			else  //no mirror no onecolor
			{
				for(uint16_t led_num = start_led   ; led_num < start_led + (nr_leds)    ; led_num++ ) 
				{
					leds[led_num].red =       leds_FFT_history[ nr_leds - (led_num - start_led) ].red  ;
					leds[led_num].green  =    leds_FFT_history[ nr_leds - (led_num - start_led) ].green ;
					leds[led_num].blue  =     leds_FFT_history[ nr_leds - (led_num - start_led) ].blue ;
				}

			}	
			

		}
	
	}	
	//debugMe("e:" + String(leds[0].red) + " : "  + String(leds[0].green) + " : " + String(leds[0].blue) );
}


/*
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
	*/


void LEDS_FFT_history_run(CRGB color_result)
{	// only move up to max leds from mixed mode.

	for (int i = led_cfg.NrLeds -1  ; i > 0  ; i--) 		
		{
					leds_FFT_history[i] = leds_FFT_history[i - 1];	
					
		}

	leds_FFT_history[0] = color_result;
	//for (int i = 0 ; i < 3 ; i++)	
	//debugMe(String(leds_FFT_history[i].red)); 
		//debugMe(String(leds_FFT_history[i].red) + " : "  + String(leds_FFT_history[i].green) + " : " + String(leds_FFT_history[i].blue) + " x " + i );
 
	
}

void LEDS_FFT_pal_mix(uint16_t start_led, uint16_t nr_leds, boolean fft_reversed, boolean fft_mirror, boolean subrtact_mode = true , boolean mask = true)
{
		// form_part[i + (z * 8)].start_led, &form_part[i + (z * 8)].nr_leds, bitRead(form_menu[z][_M_REVERSED_], i), bitRead(form_menu[z][_M_ONE_COLOR_], i), bitRead(form_menu[z][_M_MIRROR_OUT_], i));

	//debugMe("s:" + String(leds[0].red) + " : "  + String(leds[0].green) + " : " + String(leds[0].blue) );
	
	

	byte mirror_div = 1;
	byte mirror_add = 0;
			if (fft_mirror == true) 
			{
				mirror_div = 2;
				if (isODDnumber(nr_leds) == true) 
				{
					mirror_add = 1; // dosmething
				}
			}
	
	if(nr_leds != 0)
	{
		if(!fft_reversed)  // = forward
		{	
			
			for(uint16_t led_num = start_led; led_num < start_led + nr_leds  ; led_num ++ )
			{
				if(!mask)
				{
				if(subrtact_mode)
					{
						if(!fft_mirror)
						{
							leds[led_num].red 	= qsub8(leds_FFT_history[led_num - start_led  ].red,	leds[led_num].red  	);	
							leds[led_num].green = qsub8(leds_FFT_history[led_num - start_led  ].green,leds[led_num].green );
							leds[led_num].blue 	= qsub8(leds_FFT_history[led_num - start_led  ].blue,	leds[led_num].blue  );
						}
						else // mirror fft
						{
							if(led_num <= start_led + nr_leds/2)
							{
								leds[led_num].red 	= qsub8(leds_FFT_history[led_num - start_led  ].red,	leds[led_num].red  	);	
								leds[led_num].green = qsub8(leds_FFT_history[led_num - start_led  ].green,leds[led_num].green );
								leds[led_num].blue 	= qsub8(leds_FFT_history[led_num - start_led  ].blue,	leds[led_num].blue  );

							}
							else
							{
								uint16_t hist_led_num = (nr_leds) -  (led_num - start_led)   ;
								leds[led_num].red 	= qsub8(leds_FFT_history[hist_led_num ].red,	leds[led_num].red  	);	
								leds[led_num].green = qsub8(leds_FFT_history[hist_led_num ].green,leds[led_num].green );
								leds[led_num].blue 	= qsub8(leds_FFT_history[hist_led_num ].blue,	leds[led_num].blue  );		
							}
								

						}


					}  // end subtract
					else
					{
						if(!fft_mirror)
						{
							leds[led_num].red 	= qadd8(leds_FFT_history[led_num - start_led  ].red,	leds[led_num].red  	);	
							leds[led_num].green = qadd8(leds_FFT_history[led_num - start_led  ].green,leds[led_num].green );
							leds[led_num].blue 	= qadd8(leds_FFT_history[led_num - start_led  ].blue,	leds[led_num].blue  );
						}
						else
						{
								if(led_num <= start_led + nr_leds/2)
							{
								leds[led_num].red 	= qadd8(leds_FFT_history[led_num - start_led  ].red,	leds[led_num].red  	);	
								leds[led_num].green = qadd8(leds_FFT_history[led_num - start_led  ].green,leds[led_num].green );
								leds[led_num].blue 	= qadd8(leds_FFT_history[led_num - start_led  ].blue,	leds[led_num].blue  );

							}
							else
							{
								uint16_t hist_led_num = (nr_leds) -  (led_num - start_led)   ;
								leds[led_num].red 	= qadd8(leds_FFT_history[hist_led_num ].red,	leds[led_num].red  	);	
								leds[led_num].green = qadd8(leds_FFT_history[hist_led_num ].green,leds[led_num].green );
								leds[led_num].blue 	= qadd8(leds_FFT_history[hist_led_num ].blue,	leds[led_num].blue  );		
							}

						}
					}

					//leds[led_num].red 	= constrain(leds_FFT_history[led_num - start_led  ].red - leds[led_num].red , 0, 255) ;
					//leds[led_num].blue 	= constrain(leds_FFT_history[led_num - start_led].blue	- leds[led_num].blue , 0, 255) ;
					//leds[led_num].green = constrain(leds_FFT_history[led_num - start_led].green - leds[led_num].green , 0, 255) ;

					//debugMe("nr:");
				}
				else	// were masking
				{
						//leds[led_num].red 	= constrain(leds_FFT_history[led_num - start_led  ].red, 	0,	leds[led_num].red  	);	
						//leds[led_num].green = constrain(leds_FFT_history[led_num - start_led  ].green, 	0,leds[led_num].green );
						//leds[led_num].blue 	= constrain(leds_FFT_history[led_num - start_led  ].blue,	0,	leds[led_num].blue  );

					if(!fft_mirror)
					{
						leds[led_num].red 	= map(leds_FFT_history[led_num - start_led  ].red, 		0, 255, 0, leds[led_num].red  	);	
						leds[led_num].green = map(leds_FFT_history[led_num - start_led  ].green, 	0, 255, 0, leds[led_num].green );
						leds[led_num].blue 	= map(leds_FFT_history[led_num - start_led  ].blue,		0, 255, 0, leds[led_num].blue  );	
					}
					else
					{
						if(led_num <= start_led + nr_leds/2)
							{
							leds[led_num].red 	= map(leds_FFT_history[led_num - start_led  ].red, 		0, 255, 0, leds[led_num].red  	);	
							leds[led_num].green = map(leds_FFT_history[led_num - start_led  ].green, 	0, 255, 0, leds[led_num].green );
							leds[led_num].blue 	= map(leds_FFT_history[led_num - start_led  ].blue,		0, 255, 0, leds[led_num].blue  );
							}
						else
						{
							uint16_t hist_led_num = (nr_leds) -  (led_num - start_led)   ;
							leds[led_num].red 	= map(leds_FFT_history[hist_led_num  ].red, 		0, 255, 0, leds[led_num].red  	);	
							leds[led_num].green = map(leds_FFT_history[hist_led_num ].green, 	0, 255, 0, leds[led_num].green );
							leds[led_num].blue 	= map(leds_FFT_history[hist_led_num ].blue,		0, 255, 0, leds[led_num].blue  );
						}

					}

				}				
			}
			//if (mirror == true) LEDS_Copy_strip(start_led + nr_leds / 2 + mirror_add , -nr_leds / 2 , start_led);	


		}
		else  // reversed
		{
			 
			for(uint16_t led_num = start_led   ; led_num < start_led + nr_leds   ; led_num++ ) 
			{

				if(!mask)
				{
					if(subrtact_mode)
					{
						if(!fft_mirror)
						{
							leds[led_num].red =       qsub8(leds_FFT_history[ nr_leds - (led_num - start_led) ].red,leds[led_num].red   ) ;
							leds[led_num].green  =    qsub8(leds_FFT_history[ nr_leds - (led_num - start_led) ].green,leds[led_num].green   ) ;
							leds[led_num].blue  =     qsub8(leds_FFT_history[ nr_leds - (led_num - start_led) ].blue,leds[led_num].blue  ) ;
						}
						else
						{

							if(led_num <= start_led + nr_leds/2)
							{
								uint16_t hist_led_num =  nr_leds/2 - (led_num - start_led);
								leds[led_num].red =       qsub8(leds_FFT_history[ hist_led_num ].red,leds[led_num].red   ) ;
								leds[led_num].green  =    qsub8(leds_FFT_history[ hist_led_num ].green,leds[led_num].green   ) ;
								leds[led_num].blue  =     qsub8(leds_FFT_history[ hist_led_num ].blue,leds[led_num].blue  ) ;

							}
							else{
								uint16_t hist_led_num =  (led_num - start_led) - nr_leds/2    ;
								leds[led_num].red =       qsub8(leds_FFT_history[ hist_led_num ].red,leds[led_num].red   ) ;
								leds[led_num].green  =    qsub8(leds_FFT_history[ hist_led_num ].green,leds[led_num].green   ) ;
								leds[led_num].blue  =     qsub8(leds_FFT_history[ hist_led_num ].blue,leds[led_num].blue  ) ;
							}
						}
		
					}
					else
					{
						if(!fft_mirror) 	leds[led_num] += leds_FFT_history[ nr_leds - (led_num - start_led) ]; 

						else
						{
							if(led_num <= start_led + nr_leds/2) leds[led_num] += leds_FFT_history[ nr_leds/2  - (led_num - start_led) ]; 
							else 								 leds[led_num] += leds_FFT_history[ (led_num - start_led) - nr_leds/2]; 

						
							
						}

					}

				} // end mask
				else
				{
					if(!fft_mirror) 
					{
						leds[led_num].red =       map(leds_FFT_history[ nr_leds - (led_num - start_led) ].red,	 0,255,0, leds[led_num].red   ) ;
						leds[led_num].green  =    map(leds_FFT_history[ nr_leds - (led_num - start_led) ].green, 0,255,0, leds[led_num].green   ) ;
						leds[led_num].blue  =     map(leds_FFT_history[ nr_leds - (led_num - start_led) ].blue,	 0,255,0, leds[led_num].blue  ) ;
					}
					else
					{
						if(led_num <= start_led + nr_leds/2)
						{
							leds[led_num].red =       map(leds_FFT_history[ nr_leds/2 - (led_num - start_led) ].red,	 0,255,0, leds[led_num].red   ) ;
							leds[led_num].green  =    map(leds_FFT_history[ nr_leds/2 - (led_num - start_led) ].green, 0,255,0, leds[led_num].green   ) ;
							leds[led_num].blue  =     map(leds_FFT_history[ nr_leds/2 - (led_num - start_led) ].blue,	 0,255,0, leds[led_num].blue  ) ;
						}
						else
						{
							uint16_t hist_led_num =  (led_num - start_led) - nr_leds/2    ;		
							leds[led_num].red =       map(leds_FFT_history[hist_led_num].red,	 0,255,0, leds[led_num].red   ) ;
							leds[led_num].green  =    map(leds_FFT_history[ hist_led_num ].green, 0,255,0, leds[led_num].green   ) ;
							leds[led_num].blue  =     map(leds_FFT_history[ hist_led_num ].blue,	 0,255,0, leds[led_num].blue  ) ;
						}


					} 

					
				}

				
			}

			//if (mirror == true) LEDS_Copy_strip(start_led + nr_leds / 2  , -(nr_leds + mirror_add) / 2, start_led);	

		}
	
	}	
	//debugMe("e:" + String(leds[0].red) + " : "  + String(leds[0].green) + " : " + String(leds[0].blue) );
}


uint8_t LEDS_FFT_get_color_result(uint8_t color )
{
	switch(color)
	{
		case 0: return GlobalColor_result.red; break; 
		case 1: return GlobalColor_result.green; break; 
		case 2: return GlobalColor_result.blue; break; 

	}

	return 0;
}


void LEDS_FFT_check_leds(CRGB color_result)
{	// check if FFT is selected and then send it to the leds

	if (LEDS_checkIfAudioSelected())
	{
		for (byte z = 0; z < _M_NR_STRIP_BYTES_; z++) 
		{
			for (byte i = 0; i < 8; i++) 
			{ 	
			if (bitRead(strip_menu[z][_M_AUDIO_], i) == true && bitRead(strip_menu[z][_M_STRIP_], i) == false )		
							LEDS_FFT_fill_leds(part[i + (z * 8)].start_led, part[i + (z * 8)].nr_leds, bitRead(strip_menu[z][_M_AUDIO_REVERSED], i), bitRead(strip_menu[z][_M_AUDIO_ONECOLOR], i), bitRead(strip_menu[z][_M_AUDIO_MIRROR], i), part[i + (z * 8)].fft_offset );
			if (bitRead(strip_menu[z][_M_AUDIO_], i) == true && bitRead(strip_menu[z][_M_STRIP_], i) == true ) 		LEDS_FFT_pal_mix(part[i + (z * 8)].start_led, part[i + (z * 8)].nr_leds,  bitRead(strip_menu[z][_M_AUDIO_REVERSED], i), bitRead(strip_menu[z][_M_MIRROR_OUT_],i), bitRead(strip_menu[z][_M_AUDIO_SUB_FROM_FFT],i), bitRead(strip_menu[z][_M_AUDIO_PAL_MASK],i));
			}	
		}
		for (byte z = 0; z < _M_NR_FORM_BYTES_; z++) 
		{
			for (byte i = 0; i < 8; i++) 
			{
				if (bitRead(form_menu[z][_M_AUDIO_], i) == true  && bitRead(form_menu[z][_M_STRIP_], i) == false )			LEDS_FFT_fill_leds(form_part[i + (z * 8)].start_led, form_part[i + (z * 8)].nr_leds, bitRead(form_menu[z][_M_AUDIO_REVERSED], i), bitRead(form_menu[z][_M_AUDIO_ONECOLOR], i), bitRead(form_menu[z][_M_AUDIO_MIRROR], i), form_part[i + (z * 8)].fft_offset);   //, form_part[i + ( z * 8 ) ].ref_led) ;
				if (bitRead(form_menu[z][_M_AUDIO_], i) == true  && bitRead(form_menu[z][_M_STRIP_], i) == true )  			LEDS_FFT_pal_mix(form_part[i + (z * 8)].start_led,form_part[i + (z * 8)].nr_leds, bitRead(form_menu[z][_M_AUDIO_REVERSED], i), bitRead(form_menu[z][_M_AUDIO_MIRROR],i ) , bitRead(form_menu[z][_M_AUDIO_SUB_FROM_FFT], i), bitRead(form_menu[z][_M_AUDIO_PAL_MASK], i) );
				//if (bitRead(form_menu[z][_M_AUDIO_DOT_], i) == true) ramdom_audio_dot(color_result, &form_part[i + (z * 8)].start_led, &form_part[i + (z * 8)].nr_leds);
				//if (bitRead(form_menu[z][_M_AUDIO_DOT_], i) == true)    LEDS_FFT_running_dot(color_result, &form_part[i + (z * 8)].start_led, &form_part[i + (z * 8)].nr_leds, bitRead(form_menu[z][_M_AUDIO_REVERSED], i), form_part[i + (z * 8)].juggle_speed, form_part[i + (z * 8)].juggle_nr_dots);
				
			}
		}
	}

}
 
#define datarate 12
void LEDS_setup()
{	// the main led setup function
	// add the correct type of led
	 debugMe("in LED Setup");
	 LEDS_MSGEQ7_setup();
	 
	switch(led_cfg.ledMode)
	{
		case 0:
			debugMe("mix mode Mirror");
			if(get_bool(DATA1_ENABLE)){ FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(datarate)>	(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("Mode Mix-Mirror - APA102 leds added on  DATA1+CLK");}
			if(get_bool(DATA3_ENABLE)) {FastLED.addLeds<WS2812, LED_DATA_3_PIN, GRB>			(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); 	debugMe("WS2812 leds added on DATA3");}
			if(get_bool(DATA4_ENABLE)) {FastLED.addLeds<SK6822, LED_DATA_4_PIN, GRB>			(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); 	debugMe("SK6822 leds added on DATA4");}
		break;

		case 1:
			debugMe("APA102 mode line");
			if(get_bool(DATA1_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(datarate)>(leds,led_cfg.Data1StartLed , led_cfg.Data1NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+CLK"); }
			if(get_bool(DATA3_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(datarate)>(leds, led_cfg.Data3StartLed , led_cfg.Data3StartLed).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3+D4CLK"); }

			
		break;
		case 2:
			debugMe("APA102 mode Mirror");
			if(get_bool(DATA1_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR>(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+DATA2(clk)");}
			if(get_bool(DATA3_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR>(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3-DATA4(clk)");}
			
		break;
		case 3:
			debugMe("Mode LINE: WS2812b leds added on  DATA1 to DATA4");
			if(get_bool(DATA1_ENABLE)) {FastLED.addLeds<WS2812,LED_DATA_PIN  , GRB>(leds, led_cfg.Data1StartLed, led_cfg.Data1NrLeds).setCorrection(TypicalLEDStrip); debugMe(" DATA1 on");}
			if(get_bool(DATA2_ENABLE)) {FastLED.addLeds<WS2812,LED_CLK_PIN   , GRB>(leds, led_cfg.Data2StartLed, led_cfg.Data2NrLeds).setCorrection(TypicalLEDStrip); debugMe(" DATA2 on");}
			if(get_bool(DATA3_ENABLE)) {FastLED.addLeds<WS2812,LED_DATA_3_PIN, GRB>(leds, led_cfg.Data2StartLed, led_cfg.Data2NrLeds).setCorrection(TypicalLEDStrip); debugMe(" DATA3 on");}
			if(get_bool(DATA4_ENABLE)) {FastLED.addLeds<WS2812,LED_DATA_4_PIN, GRB>(leds, led_cfg.Data2StartLed, led_cfg.Data2NrLeds).setCorrection(TypicalLEDStrip); debugMe(" DATA4 on");}
		break;
		case 4:
		debugMe("ws2812 mode Mirror");
			if(get_bool(DATA1_ENABLE)) {FastLED.addLeds<WS2812,LED_DATA_PIN  , GRB>(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("Mode Mirror: WS2812b leds added on  DATA1 to DATA4");}
			if(get_bool(DATA2_ENABLE)) {FastLED.addLeds<WS2812,LED_CLK_PIN   , GRB>(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); }
			if(get_bool(DATA3_ENABLE)) {FastLED.addLeds<WS2812,LED_DATA_3_PIN, GRB>(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); }
			if(get_bool(DATA4_ENABLE)) {FastLED.addLeds<WS2812,LED_DATA_4_PIN, GRB>(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip);}
		break;
		case 5:
		debugMe("mix mode Line");
			if(get_bool(DATA1_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(datarate)>(leds, led_cfg.Data1StartLed,  uint16_t(constrain(led_cfg.Data1NrLeds, 0,MAX_NUM_LEDS - led_cfg.Data1StartLed)) ).setCorrection(TypicalLEDStrip); debugMe("Mode_LINE: APA102 leds added on  DATA1+CLK");}
			if(get_bool(DATA3_ENABLE)) {FastLED.addLeds<WS2812, LED_DATA_3_PIN, GRB>           (leds, led_cfg.Data3StartLed, uint16_t(constrain(led_cfg.Data3NrLeds, 0,MAX_NUM_LEDS - led_cfg.Data3StartLed)) ).setCorrection(TypicalLEDStrip); 	debugMe("WS2812 leds added on DATA3");}
			if(get_bool(DATA4_ENABLE)) {FastLED.addLeds<SK6822, LED_DATA_4_PIN, GRB>           (leds, led_cfg.Data4StartLed, uint16_t(constrain(led_cfg.Data4NrLeds, 0,MAX_NUM_LEDS - led_cfg.Data4StartLed)) ).setCorrection(TypicalLEDStrip); 	debugMe("SK6822 leds added on DATA4");}
		break;
		default:
			if(get_bool(DATA1_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(datarate)>(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); 	debugMe("APA102 leds added on  DATA1+CLK");}
			if(get_bool(DATA3_ENABLE)) {FastLED.addLeds<WS2812, LED_DATA_3_PIN, GRB>           (leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); 	debugMe("WS2812 leds added on DATA3");}
			if(get_bool(DATA4_ENABLE)) {FastLED.addLeds<SK6822, LED_DATA_4_PIN,GRB>            (leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); 	debugMe("SK6822 leds added on DATA4");}
		break;

	}
	debugMe("LED_MODE = " + String(led_cfg.ledMode));

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


	//led_cfg.bri = led_cfg.startup_bri;				// set the bri to the startup bri

	uint8_t core = xPortGetCoreID();
    debugMe("Main code running on core " + String(core));

    // -- Create the FastLED show task
    xTaskCreatePinnedToCore(FastLEDshowTask, "FastLEDshowTask", 2048, NULL, 2, &FastLEDshowTaskHandle, FASTLED_SHOW_CORE);


	if (FS_play_conf_read(0) == false)				
	{
		form_menu[0][_M_STRIP_] = 1;

	}
	LEDS_pal_reset_index();


	led_cnt.PotBriLast = analogRead(POTI_BRI_PIN) / ANALOG_IN_DEVIDER;			//get the initial potti status so that when we load the config the possis wont override it.
	led_cnt.PotFPSLast = analogRead(POTI_FPS_PIN) / ANALOG_IN_DEVIDER;

	fft_led_cfg.update_time = micros();
	debugMe("end LEDS setup");
}


void LEDS_loop()
{	// the main led loop

	unsigned long currentT = micros();

	#ifndef ARTNET_DISABLED
		if (get_bool(ARTNET_ENABLE)) WiFi_artnet_loop();  //  fetshing data 
	#endif


	if (currentT > led_cfg.update_time  && !get_bool(ARTNET_ENABLE) )
	{
		{
			//debugMe("IN LED LOOP - disabled fft");
			if(fft_data_fps == 0)
				led_cfg.update_time = currentT + (1000000 / led_cfg.pal_fps);
			else
				led_cfg.update_time = currentT + (1000000 / map( fft_color_fps,  0 ,255 , led_cfg.pal_fps, MAX_PAL_FPS )) ;     // if we are adding FFT data to FPS speed 

			leds.fadeToBlackBy(255);				// fade the whole led array to black so that we can add from different sources amd mix it up!

			if (LEDS_pal_check_bit() == true)
			{
				yield();
				//debugMe("pre pal advance");
				LEDS_pal_advance();
				yield();
				//debugMe("pre leds routing");
				LEDS_pal_routing();
			}

			if (LEDS_checkIfAudioSelected()) 
			{
				LEDS_FFT_process();  // Get the color from the FFT data
				LEDS_FFT_history_run(GlobalColor_result);
				LEDS_FFT_check_leds(GlobalColor_result);      // send the color to the leds.
				yield();
			}
			
			
			while (FFT_fifo.count() >= 7)		// sanity check to keep the queue down if disabled free up memory
			{
				uint8_t buffer = FFT_fifo.dequeue();
				debugMe("dequing overflow");
				buffer = 0;
			} 

		}

	
		//if (get_bool(UPDATE_LEDS) == true)
		//{
		//debugMe("pre show processing");
			LEDS_G_pre_show_processing();
			yield();
			//debugMe("pre leds SHOW");
			LEDS_show();
			yield();
		//	write_bool(UPDATE_LEDS, false);
		//}
		bool Btn_state = digitalRead(BTN_PIN);
		 if(Btn_state != get_bool(BTN_LASTSTATE))
		 {
			 	debugMe("Change BTN ");
			 	write_bool(BTN_LASTSTATE, Btn_state);
				 if (Btn_state == false )
				 {
					 if(FS_check_Conf_Available(led_cfg.Play_Nr +1 ))
					 {
					 	FS_play_conf_read(led_cfg.Play_Nr +1);
						 led_cfg.Play_Nr++;
						 debugMe("in +1");
					 }
					else{
							FS_play_conf_read(0);
							led_cfg.Play_Nr = 0;
					}
						

				 }


		 } 

		 if (get_bool(FFT_OSTC_VIZ) && currentT >= fft_led_cfg.update_time ) 
		 {

			fft_led_cfg.update_time = currentT + (1000000 / fft_led_cfg.viz_fps);	 
			 
			 osc_StC_FFT_vizIt(); 
			 //debugMe("vizzit");


		}
	}


	//debugMe("leds loop end ", false);
	//debugMe(String(xPortGetCoreID()));
}
