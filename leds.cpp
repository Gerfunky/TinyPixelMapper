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
	#include "leds_def_values.h"		// include the default values for led settings

	#include "tools.h" 
	#include "wifi-ota.h"
	#include "config_fs.h"
	#include "tpm_artnet.h"
	#include "osc.h"
	//#include "msgeq7_fft.h"

	//#define FASTLED_ALLOW_INTERRUPTS 0
	#define FASTLED_ESP32_FLASH_LOCK 1
	//#define FASTLED_RMT_MAX_CHANNELS 1
	//#define FASTLED_RMT_BUILTIN_DRIVER
	
	#include <FastLED.h>
	
	#include <RunningAverage.h>			// For Auto FFT
	#include <QueueArray.h>	   			// For buffering incoming FFT packets

	#include <TPM-FX.h>

	tpm_fx tpm_fx;					//load the FX lib

	#define ANALOG_IN_DEVIDER 16 // devide analog in by this value to get into a 0-255 range 


// -- The core to run FastLED.show()
#define FASTLED_SHOW_CORE 1

void LEDS_G_artnet_master_out();
uint8_t LEDS_fft_calc_fxbin_result(uint8_t fxbin);

extern  void osc_StC_FFT_vizIt();			// of open stage controll to send the fft data


// -- Task handles for use in the notifications
static TaskHandle_t FastLEDshowTaskHandle = 0;
static TaskHandle_t userTaskHandle = 0;


// *************** External Functions
// from wifi-ota.cpp

extern artnet_struct artnet_cfg;
extern artnet_node_struct artnetNode[ARTNET_NR_NODES_TPM];


//CRGB GlobalColor_result;


// ***************** the 2 decks
deck_struct deck[1] ;
//save_struct saves[8];

// ***************** the 16 Saves in memory
//deck_cfg_struct *mem_confs[8] ;
//deck_cfg_struct mem_confs[8] ;



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




    uint8_t fft_bin_results[7];




// ********************* LED Setup  FastLed

	CRGBArray<MAX_NUM_LEDS> tmp_array;        // used as a buffer before mirroring reversing.
	


led_controls_struct led_cnt = { 150,30,POT_SENSE_DEF };  // global 


led_cfg_struct led_cfg = { DEF_MAX_BRI ,DEF_MAX_BRI,0, 0, 1,1,1 ,DEF_LED_MODE, NUM_LEDS ,DEF_PLAY_MODE, {DEF_DATA1_START_NR,DEF_DATA2_START_NR, DEF_DATA3_START_NR,  DEF_DATA4_START_NR}, {DEF_DATA1_NR_LEDS, DEF_DATA2_NR_LEDS, DEF_DATA3_NR_LEDS,DEF_DATA4_NR_LEDS }, DEF_APA102_DATARATE, 5 , 0,15};			// The basic led config






uint16_t play_conf_time_min[MAX_NR_SAVES] = {5,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};

uint8_t squencer_bool[2]  = {0,0};		// to hold what saves o play in sequence mode



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

void LEDS_show()
{	
			if(deck[0].cfg.fft_config.fft_menu_bri != 0)
				FastLED.setBrightness(LEDS_get_real_bri() );
			else
				FastLED.setBrightness(deck[0].cfg.led_master_cfg.bri);

			if (get_bool(ARTNET_SEND) == true) 	LEDS_G_artnet_master_out();  // Send out the artnet data if enabled
			else FastLEDshowESP32();
			//FastLED.show();
			//FastLED[0].showLeds(deck[0].led_master_cfg.bri);
			//FastLED[1].showLeds(deck[0].led_master_cfg.bri);
			//FastLED[2].showLeds(deck[0].led_master_cfg.bri);
}

void LEDS_setLED_show(uint8_t ledNr, uint8_t color[3])
{	
	deck[0].run.leds[ledNr].r = color[0];
	deck[0].run.leds[ledNr].g = color[1];
	deck[0].run.leds[ledNr].b = color[2];
	LEDS_show();
}



// ************* FUNCTIONS





	
void LEDS_fadeout()
{
	// make a fadout loop goddamit!
	deck[0].run.leds.fadeToBlackBy(255);
	yield();
	FastLED.show();
	yield();
}

/*
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
*/

void LED_master_rgb(uint16_t Start_led , uint16_t number_of_leds   )
{
		// fade RGB if we are not on full
		if (deck[0].cfg.led_master_cfg.r != 255)
			for (int i = Start_led; i < Start_led + number_of_leds; i++)
				deck[0].run.leds[i].r = deck[0].run.leds[i].r  * deck[0].cfg.led_master_cfg.r / 255;
		if (deck[0].cfg.led_master_cfg.g != 255)
			for (int i = Start_led; i < Start_led + number_of_leds; i++)
				deck[0].run.leds[i].g = deck[0].run.leds[i].g * deck[0].cfg.led_master_cfg.g / 255;
		if (deck[0].cfg.led_master_cfg.b != 255)
			for (int i = Start_led; i < Start_led + number_of_leds; i++)
				deck[0].run.leds[i].b = deck[0].run.leds[i].b * deck[0].cfg.led_master_cfg.b / 255;


}


void LEDS_G_LoadSAveFade(boolean Save, uint8_t confNr)
{
	write_bool(FADE_INOUT, true);
	write_bool(FADE_INOUT_FADEBACK, false);
	write_bool(FADE_INOUT_SAVE, Save);
	led_cfg.fade_inout_val = 0;
	led_cfg.next_config_loadsave = confNr;
}

void LEDS_FX1_increment_indexes(uint8_t DeckNo)
{

	for (uint8_t z=0 ; z< NR_FX_BYTES ; z++)
	{
		if (deck[DeckNo].fx1_cfg.form_menu_strobe[z] != 0)
		{
				deck[DeckNo].run.form_fx_strobe[z].frame_pos++;
			if (deck[DeckNo].run.form_fx_strobe[z].frame_pos >= deck[DeckNo].fx1_cfg.form_fx_strobe_bytes[z].on_frames + deck[DeckNo].fx1_cfg.form_fx_strobe_bytes[z].off_frames) 
				deck[DeckNo].run.form_fx_strobe[z].frame_pos = 0;
		}
	}

}



void LEDS_G_pre_show_processing()
{	// the leds pre show prcessing 
	// run the effects and set the brightness.

	LEDS_FX1_increment_indexes(0);

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

	




	if(!get_bool(POT_DISABLE) || get_bool(POTS_LVL_MASTER))
	{
		//uint8_t bri = led_cfg.max_bri * deck[0].led_master_cfg.bri / 255;
		uint8_t bri = analogRead(POTI_BRI_PIN) / ANALOG_IN_DEVIDER;
		if (bri > led_cnt.PotBriLast + led_cnt.PotSens || bri < led_cnt.PotBriLast - led_cnt.PotSens)
		{
			deck[0].cfg.led_master_cfg.bri = map(bri, 0, 255, 0, led_cfg.max_bri);
			led_cnt.PotBriLast = bri;
		}

		//FastLED.setBrightness(deck[0].led_master_cfg.bri);  moved to show
		
		//debugMe(deck[0].led_master_cfg.bri);
		
		
		
		uint8_t fps = analogRead(POTI_FPS_PIN) / ANALOG_IN_DEVIDER;
		
		//deck[0].led_master_cfg.pal_fps = fps /4;
		///*
		if (fps > led_cnt.PotFPSLast + led_cnt.PotSens || fps < led_cnt.PotFPSLast - led_cnt.PotSens)
		{
			deck[0].cfg.led_master_cfg.pal_fps = map(fps, 0, 255, 1, MAX_PAL_FPS);   //*/
			led_cnt.PotFPSLast = fps;
		}
	//Serial.println(fps);  
	}
	
	//LED_G_bit_run();
		//= led_cfg.max_br * deck[0].led_master_cfg.bri / 255
}


boolean LEDS_checkIfAudioSelected()
{	// check if there are audi strips if so return true
	//for (byte zp = 0; zp < _M_NR_STRIP_BYTES_; zp++) if (strip_menu[zp][_M_AUDIO_] != 0)   return true;
	for (byte zf = 0; zf < _M_NR_FORM_BYTES_; zf++)  if ((deck[0].cfg.form_menu_fft[zf][_M_FORM_FFT_RUN] != 0) ) return true;
	if(deck[0].cfg.fft_config.fft_menu_bri != 0) return true;
	if(deck[0].cfg.fft_config.fft_fxbin[0].menu_select != 0) return true;
	if(deck[0].cfg.fft_config.fft_fxbin[1].menu_select != 0) return true;
	if(deck[0].cfg.fft_config.fft_fxbin[2].menu_select != 0) return true;
	if(deck[0].cfg.fft_config.fft_menu_fps != 0) return true;
	
	return false;

}


// **************************Pallets **************
void LEDS_pal_load(CRGBPalette16* palref, uint8_t pal_no, uint8_t pal_menu)
{
		
//	deck_struct localDEckCopy;
//	localDEckCopy = *deckref;
	//*deckref = localDEckCopy;


	// load a pallete from the default (FastLed)
	//debugMe("Load pal" + String(pal_menu));
	if (pal_no < NR_PALETTS && pal_menu < NR_PALETTS_SELECT + 1 )
	switch (pal_menu)
	{
	case 0: *palref = deck[0].cfg.LEDS_pal_cur[0]; break;
	case 1: *palref = deck[0].cfg.LEDS_pal_cur[1]; break;
	case 2: *palref = deck[0].cfg.LEDS_pal_cur[2]; break;
	case 3: *palref = deck[0].cfg.LEDS_pal_cur[3]; break;
	case 4: *palref = deck[0].cfg.LEDS_pal_cur[4]; break;
	case 5: *palref = deck[0].cfg.LEDS_pal_cur[5]; break;
	case 6: *palref = deck[0].cfg.LEDS_pal_cur[6]; break;
	case 7: *palref = deck[0].cfg.LEDS_pal_cur[7]; break;
/*	case 8: *palref = deck[0].cfg.LEDS_pal_cur[8]; break;
	case 9: *palref = deck[0].cfg.LEDS_pal_cur[9]; break;
	case 10: *palref = deck[0].cfg.LEDS_pal_cur[10]; break;
	case 11: *palref = deck[0].cfg.LEDS_pal_cur[11]; break;
	case 12: *palref = deck[0].cfg.LEDS_pal_cur[12]; break;
	case 13: *palref = deck[0].cfg.LEDS_pal_cur[13]; break;
	case 14: *palref = deck[0].cfg.LEDS_pal_cur[14]; break;
	case 15: *palref = deck[0].cfg.LEDS_pal_cur[15]; break;
*/
	case 19: for (int i = 0; i < 16; i++) { *palref[i] = CHSV(random8(), 255, random8());} break;
	case 20: *palref = RainbowColors_p; break;
	case 21: *palref = RainbowStripeColors_p; break;
	case 22: *palref = CloudColors_p; break;
	case 23: *palref = PartyColors_p; break;
	case 24: *palref = OceanColors_p; break;
	case 25: *palref = ForestColors_p; break;
	case 26: *palref = HeatColors_p; break;
	case 27: *palref = LavaColors_p; break;
	case 28: *palref = pal_red_green; break;
	case 29: *palref = pal_red_blue; break;
	case 30: *palref = pal_green_blue; break;
	case 31: *palref = pal_black_white_Narrow; break;
	case 32: *palref = pal_black_white_wide; break;
	
	
	default: *palref = RainbowColors_p; break;
		
	//*deckref = localDEckCopy;
	}

}

void LEDS_pal_load(deck_struct* deckref, uint8_t pal_no, uint8_t pal_menu)
{
// NOK not loading!
	deck_struct localDEckCopy;
	localDEckCopy = *deckref;
	//*deckref = localDEckCopy;


	// load a pallete from the default (FastLed)
	//debugMe("Load pal" + String(pal_menu));
	if (pal_no < NR_PALETTS && pal_menu < NR_PALETTS_SELECT + 1 )
	switch (pal_menu)
	{
	case 0: localDEckCopy.cfg.LEDS_pal_cur[pal_no] = localDEckCopy.cfg.LEDS_pal_cur[0]; break;
	case 1: localDEckCopy.cfg.LEDS_pal_cur[pal_no] = localDEckCopy.cfg.LEDS_pal_cur[1]; break;
	case 2: localDEckCopy.cfg.LEDS_pal_cur[pal_no] = localDEckCopy.cfg.LEDS_pal_cur[2]; break;
	case 3: localDEckCopy.cfg.LEDS_pal_cur[pal_no] = localDEckCopy.cfg.LEDS_pal_cur[3]; break;
	case 4: localDEckCopy.cfg.LEDS_pal_cur[pal_no] = localDEckCopy.cfg.LEDS_pal_cur[4]; break;
	case 5: localDEckCopy.cfg.LEDS_pal_cur[pal_no] = localDEckCopy.cfg.LEDS_pal_cur[5]; break;
	case 6: localDEckCopy.cfg.LEDS_pal_cur[pal_no] = localDEckCopy.cfg.LEDS_pal_cur[6]; break;
	case 7: localDEckCopy.cfg.LEDS_pal_cur[pal_no] = localDEckCopy.cfg.LEDS_pal_cur[7]; break;
/*	case 8: localDEckCopy.cfg.LEDS_pal_cur[pal_no] = localDEckCopy.cfg.LEDS_pal_cur[8]; break;
	case 9: localDEckCopy.cfg.LEDS_pal_cur[pal_no] = localDEckCopy.cfg.LEDS_pal_cur[9]; break;
	case 10: localDEckCopy.cfg.LEDS_pal_cur[pal_no] = localDEckCopy.cfg.LEDS_pal_cur[10]; break;
	case 11: localDEckCopy.cfg.LEDS_pal_cur[pal_no] = localDEckCopy.cfg.LEDS_pal_cur[11]; break;
	case 12: localDEckCopy.cfg.LEDS_pal_cur[pal_no] = localDEckCopy.cfg.LEDS_pal_cur[12]; break;
	case 13: localDEckCopy.cfg.LEDS_pal_cur[pal_no] = localDEckCopy.cfg.LEDS_pal_cur[13]; break;
	case 14: localDEckCopy.cfg.LEDS_pal_cur[pal_no] = localDEckCopy.cfg.LEDS_pal_cur[14]; break;
	case 15: localDEckCopy.cfg.LEDS_pal_cur[pal_no] = localDEckCopy.cfg.LEDS_pal_cur[15]; break;
*/
	case 19: for (int i = 0; i < 16; i++) {  localDEckCopy.cfg.LEDS_pal_cur[pal_no][i] = CHSV(random8(), 255, random8());} break;
	case 20: localDEckCopy.cfg.LEDS_pal_cur[pal_no] = RainbowColors_p; break;
	case 21: localDEckCopy.cfg.LEDS_pal_cur[pal_no] = RainbowStripeColors_p; break;
	case 22: localDEckCopy.cfg.LEDS_pal_cur[pal_no] = CloudColors_p; break;
	case 23: localDEckCopy.cfg.LEDS_pal_cur[pal_no] = PartyColors_p; break;
	case 24: localDEckCopy.cfg.LEDS_pal_cur[pal_no] = OceanColors_p; break;
	case 25: localDEckCopy.cfg.LEDS_pal_cur[pal_no] = ForestColors_p; break;
	case 26: localDEckCopy.cfg.LEDS_pal_cur[pal_no] = HeatColors_p; break;
	case 27: localDEckCopy.cfg.LEDS_pal_cur[pal_no] = LavaColors_p; break;
	case 28: localDEckCopy.cfg.LEDS_pal_cur[pal_no] = pal_red_green; break;
	case 29: localDEckCopy.cfg.LEDS_pal_cur[pal_no] = pal_red_blue; break;
	case 30: localDEckCopy.cfg.LEDS_pal_cur[pal_no] = pal_green_blue; break;
	case 31: localDEckCopy.cfg.LEDS_pal_cur[pal_no] = pal_black_white_Narrow; break;
	case 32: localDEckCopy.cfg.LEDS_pal_cur[pal_no] = pal_black_white_wide; break;
	
	
	default: localDEckCopy.cfg.LEDS_pal_cur[pal_no] = RainbowColors_p; break;
		
	*deckref = localDEckCopy;
	}

}

