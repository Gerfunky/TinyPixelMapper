// leds.h

#ifndef _LEDS_h
#define _LEDS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#endif
#include "leds_palletes.h"

//#include <FastLED.h>	
	//#include <pixeltypes.h>
	//#include "pixeltypes.h"

// defines , DO NOT CHANGE!!!!

		#define MAX_NUM_LEDS   (170*4)  // = 680  = 4 universes			// what is the max it can be set to in the config     340*4 = 20 FPS        170*4 = 29
		#define POT_SENSE_DEF 4   // only take Variable resistor value if it changes more than this.


		#define MAX_FADE_VALUE 20 // 90 is to much			// maximum for FADE effect on each frame in  amount 0-255 
		#define MAX_JD_SPEED_VALUE 30		// maximum BPM for Juggle and SAW dots
		#define MAX_GLITTER_VALUE 255		// max glitter value

		#define MAX_PAL_FPS 100				// maximum FPS 

		#define BMP_MAX_TIME 3000				

		#define MAX_INDEX_LONG 4096 //6			// must stay under this number!

		// Strip/Form settings do not change!!! 
		
		#define _M_NR_FORM_BYTES_ 4				// 2 bytes = 16 forms   // 4 bytes = 32 forms
		#define NR_FORM_PARTS	 ( _M_NR_FORM_BYTES_ * 8)				// how many forms? default 32


	//	#define NR_STRIPS		32				// how many strips  default 32
		#define NR_PALETTS 		16				// how many pallets do we have = 2
		#define NR_PALETTS_SELECT 32 			// how many to choose from with the ones from progmem

		#define MICROS_TO_MIN 60000000
						      

