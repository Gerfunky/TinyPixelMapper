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
	//#include "msgeq7_fft.h"


	#include <FastLED.h>
	
	#include <RunningAverage.h>			// For Auto FFT
	#include <QueueArray.h>	   			// For buffering incoming FFT packets

	#include <TPM-FX.h>

	tpm_fx tpm_fx;					//load the FX lib

	#define ANALOG_IN_DEVIDER 16 // devide analog in by this value to get into a 0-255 range 


// -- The core to run FastLED.show()
#define FASTLED_SHOW_CORE 0


	extern  void osc_StC_FFT_vizIt();			// of open stage controll to send the fft data


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
	//byte fft_data_menu[FFT_FX_NR_OF_BINS] = { 3,7,200,6,5,3 };   // 3 fft data bins for effects
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



fft_fxbin_struct fft_fxbin[FFT_FX_NR_OF_BINS] =
{
	{0,20,5,250}
	,{0,20,10,254}
	,{0,20,15,6}
	,{0,22,20,20}
	,{0,40,30,200}
	,{0,50,40,255}
};
	
	// uint8_t fft_color_result_data[FFT_FX_NR_OF_BINS] = {0,0,0};
	uint8_t fft_color_result_bri = 0;
	uint8_t fft_bin_autoTrigger = 0;
	uint8_t fft_color_fps = 0;

// ********************* LED Setup  FastLed
	CRGBArray<MAX_NUM_LEDS> leds;			// The Led array!    CRGBArray<NUM_LEDS> leds;
	CRGBArray<MAX_NUM_LEDS> tmp_array;        // used as a buffer before mirroring reversing.
	//CRGB leds[NUM_LEDS];
	//CRGBSet leds_p(leds, NUM_LEDS); led_cfg.NrLeds
	CRGBArray<MAX_NUM_LEDS> leds_FFT_history;
	CRGBArray<MAX_NUM_LEDS> led_FX_out;    // make a FX output array. 
	//CRGBArray<MAX_NUM_LEDS> led_pal_form_out;	// output from pallete
	//CRGBArray<MAX_NUM_LEDS> led_pal_strip_out;
	byte heat[NUM_LEDS];


	uint8_t layer_select[MAX_LAYERS_SELECT]  = {2,1,4,3,5,6,7,0,0,0,0,0,0,0,0,0};


			/*			0 = none
						1 = Form FFT
						2 = Strip FFT
						3 = Form pallete
						4 = Strip pallete
						5 = FX1
						6 = Fire
						7 = Shimmer
						
			*/

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

//led_cfg_struct led_cfg = { DEF_MAX_BRI , DEF_BRI,DEF_MAX_BRI, 255,255,255,0, 0,30, 200, 1,1,1 ,DEF_LED_MODE, NUM_LEDS ,DEF_FIRE_SPARKING,DEF_FIRE_COOLING,DEF_PLAY_MODE,DEF_DATA1_START_NR,DEF_DATA2_NR_LEDS,DEF_DATA2_START_NR,DEF_DATA3_NR_LEDS,DEF_DATA3_START_NR,DEF_DATA4_NR_LEDS,DEF_DATA4_START_NR, DEF_VIZ_UPDATE_TIME_FPS , 0};			// The basic led config
led_cfg_struct led_cfg = { DEF_MAX_BRI , DEF_BRI,DEF_MAX_BRI, 255,255,255,0, 0,30, 200, 1,1,1 ,DEF_LED_MODE, NUM_LEDS ,DEF_FIRE_SPARKING,DEF_FIRE_COOLING,DEF_PLAY_MODE, 
							{DEF_DATA1_START_NR,DEF_DATA2_START_NR, DEF_DATA3_START_NR,  DEF_DATA4_START_NR},
							{DEF_DATA1_NR_LEDS, DEF_DATA2_NR_LEDS, DEF_DATA3_NR_LEDS,DEF_DATA4_NR_LEDS} , DEF_APA102_DATARATE, 5 , 0};			// The basic led config


/*
Strip_FL_Struct part[NR_STRIPS] = {						// Holds the  Strip settings
	{ 0,  0,  0,  1,  0 , 1 ,  0,  0,255,255}  //0
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0,255,255}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0,255,255}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0,255,255}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0,255,255}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0,255,255}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0,255,255}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0,255,255}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0,255,255}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0,255,255}	//9
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0,255,255}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0,255,255}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0,255,255}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0,255,255}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0,255,255}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0,255,255}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0,255,255}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0,255,255}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0,255,255}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0,255,255}	//19
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0,255,255}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0,255,255}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0,255,255}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0,255,255}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0,255,255}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0,255,255}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0,255,255}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0,255,255}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0,255,255}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0,255,255}	//29
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0,255,255}
	,{ 0,  0,  0 , 1,  0 , 1 ,  0,  0,255,255}
};
*/

struct form_fx_test_val form_fx_test = {0,0,0};


struct form_Led_Setup_Struct form_cfg[NR_FORM_PARTS] =
{
	{0,NUM_LEDS},
	{0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0},

	{0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0}
};

struct form_fx_pal_struct form_fx_pal[NR_FORM_PARTS] = 
{
	{0,255,MIX_ADD,0,16,8,0,0},
	{0,255,MIX_ADD,0,16,8,0,0},
	{0,255,MIX_ADD,0,16,8,0,0},
	{0,255,MIX_ADD,0,16,8,0,0},
	{0,255,MIX_ADD,0,16,8,0,0},
	{0,255,MIX_ADD,0,16,8,0,0},
	{0,255,MIX_ADD,0,16,8,0,0},
	{0,255,MIX_ADD,0,16,8,0,0},

	{0,255,MIX_ADD,0,16,8,0,0},
	{0,255,MIX_ADD,0,16,8,0,0},
	{0,255,MIX_ADD,0,16,8,0,0},
	{0,255,MIX_ADD,0,16,8,0,0},
	{0,255,MIX_ADD,0,16,8,0,0},
	{0,255,MIX_ADD,0,16,8,0,0},
	{0,255,MIX_ADD,0,16,8,0,0},
	{0,255,MIX_ADD,0,16,8,0,0}

};

struct form_fx_shim_struct form_fx_shim[NR_FORM_PARTS] = 
{
	{0,MIX_ADD,255,6,5,7},
	{0,MIX_ADD,255,6,5,7},
	{0,MIX_ADD,255,6,5,7},
	{0,MIX_ADD,255,6,5,7},
	{0,MIX_ADD,255,6,5,7},
	{0,MIX_ADD,255,6,5,7},
	{0,MIX_ADD,255,6,5,7},
	{0,MIX_ADD,255,6,5,7},

	{0,MIX_ADD,255,6,5,7},
	{0,MIX_ADD,255,6,5,7},
	{0,MIX_ADD,255,6,5,7},
	{0,MIX_ADD,255,6,5,7},
	{0,MIX_ADD,255,6,5,7},
	{0,MIX_ADD,255,6,5,7},
	{0,MIX_ADD,255,6,5,7},
	{0,MIX_ADD,255,6,5,7}

};

struct form_fx_fire_struct form_fx_fire[NR_FORM_PARTS] = 
{
	{0,MIX_ADD,255},
	{0,MIX_ADD,255},
	{0,MIX_ADD,255},
	{0,MIX_ADD,255},
	{0,MIX_ADD,255},
	{0,MIX_ADD,255},
	{0,MIX_ADD,255},
	{0,MIX_ADD,255},

	{0,MIX_ADD,255},
	{0,MIX_ADD,255},
	{0,MIX_ADD,255},
	{0,MIX_ADD,255},
	{0,MIX_ADD,255},
	{0,MIX_ADD,255},
	{0,MIX_ADD,255},
	{0,MIX_ADD,255}

};

struct form_fx_fft_struct form_fx_fft[NR_FORM_PARTS] = 
{
	{MIX_ADD,255,0},
	{MIX_ADD,255,0},
	{MIX_ADD,255,0},
	{MIX_ADD,255,0},
	{MIX_ADD,255,0},
	{MIX_ADD,255,0},
	{MIX_ADD,255,0},
	{MIX_ADD,255,0},

	{MIX_ADD,255,0},
	{MIX_ADD,255,0},
	{MIX_ADD,255,0},
	{MIX_ADD,255,0},
	{MIX_ADD,255,0},
	{MIX_ADD,255,0},
	{MIX_ADD,255,0},
	{MIX_ADD,255,0}

};

struct form_fx1_struct form_fx1[NR_FORM_PARTS]= 
{
	{MIX_ADD,255},
	{MIX_ADD,255},
	{MIX_ADD,255},
	{MIX_ADD,255},
	{MIX_ADD,255},
	{MIX_ADD,255},
	{MIX_ADD,255},
	{MIX_ADD,255},