void LEDS_pal_reset_index() 
{	// reset all the pallete indexes
	// Should only be stuff in the RUN Struct.
	for (int z = 0; z < _M_NR_FORM_BYTES_; z++) 
	{

		for (int i = 0; i < 8; i++) {

			deck[0].run.form_fx_pal[i+(z * 8)].index = constrain(deck[0].cfg.form_fx_pal[i+ (z * 8)].index_start,0,255);
			deck[0].run.form_fx_pal[i + (z * 8)].indexLong = deck[0].cfg.form_fx_pal[i + (z * 8)].index_start;
			deck[0].run.form_fx_modify[i+(z * 8)].RotateFramePos = 0;
			deck[0].run.form_fx_fft[i+(z * 8)].extend_tick = 0;
			//debugMe(String(i + (z * 8) ) + " -- " + String(deck[0].form_fx_pal[i + (z * 8)].indexLong));
			if (deck[0].run.form_fx_pal[i + (z * 8)].indexLong  >= 4096) deck[0].run.form_fx_pal[i + (z * 8)].indexLong  = deck[0].run.form_fx_pal[i + (z * 8)].indexLong -4096;
		}
		
	}

	//for (int z = 0; z < NR_FX_BYTES; z++) 

		
	
	
}

void LEDS_PAL_invert(uint8_t pal = 0)
{

		for(int pal_pos = 0; pal_pos < 16; pal_pos++)
		{
		deck[0].cfg.LEDS_pal_cur[pal][pal_pos].r = qsub8(255, deck[0].cfg.LEDS_pal_cur[pal][pal_pos].r );
		deck[0].cfg.LEDS_pal_cur[pal][pal_pos].g = qsub8(255, deck[0].cfg.LEDS_pal_cur[pal][pal_pos].g );
		deck[0].cfg.LEDS_pal_cur[pal][pal_pos].b = qsub8(255, deck[0].cfg.LEDS_pal_cur[pal][pal_pos].b );
		}

}

void LEDS_pal_write(uint8_t pal, uint8_t no, uint8_t color , uint8_t value)
{
	// write incoming color information into a pallete entry
	switch (color)
	{
		case 0:
			deck[0].cfg.LEDS_pal_cur[pal][no].r = value;
		break;
		case 1:
			deck[0].cfg.LEDS_pal_cur[pal][no].g = value;
		break;
		case 2:
			deck[0].cfg.LEDS_pal_cur[pal][no].b = value;
		break;


	}

}

void LEDS_pal_write(CRGBPalette16* palref, uint8_t pal, uint8_t no, uint8_t color , uint8_t value)
{
	CRGBPalette16 LocalPalCopy;
	LocalPalCopy = *palref;

	// write incoming color information into a pallete entry
	switch (color)
	{
		case 0:
			LocalPalCopy[no].r = value;
		break;
		case 1:
			LocalPalCopy[no].g = value;
		break;
		case 2:
			LocalPalCopy[no].b = value;
		break;


	}

	*palref = LocalPalCopy;

}

uint8_t LEDS_pal_read(uint8_t pal, uint8_t no, uint8_t color)
{	// read the color info for 1 color in a pallete

	if (pal < NR_PALETTS)
	{
		switch(color)
		{
			case 0:
				return deck[0].cfg.LEDS_pal_cur[pal][no].r;
			break;
			case 1:
				return deck[0].cfg.LEDS_pal_cur[pal][no].g;
			break;
			case 2:
				return deck[0].cfg.LEDS_pal_cur[pal][no].b;
			break;

		
		}
		return 0;
	}
	if (pal >=NR_PALETTS && pal <= 32)
	{
		CRGBPalette16 TempPal; 	

		switch (pal)
		{
			case 16: TempPal = RainbowColors_p; break;
			case 17: TempPal = RainbowStripeColors_p; break;
			case 18: TempPal = CloudColors_p; break;
			case 19: TempPal = PartyColors_p; break;
			case 20: TempPal = OceanColors_p; break;
			case 21: TempPal = ForestColors_p; break;
			case 22: TempPal = HeatColors_p; break;
			case 23: TempPal = LavaColors_p; break;
			case 24: TempPal = pal_red_green; break;
			case 25: TempPal = pal_red_blue; break;
			case 26: TempPal = pal_green_blue; break;
			case 27: TempPal = pal_black_white_Narrow; break;
			case 28: TempPal = pal_black_white_wide; break;
		}

		switch(color)
		{
			case 0:
				return TempPal[no].r;
			break;
			case 1:
				return TempPal[no].g;
			break;
			case 2:
				return TempPal[no].b;
			break;

		
		}
		return 0;


	}

	return 0;	

}

uint8_t LEDS_pal_read(CRGBPalette16* palref, uint8_t pal, uint8_t no, uint8_t color)
{	// read the color info for 1 color in a pallete

	CRGBPalette16 LocalPalCopy;
	LocalPalCopy = *palref;

	if (pal < NR_PALETTS)
	{
		switch(color)
		{
			case 0:
				return LocalPalCopy[no].r;
			break;
			case 1:
				return LocalPalCopy[no].g;
			break;
			case 2:
				return LocalPalCopy[no].b;
			break;

		
		}
		return 0;
	}
	if (pal >=NR_PALETTS && pal <= 32)
	{
		CRGBPalette16 TempPal; 	

		switch (pal)
		{
			case 16: TempPal = RainbowColors_p; break;
			case 17: TempPal = RainbowStripeColors_p; break;
			case 18: TempPal = CloudColors_p; break;
			case 19: TempPal = PartyColors_p; break;
			case 20: TempPal = OceanColors_p; break;
			case 21: TempPal = ForestColors_p; break;
			case 22: TempPal = HeatColors_p; break;
			case 23: TempPal = LavaColors_p; break;
			case 24: TempPal = pal_red_green; break;
			case 25: TempPal = pal_red_blue; break;
			case 26: TempPal = pal_green_blue; break;
			case 27: TempPal = pal_black_white_Narrow; break;
			case 28: TempPal = pal_black_white_wide; break;
		}

		switch(color)
		{
			case 0:
				return TempPal[no].r;
			break;
			case 1:
				return TempPal[no].g;
			break;
			case 2:
				return TempPal[no].b;
			break;

		
		}
		return 0;


	}

	return 0;

}



// ****************************** ARTNET 

void LEDS_G_artnet_send_universe(uint8_t node_Nr,uint8_t universe, uint16_t in_pixel , uint8_t nr_pixels = 170)
{

	// Set the out universe and IP
	ARTNET_set_node( node_Nr, artnetNode[node_Nr].startU + universe  ); 

	for (uint16_t set_pixel = 0; set_pixel < nr_pixels; set_pixel++)
	{
		ARNET_set_pixel( set_pixel,  scale8(deck[0].run.leds[in_pixel].r ,deck[0].cfg.led_master_cfg.bri ) ,  scale8(deck[0].run.leds[in_pixel].g,deck[0].cfg.led_master_cfg.bri ) ,  scale8(deck[0].run.leds[in_pixel].b, deck[0].cfg.led_master_cfg.bri));
		//ARNET_set_pixel( set_pixel,  255  ,  125 ,  10 );
		in_pixel++;
	}

	// Send out the Artnet Frame
	ARTNET_send_node(node_Nr);


 

}

void LEDS_G_artnet_master_out()
{
	uint16_t pixel = 0;
	uint8_t  universeCounter = 0;


		for (uint8_t nodeNR = 0; nodeNR < ARTNET_NR_NODES_TPM; nodeNR++  )
		{
			for (uint8_t setUni = 0; setUni < artnetNode[nodeNR].numU; setUni++ )	
			{
				
				LEDS_G_artnet_send_universe(nodeNR,setUni, pixel ,  170  )		;
				universeCounter++;
				pixel = universeCounter * 170;
				
				//pixel = universeCounter * 170;
			}

		}


}


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
				deck[0].run.leds[led].r = data[i * 3];
				deck[0].run.leds[led].g = data[i * 3 + 1];
				deck[0].run.leds[led].b = data[i * 3 + 2];
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


