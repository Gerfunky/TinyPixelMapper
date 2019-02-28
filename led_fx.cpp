#include <FastLED.h>
#include "led_fx.h"
#include "leds.h"
#include "tools.h"
extern led_cfg_struct led_cfg;
extern CRGBArray<MAX_NUM_LEDS> leds;
extern CRGBPalette16 LEDS_pal_cur[NR_PALETTS];	
extern CRGBArray<MAX_NUM_LEDS> led_FX_out;    // make a FX output array. 


extern CRGB ColorFrom_LONG_Palette(uint8_t pal, uint16_t longIndex, uint8_t brightness = 255, TBlendType blendType = LINEARBLEND); // made a new fuction to spread out the 255 index/color  pallet to 16*255 = 4080 colors
//extern CRGB ColorFromPalette( const TProgmemRGBPalette32& pal, uint8_t index, uint8_t brightness, TBlendType blendType);
//extern CRGB myColorFromPalette(boolean pallete, uint8_t index , uint8_t bri , boolean blend);
extern void LEDS_mix_led(CRGB *out_array, uint16_t led_nr, CRGB color, uint8_t mode = 0);

extern CRGB ColorFrom_SHORT_Palette(uint8_t pal, uint8_t index, uint8_t level , boolean blend);
//extern CRGB LEDS_get_color_longindex(uint8_t pal, uint16_t index, uint8_t level , boolean blend);




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
	byte heat[MAX_NUM_LEDS];
	uint8_t LEDS_FFT_get_fire_cooling()
	{
		return COOLING * 2;
	}
	uint8_t LEDS_FFT_get_fire_sparking()
	{
		return SPARKING  *2 ;

	}


