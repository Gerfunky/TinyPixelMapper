// config_fs.h

#ifndef _CONFIG_FS_h
#define _CONFIG_FS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "leds.h"

#define MAX_NR_SAVES 16





// Public function
void FS_setup_SPIFFS();




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

	unsigned int FS_get_UsedBytes();
	unsigned int FS_get_TotalBytes();
	unsigned int FS_get_leftBytes();

#ifndef ARTNET_DISABLED
	void FS_artnet_write();			// osc.cpp
	boolean FS_artnet_read();			//wifi-ota.cpp
#endif

	boolean FS_Bools_read(uint8_t conf_nr);			// Tools
	boolean FS_get_PalyConfSatatus(uint8_t bit_nr);

	

#endif