void LEDS_FFT_auto()
{	// automatically calculate the trigger value and set it
	uint8_t DeckNo = 0;
	if (FFT_stage1_sample_count >= deck[DeckNo].cfg.led_master_cfg.pal_fps)				// trigger on the FPS so that we get one stage 2 sammple a second
	{
		fft_bin0stage2.addValue(deck[DeckNo].run.fft_data[0].avarage);
		fft_bin1stage2.addValue(deck[DeckNo].run.fft_data[1].avarage);
		fft_bin2stage2.addValue(deck[DeckNo].run.fft_data[2].avarage);
		fft_bin3stage2.addValue(deck[DeckNo].run.fft_data[3].avarage);
		fft_bin4stage2.addValue(deck[DeckNo].run.fft_data[4].avarage);
		fft_bin5stage2.addValue(deck[DeckNo].run.fft_data[5].avarage);
		fft_bin6stage2.addValue(deck[DeckNo].run.fft_data[6].avarage);
		
		
		if (bitRead(deck[DeckNo].cfg.fft_config.fft_bin_autoTrigger, 0)) deck[DeckNo].cfg.fft_config.trigger[0] = constrain((fft_bin0stage2.getFastAverage() + fft_bin0stage2.GetMaxInBuffer()) / 2, deck[0].cfg.fft_config.fftAutoMin, deck[0].cfg.fft_config.fftAutoMax);
		if (bitRead(deck[DeckNo].cfg.fft_config.fft_bin_autoTrigger, 1)) deck[DeckNo].cfg.fft_config.trigger[1] = constrain((fft_bin1stage2.getFastAverage() + fft_bin1stage2.GetMaxInBuffer()) / 2, deck[0].cfg.fft_config.fftAutoMin, deck[0].cfg.fft_config.fftAutoMax);
		if (bitRead(deck[DeckNo].cfg.fft_config.fft_bin_autoTrigger, 2)) deck[DeckNo].cfg.fft_config.trigger[2] = constrain((fft_bin2stage2.getFastAverage() + fft_bin2stage2.GetMaxInBuffer()) / 2, deck[0].cfg.fft_config.fftAutoMin, deck[0].cfg.fft_config.fftAutoMax);
		if (bitRead(deck[DeckNo].cfg.fft_config.fft_bin_autoTrigger, 3)) deck[DeckNo].cfg.fft_config.trigger[3] = constrain((fft_bin3stage2.getFastAverage() + fft_bin3stage2.GetMaxInBuffer()) / 2, deck[0].cfg.fft_config.fftAutoMin, deck[0].cfg.fft_config.fftAutoMax);
		if (bitRead(deck[DeckNo].cfg.fft_config.fft_bin_autoTrigger, 4)) deck[DeckNo].cfg.fft_config.trigger[4] = constrain((fft_bin4stage2.getFastAverage() + fft_bin4stage2.GetMaxInBuffer()) / 2, deck[0].cfg.fft_config.fftAutoMin, deck[0].cfg.fft_config.fftAutoMax);
		if (bitRead(deck[DeckNo].cfg.fft_config.fft_bin_autoTrigger, 5)) deck[DeckNo].cfg.fft_config.trigger[5] = constrain((fft_bin5stage2.getFastAverage() + fft_bin5stage2.GetMaxInBuffer()) / 2, deck[0].cfg.fft_config.fftAutoMin, deck[0].cfg.fft_config.fftAutoMax);
		if (bitRead(deck[DeckNo].cfg.fft_config.fft_bin_autoTrigger, 6)) deck[DeckNo].cfg.fft_config.trigger[6] = constrain((fft_bin6stage2.getFastAverage() + fft_bin6stage2.GetMaxInBuffer()) / 2, deck[0].cfg.fft_config.fftAutoMin, deck[0].cfg.fft_config.fftAutoMax);
		//fft_data[7].trigger = fft_bin7stage2.getFastAverage();
		// debugMe("max bin 0" + String(fft_bin0stage2.GetMaxInBuffer()));

		FFT_stage1_sample_count = 0;
	}


}
void LEDS_FFT_calc_avarage()
{	// automatically calculate the average fft values
	uint8_t deckNo = 0;
	fft_bin0.addValue(fft_bin_results[0]);
	fft_bin1.addValue(fft_bin_results[1]);
	fft_bin2.addValue(fft_bin_results[2]);
	fft_bin3.addValue(fft_bin_results[3]);
	fft_bin4.addValue(fft_bin_results[4]);
	fft_bin5.addValue(fft_bin_results[5]);
	fft_bin6.addValue(fft_bin_results[6]);



	deck[deckNo].run.fft_data[0].avarage = fft_bin0.getFastAverage();
	deck[deckNo].run.fft_data[1].avarage = fft_bin1.getFastAverage();
	deck[deckNo].run.fft_data[2].avarage = fft_bin2.getFastAverage();
	deck[deckNo].run.fft_data[3].avarage = fft_bin3.getFastAverage();
	deck[deckNo].run.fft_data[4].avarage = fft_bin4.getFastAverage();
	deck[deckNo].run.fft_data[5].avarage = fft_bin5.getFastAverage();
	deck[deckNo].run.fft_data[6].avarage = fft_bin6.getFastAverage();


	deck[deckNo].run.fft_data[0].max = fft_bin0.GetMaxInBuffer();
	deck[deckNo].run.fft_data[1].max = fft_bin1.GetMaxInBuffer();
	deck[deckNo].run.fft_data[2].max = fft_bin2.GetMaxInBuffer();
	deck[deckNo].run.fft_data[3].max = fft_bin3.GetMaxInBuffer();
	deck[deckNo].run.fft_data[4].max = fft_bin4.GetMaxInBuffer();
	deck[deckNo].run.fft_data[5].max = fft_bin5.GetMaxInBuffer();
	deck[deckNo].run.fft_data[6].max = fft_bin6.GetMaxInBuffer();
	
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
		fft_bin_results[i] = analogRead(MSGEQ7_INPUT_PIN) / ANALOG_IN_DEVIDER;   // 
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


void LEDS_FFT_process()
{	// process the fft data and genereat a color

	CRGB color_result = (CRGB::Black);

	int bins[7] = {0,0,0,0,0,0,0};

	LEDS_MSGEQ7_get();  // get the FFT data and put it in fft_data[i].value
	LEDS_FFT_calc_avarage(); // update the avarages for autofft.

	// debugMe("FFT fill bins");
	for (uint8_t deckNo = 0; deckNo < 1; deckNo++ )
	{

		deck[deckNo].run.fft.fft_color_result_bri = 0;
		deck[deckNo].run.fft.fft_color_fps = 0;

		for( uint8_t z = 0; z < FFT_FX_NR_OF_BINS ; z++)
			deck[deckNo].run.fft.fft_fxbin[z].sum = 0;

		
		for (byte i = 0; i < 7; i++) 
		{
			bins[i] = constrain((fft_bin_results[i]) - deck[deckNo].cfg.fft_config.trigger[i] , 0, 255);
			if (bitRead(deck[deckNo].cfg.fft_config.fft_menu[0], i) == true) color_result.r = constrain((color_result.r + bins[i]), 0, 255);
			if (bitRead(deck[deckNo].cfg.fft_config.fft_menu[1], i) == true) color_result.g = constrain((color_result.g + bins[i]), 0, 255);
			if (bitRead(deck[deckNo].cfg.fft_config.fft_menu[2], i) == true) color_result.b = constrain((color_result.b + bins[i]), 0, 255);

			if (bitRead(deck[deckNo].cfg.fft_config.fft_menu_bri, i) == true) deck[deckNo].run.fft.fft_color_result_bri 	= constrain((deck[deckNo].run.fft.fft_color_result_bri + bins[i]), 0, 255);
			if (bitRead(deck[deckNo].cfg.fft_config.fft_menu_fps, i) == true) deck[deckNo].run.fft.fft_color_fps 			= constrain((deck[deckNo].run.fft.fft_color_fps + bins[i]), 0, 255);


			for( uint8_t z = 0; z < FFT_FX_NR_OF_BINS ; z++)
			{
				if (bitRead(deck[deckNo].cfg.fft_config.fft_fxbin[z].menu_select, i) == true) deck[deckNo].run.fft.fft_fxbin[z].sum = constrain((deck[deckNo].run.fft.fft_fxbin[z].sum + bins[i]), 0, 255);

			}	
					
			//if (bitRead(fft_data_menu[1], i) == true) fft_color_result_data[1] = constrain((fft_color_result_data[1] + bins[i]), 0, 255);
			//if (bitRead(fft_data_menu[2], i) == true) fft_color_result_data[2] = constrain((fft_color_result_data[2] + bins[i]), 0, 255);

		}
		for( uint8_t z = 0; z < FFT_FX_NR_OF_BINS ; z++)  if ( deck[deckNo].cfg.fft_config.fft_fxbin[z].menu_select != 0)  deck[deckNo].run.fft.fft_fxbin[z].result  =   LEDS_fft_calc_fxbin_result(z);

		//debugMe(fft_color_result_data[1]);	
		//debugMe(fft_data_menu[0], false);
		//debugMe("..");
		// fade the RGB 

		/*
		if (deck[0].led_master_cfg.r != 255) color_result.r = color_result.r * deck[0].led_master_cfg.r / 255 ;
		if (deck[0].led_master_cfg.g != 255) color_result.g = color_result.g * deck[0].led_master_cfg.g / 255 ;
		if (deck[0].led_master_cfg.b != 255) color_result.b = color_result.b * deck[0].led_master_cfg.b / 255 ;
		*/
		/*
		if (0 != fft_led_cfg.Scale)
		{
			color_result.r = constrain((color_result.r + (fft_led_cfg.Scale * color_result.r / 100)),0,255);
			color_result.g = constrain((color_result.g + (fft_led_cfg.Scale * color_result.g / 100)),0,255);
			color_result.b = constrain((color_result.b + (fft_led_cfg.Scale * color_result.b / 100)),0,255);
		}
		*/
		// debugMe("FFT pre return color result from bins");
		//color_result = constrain((color_result + (fft_led_cfg.Scale * color_result / 100)),0,255);
		
		deck[deckNo].run.fft.GlobalColor_result = color_result;
		// debugMe(deck[deckNo].run.fft.GlobalColor_result[1]);
	}

//	return color_result;

}




void LEDS_FFT_history_run(CRGB color_result, uint8_t deckNo )
{	// only move up to max leds from mixed mode.
	//uint8_t deckNo = 0;
	for (int i = led_cfg.NrLeds -1  ; i > 0  ; i--) 		
		{
					deck[deckNo].run.leds_FFT_history[i] = deck[deckNo].run.leds_FFT_history[i - 1];	
					
		}

	deck[deckNo].run.leds_FFT_history[0] = color_result;
	//for (int i = 0 ; i < 3 ; i++)	
	//debugMe(String(leds_FFT_history[i].red)); 
	//debugMe(String(deck[deckNo].run.leds_FFT_history[i].red) + " : "  + String(deck[deckNo].run.leds_FFT_history[i].green) + " : " + String(deck[deckNo].run.leds_FFT_history[i].blue) + " x " + i );
 
	
}

uint8_t LEDS_fft_calc_fxbin_result(uint8_t fxbin)
{
   uint8_t deckNo = 0;

	if(fxbin <  10)       return deck[deckNo].run.fft.fft_fxbin[fxbin].sum;
	else if(fxbin <  20)  {if(deck[deckNo].run.fft.fft_fxbin[fxbin].sum >= deck[deckNo].cfg.fft_config.fft_fxbin[fxbin].trrig_val) return     deck[deckNo].run.fft.fft_fxbin[fxbin].sum; 												else return 0; }
	else if(fxbin <  30)  {if(deck[deckNo].run.fft.fft_fxbin[fxbin].sum >= deck[deckNo].cfg.fft_config.fft_fxbin[fxbin].trrig_val) return 255-deck[deckNo].run.fft.fft_fxbin[fxbin].sum;  											else return 255; }
	else if(fxbin <  40)  {if(deck[deckNo].run.fft.fft_fxbin[fxbin].sum >= deck[deckNo].cfg.fft_config.fft_fxbin[fxbin].trrig_val) return constrain( deck[deckNo].cfg.fft_config.fft_fxbin[fxbin].set_val + deck[deckNo].run.fft.fft_fxbin[fxbin].sum, 0,255); 	else return deck[deckNo].cfg.fft_config.fft_fxbin[fxbin].set_val; }
	else if(fxbin <  50)  {if(deck[deckNo].run.fft.fft_fxbin[fxbin].sum >= deck[deckNo].cfg.fft_config.fft_fxbin[fxbin].trrig_val) return constrain( deck[deckNo].cfg.fft_config.fft_fxbin[fxbin].set_val - deck[deckNo].run.fft.fft_fxbin[fxbin].sum, 0,255); 	else return deck[deckNo].cfg.fft_config.fft_fxbin[fxbin].set_val; }


	return 0;
}



// ******************** Get Set functions
uint8_t getrand8()
{
return random8();

}

void  LEDS_setall_color(uint8_t color = 0) {

	// set all leds to a color
	// 0 = white 50%
	// 1 = green 50%
	// 2 = black
	// 3 = red 50%

	switch(color) {

		case 0: fill_solid(&(deck[0].run.leds[0]), MAX_NUM_LEDS_BOOT, 	CRGB(180, 	180, 	180));	break;
		case 1: fill_solid(&(deck[0].run.leds[0]), MAX_NUM_LEDS_BOOT, 	CRGB(0,		127, 	0));	break;
		case 2: fill_solid(&(deck[0].run.leds[0]), MAX_NUM_LEDS_BOOT,	CRGB(0,		0, 		0));	break;
		case 3: fill_solid(&(deck[0].run.leds[0]), MAX_NUM_LEDS_BOOT, 	CRGB(127, 	0, 		0));	break;
	   default: fill_solid(&(deck[0].run.leds[0]), MAX_NUM_LEDS_BOOT, 	CRGB(180,	180, 	180)); break;	
	}
	//debugMe("Setall Leds to : " + String(color));	
}

uint8_t LEDS_get_fft_fps()
{
	return deck[0].cfg.fft_config.fps;


}

void LEDS_set_fft_fps(uint8_t inFPS)
{
	deck[0].cfg.fft_config.fps = inFPS;


}


float LEDS_get_FPS()
{	// return the FPS value
	//if (get_bool(ARTNET_SEND) == true)
	return led_cfg.realfps;
	//else 							   return float(FastLED.getFPS());
}

uint8_t  LEDS_get_FPS_setting()
{	// return the FPS value
	return deck[0].cfg.led_master_cfg.pal_fps;
}

void LEDS_set_FPS(uint8_t fps_setting)
{	// set the FPS value
	deck[0].cfg.led_master_cfg.pal_fps = constrain(fps_setting, 1 , MAX_PAL_FPS);
}



CRGBPalette16 LEDS_pal_get(uint8_t pal_no)
{
	if (pal_no < 16)
		return deck[0].cfg.LEDS_pal_cur[pal_no];

	// else 
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
	deck[0].cfg.led_master_cfg.bri = bri;

}

uint8_t LEDS_get_bri()
{
	return deck[0].cfg.led_master_cfg.bri;

}


uint8_t LEDS_get_real_bri()
{

	return qadd8(deck[0].cfg.led_master_cfg.bri,deck[0].run.fft.fft_color_result_bri ); 
}

uint8_t LEDS_FFT_get_MAX_value(uint8_t bit)
{
	// return the FFT value for the specified bit
	uint8_t DeckNo = 0;
	return deck[DeckNo].run.fft_data[bit].max;
}

uint8_t LEDS_FFT_get_value(uint8_t bit)
{
	// return the FFT value for the specified bit
	return fft_bin_results[bit];
}


uint8_t LEDS_FFT_get_color_result(uint8_t deckNo, uint8_t color )
{
	switch(color)
	{
		case 0: return deck[deckNo].run.fft.GlobalColor_result.red; break; 
		case 1: return deck[deckNo].run.fft.GlobalColor_result.green; break; 
		case 2: return deck[deckNo].run.fft.GlobalColor_result.blue; break; 

	}

	return 0;
}

uint8_t LEDS_get_playNr()
{
return led_cfg.Play_Nr;
}
void LEDS_set_playNr(uint8_t setNr)
{
	FS_play_conf_read(setNr,&deck[0].cfg, &deck[0].fx1_cfg);
}





// *************************** Sequencer 


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

void LEDS_seqencer_advance()
{
		uint8_t orig_play_nr = led_cfg.Play_Nr;




	if (get_bool(SEQUENCER_ON))	
	{	
		if (orig_play_nr < MAX_NR_SAVES-1 )
		{
			for (uint8_t play_nr = led_cfg.Play_Nr +1 ; play_nr < MAX_NR_SAVES ; play_nr++  )
			{
						//debugMe("Play switch test to " + String(play_nr));


						if(LEDS_get_sequencer(play_nr) && FS_check_Conf_Available(play_nr ) &&  play_conf_time_min[play_nr] != 0   )
						{

							LEDS_G_LoadSAveFade(false,play_nr) ;
							//FS_play_conf_read(play_nr,&deck[0].cfg, &deck[0].fx1_cfg);
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
							LEDS_G_LoadSAveFade(false,play_nr) ;
							//FS_play_conf_read(play_nr,&deck[0].cfg, &deck[0].fx1_cfg);
							break;
							
						}
						
			}			
			
		}

		led_cfg.confSwitch_time = micros() +  play_conf_time_min[led_cfg.Play_Nr] * MICROS_TO_MIN  ;

	}
	else   // Sequencer OFF
	{

		if (orig_play_nr < MAX_NR_SAVES-1 )
		{
			uint8_t load_play_nr = orig_play_nr+1;

			while (load_play_nr <= MAX_NR_SAVES )
			//for (uint8_t play_nr = led_cfg.Play_Nr +1 ; play_nr < MAX_NR_SAVES ; play_nr++  )
			{
					



						if( FS_check_Conf_Available(load_play_nr ) )
						{
							LEDS_G_LoadSAveFade(false,load_play_nr) ;
							
							//FS_play_conf_read(play_nr,&deck[0].cfg, &deck[0].fx1_cfg);
							break;
							
						}
						load_play_nr++;
						if (load_play_nr == MAX_NR_SAVES -1 )  load_play_nr = 0;
						if (load_play_nr == orig_play_nr ) break;
			}
		}
		else
		{
			for (uint8_t play_nr = 0 ; play_nr <= orig_play_nr ; play_nr++  )
			{
						
						if( FS_check_Conf_Available(play_nr ) )
						{
							LEDS_G_LoadSAveFade(false,play_nr) ;
							//FS_play_conf_read(play_nr,&deck[0].cfg, &deck[0].fx1_cfg);
							break;
							
						}
						
			}			
			
		}


	}


}







// ******************************** FFT BIN Get values

uint8_t LEDS_fft_get_fxbin_result(uint8_t fxbin, uint8_t deckNo)
{
	
	if (fxbin < FFT_FX_NR_OF_BINS)
	{
		return deck[deckNo].run.fft.fft_fxbin[fxbin].result;
	}
	else
	{
			 if(fxbin == 254)  return deck[deckNo].run.fft.GlobalColor_result.red;
		else if(fxbin == 253)  return deck[deckNo].run.fft.GlobalColor_result.green;
		else if(fxbin == 252)  return deck[deckNo].run.fft.GlobalColor_result.blue;
		else if(fxbin == 251)  return deck[deckNo].run.fft.fft_color_result_bri;
		else if(fxbin == 250)  return deck[deckNo].run.fft.fft_color_fps;	
		else if(fxbin <  230)  return deck[deckNo].run.fft_data[ 6 - (fxbin - 220 )].max     ; 
		else if(fxbin <  240)  return deck[deckNo].run.fft_data[ 6 - (fxbin - 230 )].avarage ; 
		else if(fxbin <  250)  return fft_bin_results[ 6 - (fxbin - 240 )]   ;
	}

	return 0;
}

uint8_t LEDS_fft_fxbin_get_level(uint8_t bin, uint8_t lvl_value)
{ // if the bin = 255 return the lvl value else return the FXbin result
	
	
	if (bin == 255) return lvl_value;
	
	return LEDS_fft_get_fxbin_result(bin,0) ;

}

uint8_t LEDS_data_or_fftbin(uint8_t inval)
{		
	uint8_t deckNo = 0;
	// based on the input value, return a FFT bin or the inval.
	uint8_t returnVal = 0;
	if (inval >= FFT_FX_NR_OF_BINS) inval=0;

	if(inval < 2)
	{
		if(deck[deckNo].run.fft.fft_fxbin[inval].sum > deck[deckNo].cfg.fft_config.fft_fxbin[inval].trrig_val)
				returnVal = constrain( deck[deckNo].cfg.fft_config.fft_fxbin[inval].set_val + deck[deckNo].run.fft.fft_fxbin[inval].sum, 0,255);
			else returnVal = 0;
	}
	else if (inval < 10)
	{
			if(deck[deckNo].run.fft.fft_fxbin[inval].sum >  deck[deckNo].cfg.fft_config.fft_fxbin[inval].trrig_val)
				returnVal = constrain( deck[deckNo].cfg.fft_config.fft_fxbin[inval].set_val + deck[deckNo].run.fft.fft_fxbin[inval].sum, 0,255);
			else returnVal =  deck[deckNo].cfg.fft_config.fft_fxbin[inval].set_val;

	}
	else if (inval < 18)
	{
			if(deck[deckNo].run.fft.fft_fxbin[inval].sum >  deck[deckNo].cfg.fft_config.fft_fxbin[inval].trrig_val)
				returnVal = constrain( deck[deckNo].cfg.fft_config.fft_fxbin[inval].set_val - deck[deckNo].run.fft.fft_fxbin[inval].sum, 0,255);
			else returnVal =  deck[deckNo].cfg.fft_config.fft_fxbin[inval].set_val;

	}
	else if (inval < 20)
	{
			if(deck[deckNo].run.fft.fft_fxbin[inval].sum >  deck[deckNo].cfg.fft_config.fft_fxbin[inval].trrig_val)
				returnVal = constrain( deck[deckNo].cfg.fft_config.fft_fxbin[inval].set_val - deck[deckNo].run.fft.fft_fxbin[inval].sum, 0,255);
			else returnVal = 255;
	}
	return returnVal;
}


// Gives back the color based on the dropdown menu 
CRGB LEDS_select_color(uint8_t selector, uint16_t pal_index, uint8_t deckNo)
{ 
	   if (selector < NR_PALETTS_SELECT + 1 ) 	return tpm_fx.PalGetFromLongPal(LEDS_pal_get(selector)  , pal_index , 255 , LINEARBLEND);
	else if (selector == 254 )					return CRGB::White;
	else if (selector == 250 )					return deck[deckNo].run.fft.GlobalColor_result;
	else if (selector == 251 )					return CHSV(led_cfg.hue,255,255) ; 
	else if (selector == 252 )					return {random8(),random8(),random8()};

	else if (selector == 200 )					return ColorFromPalette(LEDS_pal_get(7)  , 0 ,255 , NOBLEND );
	else if (selector == 201 )					return ColorFromPalette(LEDS_pal_get(7)  , 16 , 255 , NOBLEND );
	else if (selector == 202 )					return ColorFromPalette(LEDS_pal_get(7)  , 32 , 255 , NOBLEND );
	else if (selector == 203 )					return ColorFromPalette(LEDS_pal_get(7)  , 48 , 255 , NOBLEND );
	else if (selector == 204 )					return ColorFromPalette(LEDS_pal_get(7)  , 64 , 255 , NOBLEND );
	else if (selector == 205 )					return ColorFromPalette(LEDS_pal_get(7)  , 80 , 255 , NOBLEND );
	else if (selector == 206 )					return ColorFromPalette(LEDS_pal_get(7)  , 96 , 255 , NOBLEND );
	else if (selector == 207 )					return ColorFromPalette(LEDS_pal_get(7)  , 112 , 255 , NOBLEND );
	else if (selector == 208 )					return ColorFromPalette(LEDS_pal_get(7)  , 128 , 255 , NOBLEND );
	else if (selector == 209 )					return ColorFromPalette(LEDS_pal_get(7)  , 144 , 255 , NOBLEND );
	else if (selector == 210 )					return ColorFromPalette(LEDS_pal_get(7)  , 160 , 255 , NOBLEND );
	else if (selector == 211 )					return ColorFromPalette(LEDS_pal_get(7)  , 176 , 255 , NOBLEND );
	else if (selector == 212 )					return ColorFromPalette(LEDS_pal_get(7)  , 192 , 255 , NOBLEND );
	else if (selector == 213 )					return ColorFromPalette(LEDS_pal_get(7)  , 208 , 255 , NOBLEND );
	else if (selector == 214 )					return ColorFromPalette(LEDS_pal_get(7)  , 224 , 255 , NOBLEND );
	else if (selector == 215 )					return ColorFromPalette(LEDS_pal_get(7)  , 240 , 255 , NOBLEND );
	

	else if (selector == 220 )					return CRGB::Red;
	else if (selector == 221 )					return CRGB::Green;
	else if (selector == 222 )					return CRGB::Blue;
	else if (selector == 223 )					return CRGB::Yellow;
	else if (selector == 224 )					return CRGB::Pink;
	else if (selector == 225 )					return CRGB::Gold;
	else if (selector == 226 )					return CRGB::Silver; //DarkViolet  Maroon  DeepSkyBlue 




	return CRGB::Black;
}




// ******************************************************* Layer RUN *******************************

// ************************************ FFT ****************************************
void LEDS_run_fft(uint8_t z, uint8_t i, uint8_t DeckNo,CRGB *OutPutLedArray )
{
	if ( deck[DeckNo].cfg.form_cfg[i + (z  * 8)].nr_leds != 0 &&  bitRead(deck[DeckNo].cfg.form_menu_fft[z][_M_FORM_FFT_RUN], i) == true &&  ( deck[DeckNo].cfg.form_fx_fft_signles[z].triggerBin   == 255  ||  LEDS_fft_get_fxbin_result(deck[DeckNo].cfg.form_fx_fft_signles[z].triggerBin,0)  != 0 ) ) 
	{
		uint8_t lvl_select = LEDS_fft_fxbin_get_level(deck[DeckNo].cfg.form_fx_fft_signles[z].lvl_bin, deck[DeckNo].cfg.form_fx_fft[i + (z * 8)].level ); 

		lvl_select = scale8(lvl_select, deck[DeckNo].cfg.form_fx_fft_signles[z].master_lvl );
		
		tpm_fx.mixHistoryOntoLedArray(deck[DeckNo].run.leds_FFT_history, OutPutLedArray, deck[DeckNo].cfg.form_cfg[i + (z * 8)].nr_leds, deck[DeckNo].cfg.form_cfg[i + (z * 8)].start_led, bitRead(deck[DeckNo].cfg.form_menu_fft[z][_M_FORM_FFT_REVERSED], i),  bitRead(deck[DeckNo].cfg.form_menu_fft[z][_M_FORM_FFT_MIRROR],i ) , MixModeType(deck[DeckNo].cfg.form_fx_fft_signles[z].mix_mode),  lvl_select , bitRead(deck[DeckNo].cfg.form_menu_fft[z][_M_FORM_FFT_ONECOLOR] , i), deck[DeckNo].cfg.form_fx_fft[i + (z * 8)].offset, deck[DeckNo].cfg.form_fx_fft[i + (z * 8)].extend ,deck[DeckNo].run.form_fx_fft[i + (z * 8)].extend_tick ,deck[DeckNo].cfg.form_fx_fft_signles[z].color );

		if (deck[DeckNo].cfg.form_fx_fft[i + (z * 8)].extend != 0)
		{
			deck[DeckNo].run.form_fx_fft[i + (z * 8)].extend_tick++ ;
			if (deck[DeckNo].run.form_fx_fft[i + (z * 8)].extend_tick > deck[DeckNo].cfg.form_fx_fft[i + (z * 8)].extend) 
				deck[DeckNo].run.form_fx_fft[i + (z * 8)].extend_tick = 0;
			
			

		}
	}
}

// ************************************ Palette  ****************************************
void LEDS_run_pal(uint8_t z, uint8_t i , uint8_t selectedDeck,CRGB *OutPutLedArray  )
{


	if (deck[selectedDeck].cfg.form_cfg[i + (z  * 8)].nr_leds != 0 && bitRead(deck[selectedDeck].cfg.form_menu_pal[z][_M_FORM_PAL_RUN], i) == true && ( deck[selectedDeck].cfg.form_fx_pal_singles[z].triggerBin  == 255  || LEDS_fft_get_fxbin_result(deck[selectedDeck].cfg.form_fx_pal_singles[z].triggerBin , 0)  != 0 ) ) 
	{
		
		uint8_t lvl_select = LEDS_fft_fxbin_get_level(deck[selectedDeck].cfg.form_fx_pal_singles[z].lvl_bin, deck[selectedDeck].cfg.form_fx_pal[i + (z * 8)].level );
		

		lvl_select = scale8(lvl_select, deck[selectedDeck].cfg.form_fx_pal_singles[z].master_lvl );

	
		tpm_fx.PalFillLong(tmp_array, LEDS_pal_get(deck[selectedDeck].cfg.form_fx_pal_singles[z].pal ), deck[selectedDeck].cfg.form_cfg[i + (z * 8)].start_led,deck[selectedDeck].cfg.form_cfg[i + (z * 8)].nr_leds  , deck[selectedDeck].run.form_fx_pal[i + (z * 8)].indexLong , deck[selectedDeck].cfg.form_fx_pal[i + (z * 8)].index_add_led  ,  MIX_REPLACE, 255,  TBlendType(bitRead(deck[selectedDeck].cfg.form_menu_pal[z][_M_FORM_PAL_BLEND], i)) );
		tpm_fx.mixOntoLedArray(tmp_array, OutPutLedArray, deck[selectedDeck].cfg.form_cfg[i + (z * 8)].nr_leds , deck[selectedDeck].cfg.form_cfg[i + (z * 8)].start_led,  bitRead(deck[selectedDeck].cfg.form_menu_pal[z][_M_FORM_PAL_REVERSED], i), bitRead(deck[selectedDeck].cfg.form_menu_pal[z][_M_FORM_PAL_MIRROR], i)   , MixModeType(deck[selectedDeck].cfg.form_fx_pal_singles[z].mix_mode), lvl_select , bitRead(deck[selectedDeck].cfg.form_menu_pal[z][_M_FORM_PAL_ONECOLOR], i) );
			
		uint16_t pal_speed; 
		if (deck[selectedDeck].cfg.form_fx_pal_singles[z].palSpeedBin != 255) 	pal_speed = LEDS_fft_get_fxbin_result(deck[selectedDeck].cfg.form_fx_pal_singles[z].palSpeedBin ,0 )  ;
		else  												pal_speed = deck[selectedDeck].cfg.form_fx_pal[i + (z * 8)].index_add_frame;  

		deck[selectedDeck].run.form_fx_pal[i + (z * 8)].index = deck[selectedDeck].run.form_fx_pal[i + (z * 8)].index + pal_speed;
		deck[selectedDeck].run.form_fx_pal[i + (z * 8)].indexLong = deck[selectedDeck].run.form_fx_pal[i + (z * 8)].indexLong + pal_speed;
		if ( deck[selectedDeck].run.form_fx_pal[i + (z * 8)].indexLong >= MAX_INDEX_LONG ) 	deck[selectedDeck].run.form_fx_pal[i + (z * 8)].indexLong = deck[selectedDeck].run.form_fx_pal[i + (z * 8)].indexLong - MAX_INDEX_LONG;
	}
	else
	{	// if thers noting to do just move the index so they stay synced in the position. if its not linked to an fft bin
		if (deck[selectedDeck].cfg.form_fx_pal_singles[z].palSpeedBin == 255)
		{	
			deck[selectedDeck].run.form_fx_pal[i + (z * 8)].index = deck[selectedDeck].run.form_fx_pal[i + (z * 8)].index + deck[selectedDeck].cfg.form_fx_pal[i + (z * 8)].index_add_frame;
			deck[selectedDeck].run.form_fx_pal[i + (z * 8)].indexLong = deck[selectedDeck].run.form_fx_pal[i + (z * 8)].indexLong + deck[selectedDeck].cfg.form_fx_pal[i + (z * 8)].index_add_frame;
			if ( deck[selectedDeck].run.form_fx_pal[i + (z * 8)].indexLong >= MAX_INDEX_LONG ) 	deck[selectedDeck].run.form_fx_pal[i + (z * 8)].indexLong = deck[selectedDeck].run.form_fx_pal[i + (z * 8)].indexLong - MAX_INDEX_LONG;
		}

	}

}

// ************************************ FIRE ****************************************
void LEDS_run_fire(uint8_t z, uint8_t i , uint8_t selectedDeck,CRGB *OutPutLedArray  )
{
	//if ( deck[selectedDeck].cfg.form_cfg[i + (z  * 8)].nr_leds != 0 &&  (bitRead(deck[selectedDeck].fx1_cfg.form_menu_fire[z][_M_FORM_FIRE_RUN], i) == true)  && ( deck[selectedDeck].fx1_cfg.form_fx_fire_bytes[z].triggerBin   == 255  || LEDS_fft_get_fxbin_result(deck[selectedDeck].fx1_cfg.form_fx_fire_bytes[z].triggerBin,0)  != 0 ))
	if ( deck[selectedDeck].cfg.form_cfg[i + (z  * 8)].nr_leds != 0 &&  (bitRead(deck[selectedDeck].fx1_cfg.form_menu_fire[z][_M_FORM_FIRE_RUN], i) == true)  && (( deck[selectedDeck].fx1_cfg.form_fx_fire_bytes[z].triggerBin   == 255  ||  LEDS_fft_get_fxbin_result(deck[selectedDeck].fx1_cfg.form_fx_fire_bytes[z].triggerBin,0)  != 0 )))
	{
		//uint8_t spk_val = LEDS_data_or_fftbin( deck[selectedDeck].fx1_cfg.form_fx_fire_bytes[z].sparking);
		//uint8_t cool_val = LEDS_data_or_fftbin( deck[selectedDeck].fx1_cfg.form_fx_fire_bytes[z].cooling);

		uint8_t lvl_select = LEDS_fft_fxbin_get_level(deck[selectedDeck].fx1_cfg.form_fx_fire_bytes[z].lvl_bin, deck[selectedDeck].fx1_cfg.form_fx_fire_bytes[z].level );
		lvl_select = scale8(lvl_select, deck[selectedDeck].fx1_cfg.form_fx_fire_bytes[z].master_lvl );

		tpm_fx.Fire2012WithPalette(tmp_array, deck[0].run.heat, LEDS_pal_get(deck[selectedDeck].fx1_cfg.form_fx_fire_bytes[z].pal),  deck[selectedDeck].cfg.form_cfg[i + (z * 8)].start_led, deck[selectedDeck].cfg.form_cfg[i + (z * 8)].nr_leds, 255 , deck[selectedDeck].fx1_cfg.form_fx_fire_bytes[z].cooling ,deck[selectedDeck].fx1_cfg.form_fx_fire_bytes[z].sparking , MixModeType(MIX_REPLACE)  );
		tpm_fx.mixOntoLedArray(tmp_array, OutPutLedArray, deck[selectedDeck].cfg.form_cfg[i + (z * 8)].nr_leds , deck[selectedDeck].cfg.form_cfg[i + (z * 8)].start_led,  bitRead(deck[selectedDeck].fx1_cfg.form_menu_fire[z][_M_FORM_FIRE_REVERSED], i), bitRead(deck[selectedDeck].fx1_cfg.form_menu_fire[z][_M_FORM_FIRE_MIRROR], i)   , MixModeType(deck[selectedDeck].fx1_cfg.form_fx_fire_bytes[z].mix_mode), lvl_select, false );
	}

}

// ************************************ SHIMMER ****************************************
void LEDS_run_shimmer(uint8_t z, uint8_t i, uint8_t selectedDeck,CRGB *OutPutLedArray   )
{
	if ( deck[selectedDeck].cfg.form_cfg[i + (z  * 8)].nr_leds != 0 &&  bitRead(deck[selectedDeck].fx1_cfg.form_menu_shimmer[z][_M_FORM_SHIMMER_RUN], i) == true && ( deck[selectedDeck].fx1_cfg.form_fx_shim_bytes[z].triggerBin == 255 || LEDS_fft_get_fxbin_result(deck[selectedDeck].fx1_cfg.form_fx_shim_bytes[z].triggerBin,0)  != 0 ))
	{
		uint8_t beater_val = LEDS_data_or_fftbin( deck[selectedDeck].fx1_cfg.form_fx_shim[i + (z * 8)].beater);

		uint8_t lvl_select = LEDS_fft_fxbin_get_level(deck[selectedDeck].fx1_cfg.form_fx_shim_bytes[z].lvl_bin, deck[selectedDeck].fx1_cfg.form_fx_shim_bytes[z].level );
		lvl_select = scale8(lvl_select, deck[selectedDeck].fx1_cfg.form_fx_shim_bytes[z].master_lvl );
		deck[selectedDeck].fx1_cfg.form_fx_shim[i + (z * 8)].dist =  tpm_fx.Shimmer(OutPutLedArray,  LEDS_pal_get(deck[selectedDeck].fx1_cfg.form_fx_shim_bytes[z].pal) , deck[selectedDeck].cfg.form_cfg[i + (z * 8)].start_led, deck[selectedDeck].cfg.form_cfg[i + (z * 8)].nr_leds, deck[selectedDeck].fx1_cfg.form_fx_shim[i + (z * 8)].dist, deck[selectedDeck].fx1_cfg.form_fx_shim[i + (z * 8)].xscale, deck[selectedDeck].fx1_cfg.form_fx_shim[i + (z * 8)].yscale, beater_val ,  MixModeType(deck[selectedDeck].fx1_cfg.form_fx_shim_bytes[z].mix_mode), lvl_select ,  TBlendType(bitRead(deck[selectedDeck].fx1_cfg.form_menu_shimmer[z][_M_FORM_SHIMMER_BLEND], i) ) );
	}

}

// ************************************ STROBE ****************************************
void LEDS_run_FX_strobe(uint8_t z, uint8_t i, uint8_t selectedDeck,CRGB *OutPutLedArray   )
{
	if ( deck[selectedDeck].cfg.form_cfg[i + (z  * 8)].nr_leds != 0 &&  bitRead(deck[selectedDeck].fx1_cfg.form_menu_strobe[z][_M_FORM_STROBE_RUN], i) == true && ( deck[selectedDeck].fx1_cfg.form_fx_strobe_bytes[z].triggerBin   == 255  || LEDS_fft_get_fxbin_result(deck[selectedDeck].fx1_cfg.form_fx_strobe_bytes[z].triggerBin,0) != 0 ))
	{
		uint8_t lvl_select = LEDS_fft_fxbin_get_level(deck[selectedDeck].fx1_cfg.form_fx_strobe_bytes[z ].lvl_bin, deck[selectedDeck].fx1_cfg.form_fx_strobe_bytes[z].level );
		lvl_select = scale8(lvl_select, deck[selectedDeck].fx1_cfg.form_fx_strobe_bytes[z].master_lvl );

		tpm_fx.strobe(OutPutLedArray , deck[selectedDeck].cfg.form_cfg[i + (z * 8)].start_led, deck[selectedDeck].cfg.form_cfg[i + (z * 8)].nr_leds ,  LEDS_select_color(deck[selectedDeck].fx1_cfg.form_fx_strobe_bytes[z].pal,0,0) , deck[selectedDeck].fx1_cfg.form_fx_strobe_bytes[z].on_frames, deck[selectedDeck].fx1_cfg.form_fx_strobe_bytes[z].off_frames, deck[selectedDeck].run.form_fx_strobe[z].frame_pos , MixModeType(deck[selectedDeck].fx1_cfg.form_fx_strobe_bytes[z].mix_mode) ,lvl_select );
		
		
	}

}

// ************************************ EYES ****************************************
void LEDS_run_FX_eyes(uint8_t z, uint8_t i, uint8_t selectedDeck,CRGB *OutPutLedArray   )
{
	
	if ( deck[selectedDeck].cfg.form_cfg[i + (z  * 8)].nr_leds != 0 && bitRead(deck[selectedDeck].fx1_cfg.form_menu_eyes[z][_M_FORM_EYES_RUN], i) == true && ( deck[selectedDeck].fx1_cfg.form_fx_eyes_bytes[z].triggerBin   == 255 || LEDS_fft_get_fxbin_result(deck[selectedDeck].fx1_cfg.form_fx_eyes_bytes[z].triggerBin,0)  != 0 ))
	{
		

		uint8_t lvl_select  = LEDS_fft_fxbin_get_level(deck[selectedDeck].fx1_cfg.form_fx_eyes_bytes[z].lvl_bin, deck[selectedDeck].fx1_cfg.form_fx_eyes_bytes[z].level );
		lvl_select = scale8(lvl_select, deck[selectedDeck].fx1_cfg.form_fx_eyes_bytes[z].master_lvl );

		if (deck[selectedDeck].fx1_cfg.form_fx_eyes_bytes[z].lvl_bin != 255)  	 lvl_select = LEDS_fft_get_fxbin_result(deck[selectedDeck].fx1_cfg.form_fx_eyes_bytes[z].lvl_bin,0) ;
		else 											lvl_select = deck[selectedDeck].fx1_cfg.form_fx_eyes_bytes[z].level ;

		tpm_fx.BlinkingEyes(OutPutLedArray , deck[selectedDeck].cfg.form_cfg[i + (z * 8)].start_led, deck[selectedDeck].cfg.form_cfg[i + (z * 8)].nr_leds ,  LEDS_select_color(deck[selectedDeck].fx1_cfg.form_fx_eyes_bytes[z].color , 0 , 0 ) ,  deck[selectedDeck].fx1_cfg.form_fx_eyes_bytes[z].EyeWidth, deck[selectedDeck].fx1_cfg.form_fx_eyes_bytes[z].EyeSpace, deck[selectedDeck].run.form_fx_eyes[i + (z * 8)].eye_pos, deck[selectedDeck].fx1_cfg.form_fx_eyes_bytes[z].on_frames,  deck[selectedDeck].run.form_fx_eyes[i + (z * 8)].frame_pos, deck[selectedDeck].fx1_cfg.form_fx_eyes_bytes[z].fadeval,   MixModeType(deck[selectedDeck].fx1_cfg.form_fx_eyes_bytes[z].mix_mode), lvl_select);
		//tpm_fx.strobe(leds , deck[selectedDeck].cfg.form_cfg[i + (z * 8)].start_led, deck[selectedDeck].cfg.form_cfg[i + (z * 8)].nr_leds ,  LEDS_select_color(deck[selectedDeck].fx1_cfg.form_fx_eyes[i + (z * 8)].pal, random16()) , deck[selectedDeck].fx1_cfg.form_fx_strobe[i + (z * 8)].on_frames, deck[selectedDeck].fx1_cfg.form_fx_strobe[i + (z * 8)].off_frames, deck[selectedDeck].fx1_cfg.form_fx_strobe[i + (z * 8)].frame_pos , MixModeType(deck[selectedDeck].fx1_cfg.form_fx_strobe[i + (z * 8)].mix_mode) ,lvl_select );
		
		deck[selectedDeck].run.form_fx_eyes[i + (z * 8)].frame_pos++;
		if (deck[selectedDeck].run.form_fx_eyes[i + (z * 8)].frame_pos >=  deck[selectedDeck].fx1_cfg.form_fx_eyes_bytes[z].pause_frames + deck[selectedDeck].fx1_cfg.form_fx_eyes_bytes[z].on_frames ) 
			{ deck[selectedDeck].run.form_fx_eyes[i + (z * 8)].frame_pos = 0;  deck[selectedDeck].run.form_fx_eyes[i + (z * 8)].eye_pos = random16(deck[selectedDeck].cfg.form_cfg[i + (z * 8)].start_led , deck[selectedDeck].cfg.form_cfg[i + (z * 8)].start_led + deck[selectedDeck].cfg.form_cfg[i + (z * 8)].nr_leds);}

	}

}

// ************************************ METEOR ****************************************
void LEDS_run_FX_meteor(uint8_t z, uint8_t i, uint8_t selectedDeck ,CRGB *OutPutLedArray  )
{
	if ( ( deck[selectedDeck].fx1_cfg.form_fx_meteor_bytes[z].triggerBin   == 255  ) ||   (LEDS_fft_get_fxbin_result(deck[selectedDeck].fx1_cfg.form_fx_meteor_bytes[z].triggerBin ,0  )        != 0 ))
	{
		

		uint8_t lvl_select = LEDS_fft_fxbin_get_level(deck[selectedDeck].fx1_cfg.form_fx_meteor_bytes[i + (z * 8)].lvl_bin, deck[selectedDeck].fx1_cfg.form_fx_meteor_bytes[i + (z * 8)].level );
		lvl_select = scale8(lvl_select, deck[selectedDeck].fx1_cfg.form_fx_meteor_bytes[z].master_lvl );

		tpm_fx.meteorRain( OutPutLedArray, deck[selectedDeck].cfg.form_cfg[i + (z * 8)].start_led , deck[selectedDeck].cfg.form_cfg[i + (z * 8)].nr_leds  , LEDS_select_color(deck[selectedDeck].fx1_cfg.form_fx_meteor_bytes[z].color , 254,0 )   ,deck[selectedDeck].fx1_cfg.form_fx_meteor[i + (z * 8)].frame_pos,  deck[selectedDeck].fx1_cfg.form_fx_meteor[i + (z * 8)].meteorSize, deck[selectedDeck].fx1_cfg.form_fx_meteor[i + (z * 8)].meteorTrailDecay, bitRead(deck[selectedDeck].fx1_cfg.form_menu_meteor[z][_M_FORM_METEOR_RANDOMDECAY], i), deck[selectedDeck].fx1_cfg.form_fx_meteor_bytes[z].level ) ;
		 
		//tpm_fx.mixOntoLedArray(tmp_array, leds, deck[selectedDeck].cfg.form_cfg[i + (z * 8)].nr_leds , deck[selectedDeck].cfg.form_cfg[i + (z * 8)].start_led,  bitRead(deck[selectedDeck].fx1_cfg.form_fx_meteor[z][_M_FORM_FIRE_REVERSED], i), bitRead(deck[selectedDeck].fx1_cfg.form_fx_meteor[z][_M_FORM_METEOR_MIRROR], i)   , MixModeType(deck[selectedDeck].fx1_cfg.form_fx_meteor[i + (z * 8)].mix_mode), lvl_select, false );
		//debugMe(String( LEDS_select_color(deck[selectedDeck].fx1_cfg.form_fx_meteor_bytes[z].color , 254,0 ).r  ));
		//debugMe(deck[selectedDeck].fx1_cfg.form_fx_meteor[i + (z * 8)].frame_pos);
		deck[selectedDeck].fx1_cfg.form_fx_meteor[i + (z * 8)].frame_pos++;
		if (deck[selectedDeck].fx1_cfg.form_fx_meteor[i + (z * 8)].frame_pos >=  deck[selectedDeck].cfg.form_cfg[i + (z * 8)].nr_leds ) 
			deck[selectedDeck].fx1_cfg.form_fx_meteor[i + (z * 8)].frame_pos = 0;
	}

}


// ************************************ rotate  ****************************************
void LEDS_run_FX_rotate(uint8_t z, uint8_t i , uint8_t selectedDeck,CRGB *OutPutLedArray  )
{
	if (  deck[selectedDeck].cfg.form_cfg[i + (z  * 8)].nr_leds != 0 &&  (bitRead(deck[selectedDeck].cfg.form_menu_modify[z][_M_FORM_MODIFY_ROTATE], i) == true) && ( deck[selectedDeck].cfg.form_fx_modify_bytes[z].RotateTriggerBin   == 255   ||   LEDS_fft_get_fxbin_result(deck[selectedDeck].cfg.form_fx_modify_bytes[z].RotateTriggerBin ,0 )        != 0 ))
	{
		

		//uint8_t lvl_select = LEDS_fft_fxbin_get_level(deck[selectedDeck].fx1_cfg.form_fx_meteor[i + (z * 8)].lvl_bin, deck[selectedDeck].fx1_cfg.form_fx_meteor[i + (z * 8)].level );
		if (deck[selectedDeck].cfg.form_fx_modify[i + (z * 8)].RotateFixed != 0) 
			tpm_fx.rotate(OutPutLedArray , deck[selectedDeck].cfg.form_cfg[i + (z * 8)].nr_leds , deck[selectedDeck].cfg.form_cfg[i + (z * 8)].start_led, deck[selectedDeck].cfg.form_fx_modify[i + (z * 8)].RotateFixed );
		
		if (deck[selectedDeck].cfg.form_fx_modify_bytes[z].RotateFullFrames != 0) 
		{	
			tpm_fx.rotate(OutPutLedArray , deck[selectedDeck].cfg.form_cfg[i + (z * 8)].nr_leds , deck[selectedDeck].cfg.form_cfg[i + (z * 8)].start_led, deck[selectedDeck].cfg.form_fx_modify_bytes[z].RotateFullFrames  ,  deck[selectedDeck].run.form_fx_modify[i + (z * 8)].RotateFramePos, bitRead(deck[selectedDeck].cfg.form_menu_modify[z][_M_FORM_MODIFY_ROTATE_REVERSED],i  )  );
			deck[selectedDeck].run.form_fx_modify[i + (z * 8)].RotateFramePos++;
			if (deck[selectedDeck].run.form_fx_modify[i + (z * 8)].RotateFramePos >= deck[selectedDeck].cfg.form_fx_modify_bytes[z].RotateFullFrames  )  	deck[selectedDeck].run.form_fx_modify[i + (z * 8)].RotateFramePos = 0;
		}

	
	}

}

// ***************************************** clock  **********************

void LEDS_run_FX_clock(uint8_t z, uint8_t i , uint8_t selectedDeck,CRGB *OutPutLedArray) //
{
	uint8_t Clock_Type	 	=  deck[selectedDeck].fx1_cfg.form_fx_clock[i + (z * 8)].type ;
 	boolean  hour_bool  	= bitRead(deck[selectedDeck].fx1_cfg.form_menu_clock[z][_M_FORM_CLOCK_HOUR], i) ;
  	boolean min_bool 		= bitRead(deck[selectedDeck].fx1_cfg.form_menu_clock[z][_M_FORM_CLOCK_MINUET], i) ; 
	boolean sec_bool 		= bitRead(deck[selectedDeck].fx1_cfg.form_menu_clock[z][_M_FORM_CLOCK_SECONDS], i) ; 

	uint8_t lvl_select = scale8(deck[selectedDeck].fx1_cfg.form_fx_clock[i + (z * 8)].level , deck[selectedDeck].fx1_cfg.form_fx_clock_bytes[z].master_lvl ); 

	if (hour_bool)  		tpm_fx.CLOCK( OutPutLedArray, deck[selectedDeck].cfg.form_cfg[i + (z * 8)].start_led , deck[selectedDeck].cfg.form_cfg[i + (z * 8)].nr_leds  ,    LEDS_select_color(deck[selectedDeck].fx1_cfg.form_fx_clock[i + (z * 8)].color , 254,0 ),  MixModeType(deck[selectedDeck].fx1_cfg.form_fx_clock_bytes[z].mix_mode)   , deck[selectedDeck].fx1_cfg.form_fx_clock[i + (z * 8)].level  , true,  clock_type_selector(Clock_Type) , NTP_get_time_h() , deck[selectedDeck].fx1_cfg.form_fx_clock[i + (z * 8)].length) ; 
	if (min_bool) 			tpm_fx.CLOCK( OutPutLedArray, deck[selectedDeck].cfg.form_cfg[i + (z * 8)].start_led , deck[selectedDeck].cfg.form_cfg[i + (z * 8)].nr_leds  ,    LEDS_select_color(deck[selectedDeck].fx1_cfg.form_fx_clock[i + (z * 8)].color , 254,0 ),  MixModeType(deck[selectedDeck].fx1_cfg.form_fx_clock_bytes[z].mix_mode)   , deck[selectedDeck].fx1_cfg.form_fx_clock[i + (z * 8)].level  , false,  clock_type_selector(Clock_Type) , NTP_get_time_m(), deck[selectedDeck].fx1_cfg.form_fx_clock[i + (z * 8)].length ) ; 
	if (sec_bool)			tpm_fx.CLOCK( OutPutLedArray, deck[selectedDeck].cfg.form_cfg[i + (z * 8)].start_led , deck[selectedDeck].cfg.form_cfg[i + (z * 8)].nr_leds  ,    LEDS_select_color(deck[selectedDeck].fx1_cfg.form_fx_clock[i + (z * 8)].color , 254,0 ),  MixModeType(deck[selectedDeck].fx1_cfg.form_fx_clock_bytes[z].mix_mode)   , deck[selectedDeck].fx1_cfg.form_fx_clock[i + (z * 8)].level  , false,  clock_type_selector(Clock_Type) , NTP_get_time_s(), deck[selectedDeck].fx1_cfg.form_fx_clock[i + (z * 8)].length ) ; 

}



// ***************************************** FX 01  **********************

void LEDS_run_fx1_glitter(uint8_t z, uint8_t i , uint8_t DeckNo )
{

	
 	uint8_t glitt_val ;
	CRGB glitt_color;


	if (deck[DeckNo].fx1_cfg.form_fx_glitter_bytes[z].glit_bin != 255)  glitt_val = LEDS_fft_get_fxbin_result(deck[DeckNo].fx1_cfg.form_fx_glitter_bytes[z].glit_bin,0);
	else  										  	   glitt_val = deck[DeckNo].fx1_cfg.form_fx_glitter[i + (z * 8)].value;
	 

	glitt_color = LEDS_select_color(deck[DeckNo].fx1_cfg.form_fx_glitter_bytes[z].pal, random16(), 0  );

	tpm_fx.AddGlitter(deck[DeckNo].run.led_FX_out ,	glitt_color , glitt_val , deck[DeckNo].cfg.form_cfg[i + (z * 8)].start_led, deck[DeckNo].cfg.form_cfg[i + (z * 8)].nr_leds, deck[DeckNo].fx1_cfg.form_fx_glitter_bytes[z].level);


}

void LEDS_run_fx1_fade(uint8_t z, uint8_t i, uint8_t DeckNo )
{
	 tpm_fx.fadeLedArray(deck[DeckNo].run.led_FX_out, deck[DeckNo].cfg.form_cfg[i + (z * 8)].start_led, deck[DeckNo].cfg.form_cfg[i + (z * 8)].nr_leds, deck[DeckNo].fx1_cfg.form_fx1[z].fade);  

}

void LEDS_run_fx1_dot(uint8_t z, uint8_t i, uint8_t DeckNo  )
{
	CRGB dotcolor = CRGB::Black; 
	debugMe(deck[DeckNo].run.form_fx_dots[i + (z * 8)].indexLong);

	dotcolor = LEDS_select_color(deck[DeckNo].fx1_cfg.form_fx_dots_bytes[z].pal, deck[DeckNo].run.form_fx_dots[i + (z * 8)].indexLong, DeckNo )    ;
	debugMe("TESTIT : ", false);
	debugMe(String(dotcolor.r));
	debugMe(" : ", false);
	debugMe(String(dotcolor.g));
	debugMe(" : ", false);
	debugMe(String(dotcolor.b));
	debugMe(" : ", true);
	


	if (bitRead(deck[DeckNo].fx1_cfg.form_menu_dot[z][_M_FORM_DOT_TYPE], i) == DOT_SINE	) tpm_fx.DotSine(deck[DeckNo].run.led_FX_out, dotcolor,deck[DeckNo].fx1_cfg.form_fx_dots[i + (z * 8)].nr_dots, deck[DeckNo].cfg.form_cfg[i + (z * 8)].start_led, deck[DeckNo].cfg.form_cfg[i + (z * 8)].nr_leds, deck[DeckNo].fx1_cfg.form_fx_dots[i + (z * 8)].speed, deck[DeckNo].fx1_cfg.form_fx_dots_bytes[z].level); 
	
	else   // its a saw DOT
			tpm_fx.DotSaw(deck[DeckNo].run.led_FX_out,  dotcolor,deck[DeckNo].fx1_cfg.form_fx_dots[i + (z * 8)].nr_dots, deck[DeckNo].cfg.form_cfg[i + (z * 8)].start_led, deck[DeckNo].cfg.form_cfg[i + (z * 8)].nr_leds, deck[DeckNo].fx1_cfg.form_fx_dots[i + (z * 8)].speed, deck[DeckNo].fx1_cfg.form_fx_dots_bytes[z].level); 

	deck[DeckNo].run.form_fx_dots[i + (z * 8)].indexLong  = deck[DeckNo].run.form_fx_dots[i + (z * 8)].indexLong + deck[DeckNo].fx1_cfg.form_fx_dots[i + (z * 8)].index_add ;
	if (MAX_INDEX_LONG <= deck[DeckNo].run.form_fx_dots[i + (z * 8) ].indexLong) deck[DeckNo].run.form_fx_dots[i + (z * 8)].indexLong = deck[DeckNo].run.form_fx_dots[i + (z * 8)].indexLong - MAX_INDEX_LONG;
	

}


void LEDS_run_fx01(uint8_t z, uint8_t i, uint8_t DeckNo, CRGB *OutPutLedArray  )
{
	
	if ((deck[DeckNo].cfg.form_cfg[i + (z  * 8)].nr_leds != 0) && (deck[DeckNo].fx1_cfg.form_fx1[z].fade != 0 ) ) LEDS_run_fx1_fade(z,i,DeckNo);

	if ((deck[DeckNo].cfg.form_cfg[i + (z  * 8)].nr_leds != 0) && (bitRead(deck[DeckNo].fx1_cfg.form_menu_fx1[z][_M_FORM_FX1_RUN], i) == true)) 
	{
	
		uint8_t lvl_select = LEDS_fft_fxbin_get_level(deck[DeckNo].fx1_cfg.form_fx1[z].lvl_bin, deck[0].fx1_cfg.form_fx1[z].level );
		lvl_select = scale8(lvl_select, deck[DeckNo].fx1_cfg.form_fx1[z].master_lvl );

		if 	(bitRead(deck[DeckNo].fx1_cfg.form_menu_glitter[z][_M_FORM_GLITTER_RUN], i) == true)    	LEDS_run_fx1_glitter( z,  i, DeckNo );
		if  (bitRead(deck[DeckNo].fx1_cfg.form_menu_dot[z][_M_FORM_DOT_RUN], i) == true)   				LEDS_run_fx1_dot(z,i, DeckNo); 	  
		
		
		if  (bitRead(deck[DeckNo].fx1_cfg.form_menu_meteor[z][_M_FORM_METEOR_RUN], i) == true)  		LEDS_run_FX_meteor(z,i, DeckNo, deck[DeckNo].run.led_FX_out);

		
		if ( ( deck[DeckNo].fx1_cfg.form_fx1[z].triggerBin   == 255  ) ||   (LEDS_fft_get_fxbin_result(deck[DeckNo].fx1_cfg.form_fx1[z].triggerBin ,0 )    != 0 ))
			{tpm_fx.mixOntoLedArray(deck[DeckNo].run.led_FX_out , OutPutLedArray, deck[DeckNo].cfg.form_cfg[i + (z * 8)].nr_leds, deck[DeckNo].cfg.form_cfg[i + (z * 8)].start_led, bitRead(deck[DeckNo].fx1_cfg.form_menu_fx1[z][_M_FORM_FX1_REVERSED], i), bitRead(deck[DeckNo].fx1_cfg.form_menu_fx1[z][_M_FORM_FX1_MIRROR], i),MixModeType(deck[DeckNo].fx1_cfg.form_fx1[z].mix_mode),  lvl_select , false );}
			

		
		led_cfg.hue++;

	}

}

void LEDS_RUN_MIX_saved_layer( uint8_t DeckNo, uint8_t SaveArrayNo  )
{
		tpm_fx.mixOntoLedArray(deck[DeckNo].run.SaveLayers[SaveArrayNo] , deck[0].run.leds, deck[DeckNo].cfg.layer.save_NrLeds[SaveArrayNo]  , deck[DeckNo].cfg.layer.save_startLed[SaveArrayNo], false, false ,MixModeType(deck[DeckNo].cfg.layer.save_mix[SaveArrayNo]),  deck[DeckNo].cfg.layer.save_lvl[SaveArrayNo] , false );


}

void LEDS_RUN_save_saved_layer( uint8_t DeckNo, uint8_t SaveArrayNo  )
{
	deck[DeckNo].run.SaveLayers[SaveArrayNo](deck[DeckNo].cfg.layer.save_startLed[SaveArrayNo], deck[DeckNo].cfg.layer.save_startLed[SaveArrayNo] + deck[DeckNo].cfg.layer.save_NrLeds[SaveArrayNo]  -1).fadeToBlackBy(255); 
	deck[DeckNo].run.SaveLayers[SaveArrayNo](deck[DeckNo].cfg.layer.save_startLed[SaveArrayNo], deck[DeckNo].cfg.layer.save_startLed[SaveArrayNo] + deck[DeckNo].cfg.layer.save_NrLeds[SaveArrayNo] -1 )  = deck[DeckNo].run.leds(deck[DeckNo].cfg.layer.save_startLed[SaveArrayNo], deck[DeckNo].cfg.layer.save_startLed[SaveArrayNo] + deck[DeckNo].cfg.layer.save_NrLeds[SaveArrayNo] -1 ) ;  
	deck[DeckNo].run.leds(deck[DeckNo].cfg.layer.save_startLed[SaveArrayNo], deck[DeckNo].cfg.layer.save_startLed[SaveArrayNo] + deck[DeckNo].cfg.layer.save_NrLeds[SaveArrayNo] -1 ).fadeToBlackBy(255); 

}

// ********************************************************** 

void LEDS_clear_all_layers(uint8_t deckSelected)
{
	for ( uint8_t layer = 0 ; layer < MAX_LAYERS_SELECT ; layer++ )
		deck[deckSelected].cfg.layer.select[layer]  = 0;
}

void LEDS_default_layers(uint8_t deckSelected)
{
	for ( uint8_t layer = 0 ; layer < MAX_LAYERS_SELECT ; layer++ )
		if (layer < MAX_LAYERS_BASIC) 	deck[deckSelected].cfg.layer.select[layer]  = layer+1 ;
		else 						    deck[deckSelected].cfg.layer.select[layer]  = 0 ;

}


void LEDS_run_layers(uint8_t deckSelected)
{


	for ( uint8_t layer = 0 ; layer < MAX_LAYERS_SELECT ; layer++ )
	{
				
		if( deck[deckSelected].cfg.layer.select[layer] != 0 &&  deck[deckSelected].cfg.layer.select[layer] <= MAX_LAYERS )
		{
			// LAYERS 00 to 15   *** Z =0 ; Z<2  
			if 		( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_00_FFT  ) 		for (byte z = 0; z < 2; z++) for (byte i = 0; i < 8; i++)  LEDS_run_fft(z,i,deckSelected, deck[0].run.leds);
			else if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_00_PAL  ) 		for (byte z = 0; z < 2; z++) for (byte i = 0; i < 8; i++)  LEDS_run_pal(z,i,deckSelected, deck[0].run.leds);
			else if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_00_FX01 )		for (byte z = 0; z < 2; z++) for (byte i = 0; i < 8; i++)  {LEDS_run_fx01(z,i,deckSelected, deck[0].run.leds) ;}    
			else if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_00_FIRE ) 		for (byte z = 0; z < 2; z++) for (byte i = 0; i < 8; i++)  LEDS_run_fire(z,i,deckSelected, deck[0].run.leds);
			else if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_00_SHIMMER ) 	for (byte z = 0; z < 2; z++) for (byte i = 0; i < 8; i++)  LEDS_run_shimmer(z,i,deckSelected, deck[0].run.leds);
			else if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_00_STROBE ) 	for (byte z = 0; z < 2; z++) for (byte i = 0; i < 8; i++)  LEDS_run_FX_strobe(z,i,deckSelected, deck[0].run.leds);
			else if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_00_EYES ) 		for (byte z = 0; z < 2; z++) for (byte i = 0; i < 8; i++)  LEDS_run_FX_eyes(z,i,deckSelected, deck[0].run.leds);
			else if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_00_ROTATE ) 	for (byte z = 0; z < 2; z++) for (byte i = 0; i < 8; i++)  LEDS_run_FX_rotate(z,i,deckSelected, deck[0].run.leds);

			else if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_00_CLOCK ) 		for (byte z = 0; z < 2; z++) for (byte i = 0; i < 8; i++)  LEDS_run_FX_clock(z,i,deckSelected, deck[0].run.leds);


			// LAYERS 16 to 31   *** Z =2 ; Z<4
			
			
			if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_16_FFT ) 		for (byte z = 2; z < 4; z++) for (byte i = 0; i < 8; i++)  LEDS_run_fft(z,i,deckSelected, deck[0].run.leds);
			else if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_16_PAL ) 		for (byte z = 2; z < 4; z++) for (byte i = 0; i < 8; i++)  LEDS_run_pal(z,i,deckSelected, deck[0].run.leds);
			else if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_16_FX01 ) 		for (byte z = 2; z < 4; z++) for (byte i = 0; i < 8; i++)  LEDS_run_fx01(z,i,deckSelected, deck[0].run.leds);
			else if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_16_FIRE ) 		for (byte z = 2; z < 4; z++) for (byte i = 0; i < 8; i++)  LEDS_run_fire(z,i,deckSelected, deck[0].run.leds);
			else if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_16_SHIMMER ) 	for (byte z = 2; z < 4; z++) for (byte i = 0; i < 8; i++)  LEDS_run_shimmer(z,i,deckSelected, deck[0].run.leds);
			else if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_16_STROBE ) 	for (byte z = 2; z < 4; z++) for (byte i = 0; i < 8; i++)  LEDS_run_FX_strobe(z,i,deckSelected, deck[0].run.leds);
			else if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_16_EYES ) 		for (byte z = 2; z < 4; z++) for (byte i = 0; i < 8; i++)  LEDS_run_FX_eyes(z,i,deckSelected, deck[0].run.leds);
			else if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_16_ROTATE ) 	for (byte z = 2; z < 4; z++) for (byte i = 0; i < 8; i++)  LEDS_run_FX_rotate(z,i,deckSelected, deck[0].run.leds);
			else if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_16_CLOCK ) 		for (byte z = 2; z < 4; z++) for (byte i = 0; i < 8; i++)  LEDS_run_FX_clock(z,i,deckSelected, deck[0].run.leds);

			// LAYERS 32 to 48   *** Z =4 ; Z<6
			else if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_32_FFT ) 		for (byte z = 4; z < 6; z++) for (byte i = 0; i < 8; i++)  LEDS_run_fft(z,i,deckSelected, deck[0].run.leds);
			else if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_32_PAL ) 		for (byte z = 4; z < 6; z++) for (byte i = 0; i < 8; i++)  LEDS_run_pal(z,i,deckSelected, deck[0].run.leds);
			else if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_32_ROTATE ) 	for (byte z = 4; z < 6; z++) for (byte i = 0; i < 8; i++)  LEDS_run_FX_rotate(z,i,deckSelected, deck[0].run.leds);

			// LAYERS 32 to 48   *** Z =4 ; Z<6
			else if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_48_FFT ) 		for (byte z = 6; z < 8; z++) for (byte i = 0; i < 8; i++)  LEDS_run_fft(z,i,deckSelected, deck[0].run.leds);
			else if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_48_PAL ) 		for (byte z = 6; z < 8; z++) for (byte i = 0; i < 8; i++)  LEDS_run_pal(z,i,deckSelected, deck[0].run.leds);
			else if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_48_ROTATE ) 	for (byte z = 6; z < 8; z++) for (byte i = 0; i < 8; i++)  LEDS_run_FX_rotate(z,i,deckSelected, deck[0].run.leds);

			

			// Save Layers
			else if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_SAVE_ALPHA )     {  LEDS_RUN_save_saved_layer(deckSelected,0); }       
			else if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_RUN_ALPHA )      {  LEDS_RUN_MIX_saved_layer(deckSelected,0) ;  }       
			else if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_SAVE_BETA )       {  LEDS_RUN_save_saved_layer(deckSelected,1); }  
			else if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_RUN_BETA )       {  LEDS_RUN_MIX_saved_layer(deckSelected,1) ;  }       
			else if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_SAVE_GAMMA )     {  LEDS_RUN_save_saved_layer(deckSelected,2); }  
			else if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_RUN_GAMMA )      {  LEDS_RUN_MIX_saved_layer(deckSelected,2) ;  }       
			else if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_SAVE_OMEGA )     {  LEDS_RUN_save_saved_layer(deckSelected,3); }  
			else if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_RUN_OMEGA )      {  LEDS_RUN_MIX_saved_layer(deckSelected,3) ;  }       
			else if ( deck[deckSelected].cfg.layer.select[layer] ==_M_LAYER_CLEAR)     		{  deck[0].run.leds(deck[deckSelected].cfg.layer.clear_start_led, deck[deckSelected].cfg.layer.clear_start_led+ deck[deckSelected].cfg.layer.clear_Nr_leds -1 ).fadeToBlackBy(255); }       

				// (deck[deckSelected].cfg.layer.save_startLed[1], deck[deckSelected].cfg.layer.save_NrLeds[1] ) 


			//debugMe(layer);
			
			
		}


	} 


}

