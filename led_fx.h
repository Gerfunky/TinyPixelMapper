#ifndef _CONFIG_LED_FX_H
#define _CONFIG_LED_FX_H


//#include "config_TPM.h"
#include "arduino.h"


void Fire2012WithPalette(uint16_t start_led, uint16_t Nr_leds, bool reversed, uint8_t pal, bool mirror, uint8_t level, boolean subtract, boolean mask, uint8_t mix_mode); //
void LEDS_G_E_Form_Fade_it(uint8_t fadyBy, uint16_t *Start_led, uint16_t *nr_leds);
void LEDS_G_E_saw(uint8_t nr_dots, uint16_t *start_led, uint16_t *nr_leds, uint8_t *jd_speed, boolean reversed);  // Saw Dots that run in cirles in the form
void LEDS_G_E_juggle(uint8_t nr_dots, uint16_t *start_led, uint16_t *nr_leds, uint8_t *jd_speed, boolean reversed);		// sine dots speed = BPM
void LEDS_G_E_addGlitterRainbow(fract8 chanceOfGlitter, uint16_t *start_led, uint16_t *nr_leds);
void LEDS_G_E_addGlitter(fract8 chanceOfGlitter, uint16_t *start_led, uint16_t *nr_leds);
void LEDS_FFT_running_dot(CRGB color_result, uint16_t *Start_led, uint16_t *number_of_leds, boolean dir, uint8_t jd_speed, uint8_t nr_dots);

void LEDS_G_E_shimmer( uint16_t StartLed, uint16_t NrLeds , uint8_t pal, uint8_t mix_mode , uint8_t level , boolean mirror, boolean blend,  uint16_t xscale , uint16_t yscale , uint8_t beater );         // A time (rather than loop) based demo sequencer. This gives us full control over the length of each sequence.
void noise16_2(uint16_t StartLed, uint16_t NrLeds , uint8_t pal, boolean mirror, boolean blend) ;
void noise16_2_pallete(uint16_t StartLed, uint16_t NrLeds , boolean pal, boolean mirror, boolean blend) ;

//void FX_three_sin(uint16_t StartLed, uint16_t NrLeds ,boolean pallete, boolean mirror,  boolean blend , uint8_t distance ) ;
void FX_three_sin(uint16_t StartLed, uint16_t NrLeds ,boolean pallete, boolean mirror,   boolean blend  , uint8_t distance , uint8_t bmpWave1 = -1 ,uint8_t bmpWave2 = 4 ,uint8_t bmpWave3 = 2, int lowWave1 =-4,int hiWave1 = 4,int lowWave2 = -2,int hiWave2 = 2 ,int lowWave3 = -100,int hiWave3 =100 ) ;
//void FX_noise_fill(uint16_t StartLed, uint16_t NrLeds , uint8_t octaves ,uint16_t x, int scale ,  uint8_t hue_octaves , uint16_t hue_x, int hue_scale, uint16_t time );
void FX_noise_fill(uint16_t StartLed, uint16_t NrLeds , uint8_t octaves = 10,uint16_t x = 100, int scale = 100,  uint8_t hue_octaves =100 , uint16_t hue_x = 20, int hue_scale = 1, uint16_t time = 0  );
#endif