void Fire2012WithPalette(uint16_t start_led, uint16_t Nr_leds, bool reversed, uint8_t pal, bool mirror, uint8_t level, boolean subtract, boolean mask, uint8_t mix_mode ) //, bool mirrored)
{

	
	uint8_t cooling = led_cfg.fire_cooling;
	uint8_t sparking = led_cfg.fire_sparking;

	/*if (true == get_bool(FFT_ENABLE))
	{
		cooling = LEDS_FFT_get_fire_cooling();
		sparking = LEDS_FFT_get_fire_sparking();

	} */
	uint16_t real_nr_leds = Nr_leds;
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
		for (int i = start_led; i < start_led + real_nr_leds ; i++) 
		{
			heat[i] = qsub8(heat[i], random8(0, ((cooling * 10) / NR_leds_M) + 2));
		}

		//for (int i =  NR_leds_M -1 ; i >= start_led ; i--)  {	heat[i] = qsub8(heat[i], random8(0, ((cooling * 10) / NR_leds_M) + 2)); }

		// Step 2.  Heat from each cell drifts 'up' and diffuses a little
		if (true == reversed)
		{
			for (int k = (start_led_M + NR_leds_M - 1); k >= (start_led_M + 2); k--) {
				heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
			}
			for (int k = (start_led ); k <= start_led_M ; k++ ) { heat[k] = (heat[k + 1] + heat[k + 2] + heat[k + 2]) / 3; }
		}
		else
		{
			for (int k = (start_led_M); k < (start_led_M + NR_leds_M - 3); k++) {
				heat[k] = (heat[k + 1] + heat[k + 2] + heat[k + 2]) / 3;
			}
			for (int k = (start_led_M); k > (start_led +  3); k--)  { 	heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;  	}
		}
		// Step 3.  Randomly ignite new 'sparks' of heat near the bottom
		// needs to spark at every strip / form start if selected
		if (true == reversed)
		{
			if (random8() < sparking) {
				int y = random8(7) + start_led_M;
				heat[y] = qadd8(heat[y], random8(160, 255));
				int x = random8(7) - start_led_M;
				heat[x] = qadd8(heat[x], random8(160, 255));
			}
		}
		else
		{
			if (random8() < sparking) {
				int y = -(random8(7)) + start_led_M + NR_leds_M - 1;
				heat[y] = qadd8(heat[y], random8(160, 255));
				int x = (random8(7)) + start_led;
				heat[x] = qadd8(heat[x], random8(160, 255));
			}
		}
		// Step 4.  Map from heat cells to LED colors
		for (int j = start_led; j < NR_leds_M + start_led_M; j++) 
		{

			// Scale the heat value from 0-255 down to 0-240
			// for best results with color palettes.
			byte colorindex = scale8(heat[j], 240);
			//CRGB color = HeatColor(heat[j]);
			CRGB color;

			color = ColorFrom_SHORT_Palette(pal,colorindex,level,LINEARBLEND); 
			/*
			switch(pal)
			{
				case 0: color = ColorFromPalette(LEDS_pal_cur[0], colorindex,level, LINEARBLEND);	break;
				case 1: color = ColorFromPalette(LEDS_pal_cur[1], colorindex,level, LINEARBLEND);	break;
				case 20: color = ColorFromPalette(RainbowColors_p, colorindex,level, LINEARBLEND);	break;
				case 21: color = ColorFromPalette(RainbowStripeColors_p, colorindex,level, LINEARBLEND);break;	
				case 22: color = ColorFromPalette(CloudColors_p, colorindex,level, LINEARBLEND);	break;
				case 23: color = ColorFromPalette(PartyColors_p, colorindex,level, LINEARBLEND);	break;
				case 24: color = ColorFromPalette(OceanColors_p, colorindex,level, LINEARBLEND);	break;
				case 25: color = ColorFromPalette(ForestColors_p, colorindex,level, LINEARBLEND);	break;
				case 26: color = ColorFromPalette(HeatColors_p, colorindex,level, LINEARBLEND);	break;
				case 27: color = ColorFromPalette(LavaColors_p, colorindex,level, LINEARBLEND);	break;
				case 28: color = ColorFromPalette(pal_red_green, colorindex,level, LINEARBLEND);	break;
				case 29: color = ColorFromPalette(pal_red_blue, colorindex,level, LINEARBLEND);	break;
				case 30: color = ColorFromPalette(pal_green_blue, colorindex,level, LINEARBLEND);	break;
				case 31: color = ColorFromPalette(pal_black_white_Narrow, colorindex,level, LINEARBLEND);break;	
				case 32: color = ColorFromPalette(pal_black_white_wide, colorindex,level, LINEARBLEND);	break;

				default: color = ColorFromPalette(LEDS_pal_cur[0], colorindex,level, LINEARBLEND);break;
			}
			*/

			
			int pixelnumber;
			/*if (reversed) {
			pixelnumber = (Nr_leds + start_led - 1) - j;
			}
			else */
			{
				pixelnumber = j;
			}
			
			LEDS_mix_led(leds, pixelnumber, color, mix_mode);

			
			//led_FX_out[pixelnumber] = color;
		}

	}
	else
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
			CRGB color;
			


			color = ColorFrom_SHORT_Palette(pal,colorindex,level,LINEARBLEND); 
			/*
			switch(pal)
			{
				case 0: color = ColorFromPalette(LEDS_pal_cur[0], colorindex,level, LINEARBLEND);	break;
				case 1: color = ColorFromPalette(LEDS_pal_cur[1], colorindex,level, LINEARBLEND);	break;
				case 20: color = ColorFromPalette(RainbowColors_p, colorindex,level, LINEARBLEND);	break;
				case 21: color = ColorFromPalette(RainbowStripeColors_p, colorindex,level, LINEARBLEND);break;	
				case 22: color = ColorFromPalette(CloudColors_p, colorindex,level, LINEARBLEND);	break;
				case 23: color = ColorFromPalette(PartyColors_p, colorindex,level, LINEARBLEND);	break;
				case 24: color = ColorFromPalette(OceanColors_p, colorindex,level, LINEARBLEND);	break;
				case 25: color = ColorFromPalette(ForestColors_p, colorindex,level, LINEARBLEND);	break;
				case 26: color = ColorFromPalette(HeatColors_p, colorindex,level, LINEARBLEND);	break;
				case 27: color = ColorFromPalette(LavaColors_p, colorindex,level, LINEARBLEND);	break;
				case 28: color = ColorFromPalette(pal_red_green, colorindex,level, LINEARBLEND);	break;
				case 29: color = ColorFromPalette(pal_red_blue, colorindex,level, LINEARBLEND);	break;
				case 30: color = ColorFromPalette(pal_green_blue, colorindex,level, LINEARBLEND);	break;
				case 31: color = ColorFromPalette(pal_black_white_Narrow, colorindex,level, LINEARBLEND);break;	
				case 32: color = ColorFromPalette(pal_black_white_wide, colorindex,level, LINEARBLEND);	break;

				default: color = ColorFromPalette(LEDS_pal_cur[0], colorindex,level, LINEARBLEND);break;
			} */
			
			
			int pixelnumber;
			/*if (reversed) {
				pixelnumber = (Nr_leds + start_led - 1) - j;
			}
			else */ 
			{
				pixelnumber = j;
			} 

			LEDS_mix_led(leds, pixelnumber, color, mix_mode);
			/*

			if(!mask)
			{
				if(!subtract)
				{
				leds[pixelnumber].red = 	qadd8( color.red , leds[pixelnumber].red);
				leds[pixelnumber].green = 	qadd8( color.green , leds[pixelnumber].green);
				leds[pixelnumber].blue =	qadd8( color.blue , leds[pixelnumber].blue);
				//leds[pixelnumber] = color;
				}
				else // subtract
				{
				leds[pixelnumber].red = 	qsub8( leds[pixelnumber].red ,color.red  );
				leds[pixelnumber].green = 	qsub8( leds[pixelnumber].green, color.green  );
				leds[pixelnumber].blue =	qsub8( leds[pixelnumber].blue, color.blue  );
				//leds[pixelnumber] = color;
				}


			}
			else //mask
			{

				
				leds[pixelnumber].red = 	map( leds[pixelnumber].red , 0, 255, 0, color.red   );	
				leds[pixelnumber].green = 	map( leds[pixelnumber].green , 0, 255, 0,color.green  );
				leds[pixelnumber].blue =	map( leds[pixelnumber].blue , 0, 255, 0, color.blue  );


			}
			*/
		}

	}
}