//leds_config ;
// ****************************** Load DEfault Conf
 
void LEDS_init_config(uint8_t selected_Deck)
{
	for(uint8_t Form_Part_No = 0 ;Form_Part_No < NR_FORM_PARTS; Form_Part_No++)
		{
			deck[selected_Deck].cfg.form_cfg[Form_Part_No] 		= {LEDS_DEF_START_LED,LEDS_DEF_NR_LEDs};
			deck[selected_Deck].cfg.form_fx_pal[Form_Part_No] 	= {LEDS_DEF_MAX_BRI,LEDS_DEF_PAL_START_INDEX,LEDS_DEF_PAL_SPEED,LEDS_DEF_PAL_COMPRESSION};
			
			deck[selected_Deck].cfg.form_fx_modify[Form_Part_No] 		= {LEDS_DEF_MODIFY_ROTATEFIXED};
			deck[selected_Deck].cfg.form_fx_fft[Form_Part_No] 	= {LEDS_DEF_MAX_BRI,LEDS_DEF_FFT_OFFSET,LEDS_DEF_FFT_EXTEND};
		}
	for(uint8_t Form_Part_No = 0 ;Form_Part_No < _M_NR_FORM_BYTES_; Form_Part_No++)
	{
		deck[selected_Deck].run.form_fx_fft[Form_Part_No] 	=   {0};	

		deck[selected_Deck].cfg.form_fx_pal_singles[Form_Part_No] 	= {LEDS_DEF_COLOR_SELECT_RAINBOW_STRIPED_PAL,MIX_ADD,LEDS_DEF_BIN_NO_BIN,LEDS_DEF_BIN_NO_BIN,LEDS_DEF_BIN_NO_BIN,LEDS_DEF_MAX_BRI};
		deck[selected_Deck].cfg.form_fx_fft_signles[Form_Part_No] 	= {MIX_ADD,LEDS_DEF_BIN_NO_BIN,LEDS_DEF_BIN_NO_BIN,COLOR_RGB,LEDS_DEF_MAX_BRI};
		deck[selected_Deck].cfg.form_fx_modify_bytes[Form_Part_No] 		= {LEDS_DEF_MODIFY_ROTATEFULLFRAMES,LEDS_DEF_BIN_NO_BIN};
	}

	for(uint8_t Form_Part_No = 0 ;Form_Part_No < NR_FX_PARTS; Form_Part_No++)
		{	
			deck[selected_Deck].fx1_cfg.form_fx_glitter[Form_Part_No] 		= {LEDS_DEF_GLITTER_VAL};
			deck[selected_Deck].fx1_cfg.form_fx_dots[Form_Part_No] 			= {LEDS_DEF_DOTS_NR,LEDS_DEF_DOTS_SPEED,LEDS_DEF_DOTS_PAL_SPEED};
			
			
			deck[selected_Deck].fx1_cfg.form_fx_meteor[Form_Part_No] 		= {2,40,0};
			
			deck[selected_Deck].fx1_cfg.form_fx_shim[Form_Part_No] 			= {6,5,7,0};

			

			deck[selected_Deck].run.form_fx_dots[Form_Part_No] 				= {0};
			deck[selected_Deck].run.form_fx_eyes[Form_Part_No] 				= {0,0};

			deck[selected_Deck].fx1_cfg.form_fx_clock[Form_Part_No] 	= {26,255,0};
			

		}
	
	for(uint8_t Form_Part_No = 0 ;Form_Part_No < NR_FX_BYTES; Form_Part_No++)
	{
		deck[selected_Deck].fx1_cfg.form_fx1[Form_Part_No] 		= {MIX_ADD,LEDS_DEF_MAX_BRI,LEDS_DEF_FX1_FADE,LEDS_DEF_BIN_NO_BIN,LEDS_DEF_BIN_NO_BIN,LEDS_DEF_MAX_BRI};
		deck[selected_Deck].fx1_cfg.form_fx_shim_bytes[Form_Part_No] 	= {LEDS_DEF_COLOR_SELECT_RAINBOW_PAL,MIX_ADD,255,255,255};
		
		deck[selected_Deck].fx1_cfg.form_fx_eyes_bytes[Form_Part_No] 			= {LEDS_DEF_COLOR_SELECT_WHITE,MIX_ADD,LEDS_DEF_MAX_BRI,LEDS_DEF_EYES_ON_FRAMES,LEDS_DEF_EYES_WIDTH,LEDS_DEF_EYES_SPACE,LEDS_DEF_BIN_NO_BIN,LEDS_DEF_BIN_NO_BIN,LEDS_DEF_EYES_FADEVAL,LEDS_DEF_EYES_PAUSE_FRAMES,LEDS_DEF_MAX_BRI };
		deck[selected_Deck].fx1_cfg.form_fx_glitter_bytes[Form_Part_No] 		= {LEDS_DEF_COLOR_SELECT_RAINBOW_PAL,LEDS_DEF_MAX_BRI,LEDS_DEF_BIN_NO_BIN};
		deck[selected_Deck].fx1_cfg.form_fx_dots_bytes[Form_Part_No] 			= {LEDS_DEF_COLOR_SELECT_RAINBOW_PAL,255};

		deck[selected_Deck].fx1_cfg.form_fx_meteor_bytes[Form_Part_No] 		= {254,255,255,255};
		deck[selected_Deck].fx1_cfg.form_fx_strobe_bytes[Form_Part_No] 		= {254,MIX_ADD,255,1,5,255,255};
		deck[selected_Deck].run.form_fx_strobe[Form_Part_No] 			= {0};

		deck[selected_Deck].fx1_cfg.form_fx_fire_bytes[Form_Part_No] 	= {26,MIX_ADD,255,55,120,255,255};
		deck[selected_Deck].fx1_cfg.form_fx_clock_bytes[Form_Part_No] 	= {MIX_ADD,255};
	}
	

	for(uint8_t layerNo = 0 ;layerNo < NO_OF_SAVE_LAYERS; layerNo++)
		{
			deck[selected_Deck].cfg.layer.save_lvl[layerNo] 		= 255;
			deck[selected_Deck].cfg.layer.save_mix[layerNo]			= MIX_ADD;
			deck[selected_Deck].cfg.layer.save_NrLeds[layerNo]		= led_cfg.NrLeds;
			deck[selected_Deck].cfg.layer.save_startLed[layerNo]	= 0;

		}
		deck[selected_Deck].cfg.layer.clear_start_led = 0 ;
		deck[selected_Deck].cfg.layer.clear_Nr_leds = led_cfg.NrLeds ;


		deck[selected_Deck].cfg.form_cfg[0] = {0,200};


}


