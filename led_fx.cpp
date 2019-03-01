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









//------------- From mikeieee



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