void LEDS_G_E_addGlitter(fract8 chanceOfGlitter, uint16_t *start_led, uint16_t *nr_leds)
{	// Glitter effect origional code from  FastLed library examples DemoReel100
	if (*nr_leds != 0 && (*nr_leds + *start_led <= MAX_NUM_LEDS))
	{
		// leds(*start_led,*start_led+*nr_leds).fadeToBlackBy(chanceOfGlitter/2);
		//leds(*start_led,*start_led+*nr_leds) =(CRGB::Black);
		if (random8() < chanceOfGlitter)
		{
			led_FX_out[*start_led + (random16(*nr_leds))] += CRGB::White;
		}

		// Serial.print("G");	
	}
}


void LEDS_G_E_addGlitterRainbow(fract8 chanceOfGlitter, uint16_t *start_led, uint16_t *nr_leds)
{	// Glitter effect origional code from  FastLed library examples DemoReel100
	if (*nr_leds != 0 && (*nr_leds + *start_led <= MAX_NUM_LEDS))
	{
		//leds(start_led,start_led+nr_leds).fadeToBlackBy(chanceOfGlitter);
		//leds(start_led,start_led+nr_leds) =(CRGB::Black);
		/*if (random8() < chanceOfGlitter / 3) led_FX_out[*start_led + (random16(*nr_leds))] += CRGB::Red;
		if (random8() < chanceOfGlitter / 3) led_FX_out[*start_led + (random16(*nr_leds))] += CRGB::Blue;
		if (random8() < chanceOfGlitter / 3) led_FX_out[*start_led + (random16(*nr_leds))] += CRGB::Green; */

		if (random8() < chanceOfGlitter) led_FX_out[*start_led + (random16(*nr_leds))] += CHSV(random8(), 255, random8());
		
		//if (random8() < chanceOfGlitter / 4) led_FX_out[*start_led + (random16(*nr_leds))] += CRGB::White;
	}
}