	{MIX_ADD,255},
	{MIX_ADD,255},
	{MIX_ADD,255},
	{MIX_ADD,255},
	{MIX_ADD,255},
	{MIX_ADD,255},
	{MIX_ADD,255},
	{MIX_ADD,255}

};

struct form_fx_glitter_struct form_fx_glitter[NR_FORM_PARTS] =
{
	{0,255,15},
	{0,255,15},
	{0,255,15},
	{0,255,15},
	{0,255,15},
	{0,255,15},
	{0,255,15},
	{0,255,15},

	{0,255,15},
	{0,255,15},
	{0,255,15},
	{0,255,15},
	{0,255,15},
	{0,255,15},
	{0,255,15},
	{0,255,15}
};

struct form_fx_dots_struct form_fx_dots[NR_FORM_PARTS] =
{
	{0,255,1,20,16,16},
	{0,255,1,20,16,16},
	{0,255,1,20,16,16},
	{0,255,1,20,16,16},
	{0,255,1,20,16,16},
	{0,255,1,20,16,16},
	{0,255,1,20,16,16},
	{0,255,1,20,16,16},

	{0,255,1,20,16,16},
	{0,255,1,20,16,16},
	{0,255,1,20,16,16},
	{0,255,1,20,16,16},
	{0,255,1,20,16,16},
	{0,255,1,20,16,16},
	{0,255,1,20,16,16},
	{0,255,1,20,16,16}

};


//byte strip_menu[_M_NR_STRIP_BYTES_][_M_NR_OPTIONS_];
/*
 =				// Strip Selection menu what efferct on/off/fft ....
{
	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }
	,{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	,{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }
	,{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }
};
*/

//uint8_t global_strip_opt[_M_NR_STRIP_BYTES_][_M_NR_GLOBAL_OPTIONS_] = { { 0,0 } ,{ 0,0 } };			// Test for global mirruring and reversing even in artnet

/*
byte form_menu[_M_NR_FORM_BYTES_][_M_NR_FORM_OPTIONS_] =				// Form selection menu
{
	 { 0,0,1,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0 ,0,0 }
	,{ 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0 ,0,0 }
};
*/

byte form_menu_pal[_M_NR_FORM_BYTES_][_M_NR_FORM_PAL_OPTIONS_];
byte form_menu_fft[_M_NR_FORM_BYTES_][_M_NR_FORM_FFT_OPTIONS_];
byte form_menu_fire[_M_NR_FORM_BYTES_][_M_NR_FORM_FIRE_OPTIONS_];
byte form_menu_shimmer[_M_NR_FORM_BYTES_][_M_NR_FORM_SHIMMER_OPTIONS_];

byte form_menu_fx1[_M_NR_FORM_BYTES_][_M_NR_FORM_FX1_OPTIONS_];
byte form_menu_dot[_M_NR_FORM_BYTES_][_M_NR_FORM_DOT_OPTIONS_];
byte form_menu_glitter[_M_NR_FORM_BYTES_][_M_NR_FORM_GLITTER_OPTIONS_];

uint16_t play_conf_time_min[MAX_NR_SAVES] = {5,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};

uint8_t squencer_bool[2]  = {0,0};		// to hold what saves o play in sequence mode

void LEDS_write_sequencer(uint8_t play_nr, boolean value)
{
	uint8_t bit_nr = play_nr;
	uint8_t byte_nr = 0;

	while( bit_nr > 7)
	{
		bit_nr = bit_nr - 8;
		byte_nr++;

	}
	//debugMe(play_nr);
	//debugMe(value);
	//debugMe(byte_nr);
	//debugMe(bit_nr);

	bitWrite(squencer_bool[byte_nr], bit_nr, value);


	for (uint8_t i = 0 ; i< 8 ;i++)	
	{
		//debugMe(String(i) + " -- " + String(bitRead(squencer_bool[0], i)));	
		//debugMe(String(i) + " .. " + String(bitRead(squencer_bool[1], i)));	
	}
}

boolean LEDS_get_sequencer(uint8_t play_nr)
{
	uint8_t bit_nr = play_nr;
	uint8_t byte_nr = 0;

	while( bit_nr > 7)
	{
		bit_nr = bit_nr - 8;
		byte_nr++;

	}

	boolean returnBool = bitRead(squencer_bool[byte_nr], bit_nr);

	return returnBool;


}

CRGBPalette16 LEDS_pal_get(uint8_t pal_no)
{
	if (pal_no < 16)
		return LEDS_pal_cur[pal_no];

	CRGBPalette16 tmp_palette = RainbowColors_p;
		
	
	if (pal_no == 20) tmp_palette = RainbowColors_p; 
	if (pal_no == 21) tmp_palette = RainbowStripeColors_p; 
	if (pal_no == 22)  tmp_palette =  CloudColors_p; 
	if (pal_no == 23)  tmp_palette =  PartyColors_p; 
	if (pal_no == 24)  tmp_palette =  OceanColors_p; 
	if (pal_no == 25)  tmp_palette =  ForestColors_p; 
	if (pal_no == 26)  tmp_palette =  HeatColors_p; 
	if (pal_no == 27)  tmp_palette =  LavaColors_p; 
	if (pal_no == 28)  tmp_palette =  pal_red_green; 
	if (pal_no == 29)  tmp_palette =  pal_red_blue; 
	if (pal_no == 30)  tmp_palette =  pal_green_blue; 
	if (pal_no == 31)  tmp_palette =  pal_black_white_Narrow; 
	if (pal_no == 32)  tmp_palette =  pal_black_white_wide; 
	
	
		return tmp_palette;

}

void LEDS_set_bri(uint8_t bri)
{
	led_cfg.bri = bri;

}

uint8_t LEDS_get_bri()
{
	return led_cfg.bri;

}


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
	//debugMe("Setall Leds to : " + String(color));	
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


