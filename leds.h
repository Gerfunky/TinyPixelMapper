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

		#define MAX_NUM_LEDS   (170*9)  		// what is the max it can be set to in the config    
		#define MAX_NUM_LEDS_BOOT  21 			//	only set the first 20 for boot info green=ap / red= clinet
		#define POT_SENSE_DEF 4   				// only take Variable resistor value if it changes more than this.


		#define MAX_FADE_VALUE 20 // 90 is to much			// maximum for FADE effect on each frame in  amount 0-255 
		#define MAX_JD_SPEED_VALUE 30		// maximum BPM for Juggle and SAW dots
		#define MAX_GLITTER_VALUE 255		// max glitter value

		#define MAX_PAL_FPS 100				// maximum FPS 

		#define BMP_MAX_TIME 3000				

		#define MAX_INDEX_LONG 4096 //6			// must stay under this number!

		// Strip/Form settings do not change!!! 
		
		#define _M_NR_FORM_BYTES_ 6				// 2 bytes = 16 forms   // 4 bytes = 32 forms
		#define NR_FORM_PARTS	 ( _M_NR_FORM_BYTES_ * 8 )				// how many forms? default 6*8 = 64


	//	#define NR_STRIPS		32				// how many strips  default 32
		#define NR_PALETTS 		16				// how many pallets do we have = 2
		#define NR_PALETTS_SELECT 32 			// how many to choose from with the ones from progmem

		#define MICROS_TO_MIN 60000000
		#define DEF_CONFNAME  "Def"			      

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
//		uint8_t 		bri;				// actual BRI
		uint8_t 		startup_bri;		// the startup brightness
//		uint8_t 		r;					// max Red
//		uint8_t 		g;					// max green
//		uint8_t 		b;					// max blue
		uint8_t 		hue;				// HSV hue used for effects, every frame increments by one automatically
		unsigned long 	update_time;		// when to update the leds again
//		uint8_t 		pal_fps;			// pallete FPS ... FFT FPS overrides this if FFT is enabled
//		uint8_t 		pal_bri;			// Pallete bri so that we can dimm it down to the FFT 
		uint8_t 		bpm;				// BPM , used to time the pallete to a bpm
		unsigned long 	bpm_lastT;			// timing for BMP
		unsigned long 	bpm_diff;			// 
		uint8_t 		ledMode;			// type of led  0= APA102, 1 = WS2812 , 2 = SK6822 
		uint16_t 		NrLeds;				// how many leds total  not fully implemented TODO!!!
//		uint8_t			fire_sparking;		// For fire animation
//		uint8_t			fire_cooling;		// For fire animation
		uint8_t 		Play_Nr ;			// What sequenca are we in.
		uint16_t 		DataNR_leds[5]; 
		uint16_t 		DataStart_leds[4]; 
		uint8_t 		apa102data_rate;	// data rate for apa102 max 24
		unsigned long 	confSwitch_time;	// when to swtich to the next config for sequencer
		uint8_t 		edit_pal;
	//	unsigned long 	update_FIN_time;			// when to update the leds again
		uint8_t 		realfps;					// whats the real fps that we have at the moment.
		uint8_t 		framecounter;				// for counting FPS
		unsigned long	framecounterUpdateTime; 	// for calculationg fps

	};
	struct led_master_conf
	{
		uint8_t			fire_sparking;		// For fire animation
		uint8_t			fire_cooling;		// For fire animation
		uint8_t 		r;					// max Red
		uint8_t 		g;					// max green
		uint8_t 		b;					// max blue

		uint8_t 		pal_fps;			// pallete FPS ... FFT FPS overrides this if FFT is enabled
		uint8_t 		pal_bri;			// Pallete bri so that we can dimm it down to the FFT 

		uint8_t 		bri;				// actual BRI


	};


	struct fft_led_cfg_struct
	{
		unsigned long	update_time;		// when to update again forFFT vis in Open stage controll
		float			adjuster;			// an adjuster for the timing of the FFT_FPS
		uint8_t			fps;				// FFT
		uint8_t			fftAutoMax;			// FFT Auto mode maximum trigger
		uint8_t			fftAutoMin;			// FFT Auto mode minimum trigger
		uint16_t		Scale;
		uint8_t 		viz_fps;



	};



		//#define _M_NR_FORM_BYTES_ 4				// 2 bytes = 16 forms   // 4 bytes = 32 forms
		struct form_Led_Setup_Struct 
		{
			uint16_t	start_led;			// where the pallete starts
			uint16_t	nr_leds;			// how many ?  for mirror how many extra.
		

		};