void LEDS_G_E_juggle(uint8_t nr_dots, uint16_t *start_led, uint16_t *nr_leds, uint8_t *jd_speed, boolean reversed)		// sine dots speed = BPM
{	// Make a dot  run  a sine wave over the leds normal speed = bpm additional leds = bpm +1
	// origional code from  FastLed library examples DemoReel100
	if (*nr_leds != 0 && (*nr_leds + *start_led <= MAX_NUM_LEDS))
	{
		byte dothue = 0;
		for (int i = 0; i < nr_dots; i++)
		{
			//if (reversed == true)	led_FX_out[beatsin16(i + *jd_speed, *start_led + *nr_leds - 1, *start_led)] |= CHSV(led_cfg.hue + dothue, 255, 255);
			//else
            					led_FX_out[beatsin16(i + *jd_speed, *start_led, *start_led + *nr_leds - 1)] |= CHSV(led_cfg.hue + dothue, 255, 255);
			dothue += (255 / nr_dots);
		}
		
	}
}

void LEDS_G_E_saw(uint8_t nr_dots, uint16_t *start_led, uint16_t *nr_leds, uint8_t *jd_speed, boolean reversed)  // Saw Dots that run in cirles in the form
{	// Make a dot  run  a SAW wave over the leds normal speed = bpm additional leds = bpm +1
	if (*nr_leds != 0 && (*nr_leds + *start_led <=MAX_NUM_LEDS))
	{
		byte dothue = 64;
		for (int i = 0; i < nr_dots; i++)
		{
			//led_FX_out[beatsin16(i + *jd_speed, *start_led, *start_led + *nr_leds - 1)] |= CHSV(led_cfg.hue + dothue, 255, 255);
			if (reversed == true) 	led_FX_out[map(beat16(i + *jd_speed/2),0 , 65535, *start_led + *nr_leds - 1, *start_led)] |= CHSV(led_cfg.hue + dothue, 255, 255);
			else					led_FX_out[map(beat16(i + *jd_speed/2), 0, 65535, *start_led , *start_led + *nr_leds - 1)] |= CHSV(led_cfg.hue + dothue, 255, 255);
			dothue += (255 / nr_dots);
		}

	}
}



void LEDS_G_E_Form_Fade_it(uint8_t fadyBy, uint16_t *Start_led, uint16_t *nr_leds)				// fade effect for form
{	// Fade effect 

	if (*nr_leds != 0 && (*nr_leds + *Start_led <= MAX_NUM_LEDS))
	{
		//leds(*Start_led, *Start_led + *nr_leds - 1).fadeToBlackBy(fadyBy);
        led_FX_out(*Start_led, *Start_led + *nr_leds - 1).fadeToBlackBy(fadyBy);
	}
}



void LEDS_FFT_running_dot(CRGB color_result, uint16_t *Start_led, uint16_t *number_of_leds, boolean dir, uint8_t jd_speed, uint8_t nr_dots)
{
	if (0 != *number_of_leds && (*number_of_leds + *Start_led <= MAX_NUM_LEDS) )
	{
		
		for (int i = 0; i < nr_dots; i++)
		{
			//leds[beatsin16(i + *jd_speed, *start_led, *start_led + *nr_leds - 1)] |= CHSV(led_cfg.hue + dothue, 255, 255);
			if (dir == true) 	led_FX_out[map(beat16(i + jd_speed),0 , 65535, *Start_led + *number_of_leds - 1, *Start_led)] = color_result;
			else				led_FX_out[map(beat16(i + jd_speed), 0, 65535, *Start_led, *Start_led + *number_of_leds - 1)] = color_result;
			
		}

	}

}




