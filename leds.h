// leds.h

#ifndef _LEDS_h
#define _LEDS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#endif


// Structures
#define POT_SENSE_DEF 4

	struct led_controls_struct
	{
		uint8_t PotBriLast;
		uint8_t PotFPSLast;
		uint8_t PotSens;		// the sensitivity




	};


	struct led_cfg_struct					// LED config structure
	{										// 
		uint8_t 		max_bri;			// max Bri
		uint8_t 		bri;				// actual BRI
		uint8_t 		startup_bri;		// the startup brightness
		uint8_t 		r;					// max Red
		uint8_t 		g;					// max green
		uint8_t 		b;					// max blue
		uint8_t 		hue;				// HSV hue used for effects, every frame increments by one automatically
		unsigned long 	update_time;		// when to update the leds again
		uint8_t 		pal_fps;			// pallete FPS ... FFT FPS overrides this if FFT is enabled
		uint8_t 		pal_bri;			// Pallete bri so that we can dimm it down to the FFT 
		uint8_t 		bpm;				// BPM , used to time the pallete to a bpm
		unsigned long 	bpm_lastT;			// timing for BMP
		unsigned long 	bpm_diff;			// 
		uint8_t 		ledType;			// type of led  0= APA102, 1 = WS2812 , 2 = SK6822 
		uint16_t 		NrLeds;				// how many leds total  not fully implemented TODO!!!
		uint8_t			fire_sparking;		// For fire animation
		uint8_t			fire_cooling;		// For fire animation
	

	};

	// FFT

	struct fft_data_struct 
	{
		uint8_t bin_on_red;		// bits select what bin to trigger on
		uint8_t bin_on_green;		// bits select what bin to trigger on 
		uint8_t bin_on_blue;		// bits select what bin to trigger on
		uint8_t trigger;		// trigger value
		uint8_t value;			// actual value
		uint8_t avarage;
		uint8_t autoFFT;
	};

	struct fft_led_cfg_struct
	{
		unsigned long	update_time;		// when to update again
		float			adjuster;			// an adjuster for the timing of the FFT_FPS
		uint8_t			fps;				// FFT FPS
		uint8_t			fftAutoMax;			// FFT Auto mode maximum trigger
		uint8_t			fftAutoMin;			// FFT Auto mode minimum trigger
		uint16_t			Scale;


	};
		
		struct led_Copy_Struct				// copy strips data structure
		{
			uint16_t	start_led;
			int			nr_leds;
			uint16_t	Ref_LED;
		};
		#define NR_COPY_STRIPS 16
		#define NR_COPY_LED_BYTES 2

// Forms and Parts 
   struct Strip_FL_Struct
   {
					uint16_t start_led ;	// where the pallete starts
                    uint16_t nr_leds;		// how many ?  for mirror how many extra.
					uint16_t index_start;	// prev state A
                        int index_add;		//
                    uint8_t index ;			// the index position
					uint8_t index_add_pal;	// how much to add onto the pallet on each frame        TODO: CHECK my descrition
					uint16_t index_long; 
	};

	  struct form_Part_FL_Struct 
	  {
		  uint16_t	index_start;		// prev state A
			int		index_add;			// how much to add onto the index on ....  CHECKME!!
		  uint16_t	start_led;			// where the pallete starts
		  uint16_t	nr_leds;			// how many ?  for mirror how many extra.
		  uint8_t	fade_value;			// the fade value
		  uint8_t	scroll_speed;		// scoll speed !! NOT implemented!!!
		  uint8_t	glitter_value;		// glitter value
		  uint8_t	juggle_nr_dots;		// Nr Juggle Dots or Saw dots
		  uint8_t	juggle_speed;		// Dot speed in BPM
		  uint8_t	index;				// the pallete index
			int		rotate;				// ???
		  uint8_t	index_add_pal;		// ???
		  uint16_t	indexLong;
	  };


#define _M_NR_STRIP_BYTES_ 4			// 4 bytes = 32 strips  
#define _M_NR_FORM_BYTES_ 2				// 2 bytes = 16 forms
	  enum strip_bits
	  {
		  _S_7_0_ = 0,					// Strip / Form 0 to 7
		  _S_15_8_ = 1,					// Strip / Form 8 to 15
		  _S_23_16_ = 2,				// Strip 16 to 23
		  _S_31_24_ = 3					// Strip 24 to 31
	  };


#define _M_NR_OPTIONS_     8			// hass less options compared to forms!!
#define _M_NR_FORM_OPTIONS_  14			// Nr of options for forms 
	  enum strip_options {
		  _M_AUDIO_ = 0,				// Display FFT
		  _M_MIRROR_OUT_ = 1,			// Mirror it 
		  _M_STRIP_ = 2,				// Display Strip
		  _M_REVERSED_ = 3,				// reversed mode
		  _M_PALETTE_ = 4,				// Pallete 0 or 1
		  _M_BLEND_ = 5,				// Fade or Hard Blend
		  _M_ONE_COLOR_ = 6,			// Make all the leds show one color
		  _M_FIRE_	= 7,
		  _M_FADE_ = 8,					// Fade the leds by amount
		  _M_RBOW_GLITTER_ = 9,			// Random Glitter
		  _M_GLITTER_ = 10,				// White Glitter
		  _M_AUDIO_DOT_ = 11,			// Audio Glitter
		  _M_JUGGLE_ = 12,				// Sine wave dots
		  _M_SAW_DOT_ = 13,				// Saw wave dots
	  };

#define _M_NR_GLOBAL_OPTIONS_ 2			// This was a test to make reversing and mirroring global even in ARTNET
										// Was having werad flickering!!
										// TODO Check me again
	  enum global_strip_options {
		  _M_G_REVERSED_ = 0
		  , _M_G_MIRROR_ = 1

	  };

	


// Functions

	  void LEDS_loop();
	  void LEDS_setup();

	  float LEDS_get_FPS(); // osc.cpp

	  void LEDS_pal_write(uint8_t pal, uint8_t no, uint8_t color, uint8_t value);   // used in config_fs , osc.cpp
	  uint8_t LEDS_pal_read(uint8_t pal, uint8_t no, uint8_t color);				 // used in config_fs. osc.cpp
	  void LEDS_pal_reset_index();															//osc.cpp
	  void LEDS_pal_load(uint8_t pal_no, uint8_t pal_menu);									//osc.cpp

	  void LEDS_setLED_show(uint8_t ledNr, uint8_t color[3]);									 // wifi-ota
	  void LEDS_artnet_in(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data);  // wifi-ota
	  void LEDS_fadeout();																		 // wifi-ota
	  void LEDS_setall_color(uint8_t color);																	 // wifi-ota
	  void LEDS_FFT_enqueue(uint8_t invalue);													 // wifi-ota
	  uint8_t LEDS_FFT_get_value(uint8_t bit);													 // wifi-ota
	  void LEDS_show(); 																		//wifi-ota


#endif