void LEDS_load_default_play_conf()
{
	
	//deck[0].cfg.confname 							= String("def");
	deck[0].cfg.led_master_cfg.bri					= 240;
	deck[0].cfg.led_master_cfg.r					= 255;
	deck[0].cfg.led_master_cfg.g					= 255;
	deck[0].cfg.led_master_cfg.b					= 255;
	deck[0].cfg.led_master_cfg.pal_bri				= 255;
	deck[0].cfg.led_master_cfg.pal_fps     		   	= 25;
	
	deck[0].cfg.fft_config.Scale = 0;
	deck[0].cfg.fft_config.viz_fps = 1;
	deck[0].cfg.fft_config.fftAutoMin = 11;
	deck[0].cfg.fft_config.fftAutoMax = 240;

	LEDS_init_config(0);
	

	//uint8_t strip_no = 0;
				
/*
	part[strip_no].start_led = 0;

	part[strip_no].nr_leds = MAX_NUM_LEDS;
				
	part[strip_no].index_start = 0;
	part[strip_no].index_add = 64; 	
	part[strip_no].index_add_pal = 32;
	part[strip_no].fft_offset = 0;
				*/

	//bitWrite(strip_menu[0][_M_STRIP_],0, true);





	bitWrite(deck[0].cfg.form_menu_pal[0][_M_FORM_PAL_RUN],0, true);
	bitWrite(deck[0].cfg.form_menu_fft[0][_M_FORM_FFT_RUN],0, true);
	//bitWrite(form_menu[0][_M_AUDIO_SUB_FROM_FFT],0, false);




	
	uint8_t bin = 0; // loe bin 
	bitWrite(deck[0].cfg.fft_config.fft_menu[0], bin, false);			// RED
	bitWrite(deck[0].cfg.fft_config.fft_menu[1], bin, false);			// GREEN
	bitWrite(deck[0].cfg.fft_config.fft_menu[2], bin, false);			// BLUE

	bitWrite(deck[0].cfg.fft_config.fft_menu_bri, 			bin, false);
	bitWrite(deck[0].cfg.fft_config.fft_bin_autoTrigger,	bin, true);
	bitWrite(deck[0].cfg.fft_config.fft_menu_fps, 			bin, false);

	
	bin++;  // bin1

	bitWrite(deck[0].cfg.fft_config.fft_menu[0], bin, true);
	bitWrite(deck[0].cfg.fft_config.fft_menu[1], bin, false);
	bitWrite(deck[0].cfg.fft_config.fft_menu[2], bin, false);

	bitWrite(deck[0].cfg.fft_config.fft_menu_bri, 			bin, false);
	bitWrite(deck[0].cfg.fft_config.fft_bin_autoTrigger,	bin, true);
	bitWrite(deck[0].cfg.fft_config.fft_menu_fps, 			bin, false);


	bin++; // bin2

	bitWrite(deck[0].cfg.fft_config.fft_menu[0], bin, false);
	bitWrite(deck[0].cfg.fft_config.fft_menu[1], bin, false);
	bitWrite(deck[0].cfg.fft_config.fft_menu[2], bin, false);

	bitWrite(deck[0].cfg.fft_config.fft_menu_bri, 			bin, false);
	bitWrite(deck[0].cfg.fft_config.fft_bin_autoTrigger,	bin, true);
	bitWrite(deck[0].cfg.fft_config.fft_menu_fps, 			bin, false);

	bin++; // bin3

	bitWrite(deck[0].cfg.fft_config.fft_menu[0], bin, false);
	bitWrite(deck[0].cfg.fft_config.fft_menu[1], bin, true);
	bitWrite(deck[0].cfg.fft_config.fft_menu[2], bin, false);

	bitWrite(deck[0].cfg.fft_config.fft_menu_bri, 			bin, false);
	bitWrite(deck[0].cfg.fft_config.fft_bin_autoTrigger,	bin, true);
	bitWrite(deck[0].cfg.fft_config.fft_menu_fps, 			bin, false);			

	bin++; // bin4

	bitWrite(deck[0].cfg.fft_config.fft_menu[0], bin, false);
	bitWrite(deck[0].cfg.fft_config.fft_menu[1], bin, false);
	bitWrite(deck[0].cfg.fft_config.fft_menu[2], bin, false);

	bitWrite(deck[0].cfg.fft_config.fft_menu_bri, 			bin, false);
	bitWrite(deck[0].cfg.fft_config.fft_bin_autoTrigger,	bin, true);
	bitWrite(deck[0].cfg.fft_config.fft_menu_fps, 			bin, false);

	bin++; // bin5

	bitWrite(deck[0].cfg.fft_config.fft_menu[0], bin, false);
	bitWrite(deck[0].cfg.fft_config.fft_menu[1], bin, false);
	bitWrite(deck[0].cfg.fft_config.fft_menu[2], bin, false);

	bitWrite(deck[0].cfg.fft_config.fft_menu_bri, 			bin, false);
	bitWrite(deck[0].cfg.fft_config.fft_bin_autoTrigger,	bin, true);
	bitWrite(deck[0].cfg.fft_config.fft_menu_fps, 			bin, false);

	bin++; // bin6

	bitWrite(deck[0].cfg.fft_config.fft_menu[0], bin, false);
	bitWrite(deck[0].cfg.fft_config.fft_menu[1], bin, false);
	bitWrite(deck[0].cfg.fft_config.fft_menu[2], bin, true);

	bitWrite(deck[0].cfg.fft_config.fft_menu_bri, 			bin, false);
	bitWrite(deck[0].cfg.fft_config.fft_bin_autoTrigger,	bin, true);
	bitWrite(deck[0].cfg.fft_config.fft_menu_fps, 			bin, false);

	bin++; // bin7

	bitWrite(deck[0].cfg.fft_config.fft_menu[0], bin, false);
	bitWrite(deck[0].cfg.fft_config.fft_menu[1], bin, false);
	bitWrite(deck[0].cfg.fft_config.fft_menu[2], bin, false);

	bitWrite(deck[0].cfg.fft_config.fft_menu_bri, 			bin, false);
	bitWrite(deck[0].cfg.fft_config.fft_bin_autoTrigger,	bin, true);
	bitWrite(deck[0].cfg.fft_config.fft_menu_fps, 			bin, false);



	deck[0].cfg.fft_config.fft_bin_autoTrigger = 255;
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
				case 1: FastLED.addLeds<APA102,LED_DATA_PIN ,  LED_CLK_PIN, BGR,DATA_RATE_MHZ(1 )>	(deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("Mode Mix-Mirror - APA102 leds added on  DATA1+CLK"); break; 
				case 2: FastLED.addLeds<APA102,LED_DATA_PIN ,  LED_CLK_PIN, BGR,DATA_RATE_MHZ(2 )>	(deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("Mode Mix-Mirror - APA102 leds added on  DATA1+CLK"); break; 
				case 4: FastLED.addLeds<APA102,LED_DATA_PIN ,  LED_CLK_PIN, BGR,DATA_RATE_MHZ(4 )>	(deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("Mode Mix-Mirror - APA102 leds added on  DATA1+CLK"); break; 
				case 8: FastLED.addLeds<APA102,LED_DATA_PIN ,  LED_CLK_PIN, BGR,DATA_RATE_MHZ(8 )>	(deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("Mode Mix-Mirror - APA102 leds added on  DATA1+CLK"); break; 
				case 12: FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(12 )>	(deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("Mode Mix-Mirror - APA102 leds added on  DATA1+CLK"); break; 
				case 16: FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(16 )>	(deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("Mode Mix-Mirror - APA102 leds added on  DATA1+CLK"); break; 
				case 24: FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(24 )>	(deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("Mode Mix-Mirror - APA102 leds added on  DATA1+CLK"); break; 
				default: FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(2 )>	(deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("Mode Mix-Mirror - APA102 leds added on  DATA1+CLK"); break; 
			}

			if(get_bool(DATA3_ENABLE)) {FastLED.addLeds<WS2812, LED_DATA_3_PIN, GRB>			(deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); 	debugMe("WS2812 leds added on DATA3");}
			if(get_bool(DATA4_ENABLE)) {FastLED.addLeds<SK6822, LED_DATA_4_PIN, GRB>			(deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); 	debugMe("SK6822 leds added on DATA4");}
		break;

		case 1:
			debugMe("APA102 mode line");
			if(get_bool(DATA1_ENABLE)) 	switch(led_cfg.apa102data_rate)
			{
				case 1: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(1)>(deck[0].run.leds,led_cfg.DataStart_leds[0]  , led_cfg.DataNR_leds[0]).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+CLK"); } break;
				case 2: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(2)>(deck[0].run.leds,led_cfg.DataStart_leds[0]  , led_cfg.DataNR_leds[0]).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+CLK"); } break;
				case 4: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(4)>(deck[0].run.leds,led_cfg.DataStart_leds[0]  , led_cfg.DataNR_leds[0]).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+CLK"); } break;
				case 8: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(8)>(deck[0].run.leds,led_cfg.DataStart_leds[0]  , led_cfg.DataNR_leds[0]).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+CLK"); } break;
				case 12: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(12)>(deck[0].run.leds,led_cfg.DataStart_leds[0]  , led_cfg.DataNR_leds[0]).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+CLK"); } break;
				case 16: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(16)>(deck[0].run.leds,led_cfg.DataStart_leds[0]  , led_cfg.DataNR_leds[0]).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+CLK"); } break;
				case 24: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(24)>(deck[0].run.leds,led_cfg.DataStart_leds[0]  , led_cfg.DataNR_leds[0]).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+CLK"); } break;
				default: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(2)>(deck[0].run.leds,led_cfg.DataStart_leds[0]  , led_cfg.DataNR_leds[0]).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+CLK"); } break;

			}
			if(get_bool(DATA3_ENABLE)) switch(led_cfg.apa102data_rate)
			{
				case 1: {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(1)>(deck[0].run.leds, led_cfg.DataStart_leds[2]  , led_cfg.DataStart_leds[2] ).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3+D4CLK"); } break;
				case 2: {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(2)>(deck[0].run.leds, led_cfg.DataStart_leds[2]  , led_cfg.DataStart_leds[2] ).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3+D4CLK"); } break;
				case 4: {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(4)>(deck[0].run.leds, led_cfg.DataStart_leds[2]  , led_cfg.DataStart_leds[2] ).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3+D4CLK"); } break;
				case 8: {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(8)>(deck[0].run.leds, led_cfg.DataStart_leds[2]  , led_cfg.DataStart_leds[2] ).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3+D4CLK"); } break;
				case 12: {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(12)>(deck[0].run.leds, led_cfg.DataStart_leds[2]  , led_cfg.DataStart_leds[2] ).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3+D4CLK"); } break;
				case 16: {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(16)>(deck[0].run.leds, led_cfg.DataStart_leds[2]  , led_cfg.DataStart_leds[2] ).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3+D4CLK"); } break;
				case 24: {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(24)>(deck[0].run.leds, led_cfg.DataStart_leds[2]  , led_cfg.DataStart_leds[2] ).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3+D4CLK"); } break;
				default: {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(2)>(deck[0].run.leds, led_cfg.DataStart_leds[2]  , led_cfg.DataStart_leds[2] ).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3+D4CLK"); } break;
			}
			
		break;
		case 2:
			debugMe("APA102 mode Mirror");
			switch(led_cfg.apa102data_rate)
			{	
				case 1:
					if(get_bool(DATA1_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_PIN ,   LED_CLK_PIN,    BGR,DATA_RATE_MHZ(1)>(deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+DATA2(clk)");}
					if(get_bool(DATA3_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(1)>(deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3-DATA4(clk)");}
				break;
				case 2:
					if(get_bool(DATA1_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_PIN ,   LED_CLK_PIN,    BGR,DATA_RATE_MHZ(2)>(deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+DATA2(clk)");}
					if(get_bool(DATA3_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(2)>(deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3-DATA4(clk)");}
				break;
				case 4:
					if(get_bool(DATA1_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(4)>(deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+DATA2(clk)");}
					if(get_bool(DATA3_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(4)>(deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3-DATA4(clk)");}
				break;
				case 8:
					if(get_bool(DATA1_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(8)>(deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+DATA2(clk)");}
					if(get_bool(DATA3_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(8)>(deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3-DATA4(clk)");}
				break;
				case 12:
					if(get_bool(DATA1_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(12)>(deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+DATA2(clk)");}
					if(get_bool(DATA3_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(12)>(deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3-DATA4(clk)");}
				break;
				case 16:
					if(get_bool(DATA1_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(16)>(deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+DATA2(clk)");}
					if(get_bool(DATA3_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(16)>(deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3-DATA4(clk)");}
				break;
				case 24:
					if(get_bool(DATA1_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(24)>(deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+DATA2(clk)");}
					if(get_bool(DATA3_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(24)>(deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3-DATA4(clk)");}
				break;
				default :
					if(get_bool(DATA1_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(2)>(deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA1+DATA2(clk)");}
					if(get_bool(DATA3_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_3_PIN , LED_DATA_4_PIN, BGR,DATA_RATE_MHZ(2)>(deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); debugMe("APA102 leds added on  DATA3-DATA4(clk)");}
				break;

			}
		break; 
		case 3:
			debugMe("Mode LINE: WS2812b leds added on  DATA1 to DATA4");
			if(get_bool(DATA1_ENABLE)) {FastLED.addLeds<WS2812,LED_DATA_PIN  , GRB>(deck[0].run.leds, led_cfg.DataStart_leds[0] , led_cfg.DataNR_leds[0]).setCorrection(TypicalLEDStrip); debugMe(" DATA1 on");}
			if(get_bool(DATA2_ENABLE)) {FastLED.addLeds<WS2812,LED_CLK_PIN   , GRB>(deck[0].run.leds, led_cfg.DataStart_leds[1] , led_cfg.DataNR_leds[1]).setCorrection(TypicalLEDStrip); debugMe(" DATA2 on");}
			if(get_bool(DATA3_ENABLE)) {FastLED.addLeds<WS2812,LED_DATA_3_PIN, GRB>(deck[0].run.leds, led_cfg.DataStart_leds[2] , led_cfg.DataNR_leds[2]).setCorrection(TypicalLEDStrip); debugMe(" DATA3 on");}
			if(get_bool(DATA4_ENABLE)) {FastLED.addLeds<WS2812,LED_DATA_4_PIN, GRB>(deck[0].run.leds, led_cfg.DataStart_leds[3] , led_cfg.DataNR_leds[3]).setCorrection(TypicalLEDStrip); debugMe(" DATA4 on");}
		break;
		case 4:
		debugMe("ws2812 mode Mirror");
			if(get_bool(DATA1_ENABLE)) {FastLED.addLeds<WS2812,LED_DATA_PIN  , GRB>(deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); }
			if(get_bool(DATA2_ENABLE)) {FastLED.addLeds<WS2812,LED_CLK_PIN   , GRB>(deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); }
			if(get_bool(DATA3_ENABLE)) {FastLED.addLeds<WS2812,LED_DATA_3_PIN, GRB>(deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); }
			if(get_bool(DATA4_ENABLE)) {FastLED.addLeds<WS2812,LED_DATA_4_PIN, GRB>(deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip);}
		break;
		
		case 5:
		debugMe("mix mode Line");

			if(get_bool(DATA1_ENABLE)) switch(led_cfg.apa102data_rate)
			{	case 1: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(1)>(deck[0].run.leds, led_cfg.DataStart_leds[0] ,  uint16_t(constrain(led_cfg.DataNR_leds[0], 0,led_cfg.NrLeds - led_cfg.DataStart_leds[0] )) ).setCorrection(TypicalLEDStrip); debugMe("Mode_LINE: APA102 leds added on  DATA1+CLK");}break;
				case 2: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(2)>(deck[0].run.leds, led_cfg.DataStart_leds[0] ,  uint16_t(constrain(led_cfg.DataNR_leds[0], 0,led_cfg.NrLeds - led_cfg.DataStart_leds[0] )) ).setCorrection(TypicalLEDStrip); debugMe("Mode_LINE: APA102 leds added on  DATA1+CLK");}break;
				case 4: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(4)>(deck[0].run.leds, led_cfg.DataStart_leds[0] ,  uint16_t(constrain(led_cfg.DataNR_leds[0], 0,led_cfg.NrLeds - led_cfg.DataStart_leds[0] )) ).setCorrection(TypicalLEDStrip); debugMe("Mode_LINE: APA102 leds added on  DATA1+CLK");}break;
				case 8: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(8)>(deck[0].run.leds, led_cfg.DataStart_leds[0] ,  uint16_t(constrain(led_cfg.DataNR_leds[0], 0,led_cfg.NrLeds - led_cfg.DataStart_leds[0] )) ).setCorrection(TypicalLEDStrip); debugMe("Mode_LINE: APA102 leds added on  DATA1+CLK");}break;
				case 12: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(12)>(deck[0].run.leds, led_cfg.DataStart_leds[0] ,  uint16_t(constrain(led_cfg.DataNR_leds[0], 0,led_cfg.NrLeds - led_cfg.DataStart_leds[0] )) ).setCorrection(TypicalLEDStrip); debugMe("Mode_LINE: APA102 leds added on  DATA1+CLK");}break;
				case 16: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(16)>(deck[0].run.leds, led_cfg.DataStart_leds[0] ,  uint16_t(constrain(led_cfg.DataNR_leds[0], 0,led_cfg.NrLeds - led_cfg.DataStart_leds[0] )) ).setCorrection(TypicalLEDStrip); debugMe("Mode_LINE: APA102 leds added on  DATA1+CLK");}break;
				case 24: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(24)>(deck[0].run.leds, led_cfg.DataStart_leds[0] ,  uint16_t(constrain(led_cfg.DataNR_leds[0], 0,led_cfg.NrLeds - led_cfg.DataStart_leds[0] )) ).setCorrection(TypicalLEDStrip); debugMe("Mode_LINE: APA102 leds added on  DATA1+CLK");}break;
				default: {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(2)>(deck[0].run.leds, led_cfg.DataStart_leds[0] ,  uint16_t(constrain(led_cfg.DataNR_leds[0], 0,led_cfg.NrLeds - led_cfg.DataStart_leds[0] )) ).setCorrection(TypicalLEDStrip); debugMe("Mode_LINE: APA102 leds added on  DATA1+CLK");}break;
			}
			if(get_bool(DATA3_ENABLE)) {FastLED.addLeds<WS2812, LED_DATA_3_PIN, GRB>           (deck[0].run.leds, led_cfg.DataStart_leds[2] , uint16_t(constrain(led_cfg.DataNR_leds[2], 0,led_cfg.NrLeds - led_cfg.DataStart_leds[2] )) ).setCorrection(TypicalLEDStrip); 	debugMe("WS2812 leds added on DATA3");}
			if(get_bool(DATA4_ENABLE)) {FastLED.addLeds<SK6822, LED_DATA_4_PIN, GRB>           (deck[0].run.leds, led_cfg.DataStart_leds[3] , uint16_t(constrain(led_cfg.DataNR_leds[3], 0,led_cfg.NrLeds - led_cfg.DataStart_leds[3] )) ).setCorrection(TypicalLEDStrip); 	debugMe("SK6822 leds added on DATA4");}
		break;
		default:
			if(get_bool(DATA1_ENABLE)) {FastLED.addLeds<APA102,LED_DATA_PIN , LED_CLK_PIN, BGR,DATA_RATE_MHZ(2)>(deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); 	debugMe("APA102 leds added on  DATA1+CLK");}
			if(get_bool(DATA3_ENABLE)) {FastLED.addLeds<WS2812, LED_DATA_3_PIN, GRB>           (deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); 	debugMe("WS2812 leds added on DATA3");}
			if(get_bool(DATA4_ENABLE)) {FastLED.addLeds<SK6822, LED_DATA_4_PIN,GRB>            (deck[0].run.leds, led_cfg.NrLeds).setCorrection(TypicalLEDStrip); 	debugMe("SK6822 leds added on DATA4");}
		break;
			
	}
	debugMe("LED_MODE = " + String(led_cfg.ledMode));

	for (int i = 0; i < NR_PALETTS; i++) 
	{
//#ifdef BLEND_PATTERN
		//for ( int i = 0 ; i < NR_STRIPS ; i++)
//		LEDS_pal_work[i] = &LEDS_pal_cur[i];
//#else
//		LEDS_pal_work[i] = &LEDS_pal_cur[i];
//#endif
	}
	//LEDS_pal_cur[0] = pal_red_green;
	//LEDS_pal_cur[1] = pal_red_green;


	//deck[0].cfg.led_master_cfg.bri = led_cfg.startup_bri;				// set the bri to the startup bri

	uint8_t core = xPortGetCoreID();
    debugMe("Main code running on core " + String(core));

    // -- Create the FastLED show task
    xTaskCreatePinnedToCore(FastLEDshowTask, "FastLEDshowTask", 2048, NULL, 2, &FastLEDshowTaskHandle, FASTLED_SHOW_CORE);

	 LEDS_load_default_play_conf();	
	if (led_cfg.bootCFG != 16) FS_play_conf_read(led_cfg.bootCFG ,&deck[0].cfg ,&deck[0].fx1_cfg ) ;	
;	

	LEDS_pal_reset_index();



	led_cnt.PotBriLast = analogRead(POTI_BRI_PIN) / ANALOG_IN_DEVIDER;			//get the initial potti status so that when we load the config the possis wont override it.
	led_cnt.PotFPSLast = analogRead(POTI_FPS_PIN) / ANALOG_IN_DEVIDER;

	deck[0].run.fft.update_time = micros();
	//fft_led_cfg.viz_fps = DEF_VIZ_UPDATE_TIME_FPS ;

	led_cfg.confSwitch_time = micros() +  play_conf_time_min[led_cfg.Play_Nr] * MICROS_TO_MIN  ;


	//static deck_cfg_struct mem_confs_real[5] ;
	//mem_confs[0] = &mem_confs_real[0] ;


	debugMe("end LEDS setup");


}



void LEDS_loop()
{	// the main led loop

	unsigned long currentT = micros();

	

	#ifndef ARTNET_DISABLED
		if (get_bool(ARTNET_RECIVE)) WiFi_artnet_recive_loop();  //  fetshing data 
	#endif


	// Calculate the real FPS 
	// increment the led_cfg.framecounter by one on each pass
	// reset every second.
	if (currentT > led_cfg.framecounterUpdateTime ) 
	{
		led_cfg.framecounterUpdateTime = currentT + 1000000;
		led_cfg.realfps = led_cfg.framecounter;					// frps = framecounter
		led_cfg.framecounter = 0	;							// reset framecounter to 0
	}


	if (currentT > led_cfg.update_time  && !get_bool(ARTNET_RECIVE) )
	{
		
		{	
			//debugMe(String(ESP.getFreeHeap()));

			



			led_cfg.framecounter++;   // increment the framecounter by one to calculate the fps
			
			uint8_t DeckNo = 0;
			//debugMe("IN LED LOOP - disabled fft");
			if(deck[DeckNo].cfg.fft_config.fft_menu_fps == 0)
				led_cfg.update_time = currentT + (1000000 / deck[DeckNo].cfg.led_master_cfg.pal_fps);
			else
				led_cfg.update_time = currentT + (1000000 / map( deck[DeckNo].run.fft.fft_color_fps,  0 ,255 , deck[DeckNo].cfg.led_master_cfg.pal_fps, MAX_PAL_FPS )) ;     // if we are adding FFT data to FPS speed 

			//deck[0].run.leds[0,led_cfg.NrLeds].fadeToBlackBy(255);				// fade the whole led array to black so that we can add from different sources amd mix it up!
			tpm_fx.fadeLedArray(deck[0].run.leds, 0, led_cfg.NrLeds, 255 );

			if (LEDS_checkIfAudioSelected()) 
			{
				LEDS_FFT_process();  // Get the color from the FFT data
				LEDS_FFT_history_run(deck[DeckNo].run.fft.GlobalColor_result, DeckNo);
				
				yield();
			}


			LEDS_run_layers(0);


			/* 
			while (FFT_fifo.count() >= 7)		// sanity check to keep the queue down if disabled free up memory
			{
				uint8_t buffer = FFT_fifo.dequeue();
				debugMe("dequing overflow");
				buffer = 0;
			} 
			*/

		}


			LEDS_G_pre_show_processing();
			yield();

			if (get_bool(FADE_INOUT))    // Fade leds if we are loading or saving so that we have a smooth tansition.
			{
					if (!get_bool(FADE_INOUT_FADEBACK))
					{
						led_cfg.fade_inout_val = constrain( (led_cfg.fade_inout_val + 5), 0 , 255);
					}
					else  led_cfg.fade_inout_val = constrain( (led_cfg.fade_inout_val - 5), 0 , 255);

					tpm_fx.fadeLedArray(deck[0].run.leds,0,led_cfg.NrLeds,led_cfg.fade_inout_val );

			}

			if (!get_bool(PAUSE_DISPLAY)) LEDS_show();			// THE MAIN SHOW TASK Eport LED data to Strips

			if (get_bool(FADE_INOUT))
			{
					if (led_cfg.fade_inout_val == 255)
					{
							if (get_bool(FADE_INOUT_SAVE)) FS_play_conf_write(led_cfg.next_config_loadsave) ;
								
							else 
							{
								FS_play_conf_read(led_cfg.next_config_loadsave ,&deck[0].cfg, &deck[0].fx1_cfg  );
								LEDS_pal_reset_index(); 
							}

						write_bool(FADE_INOUT_FADEBACK, true);
						
						osc_StC_Load_confname_Refresh( led_cfg.next_config_loadsave);

					}
					else if (led_cfg.fade_inout_val == 0)  write_bool(FADE_INOUT, false);


			}



			yield();

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

		 if (get_bool(FFT_OSTC_VIZ) && currentT >= deck[0].run.fft.update_time ) 
		 {

			deck[0].run.fft.update_time = currentT + (1000000 / deck[0].cfg.fft_config.viz_fps);	 
			 
			 osc_StC_FFT_vizIt(); 
			 //debugMe("vizzit");


		

		}

		//FS_play_conf_loop();


	if (currentT > led_cfg.confSwitch_time && get_bool(SEQUENCER_ON) ) LEDS_seqencer_advance();







	//if (micros() > led_cfg.update_time ) {deck[0].cfg.led_master_cfg.pal_fps--; debugMe("To slow");}

	}


	
	

	
	//debugMe("leds loop end ", false);
	//debugMe(String(xPortGetCoreID()));
}