//------------- From mikeieee



void LEDS_G_E_shimmer(uint16_t StartLed, uint16_t NrLeds , uint8_t pal, uint8_t mix_mode,uint8_t level, boolean mirror, boolean blend, uint16_t xscale = 6 , uint16_t yscale = 5, uint8_t beater = 7) 
{          // A time (rather than loop) based demo sequencer. This gives us full control over the length of each sequence.

   static int16_t dist = random8();
	byte mirror_add = 0;

	if (mirror == true) 
	{
			
					
			if (isODDnumber(NrLeds) == true) 
			{
				mirror_add = 1; // dosmething
			}
	}
	/*	TBlendType currentBlendingTB;
		if (get_bool(BLEND_INVERT) == true)
				blend = !blend;
			if (blend == true)
				currentBlendingTB = LINEARBLEND;
			else
				currentBlendingTB = NOBLEND;
*/

	CRGB color;

  for(int i = StartLed ; i < StartLed + (NrLeds/(1+1*mirror_add)) ; i++)    // Just ONE loop to fill up the LED array as all of the pixels change.
  {                                     
    uint8_t index = inoise8(i*xscale, dist+i*yscale) % 255; 			 // Get a value from the noise function. I'm using both x and y axis. 
    
	color = ColorFrom_SHORT_Palette(pal,index,level,blend);
	
	/*
	switch(pal)
			{
				case 0: color = ColorFromPalette(LEDS_pal_cur[0], 		index,level, currentBlendingTB);	break;
				case 1: color = ColorFromPalette(LEDS_pal_cur[1], 		index,level, currentBlendingTB);	break;
				case 20: color = ColorFromPalette(RainbowColors_p, 		index,level, currentBlendingTB);	break;
				case 21: color = ColorFromPalette(RainbowStripeColors_p, index,level, currentBlendingTB);break;	
				case 22: color = ColorFromPalette(CloudColors_p, 		index,level, currentBlendingTB);	break;
				case 23: color = ColorFromPalette(PartyColors_p, 		index,level, currentBlendingTB);	break;
				case 24: color = ColorFromPalette(OceanColors_p, 		index,level, currentBlendingTB);	break;
				case 25: color = ColorFromPalette(ForestColors_p, 		index,level, currentBlendingTB);	break;
				case 26: color = ColorFromPalette(HeatColors_p, 		index,level, currentBlendingTB);	break;
				case 27: color = ColorFromPalette(LavaColors_p, 		index,level, currentBlendingTB);	break;
				case 28: color = ColorFromPalette(pal_red_green, 		index,level, currentBlendingTB);	break;
				case 29: color = ColorFromPalette(pal_red_blue, 		index,level, currentBlendingTB);	break;
				case 30: color = ColorFromPalette(pal_green_blue, 		index,level, currentBlendingTB);	break;
				case 31: color = ColorFromPalette(pal_black_white_Narrow, index,level, currentBlendingTB);break;	
				case 32: color = ColorFromPalette(pal_black_white_wide, index,level, currentBlendingTB);	break;

				default: color = ColorFromPalette(LEDS_pal_cur[0], index,level, LINEARBLEND);break;
			}
	*/
	
	LEDS_mix_led(leds, i, color, mix_mode);
	//led_FX_out[i] = ColorFromPalette(LEDS_pal_cur[pal], index, 255, currentBlendingTB);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.
  }
  
  //dist += beatsin8(beater,1,4);                                                // Moving along the distance (that random number we started out with). Vary it a bit with a sine wave.
 dist += beater;

} // shimmer()


 


