#ifndef _CONFIG_LED_FX_H
#define _CONFIG_LED_FX_H


//#include "config_TPM.h"
#include "arduino.h"


void Fire2012WithPalette(uint16_t start_led, uint16_t Nr_leds, bool reversed, bool pal, bool mirror); //
void LEDS_G_E_Form_Fade_it(uint8_t fadyBy, uint16_t *Start_led, uint16_t *nr_leds);
void LEDS_G_E_juggle2(uint8_t nr_dots, uint16_t *start_led, uint16_t *nr_leds, uint8_t *jd_speed, boolean reversed);  // Saw Dots that run in cirles in the form
void LEDS_G_E_juggle(uint8_t nr_dots, uint16_t *start_led, uint16_t *nr_leds, uint8_t *jd_speed, boolean reversed);		// sine dots speed = BPM
void LEDS_G_E_addGlitterRainbow(fract8 chanceOfGlitter, uint16_t *start_led, uint16_t *nr_leds);
void LEDS_G_E_addGlitter(fract8 chanceOfGlitter, uint16_t *start_led, uint16_t *nr_leds);
void LEDS_FFT_running_dot(CRGB color_result, uint16_t *Start_led, uint16_t *number_of_leds, boolean dir, uint8_t jd_speed, uint8_t nr_dots);

//void LEDS_G_E_shimmer(uint16_t StartLed, uint16_t NrLeds , boolean pal, uint16_t xscale = 6 , uint16_t yscale = 5, uint8_t beater = 7) 
void LEDS_G_E_shimmer( uint16_t StartLed, uint16_t NrLeds , boolean pal, boolean mirror, uint16_t xscale , uint16_t yscale , uint8_t beater );         // A time (rather than loop) based demo sequencer. This gives us full control over the length of each sequence.
void noise16_2(uint16_t StartLed, uint16_t NrLeds , boolean pal, boolean mirror, boolean blend) ;

#endif