// *************** Layers  ************************


	#define MAX_LAYERS_SELECT 48  // up to how many layers can you add
	#define MAX_LAYERS 33				// what is the max layer Number

	enum layer_options
	{
		_M_LAYER_00_FFT = 1,
		_M_LAYER_00_PAL = 2,
		_M_LAYER_00_FX01 = 3,
		_M_LAYER_00_FIRE = 4,
		_M_LAYER_00_SHIMMER = 5,
		_M_LAYER_00_STROBE = 11,
		_M_LAYER_00_EYES = 13,
		_M_LAYER_00_ROTATE = 15,

		_M_LAYER_16_FFT = 6,
		_M_LAYER_16_PAL = 7,
		_M_LAYER_16_FX01 = 8,
		_M_LAYER_16_FIRE = 9,
		_M_LAYER_16_SHIMMER = 10,
		_M_LAYER_16_STROBE = 12,
		_M_LAYER_16_EYES = 14,
		_M_LAYER_16_ROTATE = 16,

		_M_LAYER_32_FFT = 17,
		_M_LAYER_32_PAL = 18,
		_M_LAYER_32_FX01 = 19,
		_M_LAYER_32_FIRE = 20,
		_M_LAYER_32_SHIMMER = 21,
		_M_LAYER_32_STROBE = 22,
		_M_LAYER_32_EYES = 23,
		_M_LAYER_32_ROTATE = 24,

		_M_LAYER_SAVE_ALPHA = 25,
		_M_LAYER_SAVE_BETA = 26,
		_M_LAYER_SAVE_GAMMA = 27,
		_M_LAYER_SAVE_OMEGA = 28,

		_M_LAYER_RUN_ALPHA = 29,
		_M_LAYER_RUN_BETA = 30,
		_M_LAYER_RUN_GAMMA = 31,
		_M_LAYER_RUN_OMEGA = 32,

		_M_LAYER_CLEAR = 33,


	};

// ***************Palette  ************************

		#define _M_NR_FORM_PAL_OPTIONS_ 6
		enum form_pal_options
		{
			_M_FORM_PAL_RUN,
			_M_FORM_PAL_REVERSED,
			_M_FORM_PAL_MIRROR,
			_M_FORM_PAL_BLEND,
			_M_FORM_PAL_ONECOLOR,
			_M_FORM_PAL_SPEED_FROM_FFT
		};

		struct form_fx_pal_struct
		{
					uint8_t 	pal;			// what pallete for pallete run
					uint8_t 	level;
					uint8_t 	mix_mode;
					uint16_t	index_start;		// Where to start at reset
					uint16_t	index_add_led;		// how much to add onto the index on 
					uint16_t	index_add_frame;	// ???
					uint16_t 	indexLong;
					uint8_t		index;				// the pallete index
					uint8_t 	triggerBin; 		// what fft fx bin to trigger from 255 = none.
					uint8_t 	palSpeedBin; 		// index_add_frame  + trigger from bin     , 255 = none 
					uint8_t 	lvl_bin;			// whats the lvl bin to add to the lvl   , 255 = none

		};



// ***************FX 01 ************************

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
			uint8_t triggerBin; 		// what fft fx bin to trigger from 255 = none.
			uint8_t lvl_bin;

		};

// *************** FX01 Glitter ************************

	 #define _M_NR_FORM_GLITTER_OPTIONS_ 2
		enum form_glitter_options
		{
			_M_FORM_GLITTER_RUN,
			_M_FORM_GLITTER_FFT,
		};

		struct form_fx_glitter_struct
		{
			uint8_t pal;
			//uint8_t mix_mode;
			uint8_t level;
			uint8_t	value;
			uint8_t glit_bin;
			//uint8_t color_source;  // 0 = palette, 1 = fft
			
		};


// *************** FX01 Dots BPM ************************

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
// *************** FX01 Meteor ************************
		#define _M_NR_FORM_METEOR_OPTIONS_ 2
		enum form_meteor_options
		{
			_M_FORM_METEOR_RUN,
			_M_FORM_METEOR_RANDOMDECAY
		//	_M_FORM_METEOR_MIRROR,
		//	_M_FORM_METEOR_REVERSED


		};	


		struct form_fx_meteor_struct
		{
			uint8_t color; 
			uint8_t mix_mode;
			uint8_t level;
			uint16_t meteorSize;
			uint8_t meteorTrailDecay;
			uint8_t lvl_bin;
			uint8_t triggerBin;
			uint16_t frame_pos; 		

		};
	