void noise16_2(uint16_t StartLed, uint16_t NrLeds , uint8_t pal, boolean mirror, boolean blend = true) 
{  
    TBlendType currentBlendingTB;
    if (get_bool(BLEND_INVERT) == true)
			blend = !blend;
		if (blend == true)
			currentBlendingTB = LINEARBLEND;
		else
			currentBlendingTB = NOBLEND;

  //FastLED.setBrightness(255);// just moving along one axis = "lavalamp effect"

  uint8_t scale = 100;                                       // the "zoom factor" for the noise

  for (uint16_t i = StartLed; i < StartLed+NrLeds; i++) 
  {

    uint16_t shift_x = millis() / 10;                         // x as a function of time
    uint16_t shift_y = 0;

    uint32_t real_x = (i + shift_x) * scale;                  // calculate the coordinates within the noise field
    uint32_t real_y = (i + shift_y) * scale;                  // based on the precalculated positions
    uint32_t real_z = 4223;
    
    uint8_t noise = inoise16(real_x, real_y, real_z) >> 8;    // get the noise data and scale it down
    uint8_t trip = -4;
    uint8_t index = sin8(noise*trip);                            // map led color based on noise data
    uint8_t bri   = noise;



	led_FX_out[i] = ColorFromPalette(LEDS_pal_cur[0], index, bri, currentBlendingTB); 
/*switch(pal)
	{
		case 0: led_FX_out[i] = ColorFromPalette(LEDS_pal_cur[0], index, bri, currentBlendingTB);  break ;
		case 1: led_FX_out[i] = ColorFromPalette(LEDS_pal_cur[1], index, bri, currentBlendingTB);  break ;
		case 20: led_FX_out[i] = ColorFromPalette(RainbowColors_p, index, bri, currentBlendingTB);  break ; 
		case 21: led_FX_out[i] = ColorFromPalette(RainbowStripeColors_p, index, bri, currentBlendingTB);  break ; 
		case 22: led_FX_out[i] = ColorFromPalette(CloudColors_p, index, bri, currentBlendingTB);  break ; 
		case 23: led_FX_out[i] = ColorFromPalette(PartyColors_p, index, bri, currentBlendingTB);  break ; 
		case 24: led_FX_out[i] = ColorFromPalette(OceanColors_p, index, bri, currentBlendingTB);  break ; 
		case 25: led_FX_out[i] = ColorFromPalette(ForestColors_p, index, bri, currentBlendingTB);  break ; 
		case 26: led_FX_out[i] = ColorFromPalette(HeatColors_p, index, bri, currentBlendingTB);  break ; 
		case 27: led_FX_out[i] = ColorFromPalette(LavaColors_p, index, bri, currentBlendingTB);  break ; 
		case 28: led_FX_out[i] = ColorFromPalette(pal_red_green, index, bri, currentBlendingTB);  break ; 
		case 20: led_FX_out[i] = ColorFromPalette(pal_red_blue, index, bri, currentBlendingTB);  break ; 
		case 20: led_FX_out[i] = ColorFromPalette(pal_green_blue, index, bri, currentBlendingTB);  break ; 
		case 20: led_FX_out[i] = ColorFromPalette(pal_black_white_Narrow, index, bri, currentBlendingTB);  break ; 
		case 20: led_FX_out[i] = ColorFromPalette(pal_black_white_wide, index, bri, currentBlendingTB);  break ; 
		
/*
		case 20: color1 = ColorFromPalette(RainbowColors_p, 			indexC1 * 16, brightness , blendType); 	color2 = ColorFromPalette(RainbowColors_p, 			indexC2 * 16, brightness , blendType); break;
		case 21: color1 = ColorFromPalette(RainbowStripeColors_p, 	indexC1 * 16, brightness , blendType); 	color2 = ColorFromPalette(RainbowStripeColors_p, 	indexC2 * 16, brightness , blendType); break; 
		case 22: color1 = ColorFromPalette(CloudColors_p, 			indexC1 * 16, brightness , blendType); 	color2 = ColorFromPalette(CloudColors_p, 			indexC2 * 16, brightness , blendType); break; 
		case 23: color1 = ColorFromPalette(PartyColors_p, 			indexC1 * 16, brightness , blendType); 	color2 = ColorFromPalette(PartyColors_p, 			indexC2 * 16, brightness , blendType); break;
		case 24: color1 = ColorFromPalette(OceanColors_p, 			indexC1 * 16, brightness , blendType); 	color2 = ColorFromPalette(OceanColors_p, 			indexC2 * 16, brightness , blendType); break ;
		case 25: color1 = ColorFromPalette(ForestColors_p, 			indexC1 * 16, brightness , blendType); 	color2 = ColorFromPalette(ForestColors_p, 			indexC2 * 16, brightness , blendType); break ;
		case 26: color1 = ColorFromPalette(HeatColors_p, 			indexC1 * 16, brightness , blendType); 	color2 = ColorFromPalette(HeatColors_p, 			indexC2 * 16, brightness , blendType); break ;
		case 27: color1 = ColorFromPalette(LavaColors_p , 			indexC1 * 16, brightness , blendType); 	color2 = ColorFromPalette(LavaColors_p , 			indexC2 * 16, brightness , blendType); break ;
		case 28: color1 = ColorFromPalette(pal_red_green, 			indexC1 * 16, brightness , blendType); 	color2 = ColorFromPalette(pal_red_green, 			indexC2 * 16, brightness , blendType); break ;
		case 29: color1 = ColorFromPalette(pal_red_blue, 			indexC1 * 16, brightness , blendType); 	color2 = ColorFromPalette(pal_red_blue, 			indexC2 * 16, brightness , blendType); break ;
		case 30: color1 = ColorFromPalette(pal_green_blue, 			indexC1 * 16, brightness , blendType); 	color2 = ColorFromPalette(pal_green_blue, 			indexC2 * 16, brightness , blendType); break ;
		case 31: color1 = ColorFromPalette(pal_black_white_Narrow, 	indexC1 * 16, brightness , blendType); 	color2 = ColorFromPalette(pal_black_white_Narrow, 	indexC2 * 16, brightness , blendType); break ;
		case 32: color1 = ColorFromPalette(pal_black_white_wide, 	indexC1 * 16, brightness , blendType); 	color2 = ColorFromPalette(pal_black_white_wide, 	indexC2 * 16, brightness , blendType); break ;
		//case 26: color1 = ColorFromPalette(HeatColors_p, 			indexC1 * 16, brightness , blendType); 	color2 = ColorFromPalette(HeatColors_p, 			indexC2 * 16, brightness , blendType); break ;
		//case 26: color1 = ColorFromPalette(HeatColors_p, 			indexC1 * 16, brightness , blendType); 	color2 = ColorFromPalette(HeatColors_p, 			indexC2 * 16, brightness , blendType); break ;
		 
		
	}*/




       // With that value, look up the 8 bit colour palette value and assign it to the current LED.

    //leds2[i] = ColorFromPalette(currentPalette, index, bri, LINEARBLEND);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.
  }
  
} // noise16_2()