void LED_master_rgb(uint16_t Start_led , uint16_t number_of_leds   )
{
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


/*
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


} */

// pre show precessing
/*
void LEDS_G_form_FX1_run()				// Chcek wwhat effect bits are set and do it
{	// main routing function for effects
	// read the bit from the menu and run if active

	for (byte z = 0; z < _M_NR_FORM_BYTES_; z++)
	{

		for (byte i = 0; i < 8; i++)
		{
			if(bitRead(form_menu_fx1[z][_M_FORM_FX1_RUN], i ))
			{
				if (form_cfg[i + (z * 8)].nr_leds != 0)  // only run if we actualy have leds to do 
				{										 // fade first so that we only fade the new effects on next go
				
					if (form_fx1[i + (z * 8)].fade != 0 )         	   tpm_fx.fadeLedArray(led_FX_out, form_cfg[i + (z * 8)].start_led, form_cfg[i + (z * 8)].nr_leds, form_fx1[i + (z * 8)].fade);

					if (bitRead(form_menu_glitter[z][_M_FORM_GLITTER_RUN], i) == true)      
					{ 
						if (form_fx_glitter[i + (z * 8)].pal  != 250)   // 250 = from FFT,  other = from Pallete
							tpm_fx.AddGlitter(led_FX_out, LEDS_pal_get(form_fx_glitter[i + (z * 8)].pal) ,form_fx_glitter[i + (z * 8)].value, form_cfg[i + (z * 8)].start_led, form_cfg[i + (z * 8)].nr_leds);	
						else
							tpm_fx.AddGlitter(led_FX_out ,GlobalColor_result ,form_fx_glitter[i + (z * 8)].value, form_cfg[i + (z * 8)].start_led, form_cfg[i + (z * 8)].nr_leds);
					}																				
					
					
					
					//if (bitRead(form_menu_glitter[z][_M_FORM_GLITTER_FFT], i) == true)  	{ tpm_fx.AddGlitter(led_FX_out ,GlobalColor_result ,form_fx_glitter[i + (z * 8)].value, form_cfg[i + (z * 8)].start_led, form_cfg[i + (z * 8)].nr_leds); }

				
					
					
					if (bitRead(form_menu_dot[z][_M_FORM_DOT_RUN], i) == true)         
					{
						CRGB dotcolor;
						
						if (form_fx_dots[i + (z * 8)].pal < 250)
							dotcolor = tpm_fx.PalGetFromLongPal(LEDS_pal_get(form_fx_dots[i + (z * 8)].pal ),form_fx_dots[i + (z * 8)].indexLong,form_fx_dots[i + (z * 8)].level,TBlendType(LINEARBLEND) );
						else if (form_fx_dots[i + (z * 8)].pal == 250) // 250 = fft
							dotcolor = GlobalColor_result;
						
						
						
						if (bitRead(form_menu_dot[z][_M_FORM_DOT_TYPE], i) == DOT_SINE	)
						{
							if (form_fx_dots[i + (z * 8)].pal <= 250)
								tpm_fx.DotSine(led_FX_out, dotcolor,form_fx_dots[i + (z * 8)].nr_dots, form_cfg[i + (z * 8)].start_led, form_cfg[i + (z * 8)].nr_leds, form_fx_dots[i + (z * 8)].speed, form_fx_dots[i + (z * 8)].level, 255); 
							else if (form_fx_dots[i + (z * 8)].pal == 251) // Hue Dot
								tpm_fx.DotSine(led_FX_out, led_cfg.hue,form_fx_dots[i + (z * 8)].nr_dots, form_cfg[i + (z * 8)].start_led, form_cfg[i + (z * 8)].nr_leds, form_fx_dots[i + (z * 8)].speed, form_fx_dots[i + (z * 8)].level, 255);
						}
						else   // its a saw DOT
						{ 

							if (form_fx_dots[i + (z * 8)].pal <= 250)
								tpm_fx.DotSaw(led_FX_out,  dotcolor,form_fx_dots[i + (z * 8)].nr_dots, form_cfg[i + (z * 8)].start_led, form_cfg[i + (z * 8)].nr_leds, form_fx_dots[i + (z * 8)].speed, form_fx_dots[i + (z * 8)].level, 255); 
							else if (form_fx_dots[i + (z * 8)].pal == 251) // Hue Dot
								 tpm_fx.DotSaw(led_FX_out, led_cfg.hue,form_fx_dots[i + (z * 8)].nr_dots, form_cfg[i + (z * 8)].start_led, form_cfg[i + (z * 8)].nr_leds, form_fx_dots[i + (z * 8)].speed, form_fx_dots[i + (z * 8)].level, 255); 
						}
							
						
						



					}	 

					//if (bitRead(form_menu_dot[z][_M_FORM_DOT_SAW], i) == true)        { tpm_fx.DotSaw(led_FX_out, led_cfg.hue,form_fx_dots[i + (z * 8)].nr_dots, form_cfg[i + (z * 8)].start_led, form_cfg[i + (z * 8)].nr_leds, form_fx_dots[i + (z * 8)].speed, form_fx_dots[i + (z * 8)].level, 255); }
																					
					
					//if (bitRead(form_menu_dot[z][_M_FORM_DOT_FFT], i) == true)     { tpm_fx.DotSaw(led_FX_out, GlobalColor_result, form_fx_dots[i + (z * 8)].nr_dots, form_cfg[i + (z * 8)].start_led, form_cfg[i + (z * 8)].nr_leds, form_fx_dots[i + (z * 8)].speed, form_fx_dots[i + (z * 8)].level, 255);}


				}
			}
			else
			{
					for (byte i = 0; i < 8; i++)
						if (form_cfg[i + (z * 8)].nr_leds != 0)  // only run if we actualy have leds to do 
							if (form_fx1[i + (z * 8)].fade != 0 )  
								tpm_fx.fadeLedArray(led_FX_out, form_cfg[i + (z * 8)].start_led, form_cfg[i + (z * 8)].nr_leds, form_fx1[i + (z * 8)].fade);       	 
								

			}

		}
	}

	led_cfg.hue++;
} */


void LEDS_G_pre_show_processing()
{	// the leds pre show prcessing 
	// run the effects and set the brightness.



	if(led_cfg.ledMode == 0 || led_cfg.ledMode == 2 || led_cfg.ledMode == 4 )
	{
		LED_master_rgb(0, led_cfg.NrLeds   );

	}
	else
	{
		if(get_bool(DATA1_ENABLE))  LED_master_rgb(led_cfg.DataStart_leds[0] , led_cfg.DataNR_leds[0]   );
		if(get_bool(DATA2_ENABLE))  LED_master_rgb(led_cfg.DataStart_leds[1] , led_cfg.DataNR_leds[1]   );
		if(get_bool(DATA3_ENABLE))  LED_master_rgb(led_cfg.DataStart_leds[2] , led_cfg.DataNR_leds[2]   );
		if(get_bool(DATA4_ENABLE))  LED_master_rgb(led_cfg.DataStart_leds[3] , led_cfg.DataNR_leds[3]   );
	}

	

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
	//for (byte zp = 0; zp < _M_NR_STRIP_BYTES_; zp++) if (strip_menu[zp][_M_AUDIO_] != 0)   return true;
	for (byte zf = 0; zf < _M_NR_FORM_BYTES_; zf++)  if ((form_menu_fft[zf][_M_FORM_FFT_RUN] != 0) ) return true;
	if(fft_data_bri != 0) return true;
	if(fft_fxbin[0].menu_select != 0) return true;
	if(fft_fxbin[1].menu_select != 0) return true;
	if(fft_fxbin[2].menu_select != 0) return true;
	if(fft_data_fps != 0) return true;
	return false;

}


uint8_t getrand8()
{
return random8();

}

// palletes






void LEDS_pal_load(uint8_t pal_no, uint8_t pal_menu)
{
	// load a pallete from the default (FastLed)
	//debugMe("Load pal" + String(pal_menu));
	if (pal_no < NR_PALETTS && pal_menu < NR_PALETTS_SELECT )
	switch (pal_menu)
	{
	case 0: LEDS_pal_cur[pal_no] = LEDS_pal_cur[0]; break;
	case 1: LEDS_pal_cur[pal_no] = LEDS_pal_cur[1]; break;
	case 2: LEDS_pal_cur[pal_no] = LEDS_pal_cur[2]; break;
	case 3: LEDS_pal_cur[pal_no] = LEDS_pal_cur[3]; break;
	case 4: LEDS_pal_cur[pal_no] = LEDS_pal_cur[4]; break;
	case 5: LEDS_pal_cur[pal_no] = LEDS_pal_cur[5]; break;
	case 6: LEDS_pal_cur[pal_no] = LEDS_pal_cur[6]; break;
	case 7: LEDS_pal_cur[pal_no] = LEDS_pal_cur[7]; break;
	case 8: LEDS_pal_cur[pal_no] = LEDS_pal_cur[8]; break;
	case 9: LEDS_pal_cur[pal_no] = LEDS_pal_cur[9]; break;
	case 10: LEDS_pal_cur[pal_no] = LEDS_pal_cur[10]; break;
	case 11: LEDS_pal_cur[pal_no] = LEDS_pal_cur[11]; break;
	case 12: LEDS_pal_cur[pal_no] = LEDS_pal_cur[12]; break;
	case 13: LEDS_pal_cur[pal_no] = LEDS_pal_cur[13]; break;
	case 14: LEDS_pal_cur[pal_no] = LEDS_pal_cur[14]; break;
	case 15: LEDS_pal_cur[pal_no] = LEDS_pal_cur[15]; break;

	case 19: for (int i = 0; i < 16; i++) { LEDS_pal_cur[pal_no][i] = CHSV(random8(), 255, random8());} break;
	case 20: LEDS_pal_cur[pal_no] = RainbowColors_p; break;
	case 21: LEDS_pal_cur[pal_no] = RainbowStripeColors_p; break;
	case 22: LEDS_pal_cur[pal_no] = CloudColors_p; break;
	case 23: LEDS_pal_cur[pal_no] = PartyColors_p; break;
	case 24: LEDS_pal_cur[pal_no] = OceanColors_p; break;
	case 25: LEDS_pal_cur[pal_no] = ForestColors_p; break;
	case 26: LEDS_pal_cur[pal_no] = HeatColors_p; break;
	case 27: LEDS_pal_cur[pal_no] = LavaColors_p; break;
	case 28: LEDS_pal_cur[pal_no] = pal_red_green; break;
	case 29: LEDS_pal_cur[pal_no] = pal_red_blue; break;
	case 30: LEDS_pal_cur[pal_no] = pal_green_blue; break;
	case 31: LEDS_pal_cur[pal_no] = pal_black_white_Narrow; break;
	case 32: LEDS_pal_cur[pal_no] = pal_black_white_wide; break;
	
	
	default: LEDS_pal_cur[pal_no] = RainbowColors_p; break;
		

	}

}



void LEDS_pal_reset_index() 
{	// reset all the pallete indexes

	/*for (int z = 0; z < _M_NR_STRIP_BYTES_; z++) {
		for (int i = 0; i < 8; i++) {

			part[i + (z * 8)].index = part[i + (z * 8)].index_start;
			part[i + (z * 8)].index_long = part[i + (z * 8)].index_start;

		}
	} */

	for (int z = 0; z < _M_NR_FORM_BYTES_; z++) {
		for (int i = 0; i < 8; i++) {

			form_fx_pal[i+(z * 8)].index = form_fx_pal[i+ (z * 8)].index_start;
			form_fx_pal[i + (z * 8)].indexLong = form_fx_pal[i + (z * 8)].index_start;
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






// Artnet
void LEDS_artnet_in(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{	// process the ARTNET information and send it to the leds
	
	//debugMe("in artnet uni:" + String(universe));
	//debugMe(String( artnet_cfg.startU));
	if ((universe >= artnet_cfg.startU) && (universe < artnet_cfg.startU + artnet_cfg.numU))
	{
		//FastLED.show();
		byte internal_universe = universe - artnet_cfg.startU;

		uint8_t max_length = length / 3;
		//debugMe(max_length);
		// read universe and put into the right part of the display buffer
		for (int i = 0; i < max_length ; i++)
		{
			
			int led = i + (internal_universe * 170);
			//debugMe(led);
			if (led < MAX_NUM_LEDS) 
			{
				leds[led].r = data[i * 3];
				leds[led].g = data[i * 3 + 1];
				leds[led].b = data[i * 3 + 2];
				//debugMe(leds[led].r);
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
	fft_color_result_bri = 0;
	fft_color_fps = 0;
	int bins[7] = {0,0,0,0,0,0,0};

	LEDS_MSGEQ7_get();  // get the FFT data and put it in fft_data[i].value
	LEDS_FFT_calc_avarage(); // update the avarages for autofft.

	// debugMe("FFT fill bins");

	for( uint8_t z = 0; z < FFT_FX_NR_OF_BINS ; z++)
		fft_fxbin[z].sum = 0;

	
	for (byte i = 0; i < 7; i++) 
	{
		bins[i] = constrain((fft_data[i].value) - fft_data[i].trigger, 0, 255);
		if (bitRead(fft_menu[0], i) == true) color_result.r = constrain((color_result.r + bins[i]), 0, 255);
		if (bitRead(fft_menu[1], i) == true) color_result.g = constrain((color_result.g + bins[i]), 0, 255);
		if (bitRead(fft_menu[2], i) == true) color_result.b = constrain((color_result.b + bins[i]), 0, 255);

		if (bitRead(fft_data_bri, i) == true) fft_color_result_bri = constrain((fft_color_result_bri + bins[i]), 0, 255);
		if (bitRead(fft_data_fps, i) == true) fft_color_fps = constrain((fft_color_fps + bins[i]), 0, 255);


		for( uint8_t z = 0; z < FFT_FX_NR_OF_BINS ; z++)
		{
			if (bitRead(fft_fxbin[z].menu_select, i) == true) fft_fxbin[z].sum = constrain((fft_fxbin[z].sum + bins[i]), 0, 255);

		}	
				
		//if (bitRead(fft_data_menu[1], i) == true) fft_color_result_data[1] = constrain((fft_color_result_data[1] + bins[i]), 0, 255);
		//if (bitRead(fft_data_menu[2], i) == true) fft_color_result_data[2] = constrain((fft_color_result_data[2] + bins[i]), 0, 255);

	}
	//debugMe(fft_color_result_data[1]);	
	//debugMe(fft_data_menu[0], false);
	//debugMe("..");
	// fade the RGB 

	/*
	if (led_cfg.r != 255) color_result.r = color_result.r * led_cfg.r / 255 ;
	if (led_cfg.g != 255) color_result.g = color_result.g * led_cfg.g / 255 ;
	if (led_cfg.b != 255) color_result.b = color_result.b * led_cfg.b / 255 ;
	*/

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


 
void LEDS_load_default_play_conf()
{


	led_cfg.bri					= 255;
	led_cfg.fire_cooling		= DEF_FIRE_COOLING ;
	led_cfg.fire_sparking		= DEF_FIRE_SPARKING ;
	led_cfg.r					= 255;
	led_cfg.g					= 255;
	led_cfg.b					= 255;
	led_cfg.pal_bri				= 255;
	led_cfg.pal_fps     		= 255;
	
	fft_led_cfg.Scale = 0;

	

	uint8_t strip_no = 0;
				
/*
	part[strip_no].start_led = 0;

	part[strip_no].nr_leds = MAX_NUM_LEDS;
				
	part[strip_no].index_start = 0;
	part[strip_no].index_add = 64; 	
	part[strip_no].index_add_pal = 32;
	part[strip_no].fft_offset = 0;
				*/

	//bitWrite(strip_menu[0][_M_STRIP_],0, true);
	uint8_t form_nr = 0;

	form_cfg[form_nr].start_led = 0;
	form_cfg[form_nr].nr_leds = MAX_NUM_LEDS;
	form_fx1[form_nr].fade = 1;

	form_fx_pal[form_nr].index_start = 0 ;
	form_fx_pal[form_nr].index_add_led = 64;
	form_fx_pal[form_nr].index_add_frame = 32;

	
	form_fx1[form_nr].level = 255;
	form_fx_glitter[form_nr].value = 20;
	form_fx_dots[form_nr].nr_dots = 2;
	form_fx_dots[form_nr].speed = 10;
	
	form_fx_fft[form_nr].offset = 0;


	bitWrite(form_menu_pal[0][_M_FORM_PAL_RUN],0, true);
	bitWrite(form_menu_fft[0][_M_FORM_FFT_RUN],0, true);
	//bitWrite(form_menu[0][_M_AUDIO_SUB_FROM_FFT],0, false);




	
	uint8_t bin = 0; // loe bin 
	bitWrite(fft_menu[0], bin, true);			// RED
	bitWrite(fft_menu[1], bin, false);			// GREEN
	bitWrite(fft_menu[2], bin, false);			// BLUE

	bitWrite(fft_data_bri, 			bin, true);
	bitWrite(fft_bin_autoTrigger,	bin, true);
	bitWrite(fft_data_fps, 			bin, true);

	
	bin++;  // bin1

	bitWrite(fft_menu[0], bin, true);
	bitWrite(fft_menu[1], bin, false);
	bitWrite(fft_menu[2], bin, false);

	bitWrite(fft_data_bri, 			bin, true);
	bitWrite(fft_bin_autoTrigger,	bin, true);
	bitWrite(fft_data_fps, 			bin, true);


	bin++; // bin2

	bitWrite(fft_menu[0], bin, false);
	bitWrite(fft_menu[1], bin, true);
	bitWrite(fft_menu[2], bin, false);

	bitWrite(fft_data_bri, 			bin, false);
	bitWrite(fft_bin_autoTrigger,	bin, true);
	bitWrite(fft_data_fps, 			bin, false);

	bin++; // bin3

	bitWrite(fft_menu[0], bin, false);
	bitWrite(fft_menu[1], bin, true);
	bitWrite(fft_menu[2], bin, false);

	bitWrite(fft_data_bri, 			bin, false);
	bitWrite(fft_bin_autoTrigger,	bin, true);
	bitWrite(fft_data_fps, 			bin, false);			

	bin++; // bin4

	bitWrite(fft_menu[0], bin, false);
	bitWrite(fft_menu[1], bin, true);
	bitWrite(fft_menu[2], bin, false);

	bitWrite(fft_data_bri, 			bin, false);
	bitWrite(fft_bin_autoTrigger,	bin, true);
	bitWrite(fft_data_fps, 			bin, false);

	bin++; // bin5

	bitWrite(fft_menu[0], bin, false);
	bitWrite(fft_menu[1], bin, true);
	bitWrite(fft_menu[2], bin, false);

	bitWrite(fft_data_bri, 			bin, false);
	bitWrite(fft_bin_autoTrigger,	bin, true);
	bitWrite(fft_data_fps, 			bin, false);

	bin++; // bin6

	bitWrite(fft_menu[0], bin, false);
	bitWrite(fft_menu[1], bin, false);
	bitWrite(fft_menu[2], bin, true);

	bitWrite(fft_data_bri, 			bin, false);
	bitWrite(fft_bin_autoTrigger,	bin, true);
	bitWrite(fft_data_fps, 			bin, false);

	bin++; // bin7

	bitWrite(fft_menu[0], bin, false);
	bitWrite(fft_menu[1], bin, false);
	bitWrite(fft_menu[2], bin, true);

	bitWrite(fft_data_bri, 			bin, true);
	bitWrite(fft_bin_autoTrigger,	bin, true);
	bitWrite(fft_data_fps, 			bin, true);

}

uint8_t LEDS_get_playNr()
{
return led_cfg.Play_Nr;
}
void LEDS_set_playNr(uint8_t setNr)
{
	FS_play_conf_read(setNr);
}


void LEDS_seqencer_advance()
{
		uint8_t orig_play_nr = led_cfg.Play_Nr;

		if (orig_play_nr < MAX_NR_SAVES-1 )
		{
			for (uint8_t play_nr = led_cfg.Play_Nr +1 ; play_nr < MAX_NR_SAVES ; play_nr++  )
			{
						//debugMe("Play switch test to " + String(play_nr));

						if(LEDS_get_sequencer(play_nr) && FS_check_Conf_Available(play_nr ) &&  play_conf_time_min[play_nr] != 0   )
						{
							FS_play_conf_read(play_nr);
							break;
							
						}
						if (play_nr == MAX_NR_SAVES -1 )  play_nr = 0;
						if (play_nr == orig_play_nr ) break;
			}
		}
		else
		{
			for (uint8_t play_nr = 0 ; play_nr <= orig_play_nr ; play_nr++  )
			{
						//debugMe("15-Play switch test to " + String(play_nr));
						if(LEDS_get_sequencer(play_nr) && FS_check_Conf_Available(play_nr ) &&  play_conf_time_min[play_nr] != 0   )
						{
							FS_play_conf_read(play_nr);
							break;
							
						}
						
			}			
			
		}

		led_cfg.confSwitch_time = micros() +  play_conf_time_min[led_cfg.Play_Nr] * MICROS_TO_MIN  ;

	}



void LEDS_setup()
{	// the main led setup function
	// add the correct type of led
	 debugMe("in LED Setup");
	 LEDS_MSGEQ7_setup();
	 
	switch(led_cfg.ledMode)
	{
		case 0:
			//debugMe("mix mode Mirror");
			if(get_bool(DATA1_ENABLE))
			switch(led_cfg.apa102data_rate)
			{
				case 1: FastLED.addLeds<APA102,LED_DATA_PIN ,  LED_CLK_PIN, BGR,DATA_RATE_MHZ(1 )>	(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("Mode Mix-Mirror - APA102 leds added on  DATA1+CLK"); break; 
				case 2: FastLED.addLeds<APA102,LED_DATA_PIN ,  LED_CLK_PIN, BGR,DATA_RATE_MHZ(2 )>	(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("Mode Mix-Mirror - APA102 leds added on  DATA1+CLK"); break; 
				case 4: FastLED.addLeds<APA102,LED_DATA_PIN ,  LED_CLK_PIN, BGR,DATA_RATE_MHZ(4 )>	(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("Mode Mix-Mirror - APA102 leds added on  DATA1+CLK"); break; 
				case 8: FastLED.addLeds<APA102,LED_DATA_PIN ,  LED_CLK_PIN, BGR,DATA_RATE_MHZ(8 )>	(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("Mode Mix-Mirror - APA102 leds added on  DATA1+CLK"); break; 
				case 12: FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(12 )>	(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("Mode Mix-Mirror - APA102 leds added on  DATA1+CLK"); break; 
				case 16: FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(16 )>	(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("Mode Mix-Mirror - APA102 leds added on  DATA1+CLK"); break; 
				case 24: FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(24 )>	(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("Mode Mix-Mirror - APA102 leds added on  DATA1+CLK"); break; 
				default: FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(2 )>	(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("Mode Mix-Mirror - APA102 leds added on  DATA1+CLK"); break; 
			}

			if(get_bool(DATA3_ENABLE)) {FastLED.addLeds<WS2812, LED_DATA_3_PIN, GRB>			(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); 	debugMe("WS2812 leds added on DATA3");}
			if(get_bool(DATA4_ENABLE)) {FastLED.addLeds<SK6822, LED_DATA_4_PIN, GRB>			(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); 	debugMe("SK6822 leds added on DATA4");}
		break;

		case 1:
			debugMe("APA102 mode line");
			if(get_bool(DATA1_ENABLE)) 	switch(led_cfg.apa102data_rate)
			{
				case 1: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(1)>(leds,led_cfg.DataStart_leds[0]  , led_cfg.DataNR_leds[0]).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+CLK"); } break;
				case 2: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(2)>(leds,led_cfg.DataStart_leds[0]  , led_cfg.DataNR_leds[0]).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+CLK"); } break;
				case 4: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(4)>(leds,led_cfg.DataStart_leds[0]  , led_cfg.DataNR_leds[0]).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+CLK"); } break;
				case 8: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(8)>(leds,led_cfg.DataStart_leds[0]  , led_cfg.DataNR_leds[0]).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+CLK"); } break;
				case 12: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(12)>(leds,led_cfg.DataStart_leds[0]  , led_cfg.DataNR_leds[0]).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+CLK"); } break;
				case 16: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(16)>(leds,led_cfg.DataStart_leds[0]  , led_cfg.DataNR_leds[0]).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+CLK"); } break;
				case 24: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(24)>(leds,led_cfg.DataStart_leds[0]  , led_cfg.DataNR_leds[0]).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+CLK"); } break;
				default: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(2)>(leds,led_cfg.DataStart_leds[0]  , led_cfg.DataNR_leds[0]).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+CLK"); } break;

			}
			if(get_bool(DATA3_ENABLE)) switch(led_cfg.apa102data_rate)
			{
				case 1: {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(1)>(leds, led_cfg.DataStart_leds[2]  , led_cfg.DataStart_leds[2] ).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3+D4CLK"); } break;
				case 2: {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(2)>(leds, led_cfg.DataStart_leds[2]  , led_cfg.DataStart_leds[2] ).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3+D4CLK"); } break;
				case 4: {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(4)>(leds, led_cfg.DataStart_leds[2]  , led_cfg.DataStart_leds[2] ).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3+D4CLK"); } break;
				case 8: {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(8)>(leds, led_cfg.DataStart_leds[2]  , led_cfg.DataStart_leds[2] ).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3+D4CLK"); } break;
				case 12: {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(12)>(leds, led_cfg.DataStart_leds[2]  , led_cfg.DataStart_leds[2] ).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3+D4CLK"); } break;
				case 16: {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(16)>(leds, led_cfg.DataStart_leds[2]  , led_cfg.DataStart_leds[2] ).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3+D4CLK"); } break;
				case 24: {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(24)>(leds, led_cfg.DataStart_leds[2]  , led_cfg.DataStart_leds[2] ).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3+D4CLK"); } break;
				default: {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(2)>(leds, led_cfg.DataStart_leds[2]  , led_cfg.DataStart_leds[2] ).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3+D4CLK"); } break;
			}
			
		break;
		case 2:
			debugMe("APA102 mode Mirror");
			switch(led_cfg.apa102data_rate)
			{	
				case 1:
					if(get_bool(DATA1_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_PIN ,   LED_CLK_PIN,    BGR,DATA_RATE_MHZ(1)>(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+DATA2(clk)");}
					if(get_bool(DATA3_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(1)>(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3-DATA4(clk)");}
				break;
				case 2:
					if(get_bool(DATA1_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_PIN ,   LED_CLK_PIN,    BGR,DATA_RATE_MHZ(2)>(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+DATA2(clk)");}
					if(get_bool(DATA3_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(2)>(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3-DATA4(clk)");}
				break;
				case 4:
					if(get_bool(DATA1_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(4)>(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+DATA2(clk)");}
					if(get_bool(DATA3_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(4)>(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3-DATA4(clk)");}
				break;
				case 8:
					if(get_bool(DATA1_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(8)>(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+DATA2(clk)");}
					if(get_bool(DATA3_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(8)>(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3-DATA4(clk)");}
				break;
				case 12:
					if(get_bool(DATA1_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(12)>(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+DATA2(clk)");}
					if(get_bool(DATA3_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(12)>(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3-DATA4(clk)");}
				break;
				case 16:
					if(get_bool(DATA1_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(16)>(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+DATA2(clk)");}
					if(get_bool(DATA3_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(16)>(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3-DATA4(clk)");}
				break;
				case 24:
					if(get_bool(DATA1_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(24)>(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+DATA2(clk)");}
					if(get_bool(DATA3_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(24)>(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3-DATA4(clk)");}
				break;
				default :
					if(get_bool(DATA1_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(2)>(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+DATA2(clk)");}
					if(get_bool(DATA3_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(2)>(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3-DATA4(clk)");}
				break;

			}
		break;
		case 3:
			debugMe("Mode LINE: WS2812b leds added on  DATA1 to DATA4");
			if(get_bool(DATA1_ENABLE)) {FastLED.addLeds<WS2812,LED_DATA_PIN  , GRB>(leds, led_cfg.DataStart_leds[0] , led_cfg.DataNR_leds[0]).setCorrection(TypicalLEDStrip); debugMe(" DATA1 on");}
			if(get_bool(DATA2_ENABLE)) {FastLED.addLeds<WS2812,LED_CLK_PIN   , GRB>(leds, led_cfg.DataStart_leds[1] , led_cfg.DataNR_leds[1]).setCorrection(TypicalLEDStrip); debugMe(" DATA2 on");}
			if(get_bool(DATA3_ENABLE)) {FastLED.addLeds<WS2812,LED_DATA_3_PIN, GRB>(leds, led_cfg.DataStart_leds[2] , led_cfg.DataNR_leds[2]).setCorrection(TypicalLEDStrip); debugMe(" DATA3 on");}
			if(get_bool(DATA4_ENABLE)) {FastLED.addLeds<WS2812,LED_DATA_4_PIN, GRB>(leds, led_cfg.DataStart_leds[3] , led_cfg.DataNR_leds[3]).setCorrection(TypicalLEDStrip); debugMe(" DATA4 on");}
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

			if(get_bool(DATA1_ENABLE)) switch(led_cfg.apa102data_rate)
			{	case 1: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(1)>(leds, led_cfg.DataStart_leds[0] ,  uint16_t(constrain(led_cfg.DataNR_leds[0], 0,MAX_NUM_LEDS - led_cfg.DataStart_leds[0] )) ).setCorrection(TypicalLEDStrip); debugMe("Mode_LINE: APA102 leds added on  DATA1+CLK");}break;
				case 2: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(2)>(leds, led_cfg.DataStart_leds[0] ,  uint16_t(constrain(led_cfg.DataNR_leds[0], 0,MAX_NUM_LEDS - led_cfg.DataStart_leds[0] )) ).setCorrection(TypicalLEDStrip); debugMe("Mode_LINE: APA102 leds added on  DATA1+CLK");}break;
				case 4: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(4)>(leds, led_cfg.DataStart_leds[0] ,  uint16_t(constrain(led_cfg.DataNR_leds[0], 0,MAX_NUM_LEDS - led_cfg.DataStart_leds[0] )) ).setCorrection(TypicalLEDStrip); debugMe("Mode_LINE: APA102 leds added on  DATA1+CLK");}break;
				case 8: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(8)>(leds, led_cfg.DataStart_leds[0] ,  uint16_t(constrain(led_cfg.DataNR_leds[0], 0,MAX_NUM_LEDS - led_cfg.DataStart_leds[0] )) ).setCorrection(TypicalLEDStrip); debugMe("Mode_LINE: APA102 leds added on  DATA1+CLK");}break;
				case 12: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(12)>(leds, led_cfg.DataStart_leds[0] ,  uint16_t(constrain(led_cfg.DataNR_leds[0], 0,MAX_NUM_LEDS - led_cfg.DataStart_leds[0] )) ).setCorrection(TypicalLEDStrip); debugMe("Mode_LINE: APA102 leds added on  DATA1+CLK");}break;
				case 16: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(16)>(leds, led_cfg.DataStart_leds[0] ,  uint16_t(constrain(led_cfg.DataNR_leds[0], 0,MAX_NUM_LEDS - led_cfg.DataStart_leds[0] )) ).setCorrection(TypicalLEDStrip); debugMe("Mode_LINE: APA102 leds added on  DATA1+CLK");}break;
				case 24: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(24)>(leds, led_cfg.DataStart_leds[0] ,  uint16_t(constrain(led_cfg.DataNR_leds[0], 0,MAX_NUM_LEDS - led_cfg.DataStart_leds[0] )) ).setCorrection(TypicalLEDStrip); debugMe("Mode_LINE: APA102 leds added on  DATA1+CLK");}break;
				default: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(2)>(leds, led_cfg.DataStart_leds[0] ,  uint16_t(constrain(led_cfg.DataNR_leds[0], 0,MAX_NUM_LEDS - led_cfg.DataStart_leds[0] )) ).setCorrection(TypicalLEDStrip); debugMe("Mode_LINE: APA102 leds added on  DATA1+CLK");}break;
			}
			if(get_bool(DATA3_ENABLE)) {FastLED.addLeds<WS2812, LED_DATA_3_PIN, GRB>           (leds, led_cfg.DataStart_leds[2] , uint16_t(constrain(led_cfg.DataNR_leds[2], 0,MAX_NUM_LEDS - led_cfg.DataStart_leds[2] )) ).setCorrection(TypicalLEDStrip); 	debugMe("WS2812 leds added on DATA3");}
			if(get_bool(DATA4_ENABLE)) {FastLED.addLeds<SK6822, LED_DATA_4_PIN, GRB>           (leds, led_cfg.DataStart_leds[3] , uint16_t(constrain(led_cfg.DataNR_leds[3], 0,MAX_NUM_LEDS - led_cfg.DataStart_leds[3] )) ).setCorrection(TypicalLEDStrip); 	debugMe("SK6822 leds added on DATA4");}
		break;
		default:
			if(get_bool(DATA1_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(2)>(leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); 	debugMe("APA102 leds added on  DATA1+CLK");}
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
	LEDS_pal_cur[0] = pal_red_green;
	LEDS_pal_cur[1] = pal_red_green;


	//led_cfg.bri = led_cfg.startup_bri;				// set the bri to the startup bri

	uint8_t core = xPortGetCoreID();
    debugMe("Main code running on core " + String(core));

    // -- Create the FastLED show task
    xTaskCreatePinnedToCore(FastLEDshowTask, "FastLEDshowTask", 2048, NULL, 2, &FastLEDshowTaskHandle, FASTLED_SHOW_CORE);


	if (FS_play_conf_read(0) == false)				
	{
		LEDS_load_default_play_conf();

		//form_menu[0][_M_STRIP_] = 1;






	}
	LEDS_pal_reset_index();


	led_cnt.PotBriLast = analogRead(POTI_BRI_PIN) / ANALOG_IN_DEVIDER;			//get the initial potti status so that when we load the config the possis wont override it.
	led_cnt.PotFPSLast = analogRead(POTI_FPS_PIN) / ANALOG_IN_DEVIDER;

	fft_led_cfg.update_time = micros();
	fft_led_cfg.viz_fps = DEF_VIZ_UPDATE_TIME_FPS ;

	led_cfg.confSwitch_time = micros() +  play_conf_time_min[led_cfg.Play_Nr] * MICROS_TO_MIN  ;

	debugMe("end LEDS setup");
}


uint8_t LEDS_fft_get_fxbin_result(uint8_t fxbin)
{
	uint8_t returnVal = 0;

	if(fxbin < 2) 	
	{
		if(fft_fxbin[fxbin].sum >= fft_fxbin[fxbin].trrig_val)
				returnVal = constrain( fft_fxbin[fxbin].set_val + fft_fxbin[fxbin].sum, 0,255);
			else returnVal = 0;

	}
	else if(fxbin < 10) 
	{
		if(fft_fxbin[fxbin].sum >= fft_fxbin[fxbin].trrig_val)
				returnVal = constrain( fft_fxbin[fxbin].set_val + fft_fxbin[fxbin].sum, 0,255);
		else returnVal = fft_fxbin[fxbin].set_val;
	}
	else if(fxbin < 18) 
	{
		if(fft_fxbin[fxbin].sum >= fft_fxbin[fxbin].trrig_val)
				returnVal = constrain( fft_fxbin[fxbin].set_val - fft_fxbin[fxbin].sum, 0,255);
		else returnVal = fft_fxbin[fxbin].set_val;
	}
	else if(fxbin < 20) 
	{
		if(fft_fxbin[fxbin].sum >= fft_fxbin[fxbin].trrig_val)
				returnVal = constrain( fft_fxbin[fxbin].set_val - fft_fxbin[fxbin].sum, 0,255);
		else returnVal =255;
	}
	return returnVal;
}


uint8_t LEDS_data_or_fftbin(uint8_t inval)
{		
	// based on the input value, return a FFT bin or the inval.
	uint8_t returnVal = 0;
	if (inval >= FFT_FX_NR_OF_BINS) inval=0;

	if(inval < 2)
	{
		if(fft_fxbin[inval].sum > fft_fxbin[inval].trrig_val)
				returnVal = constrain( fft_fxbin[inval].set_val + fft_fxbin[inval].sum, 0,255);
			else returnVal = 0;
	}
	else if (inval < 10)
	{
			if(fft_fxbin[inval].sum > fft_fxbin[inval].trrig_val)
				returnVal = constrain( fft_fxbin[inval].set_val + fft_fxbin[inval].sum, 0,255);
			else returnVal = fft_fxbin[inval].set_val;

	}
	else if (inval < 18)
	{
			if(fft_fxbin[inval].sum > fft_fxbin[inval].trrig_val)
				returnVal = constrain( fft_fxbin[inval].set_val - fft_fxbin[inval].sum, 0,255);
			else returnVal = fft_fxbin[inval].set_val;

	}
	else if (inval < 20)
	{
			if(fft_fxbin[inval].sum > fft_fxbin[inval].trrig_val)
				returnVal = constrain( fft_fxbin[inval].set_val - fft_fxbin[inval].sum, 0,255);
			else returnVal = 255;
	}
	return returnVal;
}

void LEDS_run_fft(uint8_t z, uint8_t i )
{
	tpm_fx.mixHistoryOntoLedArray(leds_FFT_history, leds, form_cfg[i + (z * 8)].nr_leds, form_cfg[i + (z * 8)].start_led, bitRead(form_menu_fft[z][_M_FORM_FFT_REVERSED], i),  bitRead(form_menu_fft[z][_M_FORM_FFT_MIRROR],i ) , MixModeType(form_fx_fft[i + (z * 8)].mix_mode),  form_fx_fft[i + (z * 8)].level, bitRead(form_menu_fft[z][_M_FORM_FFT_ONECOLOR] , i), form_fx_fft[i + (z * 8)].offset  );
}

void LEDS_run_pal(uint8_t z, uint8_t i )
{
	
	tpm_fx.PalFillLong(tmp_array, LEDS_pal_get(form_fx_pal[i + (z * 8)].pal ), form_cfg[i + (z * 8)].start_led,form_cfg[i + (z * 8)].nr_leds  , form_fx_pal[i + (z * 8)].indexLong , form_fx_pal[i + (z * 8)].index_add_led,  MIX_REPLACE, 255,  TBlendType(bitRead(form_menu_pal[z][_M_FORM_PAL_BLEND], i)) );
	tpm_fx.mixOntoLedArray(tmp_array, leds, form_cfg[i + (z * 8)].nr_leds , form_cfg[i + (z * 8)].start_led,  bitRead(form_menu_pal[z][_M_FORM_PAL_REVERSED], i), bitRead(form_menu_pal[z][_M_FORM_PAL_MIRROR], i)   , MixModeType(form_fx_pal[i + (z * 8)].mix_mode), form_fx_pal[i + (z * 8)].level , bitRead(form_menu_pal[z][_M_FORM_PAL_ONECOLOR], i) );

	uint16_t pal_speed	= form_fx_pal[i + (z * 8)].index_add_frame   ;
	if(bitRead(form_menu_pal[z][_M_FORM_PAL_SPEED_FROM_FFT], i)) pal_speed = LEDS_data_or_fftbin(form_fx_pal[i + (z * 8)].index_add_frame) *8; 

	//debugMe("palSpeed" + String(pal_speed));
	form_fx_pal[i + (z * 8)].index = form_fx_pal[i + (z * 8)].index + pal_speed;
	form_fx_pal[i + (z * 8)].indexLong = form_fx_pal[i + (z * 8)].indexLong + pal_speed;
	//if (MAX_INDEX_LONG <= form_fx_pal[i + (z * 8)].indexLong)
	//			form_fx_pal[i + (z * 8)].indexLong = form_fx_pal[i + (z * 8)].indexLong - MAX_INDEX_LONG;

}

void LEDS_run_fx1_glitter(uint8_t z, uint8_t i )
{
 	uint8_t glitt_val = form_fx_glitter[i + (z * 8)].value;
	 
	 if (bitRead(form_menu_glitter[z][_M_FORM_GLITTER_FFT], i) == true)   glitt_val = LEDS_data_or_fftbin(form_fx_glitter[i + (z * 8)].value);

	if (form_fx_glitter[i + (z * 8)].pal  != 250)   // 250 = from FFT,  other = from Pallete
	{



			tpm_fx.AddGlitter(led_FX_out,  ColorFromPalette(LEDS_pal_get(form_fx_glitter[i + (z * 8)].pal)  , random8(), form_fx_glitter[i + (z * 8)].level , LINEARBLEND)  ,  glitt_val  , form_cfg[i + (z * 8)].start_led, form_cfg[i + (z * 8)].nr_leds);	
			
	}

	else
			tpm_fx.AddGlitter(led_FX_out ,GlobalColor_result , glitt_val , form_cfg[i + (z * 8)].start_led, form_cfg[i + (z * 8)].nr_leds);

}

void LEDS_run_fx1_fade(uint8_t z, uint8_t i )
{
	 tpm_fx.fadeLedArray(led_FX_out, form_cfg[i + (z * 8)].start_led, form_cfg[i + (z * 8)].nr_leds, form_fx1[i + (z * 8)].fade);  

}

void LEDS_run_fx1_dot(uint8_t z, uint8_t i )
{
	CRGB dotcolor = CRGB::Black; 
	
	if (form_fx_dots[i + (z * 8)].pal < 250) 
	{
		//dotcolor =  ColorFromPalette(LEDS_pal_get(form_fx_dots[i + (z * 8)].pal)  , random8(),255, LINEARBLEND) ;
		dotcolor = tpm_fx.PalGetFromLongPal(LEDS_pal_get(form_fx_dots[i + (z * 8)].pal ),form_fx_dots[i + (z * 8)].indexLong,form_fx_dots[i + (z * 8)].level,TBlendType(LINEARBLEND) );
		//debugMe(String(dotcolor[0]) + "  " + String(dotcolor[1])   + " - " + String(dotcolor[2]) );
	}
	else if (form_fx_dots[i + (z * 8)].pal == 250) // 250 = fft
		dotcolor = GlobalColor_result;
	//else { debugMe("no dot color pal : ", false);   debugMe(form_fx_dots[i + (z * 8)].pal);  }
	
	//debugMe(String(dotcolor[0]) + "  " + String(dotcolor[1])   + " - " + String(dotcolor[2]) );
	
	if (bitRead(form_menu_dot[z][_M_FORM_DOT_TYPE], i) == DOT_SINE	)
	{
		if (form_fx_dots[i + (z * 8)].pal <= 250)
			tpm_fx.DotSine(led_FX_out, dotcolor,form_fx_dots[i + (z * 8)].nr_dots, form_cfg[i + (z * 8)].start_led, form_cfg[i + (z * 8)].nr_leds, form_fx_dots[i + (z * 8)].speed, form_fx_dots[i + (z * 8)].level); 
		else if (form_fx_dots[i + (z * 8)].pal == 251) // Hue Dot
			tpm_fx.DotSine(led_FX_out, led_cfg.hue,form_fx_dots[i + (z * 8)].nr_dots, form_cfg[i + (z * 8)].start_led, form_cfg[i + (z * 8)].nr_leds, form_fx_dots[i + (z * 8)].speed, form_fx_dots[i + (z * 8)].level, 255);
	}
	else   // its a saw DOT
	{ 

		if (form_fx_dots[i + (z * 8)].pal <= 250)
			tpm_fx.DotSaw(led_FX_out,  dotcolor,form_fx_dots[i + (z * 8)].nr_dots, form_cfg[i + (z * 8)].start_led, form_cfg[i + (z * 8)].nr_leds, form_fx_dots[i + (z * 8)].speed, form_fx_dots[i + (z * 8)].level); 
		else if (form_fx_dots[i + (z * 8)].pal == 251) // Hue Dot
			tpm_fx.DotSaw(led_FX_out, led_cfg.hue,form_fx_dots[i + (z * 8)].nr_dots, form_cfg[i + (z * 8)].start_led, form_cfg[i + (z * 8)].nr_leds, form_fx_dots[i + (z * 8)].speed, form_fx_dots[i + (z * 8)].level, 255); 
	}
	form_fx_dots[i ].indexLong  = form_fx_dots[i ].indexLong + form_fx_dots[i ].index_add ;
	//if (MAX_INDEX_LONG <= form_fx_dots[i ].indexLong) form_fx_dots[i ].indexLong = form_fx_dots[i ].indexLong - MAX_INDEX_LONG;


}

void LEDS_run_fire(uint8_t z, uint8_t i )
{
 	uint8_t spk_val = LEDS_data_or_fftbin( form_fx_fire[i + (z * 8)].sparking);
	uint8_t cool_val = LEDS_data_or_fftbin( form_fx_fire[i + (z * 8)].cooling);

	tpm_fx.Fire2012WithPalette(tmp_array, heat, LEDS_pal_get(form_fx_fire[i + (z * 8)].pal),  form_cfg[i + (z * 8)].start_led, form_cfg[i + (z * 8)].nr_leds, 255 , cool_val ,spk_val , MixModeType(MIX_REPLACE)  );
	tpm_fx.mixOntoLedArray(tmp_array, leds, form_cfg[i + (z * 8)].nr_leds , form_cfg[i + (z * 8)].start_led,  bitRead(form_menu_fire[z][_M_FORM_FIRE_REVERSED], i), bitRead(form_menu_fire[z][_M_FORM_FIRE_MIRROR], i)   , MixModeType(form_fx_fire[i + (z * 8)].mix_mode), form_fx_fire[i + (z * 8)].level , false );


}

void LEDS_run_shimmer(uint8_t z, uint8_t i )
{
 	uint8_t beater_val = LEDS_data_or_fftbin( form_fx_shim[i + (z * 8)].beater);
	form_fx_shim[i + (z * 8)].dist =  tpm_fx.Shimmer(leds,  LEDS_pal_get(form_fx_shim[i + (z * 8)].pal) , form_cfg[i + (z * 8)].start_led, form_cfg[i + (z * 8)].nr_leds, form_fx_shim[i + (z * 8)].dist, form_fx_shim[i + (z * 8)].xscale, form_fx_shim[i + (z * 8)].yscale, beater_val ,  MixModeType(form_fx_shim[i + (z * 8)].mix_mode), form_fx_shim[i + (z * 8)].level,  TBlendType(bitRead(form_menu_shimmer[z][_M_FORM_SHIMMER_BLEND], i) ) );


}

void LEDS_run_layers()
{

/*	{
  "None": 0,
  "FFT Form": 1,
  "FFT Strip": 2, removed
  "Pal Form": 3,
  "Pal Strip": 4,
  "FX1": 5,
  "FX: Fire": 6,
  "FX: Shimmer": 7
*/

			for ( uint8_t layer = 0 ; layer < MAX_LAYERS_SELECT ; layer++ )
			{
				
				if(layer_select[layer] != 0 && layer_select[layer] <= MAX_LAYERS )
				{
					yield();
					
					switch(layer_select[layer])	
					{
						case 1: 
						for (byte z = 0; z < 2; z++) 
							for (byte i = 0; i < 8; i++) 
								if (bitRead(form_menu_fft[z][_M_FORM_FFT_RUN], i) == true )  LEDS_run_fft(z,i);
						break;     
						case 2: 
							for (byte z = 0; z < 2; z++)
								{
									for (byte i = 0; i < 8; i++)
									{
										
										if ((form_cfg[i + (z  * 8)].nr_leds != 0) && (bitRead(form_menu_pal[z][_M_FORM_PAL_RUN], i) == true)) 
										{
											LEDS_run_pal(z,i);
										}
										
									}
								}
						break;   

						case 3: 
						
							for (byte z = 0; z < 2 ; z++)
							{
								
								for (byte i = 0; i < 8; i++)
								{

									if ((form_cfg[i + (z  * 8)].nr_leds != 0) && (form_fx1[i + (z * 8)].fade != 0 ) ) LEDS_run_fx1_fade(z,i);
									



									if ((form_cfg[i + (z  * 8)].nr_leds != 0) && (bitRead(form_menu_fx1[z][_M_FORM_FX1_RUN], i) == true)) 
									//if(bitRead(form_menu_fx1[z][_M_FORM_FX1_RUN], i ) == true)
									{
										//debugMe("IN2");
										
											
								
											
											if 	(bitRead(form_menu_glitter[z][_M_FORM_GLITTER_RUN], i) == true)       
											 { 
												 LEDS_run_fx1_glitter( z,  i );
											}	

											
											
											if  (bitRead(form_menu_dot[z][_M_FORM_DOT_RUN], i) == true)  
											{
												LEDS_run_fx1_dot(z,i);
											}	  
											// Mix FX1 onto output.
											led_cfg.hue++;	
											tpm_fx.mixOntoLedArray(led_FX_out , leds, form_cfg[i + (z * 8)].nr_leds, form_cfg[i + (z * 8)].start_led, bitRead(form_menu_fx1[z][_M_FORM_FX1_REVERSED], i), bitRead(form_menu_fx1[z][_M_FORM_FX1_MIRROR], i),MixModeType(form_fx1[i + (z * 8)].mix_mode),  form_fx1[i + (z * 8)].level  , false );

											
										
									}
								}
							}
						
						
						break;
						case 4: 
							for (byte z = 0; z < 2; z++)
								for (byte i = 0; i < 8; i++)
									
										if ( (bitRead(form_menu_fire[z][_M_FORM_FIRE_RUN], i) == true)	)  
										{ 
											LEDS_run_fire( z,  i );
										}
									
						break;
						case 5: 
							for (byte z = 0; z < 2; z++)
								for (byte i = 0; i < 8; i++)
									

										if (bitRead(form_menu_shimmer[z][_M_FORM_SHIMMER_RUN], i) == true)
										{
											LEDS_run_shimmer(z,i);
										}
									
						break;
						case 6: 
						for (byte z = 2; z < 4; z++) 
							for (byte i = 0; i < 8; i++) 
								if (bitRead(form_menu_fft[z][_M_FORM_FFT_RUN], i) == true )  LEDS_run_fft(z,i);
						break;     
						case 7: 
							for (byte z = 2; z < 4; z++)
								{
									for (byte i = 0; i < 8; i++)
									{
										
										if ((form_cfg[i + (z  * 8)].nr_leds != 0) && (bitRead(form_menu_pal[z][_M_FORM_PAL_RUN], i) == true)) 
										{
											LEDS_run_pal(z,i);
										}
										
									}
								}
						break;   

						case 8: 
						
							for (byte z = 2; z < 4 ; z++)
							{
								
								for (byte i = 0; i < 8; i++)
								{

									if ((form_cfg[i + (z  * 8)].nr_leds != 0) && (form_fx1[i + (z * 8)].fade != 0 ) )  {tpm_fx.fadeLedArray(led_FX_out, form_cfg[i + (z * 8)].start_led, form_cfg[i + (z * 8)].nr_leds, form_fx1[i + (z * 8)].fade);  }
									



									if ((form_cfg[i + (z  * 8)].nr_leds != 0) && (bitRead(form_menu_fx1[z][_M_FORM_FX1_RUN], i) == true)) 
									//if(bitRead(form_menu_fx1[z][_M_FORM_FX1_RUN], i ) == true)
									{
										//debugMe("IN2");
										
											
								
											
											if 	(bitRead(form_menu_glitter[z][_M_FORM_GLITTER_RUN], i) == true)       
											 { 

													LEDS_run_fx1_glitter( z,  i );
											}	

											
											
											if  (bitRead(form_menu_dot[z][_M_FORM_DOT_RUN], i) == true)  
											{
												LEDS_run_fx1_dot(z,i);
						
						
											}
											// Mix FX1 onto output.
											led_cfg.hue++;	
											tpm_fx.mixOntoLedArray(led_FX_out , leds, form_cfg[i + (z * 8)].nr_leds, form_cfg[i + (z * 8)].start_led, bitRead(form_menu_fx1[z][_M_FORM_FX1_REVERSED], i), bitRead(form_menu_fx1[z][_M_FORM_FX1_MIRROR], i),MixModeType(form_fx1[i + (z * 8)].mix_mode),  form_fx1[i + (z * 8)].level  , false );

											
										
									}
								}
							}
						
						
						break;
						case 9: 
							for (byte z = 2; z < 4; z++)
								for (byte i = 0; i < 8; i++)
									
										if ( (bitRead(form_menu_fire[z][_M_FORM_FIRE_RUN], i) == true)	)  
										{ 
											LEDS_run_fire( z,  i );
										}
									
						break;
						case 10: 
							for (byte z = 2; z < 4; z++)
								for (byte i = 0; i < 8; i++)
									

										if (bitRead(form_menu_shimmer[z][_M_FORM_SHIMMER_RUN], i) == true)
											{
												LEDS_run_shimmer(z,i);
											}
									
						break;

					}

				}


			}  //*/


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

			if (LEDS_checkIfAudioSelected()) 
			{
				LEDS_FFT_process();  // Get the color from the FFT data
				LEDS_FFT_history_run(GlobalColor_result);
				
				yield();
			}


			LEDS_run_layers();
			

			for (byte i = 0; i < 8; i++)
			{
				if (bitRead(copy_leds_mode[0], i) == true) LEDS_Copy_strip(copy_leds[i].start_led, copy_leds[i].nr_leds, copy_leds[i].Ref_LED);
				if (bitRead(copy_leds_mode[1], i) == true) LEDS_Copy_strip(copy_leds[i + 8].start_led, copy_leds[i + 8].nr_leds, copy_leds[i + 8].Ref_LED);

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
					LEDS_seqencer_advance();
						

				 }


		 } 

		 if (get_bool(FFT_OSTC_VIZ) && currentT >= fft_led_cfg.update_time ) 
		 {

			fft_led_cfg.update_time = currentT + (1000000 / fft_led_cfg.viz_fps);	 
			 
			 osc_StC_FFT_vizIt(); 
			 //debugMe("vizzit");


		}

		FS_play_conf_loop();


	if (currentT > led_cfg.confSwitch_time && get_bool(SEQUENCER_ON) ) LEDS_seqencer_advance();
	}


	
	


	//debugMe("leds loop end ", false);
	//debugMe(String(xPortGetCoreID()));
}