// *************** Fire  ************************
		#define _M_NR_FORM_FIRE_OPTIONS_ 3
		enum form_fire_options
		{
			_M_FORM_FIRE_RUN,
			_M_FORM_FIRE_REVERSED,
			_M_FORM_FIRE_MIRROR,
			_M_FORM_FIRE_SPK_FFT,
			_M_FORM_FIRE_COOL_FFT

		};	


		struct form_fx_fire_struct
		{
			uint8_t pal; 
			uint8_t mix_mode;
			uint8_t level;
			uint8_t cooling;
			uint8_t sparking;
			uint8_t triggerBin; 		// what fft fx bin to trigger from 255 = none.
			uint8_t lvl_bin;

		};

// *************** Strobe ************************
		#define _M_NR_FORM_STROBE_OPTIONS_ 1
		enum form_strobe_options
		{
			_M_FORM_STROBE_RUN,


		};	


		struct form_fx_strobe_struct
		{
			uint8_t pal; 
			uint8_t mix_mode;
			uint8_t level;
			uint8_t on_frames;
			uint8_t off_frames;
			uint16_t frame_pos; 		// what fft fx bin to trigger from 255 = none.
			uint8_t lvl_bin;
			uint8_t triggerBin;

		};


// *************** Eyes ************************
		#define _M_NR_FORM_EYES_OPTIONS_ 1
		enum form_eyes_options
		{
			_M_FORM_EYES_RUN,


		};	


		struct form_fx_eyes_struct
		{
			uint8_t color; 
			uint8_t mix_mode;
			uint8_t level;
			uint16_t on_frames;
			uint16_t EyeWidth;
			uint16_t EyeSpace;
			uint8_t lvl_bin;
			uint8_t triggerBin;
			uint16_t frame_pos; 
			uint16_t eye_pos;
			uint8_t  fadeval;	
			uint16_t pause_frames;	

		};



// *************** FFT ************************

	#define _M_NR_FORM_FFT_OPTIONS_ 4
		enum form_fft_options
		{
			_M_FORM_FFT_RUN,
			_M_FORM_FFT_REVERSED,
			_M_FORM_FFT_MIRROR,
			_M_FORM_FFT_ONECOLOR
		};

		


#define FFT_FX_NR_OF_BINS 50
	struct fft_fxbin_struct 
	{
				
			uint8_t set_val ;
			uint8_t trrig_val;
			uint8_t menu_select;

	};
	struct fft_fxbin_run_struct 
	{
			uint8_t sum ;					
			uint8_t result;
	};

		struct form_fx_fft_struct
		{
			uint8_t mix_mode;
			uint8_t level;
			uint8_t offset;
			uint8_t extend;
			uint8_t triggerBin; 		// what fft fx bin to trigger from 255 = none.
			uint8_t lvl_bin;
			uint8_t color;
			uint8_t extend_tick;

		};



// *************** Shimmer ************************
		
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
			uint8_t triggerBin; 		// what fft fx bin to trigger from 255 = none.
			uint8_t lvl_bin;

			uint16_t dist;
			


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


// ********* Modify**********************
#	define _M_NR_FORM_MODIFY_OPTIONS_ 2
	enum form_modify_options
	{
		_M_FORM_MODIFY_ROTATE,
		_M_FORM_MODIFY_ROTATE_REVERSED,
		_M_FORM_MODIFY_MIRROR,
	};

	struct  form_fx_modify_struct
	{
		uint16_t RotateFullFrames;
		uint16_t RotateFixed;
		uint8_t RotateTriggerBin;
		//uint8_t RotateMixMode;


		uint16_t RotateFramePos;

	};



// ***************Copy Leds ************************
		struct led_Copy_Struct				// copy strips data structure
		{
			uint16_t	start_led;
			int			nr_leds;
			uint16_t	Ref_LED;
		};
		#define NR_COPY_STRIPS 16
		#define NR_COPY_LED_BYTES 2