// Structures



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
		uint8_t 		ledMode;			// type of led  0= APA102, 1 = WS2812 , 2 = SK6822 
		uint16_t 		NrLeds;				// how many leds total  not fully implemented TODO!!!
		uint8_t			fire_sparking;		// For fire animation
		uint8_t			fire_cooling;		// For fire animation
		uint8_t 		Play_Nr ;			// What sequenca are we in.
		uint16_t		Data1NrLeds;		// Nr of leds for data1;
		uint16_t		Data1StartLed;		// Start led for data1;
		uint16_t		Data2NrLeds;		// Nr of leds for data2;
		uint16_t		Data2StartLed;		// Start led for data2;
		uint16_t		Data3NrLeds;		// Nr of leds for data3;
		uint16_t		Data3StartLed;		// Start led for data3;
		uint16_t		Data4NrLeds;		// Nr of leds for data4;
		uint16_t		Data4StartLed;		// Start led for data4;
		uint8_t 		apa102data_rate;	// data rate for apa102 max 24
		unsigned long 	confSwitch_time;	// when to swtich to the next config
		uint8_t 		edit_pal;
		
	

	};



	struct fft_led_cfg_struct
	{
		unsigned long	update_time;		// when to update again forFFT vis in Open stage controll
		float			adjuster;			// an adjuster for the timing of the FFT_FPS
		uint8_t			fps;				// FFT
		uint8_t			fftAutoMax;			// FFT Auto mode maximum trigger
		uint8_t			fftAutoMin;			// FFT Auto mode minimum trigger
		uint16_t			Scale;
		uint8_t 		viz_fps;



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
/*
   struct Strip_FL_Struct
   {
					uint16_t start_led ;	// where the pallete starts
                    uint16_t nr_leds;		// how many ?  for mirror how many extra.
					uint16_t index_start;	// prev state A
                        int index_add;		//
                    uint8_t index ;			// the index position
					uint8_t index_add_pal;	// how much to add onto the pallet on each frame        TODO: CHECK my descrition
					uint16_t index_long; 
					uint8_t 	fft_offset;			//
					uint8_t pal_level;
					uint8_t fft_level;
					uint8_t 	pal_mix_mode;
		  		uint8_t 	fft_mix_mode;
					uint8_t 	pal_pal;
	};
*/
		struct form_Led_Setup_Struct 
		{
			uint16_t	start_led;			// where the pallete starts
			uint16_t	nr_leds;			// how many ?  for mirror how many extra.
		//	uint8_t		fade_value;			// the fade value

		};



// PAL
		#define _M_NR_FORM_PAL_OPTIONS_ 5
		enum form_pal_options
		{
			_M_FORM_PAL_RUN,
			_M_FORM_PAL_REVERSED,
			_M_FORM_PAL_MIRROR,
			_M_FORM_PAL_BLEND,
			_M_FORM_PAL_ONECOLOR
		};


		struct form_fx_pal_struct
		{
					uint8_t 	pal;			// what pallete for pallete run
					uint8_t 	level;
					uint8_t 	mix_mode;
					uint16_t	index_start;		// Where to start at reset
					int				index_add_led;			// how much to add onto the index on 
					uint8_t		index_add_frame;		// ???
					uint16_t 	indexLong;
					uint8_t		index;				// the pallete index
					

		};



// FX1

		#define _M_NR_FORM_FX1_OPTIONS_ 3
		enum form_fx1_options
		{
			_M_FORM_FX1_RUN,
			_M_FORM_FX1_REVERSED,
			_M_FORM_FX1_MIRROR,
		};


	struct form_fx1_struct
		{
			uint8_t mix_mode;
			uint8_t level;
			uint8_t fade;

		};




		// GLITTER

	 #define _M_NR_FORM_GLITTER_OPTIONS_ 1
		enum form_glitter_options
		{
			_M_FORM_GLITTER_RUN,
		//	_M_FORM_GLITTER_FFT,
		};

		struct form_fx_glitter_struct
		{
			uint8_t pal;
			//uint8_t mix_mode;
			uint8_t level;
			uint8_t	value;
			//uint8_t color_source;  // 0 = palette, 1 = fft
			
		};


// DOTS

	#define _M_NR_FORM_DOT_OPTIONS_ 2
			enum form_dot_options
		{
			_M_FORM_DOT_RUN,
			_M_FORM_DOT_TYPE,  // 0 = sine , 1 = saw
			//_M_FORM_DOT_COLOR,		// 0 = Pallete , 1  FFT

		};

			enum form_dot_type
		{	
			DOT_SAW = 0,
			DOT_SINE = 1
			
		};


		struct form_fx_dots_struct
		{
			uint8_t pal; 					// "FFT color result": 250,"Rotating Hue": 251   others = pal
			//uint8_t mix_mode;
			uint8_t level;
			uint8_t	nr_dots;		// Nr Juggle Dots or Saw dots
		  uint8_t	speed;		// Dot speed in BPM
			uint16_t 	indexLong;
			uint16_t  index_add;
			
			//uint8_t fft_dot_type;
			//uint8_t normal_dot_type;
			//uint8_t normal_dot_color;


		};

	
// Fire
		#define _M_NR_FORM_FIRE_OPTIONS_ 3
		enum form_fire_options
		{
			_M_FORM_FIRE_RUN,
			_M_FORM_FIRE_REVERSED,
			_M_FORM_FIRE_MIRROR,
		};	


		struct form_fx_fire_struct
		{
			uint8_t pal; 
			uint8_t mix_mode;
			uint8_t level;
			uint8_t cooling;
			uint8_t sparking;

		};


	// FFT

	#define _M_NR_FORM_FFT_OPTIONS_ 4
		enum form_fft_options
		{
			_M_FORM_FFT_RUN,
			_M_FORM_FFT_REVERSED,
			_M_FORM_FFT_MIRROR,
			_M_FORM_FFT_ONECOLOR
		};

		
	struct fft_data_struct 
	{
		//uint8_t bin_on_red;		// bits select what bin to trigger on
		//uint8_t bin_on_green;		// bits select what bin to trigger on 
		//uint8_t bin_on_blue;		// bits select what bin to trigger on
		uint8_t trigger;		// trigger value
		uint8_t value;			// actual value
		uint8_t avarage;
		uint8_t autoFFT;
		uint8_t max;
		
	};

		struct form_fx_fft_struct
		{
			uint8_t mix_mode;
			uint8_t level;
			uint8_t offset;

		};



		// Shimmer
		
		#define _M_NR_FORM_SHIMMER_OPTIONS_ 2
		enum form_shimmer_options
		{
			_M_FORM_SHIMMER_RUN,
			_M_FORM_SHIMMER_BLEND,
		};


		struct form_fx_shim_struct
		{
			uint8_t pal; 
			uint8_t mix_mode;
			uint8_t level;
			uint8_t xscale;
		  uint8_t yscale;
		  uint8_t beater;


		};
	  
		
	  struct form_fx_shim_global_struct
	  {
		   int waveA;
		   int waveB;
		   int waveC;

		   uint8_t Abpm;
		   int Ahigh;
		   int Alow;
		   uint8_t index_addA;
		   uint8_t indexA;

		   uint8_t Bbpm;
		   int Bhigh;
		   int Blow;
		   uint8_t index_addB;
		   uint8_t indexB;

		   uint8_t Cbpm;
		   int Chigh;
		   int Clow;
			 uint8_t index_addC;
		   uint8_t indexC;

		

	  };

	  struct form_fx_test_val
	  {
		  uint8_t 	val_0;
		  uint8_t 	val_1;
		  uint8_t 	val_2;

	  };

#define _M_NR_FORM_BYTES_ 4				// 2 bytes = 16 forms   // 4 bytes = 32 forms


/*
#define _M_NR_STRIP_BYTES_ 4			// 4 bytes = 32 strips  

	  enum strip_bits
	  {
		  _S_7_0_ = 0,					// Strip / Form 0 to 7
		  _S_15_8_ = 1,					// Strip / Form 8 to 15
		  _S_23_16_ = 2,				// Strip 16 to 23
		  _S_31_24_ = 3					// Strip 24 to 31
	  };


#define _M_NR_OPTIONS_     16 //40 //10			// hass less options compared to forms!!
*/ 

//#define _M_NR_FORM_OPTIONS_  60			// Nr of options for forms 
	 /* enum strip_options {
		  _M_AUDIO_ = 0,				// Display FFT
		  _M_AUDIO_REVERSED = 1,
		  _M_STRIP_ = 2,				// Display Strip
  		  _M_REVERSED_ = 3,				// reversed mode
		  _M_MIRROR_OUT_ = 4,			// Mirror it
		  _M_PALETTE_ = 5,				// Pallete 0 or 1
		  _M_BLEND_ = 6,				// Fade or Hard Blend
		  _M_ONE_COLOR_ = 7,			// Make all the leds show one color
		  _M_FIRE_	= 8,				// Fire animation
		  //_M_FADE_ = 8,				// Fade the leds by amount
		  _M_RBOW_GLITTER_ = 9,			// Random Glitter
		  _M_GLITTER_ = 10,				// White Glitter
		  _M_AUDIO_DOT_ = 11,			// Audio Glitter
		  _M_JUGGLE_ = 12,				// Sine wave dots
		  _M_SAW_DOT_ = 13,				// Saw wave dots
		  _M_TEST_FX = 14,
		  _M_FX_SUBTRACT = 15				// add the FX channel to the leds
	  }; */



	


	

	
/*

	  enum strip_options {
		  _M_AUDIO_REVERSED 	,
		  _M_AUDIO_ 			,				// Display FFT
		  _M_AUDIO_PAL_MASK		,			// use the pallete to mask the fft data or +-
		  _M_AUDIO_SUB_FROM_FFT ,		//  add or subtract the pallete from the FFT data
		  _M_MIRROR_OUT_ 		,			// Mirror it
		  _M_ONE_COLOR_ 		,			// Make all the leds show one color
		  _M_STRIP_				,				// Display Strip
		  _M_REVERSED_ 			,				// reversed mod
		  _M_PALETTE_			,				// Pallete 0 or 1
		  _M_BLEND_ 			,				// Fade or Hard Blend

		  
		  _M_AUDIO_MIRROR  		,			//
		  _M_AUDIO_ONECOLOR		,			//


			_M_FIRE_				,				// Fire animation
		  _M_FIRE_PAL			,				// Fire animation
		  _M_FIRE_MIRROR  		,			//
		  _M_FIRE_REV			,			//
*/

		  /*_M_AUDIO_MASK  		,			//
		  _M_AUDIO_SUBTRACT		,

		  _M_FX_MIRROR  		,				//
		  _M_FX_REVERSED		,	
		  _M_FX_MASK			,
		  _M_FX_SUBTRACT		,				// add the FX channel to the leds
		  _M_FX1_ON  			,			//
		  _M_FX_LAYERS_ON		, 

		  
		  _M_GLITTER_FROM_FFT_DATA1  , 
		  _M_RBOW_GLITTER_ 		,			// Random Glitter
		  _M_GLITTER_			,				// White Glitter
		  _M_JUGGLE_ 			,				// Sine wave dots
		  _M_SAW_DOT_ 			,				// Saw wave dots
		  _M_AUDIO_DOT_ 		,			// 

		  _M_AUDIO_FX4  		,			// 
		  _M_AUDIO_FX5  		,			//
		  _M_AUDIO_FX6  		,			//

	  	_M_FX_SHIMMER  		,			//
		  _M_FX_SHIM_PAL  		,			//
		  _M_FX_SHIM_BLEND , 
		  _M_FX_SHIM_SUBTRACT , 
		  _M_FX_SHIM_MASK , 
		 	
		  
		  _M_FIRE_				,				// Fire animation
		  _M_FIRE_PAL			,				// Fire animation
		  _M_FIRE_MIRROR  		,			//
		  _M_FIRE_REV			,			//
		  _M_FIRE_SUBTRACT		,			//
		  _M_FIRE_MASK			,			//

		
				  			//
		  _M_FX_SIN_PAL  		,			//
		  _M_FX_3_SIN	  		,			//40
		  _M_FX_2_SIN  		,			//
		 */
		

		  

	//  };

/*
#define HARD_MIX_TRIGGER 128
	  enum mix_enum 
	  {  MIX_ADD 			
	  	,MIX_SUBTRACT		
		,MIX_MASK			
		,MIX_OR		
		,MIX_XOR
		,MIX_AND
		,MIX_DIFF
		,MIX_HARD
		,MIX_MULTIPLY
		,MIX_HARD_LIGHT
		,MIX_OVERLAY
		,MIX_TADA
		,MIX_DARKEN
		,MIX_LIGHTEN
		,MIX_LINEAR_BURN
		,MIX_SCREEN
		
	  };

*/
#define _M_NR_GLOBAL_OPTIONS_ 2			// This was a test to make reversing and mirroring global even in ARTNET
										// Was having werad flickering!!
										// TODO Check me again
	  enum global_strip_options {
		  _M_G_REVERSED_ = 0
		  , _M_G_MIRROR_ = 1

	  };

	#define MAX_LAYERS_SELECT 16  // up to how many layers can you add
	#define MAX_LAYERS 7					// what is the max layer Number 


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
	void LEDS_PAL_invert(uint8_t pal );
	void LEDS_Copy_strip(uint16_t start_LED, int nr_LED, uint16_t ref_LED);
	 //CRGB ColorFrom_LONG_Palette(boolean pal, uint16_t longIndex, uint8_t brightness = 255, TBlendType blendType = LINEARBLEND) // made a new fuction to spread out the 255 index/color  pallet to 16*255 = 4080 colors

	uint8_t getrand8() ;  // returns a random number from 0-255 
	uint8_t LEDS_get_real_bri();   // gets the real BRi including fft shift
	uint8_t LEDS_FFT_get_color_result(uint8_t color );   // Get the FFT color result 0= red 1= green 2 = blue

	boolean LEDS_get_sequencer(uint8_t play_nr); 

	void LEDS_write_sequencer(uint8_t play_nr, boolean value);


	uint8_t LEDS_get_playNr(); 
	uint8_t LEDS_get_bri();
	void LEDS_set_bri(uint8_t bri);
	void LEDS_set_playNr(uint8_t setNr);

	
#endif

