#include <FastLED.h>
#include "led_fx.h"
#include "leds.h"
#include "tools.h"
extern led_cfg_struct led_cfg;
extern CRGBArray<MAX_NUM_LEDS> leds;
extern CRGBPalette16 LEDS_pal_cur[NR_PALETTS];	
extern CRGBArray<MAX_NUM_LEDS> led_FX_out;    // make a FX output array. 


extern CRGB ColorFrom_LONG_Palette(boolean pal, uint16_t longIndex, uint8_t brightness = 255, TBlendType blendType = LINEARBLEND); // made a new fuction to spread out the 255 index/color  pallet to 16*255 = 4080 colors



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
			led_FX_out[pixelnumber] = color;
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
			led_FX_out[pixelnumber] = color;
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

void LEDS_G_E_juggle2(uint8_t nr_dots, uint16_t *start_led, uint16_t *nr_leds, uint8_t *jd_speed, boolean reversed)  // Saw Dots that run in cirles in the form
{	// Make a dot  run  a SAW wave over the leds normal speed = bpm additional leds = bpm +1
	if (*nr_leds != 0 && (*nr_leds + *start_led <=MAX_NUM_LEDS))
	{
		byte dothue = 0;
		for (int i = 0; i < nr_dots; i++)
		{
			//led_FX_out[beatsin16(i + *jd_speed, *start_led, *start_led + *nr_leds - 1)] |= CHSV(led_cfg.hue + dothue, 255, 255);
			if (reversed == true) 	led_FX_out[map(beat16(i + *jd_speed),0 , 65535, *start_led + *nr_leds - 1, *start_led)] |= CHSV(led_cfg.hue + dothue, 255, 255);
			else					led_FX_out[map(beat16(i + *jd_speed), 0, 65535, *start_led , *start_led + *nr_leds - 1)] |= CHSV(led_cfg.hue + dothue, 255, 255);
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



void LEDS_G_E_shimmer(uint16_t StartLed, uint16_t NrLeds , boolean pal, boolean mirror, uint16_t xscale = 6 , uint16_t yscale = 5, uint8_t beater = 7) 
{          // A time (rather than loop) based demo sequencer. This gives us full control over the length of each sequence.

   static int16_t dist = random8();
  //FastLED.setBrightness(255);
  for(int i = StartLed ; i < StartLed+NrLeds ; i++) {                                      // Just ONE loop to fill up the LED array as all of the pixels change.
    uint8_t index = inoise8(i*xscale, dist+i*yscale) % 255; 
    //uint8_t index2 = inoise8(i*xscale, dist-i*yscale) % 255;    // Get a value from the noise function. I'm using both x and y axis.
    led_FX_out[i] = ColorFromPalette(LEDS_pal_cur[pal], index, 255, LINEARBLEND);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.
    //led_FX_out[i] = ColorFromPalette(LEDS_pal_cur[pal], index2, 255, LINEARBLEND);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.

  }
  
  dist += beatsin8(beater,1,4);                                                // Moving along the distance (that random number we started out with). Vary it a bit with a sine wave.
  if (mirror == true) 
  {
	    byte mirror_add = 0;
			       
        if (isODDnumber(NrLeds) == true) 
        {
            mirror_add = 1; // dosmething
        }
		
        //LEDS_Copy_strip(StartLed + NrLeds / 2  , -(NrLeds + mirror_add) / 2, StartLed);                                                                     // In some sketches, I've used millis() instead of an incremented counter. Works a treat.
  }

} // shimmer()



void noise16_2(uint16_t StartLed, uint16_t NrLeds , boolean pal, boolean mirror, boolean blend = true) 
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

    led_FX_out[i] = ColorFromPalette(LEDS_pal_cur[pal], index, bri, currentBlendingTB);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.

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