#define _M_NR_GLOBAL_OPTIONS_ 2			// This was a test to make reversing and mirroring global even in ARTNET
										// Was having werad flickering!!
										// TODO Check me again
	  enum global_strip_options {
		  _M_G_REVERSED_ = 0
		  , _M_G_MIRROR_ = 1

	  };


	struct fft_data_struct   // run values
	{
		//uint8_t bin_on_red;		// bits select what bin to trigger on
		//uint8_t bin_on_green;		// bits select what bin to trigger on 
		//uint8_t bin_on_blue;		// bits select what bin to trigger on

		//uint8_t value;			// actual value
		uint8_t avarage;
	//	uint8_t autoFFT;
		uint8_t max;
		uint8_t result; 
	};
	struct fft_config_struct  // Config Values
	{
		byte fft_menu[3] = { 3,7,200 };	// 3 fft data bins for RGB 
		byte fft_menu_bri = 0;			// howmuch to add to bri based on fft data 	
		byte fft_menu_fps = 0;   		// howmuch to add to the FPS based on FFT data selected.
		//fft_led_cfg_struct fft_led_cfg = { 0,1,25,240,11,1};
		uint8_t fft_bin_autoTrigger = 0;
		uint8_t trigger[7];		// trigger value

		//uint8_t fft_color_result_bri = 0;
		//uint8_t fft_color_fps = 0;
		//CRGB GlobalColor_result;
		fft_fxbin_struct fft_fxbin[FFT_FX_NR_OF_BINS] ;
		
	};
	struct fft_run_struct  // run vals 
	{
		
		//byte fft_data_bri = 0;			// howmuch to add to bri based on fft data 	
		//byte fft_data_fps = 0;   		// howmuch to add to the FPS based on FFT data selected.
		//fft_led_cfg_struct fft_led_cfg = { 0,1,25,240,11,1};
		uint8_t fft_color_result_bri = 0;
		uint8_t fft_color_fps = 0;
		CRGB GlobalColor_result;

		fft_fxbin_run_struct  fft_fxbin[FFT_FX_NR_OF_BINS] ;
		
	};
		
#define NO_OF_SAVE_LAYERS 5
	struct layer_struct
	{
		uint8_t save_mix[NO_OF_SAVE_LAYERS];
		uint8_t save_lvl[NO_OF_SAVE_LAYERS];
		uint16_t save_startLed[NO_OF_SAVE_LAYERS];
		uint16_t save_NrLeds[NO_OF_SAVE_LAYERS];

		uint8_t select[MAX_LAYERS_SELECT] ;
		uint16_t clear_start_led;
		uint16_t clear_Nr_leds;
	};


//******************** DEcks *******************



struct deck_run_struct
{
		CRGBArray<MAX_NUM_LEDS> leds_FFT_history;
		CRGBArray<MAX_NUM_LEDS> led_FX_out; 
		CRGBArray<MAX_NUM_LEDS> leds;
		//CRGB leds_FFT_history[MAX_NUM_LEDS];
		//CRGB led_FX_out[MAX_NUM_LEDS]; 
		//CRGB leds[MAX_NUM_LEDS];
		byte heat[MAX_NUM_LEDS	];
		fft_data_struct 	fft_data[7];
		fft_run_struct  	fft;
		CRGBArray<MAX_NUM_LEDS> SaveLayers[NO_OF_SAVE_LAYERS];
};

struct deck_cfg_struct
{
	char confname[32];
	
	led_master_conf led_master_cfg;

	layer_struct layer;

	fft_config_struct 	fft_config;

	form_Led_Setup_Struct form_cfg[NR_FORM_PARTS];
	form_fx_pal_struct form_fx_pal[NR_FORM_PARTS];
	form_fx_shim_struct form_fx_shim[NR_FORM_PARTS];
	form_fx_fire_struct form_fx_fire[NR_FORM_PARTS]; 
	form_fx_fft_struct form_fx_fft[NR_FORM_PARTS];
	form_fx1_struct form_fx1[NR_FORM_PARTS];
	form_fx_glitter_struct form_fx_glitter[NR_FORM_PARTS];
	form_fx_dots_struct form_fx_dots[NR_FORM_PARTS];
	form_fx_strobe_struct form_fx_strobe[NR_FORM_PARTS] ;
	form_fx_eyes_struct form_fx_eyes[NR_FORM_PARTS];
	form_fx_meteor_struct form_fx_meteor[NR_FORM_PARTS];
	form_fx_modify_struct form_fx_modify[NR_FORM_PARTS];

	CRGBPalette16 LEDS_pal_cur[NR_PALETTS];