void noise16_2_pallete(uint16_t StartLed, uint16_t NrLeds , boolean pal, boolean mirror, boolean blend = true) 
{  
    TBlendType currentBlendingTB;
    if (get_bool(BLEND_INVERT) == true)
			blend = !blend;
		if (blend == true)
			currentBlendingTB = LINEARBLEND;
		else
			currentBlendingTB = NOBLEND;

  //FastLED.setBrightness(255);// just moving along one axis = "lavalamp effect"

  uint8_t scale = 10;                                       // the "zoom factor" for the noise

  for (uint16_t i = StartLed; i < StartLed+NrLeds; i++) 
  {

    uint16_t shift_x = millis() /  50;                         // x as a function of time
    uint16_t shift_y = 0;

    uint32_t real_x = (i + shift_x) * scale;                  // calculate the coordinates within the noise field
    uint32_t real_y = (i + shift_y) * scale;                  // based on the precalculated positions
    uint32_t real_z = 42;
    
    //uint8_t noise_8 = inoise16(real_x, real_y, real_z)  >> 8;    // get the noise data and scale it down
    uint8_t trip = 200;
    //uint8_t index = sin8(noise_8*trip);                            // map led color based on noise data
    //uint8_t bri   = noise_8;

    //debugMe(trip);
    uint16_t noise_16 = inoise16(real_x, real_y, real_z); 
    uint16_t indexLongs =  map(sin16(noise_16*trip),-32767,32767,0,MAX_INDEX_LONG-1); 
    uint8_t bri   = noise_16 >> 8 ;

    if (MAX_INDEX_LONG <= indexLongs)
			indexLongs = indexLongs - MAX_INDEX_LONG;



    led_FX_out[i] = ColorFrom_LONG_Palette(pal, indexLongs, bri , currentBlendingTB);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.

    //leds2[i] = ColorFromPalette(currentPalette, index, bri, LINEARBLEND);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.
  }
  
} // noise16_2()


