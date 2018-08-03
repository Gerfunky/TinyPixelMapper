#ifndef _LEDS_PAL_h
#define _LEDS_PAL_h

 #include "FastLED.h"
//FASTLED_USING_NAMESPACE 





const TProgmemPalette16 pal_red_green PROGMEM =
{
	CRGB::Red,
	CRGB::Red, 
	CRGB::Green,
	CRGB::Green,

	CRGB::Red,
	CRGB::Green,
	CRGB::Red,
	CRGB::Green,

	CRGB::Red,
	CRGB::Black,
	CRGB::Green,
	CRGB::Black,

	CRGB::Red,
	CRGB::Green,
	CRGB::Red,
	CRGB::Green
};

const TProgmemPalette16 pal_green_blue PROGMEM =
{
	CRGB::Green,
	CRGB::Green, 
	CRGB::Blue,
	CRGB::Blue,

	CRGB::Green,
	CRGB::Blue,
	CRGB::Green,
	CRGB::Blue,

	CRGB::Green,
	CRGB::Black,
	CRGB::Blue,
	CRGB::Black,

	CRGB::Green,
	CRGB::Blue,
	CRGB::Green,
	CRGB::Blue
};

const TProgmemPalette16 pal_red_blue PROGMEM =
{
	CRGB::Red,
	CRGB::Red, 
	CRGB::Blue,
	CRGB::Blue,

	CRGB::Red,
	CRGB::Blue,
	CRGB::Red,
	CRGB::Blue,

	CRGB::Red,
	CRGB::Black,
	CRGB::Blue,
	CRGB::Black,

	CRGB::Red,
	CRGB::Blue,
	CRGB::Red,
	CRGB::Blue
};



const TProgmemPalette16 pal_black_white_wide PROGMEM =
{
	CRGB::Black,
	CRGB::Black, 
	CRGB::White,
	CRGB::White,

	CRGB::Black,
	CRGB::Black,
	CRGB::White,
	CRGB::White,

	CRGB::Black,
	CRGB::Black,
	CRGB::White,
	CRGB::White,

	CRGB::Black,
	CRGB::Black,
	CRGB::White,
	CRGB::White
};
const TProgmemPalette16 pal_black_white_Narrow PROGMEM =
{
	CRGB::Black,
	CRGB::White, 
	CRGB::Black,
	CRGB::White,

	CRGB::Black,
	CRGB::White, 
	CRGB::Black,
	CRGB::White,

	CRGB::Black,
	CRGB::White, 
	CRGB::Black,
	CRGB::White,

	CRGB::Black,
	CRGB::White, 
	CRGB::Black,
	CRGB::White
};


#endif