	byte form_menu_pal[_M_NR_FORM_BYTES_][_M_NR_FORM_PAL_OPTIONS_];
	byte form_menu_fft[_M_NR_FORM_BYTES_][_M_NR_FORM_FFT_OPTIONS_];
	byte form_menu_fire[_M_NR_FORM_BYTES_][_M_NR_FORM_FIRE_OPTIONS_];
	byte form_menu_shimmer[_M_NR_FORM_BYTES_][_M_NR_FORM_SHIMMER_OPTIONS_];
	byte form_menu_fx1[_M_NR_FORM_BYTES_][_M_NR_FORM_FX1_OPTIONS_];
	byte form_menu_dot[_M_NR_FORM_BYTES_][_M_NR_FORM_DOT_OPTIONS_];
	byte form_menu_glitter[_M_NR_FORM_BYTES_][_M_NR_FORM_GLITTER_OPTIONS_];
	byte form_menu_strobe[_M_NR_FORM_BYTES_][_M_NR_FORM_STROBE_OPTIONS_];
	byte form_menu_eyes[_M_NR_FORM_BYTES_][_M_NR_FORM_EYES_OPTIONS_];
	byte form_menu_meteor[_M_NR_FORM_BYTES_][_M_NR_FORM_METEOR_OPTIONS_];
	byte form_menu_modify[_M_NR_FORM_BYTES_][_M_NR_FORM_MODIFY_OPTIONS_];




};

struct deck_struct
{
	deck_cfg_struct cfg;
	deck_run_struct run;
};


		//struct Master_Leds_struct
		//{
			//led_cfg_struct led_cfg;									// Main Config
			//struct form_Led_Setup_Struct form_cfg[NR_FORM_PARTS];  // the Lines / forms

			//CRGBPalette16 LEDS_pal_cur[NR_PALETTS];	  //pallets

		//};


 



// Functions

	void LEDS_loop();
	void LEDS_setup();

	float LEDS_get_FPS(); // osc.cpp

	void LEDS_pal_write(uint8_t pal, uint8_t no, uint8_t color, uint8_t value);   // used in config_fs , osc.cpp
	void LEDS_pal_write(CRGBPalette16* palref, uint8_t pal, uint8_t no, uint8_t color , uint8_t value);

	uint8_t LEDS_pal_read(uint8_t pal, uint8_t no, uint8_t color);				 // used in config_fs. osc.cpp
	uint8_t LEDS_pal_read(CRGBPalette16* palref, uint8_t pal, uint8_t no, uint8_t color);

	void LEDS_pal_reset_index();															//osc.cpp
	void LEDS_pal_load(deck_struct* deckref, uint8_t pal_no, uint8_t pal_menu)	;
	void LEDS_pal_load(CRGBPalette16* palref, uint8_t pal_no, uint8_t pal_menu) ;
	
								//osc.cpp

	void LEDS_setLED_show(uint8_t ledNr, uint8_t color[3]);									 // wifi-ota
	void LEDS_artnet_in(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data);  // wifi-ota
	void LEDS_fadeout();																		 // wifi-ota
	void LEDS_setall_color(uint8_t color);																	 // wifi-ota
	void LEDS_FFT_enqueue(uint8_t invalue);													 // wifi-ota
	uint8_t LEDS_FFT_get_value(uint8_t bit);													 // wifi-ota
	void LEDS_show(); 
	void FastLEDshowESP32();																		//wifi-ota
	void LEDS_PAL_invert(uint8_t pal );
	void LEDS_Copy_strip(uint16_t start_LED, int nr_LED, uint16_t ref_LED);
	 //CRGB ColorFrom_LONG_Palette(boolean pal, uint16_t longIndex, uint8_t brightness = 255, TBlendType blendType = LINEARBLEND) // made a new fuction to spread out the 255 index/color  pallet to 16*255 = 4080 colors

	uint8_t getrand8() ;  // returns a random number from 0-255 
	uint8_t LEDS_get_real_bri();   // gets the real BRi including fft shift
	uint8_t LEDS_FFT_get_color_result(uint8_t deckNo, uint8_t color );   // Get the FFT color result 0= red 1= green 2 = blue

	boolean LEDS_get_sequencer(uint8_t play_nr); 

	void LEDS_write_sequencer(uint8_t play_nr, boolean value);


	uint8_t LEDS_get_playNr(); 
	uint8_t LEDS_get_bri();
	void LEDS_set_bri(uint8_t bri);
	void LEDS_set_playNr(uint8_t setNr);

	
#endif

