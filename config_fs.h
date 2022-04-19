// config_fs.h



#ifndef _CONFIG_FS_h
#define _CONFIG_FS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "leds.h"

#define MAX_NR_SAVES 64



#define NR_Confbits   8
#define Nr_CustomConfs 16
#define NR_CustomConfbits   2

struct SaveStruct
{
	uint8_t confStatus[NR_Confbits];
	uint8_t confCustomLoadNrs[Nr_CustomConfs];
	uint8_t confCustomStatus[NR_CustomConfbits] ;
	
};


/*
uint8_t confStatus[NR_Confbits] = {0,0,0,0,0,0,0,0};		// to hold the save ststus so we dont need to read from flash every time
uint8_t confCustomLoadNrs[Nr_CustomConfs] =  {64,65,66,67,68,69,70,71,71,73,74,75,76,78,79};   // The Conf to load on cutom load.
uint8_t confStatus[NR_CustomConfbits] = {0,0};

*/















void	FS_play_conf_write1(uint8_t val);
void   FS_play_conf_write_append1(uint8_t val);


// Public function
void FS_setup();




// Wifi
	void FS_wifi_write();		// wifi-ota , osc
	boolean FS_wifi_read();		// wifi-ota



// OSC
	void FS_Bools_write(uint8_t conf_nr);			// osc.cpp
	void FS_osc_delete_all_saves();					//osc.cpp
	
	boolean FS_play_conf_read(uint8_t conf_nr, deck_cfg_struct* targetConf, deck_fx1_struct* targetFXConf)  ;		//osc.cpp  , leds.cpp
	boolean FS_check_Conf_Available(uint8_t play_NR); // leds.cpp	
	void FS_play_conf_write(uint8_t conf_nr);		//osc.cpp
	void FS_play_conf_clear(uint8_t conf_nr) ;		// delete a conf file

	void FS_FFT_write(uint8_t conf_nr);				//osc.cpp
	void FS_play_conf_loop();
	boolean FS_FFT_read(uint8_t conf_nr);			// wifi-ota.cpp , osc.cpp , leds.cpp
	
	void FS_pal_load(uint8_t load_nr,uint8_t pal_no);
	void FS_pal_save(uint8_t save_no, uint8_t pal_no);

	boolean FS_mqtt_read();
	void FS_mqtt_write();


	unsigned int FS_get_UsedKBytes();
	unsigned int FS_get_TotalKBytes();
	unsigned int FS_get_leftKBytes();

#ifndef ARTNET_DISABLED
	void FS_artnet_write();			// osc.cpp
	boolean FS_artnet_read();			//wifi-ota.cpp
#endif

	boolean FS_Bools_read(uint8_t conf_nr);			// Tools
	boolean FS_get_PalyConfSatatus(uint8_t bit_nr);

	void FS_play_conf_readSendSavenames( ) ;
	void FS_play_conf_custom_readSendSavenames( ) ;

	

#endif