void FX_noise_fill(uint16_t StartLed, uint16_t NrLeds , uint8_t octaves ,uint16_t x, int scale ,  uint8_t hue_octaves , uint16_t hue_x, int hue_scale, uint16_t time )
{
     time = millis() / 10 ;
    x = x+time;
 

        fill_noise8(&led_FX_out[StartLed], NrLeds,  octaves,  x,  scale,  hue_octaves,  hue_x,  hue_scale,  time);

       // led_FX_out[i] = inoise8(x + ioffset,y + joffset,z);
    
  
  //z += speed;


   // led_FX_out[i] = 

} 


int wave1=0;                                                  // Current phase is calculated.
int wave2=0;
int wave3=0;

//uint8_t mul1 = 7;                                            // Frequency, thus the distance between waves
//uint8_t mul2 = 6;
//uint8_t mul3 = 9;

void FX_three_sin(uint16_t StartLed, uint16_t NrLeds ,boolean pallete, boolean mirror,   boolean blend , uint8_t distance , uint8_t bmpWave1  ,uint8_t bmpWave2  ,uint8_t bmpWave3 , int lowWave1 ,int hiWave1 ,int lowWave2 ,int hiWave2 ,int lowWave3 ,int hiWave3  ) 
{
 
 

  wave1 += beatsin8(bmpWave1,lowWave1,hiWave1);
  wave2 += beatsin8(bmpWave2,lowWave2,hiWave2);
  wave3 += beatsin8(bmpWave3,lowWave3, hiWave3);
//debugMe(distance);

  for (uint16_t i = StartLed; i < StartLed+NrLeds; i++) 
  {
 
    uint8_t tmp = sin8(distance*i + wave1) + sin8(distance*i + wave2) + sin8(distance*i + wave3);

    led_FX_out[i] = ColorFrom_SHORT_Palette(pallete, tmp, 255,blend);
    
  }
 //m++;
} 
/*

void two_sin(uint16_t indexA, uimt16_t indexB, boolean waveAdir =true , boolean waveBdir = false, waveAspeed =10 , waveBspeed =4 ) 
{
  FastLED.setBrightness(255);

    waveAdir ? thisphase += beatsin8(thisspeed, 2, 10) : thisphase -= beatsin8(thisspeed, 2, 10);
    waveBdir ? thatphase += beatsin8(thisspeed, 2, 10) : thatphase -= beatsin8(thatspeed, 2, 10);
    indexA += thisrot;                                        // Hue rotation is fun for thiswave.
    indexB += thatrot;                                        // It's also fun for thatwave.
  
  for (int k=0; k<NUM_LEDS-1; k++) {
    int thisbright = qsuba(cubicwave8((k*allfreq)+thisphase), thiscutoff);      // qsub sets a minimum value called thiscutoff. If < thiscutoff, then bright = 0. Otherwise, bright = 128 (as defined in qsub)..
    int thatbright = qsuba(cubicwave8((k*allfreq)+128+thatphase), thatcutoff);  // This wave is 180 degrees out of phase (with the value of 128).

    leds1[k] = ColorFromPalette(currentPalette, indexA, 255, currentBlending);
    leds1[k] += ColorFromPalette(targetPalette, indexB, 255, currentBlending);
    
  }
     
     
} // two_sin()

//*/