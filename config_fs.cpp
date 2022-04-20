
/* 
		This is part of the Tinypixelmapper.
		Here we store the functions for writing and reading the configuration.
		either from SD card (Olimex POE Board)  or in SPIFFS 

*/

    #include <FS.h>


#include "config_TPM.h"
#include "config_fs.h"
#include "tools.h"
#include "tpm_artnet.h"
#include "osc.h"


#ifdef USE_SD
	#include "SD_MMC.h"
	fs::SDMMCFS  &selectedFS = SD_MMC;	
#else

	#include<SPIFFS.h>
	fs::SPIFFSFS  &selectedFS = SPIFFS;
#endif


// ***************** External Structures
	#include "wifi-ota.h"
	extern wifi_Struct wifi_cfg;


	#include "leds.h"
	#include "leds_def_values.h"		// include the default values for led settings
	#ifndef ARTNET_DISABLED
		extern artnet_struct artnet_cfg;
		extern artnet_node_struct artnetNode[ARTNET_NR_NODES_TPM];
	#endif

	extern deck_struct deck[2] ;   // The 2 main decks config
	extern deck_struct mem_confs[2] ;   // The 16 memory confs 
	

	extern fft_ip_cfg_struct fft_ip_cfg;
	extern led_cfg_struct led_cfg;
	extern uint16_t play_conf_time_min[MAX_NR_SAVES];

	#include "mmqt.h"
	extern mqtt_Struct mqtt_cfg;

//**************** Functions 



SaveStruct  SaveConf = { {0,0,0,0,0,0,0,0} ,
						  {64,65,66,67,68,69,70,71,71,73,74,75,76,78,79},
						  {0,0}
						  };

//uint8_t savelooppos = 255;         // Wher are we in the Fade animation when loading ans saving
//uint8_t saveloopConfNr = 255;		// What conf Nr are we loading or saving




unsigned int FS_get_UsedKBytes()
{

	

	return (selectedFS.usedBytes() /1024);

}

unsigned int FS_get_TotalKBytes()
{

	return (selectedFS.totalBytes() /1024 );

}

unsigned int FS_get_leftKBytes()
{
	//debugMe("TotalB :" + String ((selectedFS.totalBytes())));
	//debugMe("usedB :" + String ((selectedFS.usedBytes())));
	//debugMe("LeftB :" + String ((selectedFS.totalBytes()-selectedFS.usedBytes()  )));
	//debugMe("LeftKB :" + String ((selectedFS.totalBytes()-selectedFS.usedBytes()  )/1024));


	return ((selectedFS.totalBytes()-selectedFS.usedBytes()  ) /1024);

}

boolean FS_get_PalyConfSatatus(uint8_t play_NR)
{

	String addr = String("/conf/" + String(play_NR) + ".playConf.txt");
	File conf_file = selectedFS.open(addr,"r"); 

	if(conf_file && conf_file.isDirectory() == false) 
	{ //exists and its a file 
		conf_file.close();
		return true;
	}
	conf_file.close();
	return false;

}






void FS_load_PlayConf_status()
{


	for(uint8_t bit_nr = 0; bit_nr < sizeof(SaveConf.confStatus); bit_nr++)
	{
			for(uint8_t conf_nr = 0; conf_nr < 8; conf_nr++)
			{
				
				bitWrite(SaveConf.confStatus[bit_nr], conf_nr, FS_get_PalyConfSatatus( (8*bit_nr) + conf_nr   ) );

			}

	}

}



boolean FS_check_Conf_Available(uint8_t play_NR)
{

	boolean return_bool = 0;
	uint8_t byte_nr = 0;
	uint8_t bit_nr = play_NR;
	while (bit_nr > 7)
	{
		byte_nr++;
		bit_nr = bit_nr - 8;
	}
	return_bool = bitRead(SaveConf.confStatus[byte_nr], bit_nr);

	return return_bool;

}


void  FS_write_Conf_status(uint8_t play_NR, boolean value)
{
	if (play_NR >= MAX_NR_SAVES) return;
	
	uint8_t byte_nr = 0;
	uint8_t bit_nr = play_NR;
	while (bit_nr > 7)
	{
		byte_nr++;
		bit_nr = bit_nr - 8;
	}
	bitWrite(SaveConf.confStatus[byte_nr], bit_nr, value);

	

}

void  FS_write_Custom_Conf_status(uint8_t play_NR, boolean value)
{
	if (play_NR >= Nr_CustomConfs) return;
	
	uint8_t byte_nr = 0;
	uint8_t bit_nr = play_NR;
	while (bit_nr > 7)
	{
		byte_nr++;
		bit_nr = bit_nr - 8;
	}
	bitWrite(SaveConf.confCustomStatus[byte_nr], bit_nr, value);

	

}



// Reading Conf values from file.

bool get_bool_byte(uint8_t in_byte, uint8_t in_bit) 
{	
	// gives back the value of in_bit, the in bit is wrapped into a value from 0-7
	// 

	while (in_bit > 7) in_bit = in_bit - 8;


	if (bitRead(in_byte, in_bit) == true)
		return true;
	else return false;

}



// Check if thers another seiing in the config string to load
boolean AnotherSetting(char *character)
{
	if (*character != ']') 
	return true;
	else return false;

}


int	get_int_conf_value(File myFile, char *character, int def_value = 0 ) 
{
	// When reading from a file give back a INT value

	//char character;
	String settingName;
	String settingValue;

	if (*character != ']') 
	{
		*character = myFile.read();
		while ((myFile.available()) && (*character != ':')  && (*character != '.') && *character != ']') {
			settingValue = settingValue + *character;
			*character = myFile.read();
		}
		
		if (settingValue != NULL)
		{
			int outval = settingValue.toInt();
			yield();
			
			return outval;
		}
		else {
			return def_value;
			}
	}
	else
		return def_value;
}

bool get_bool_conf_value(File myFile, char *character) 
{
	// Read a bool value from a file	

	String settingValue;

	if (*character != ']') {
		*character = myFile.read();
		while ((myFile.available()) && (*character != ':') && *character != ']')
		{
			settingValue = settingValue + *character;
			*character = myFile.read();
		}
	}
	yield();
	if (settingValue[0] == '1')
		return true;
	else
		return false;
}

String get_string_conf_value(File myFile, char *character)
{

	String settingValue;

	if (*character != ']') {
		*character = myFile.read();
		while ((myFile.available()) && (*character != ':') && *character != ']') {
			settingValue = settingValue + *character;
			*character = myFile.read();
		}
		yield();
		return settingValue;
	}
	else
		return String("none");
}


int	get_IP_conf_value(File myFile, char *character)
{
	// When reading from a file give back a INT value but accept . as a value seperator makes reading the config easyser for IP addresses

	//char character;
	String settingName;
	String settingValue;

	if (*character != ']') {
		*character = myFile.read();
		while ((myFile.available()) && (*character != ':') && *character != '.' && *character != ']') {
			settingValue = settingValue + *character;
			*character = myFile.read();
		}

		if (settingValue != NULL)
		{
			int outval = settingValue.toInt();
			yield();

			return outval;
		}
		else {
			return 0;
		}
	}
	else
		return 0;
}


// fft settings

boolean FS_FFT_read(uint8_t conf_nr) 
{
	// read the FFT settings from file.
	// int conf_nr = what file NR to read

	String addr = String("/conf/" + String(conf_nr) + ".fft.txt");
	File conf_file = selectedFS.open(addr, "r");
	delay(100);
	if (conf_file && !conf_file.isDirectory()) {

		char character;
		String settingName;
		String settingValue;
		char type;
		//int bin_no = 0;
		 debugMe("FFT_File-opened");

		while (conf_file.available()) 
		{

			character = conf_file.read();
			while ((conf_file.available()) && (character != '[')) {  // Go to first setting
				character = conf_file.read();
			}

			type = conf_file.read();
			character = conf_file.read(); // go past the first ":"

			 if (type == 'I')
			{
				write_bool(FFT_ENABLE, get_bool_conf_value(conf_file, &character));
				write_bool(FFT_MASTER, get_bool_conf_value(conf_file, &character)) ;
				for (uint8_t i = 0; i < 4; i++) fft_ip_cfg.IP_multi[i] = get_int_conf_value(conf_file, &character);


				fft_ip_cfg.port_master		= get_int_conf_value(conf_file, &character);
				fft_ip_cfg.port_slave		= get_int_conf_value(conf_file, &character);

			}

			while ((conf_file.available()) && (character != ']'))   // go to end
				character = conf_file.read();

		}
		// close the file:
		conf_file.close();
		 debugMe("fft File-Closed");
		return true;
	}
	else {
		// if the file didn't open, print an error:
		Serial.println("error opening fft config " + addr);
	}
	
	return false;

}


void FS_FFT_write(uint8_t conf_nr) 
{
	// write a FFT conf file NR = conf_nr


	String title = "FFT";

	String addr = String("/conf/" + String(conf_nr) + ".fft.txt");

	File conf_file = selectedFS.open(addr, "w");

	if (!conf_file && !conf_file.isDirectory()) 
	{
		Serial.println("fft file creation failed");
	}
	else 
	{   // yeah its open

		conf_file.println(title);
		conf_file.print(String("FFT Settings I:  FFT Send enable =1 : FFT Master Mode = 1 : Multicast IP : Master Port : slave Port   "));
		conf_file.print(String("[I:" + String(get_bool(FFT_MASTER_SEND))));
		conf_file.print(String(":" + String(get_bool(FFT_MASTER))));
		conf_file.print(String(":" + String(fft_ip_cfg.IP_multi[0])));
		conf_file.print(String(":" + String(fft_ip_cfg.IP_multi[1])));
		conf_file.print(String(":" + String(fft_ip_cfg.IP_multi[2])));
		conf_file.print(String(":" + String(fft_ip_cfg.IP_multi[3])));
		conf_file.print(String(":" + String(fft_ip_cfg.port_master)));
		conf_file.print(String(":" + String(fft_ip_cfg.port_slave)));	
		conf_file.println("] ");
		conf_file.close();
	}
	
} // end FS_FFT_write()




// wifi
void FS_wifi_write()
{	//also save the bools
	FS_Bools_write(0);

	// write out the wifi config
	String addr = String("/conf/wifi.txt");
	//String title = "Main Config for ESP.";
	File conf_file = selectedFS.open(addr, "w");

	if (!conf_file && !conf_file.isDirectory())
	{
		 debugMe("Cant write Main Conf file");
	}
	else  // it opens
	{
		conf_file.println("Main Wifi Config for ESP.");
		conf_file.println("b = Wifi-booleans: Wifi Power 0=0ff, 1=on: Mode 0= Client 1 = Access point : 0= DHCP 1= static IP: OTA Update 1=on : HTTP Server 1=on: ");
		conf_file.print(String("[b:" 	+ String(get_bool(WIFI_POWER))));
		conf_file.print(String(":" 		+ String(get_bool(WIFI_MODE_TPM))));		// Wifi
		conf_file.print(String(":"	 	+ String(get_bool(STATIC_IP_ENABLED))));
		conf_file.print(String(":" 		+ String(get_bool(OTA_SERVER))));
		conf_file.print(String(":" 		+ String(get_bool(HTTP_ENABLED))));

		conf_file.println("] ");


		conf_file.println("w = Wifi : name and APname: AP Password : SSID : Password : wifi-channe (1-12): ip1-4: IP subnet 1-4 : IP DGW 1-4: IP DNS 1-4: NTP-FQDN l : wifi channel 1-12");
		conf_file.print(String("[w:" + String(wifi_cfg.APname)));
		conf_file.print(String(":" + String(wifi_cfg.APpassword)));
		conf_file.print(String(":" + String(wifi_cfg.ssid)));
		conf_file.print(String(":" + String(wifi_cfg.pwd)));
		conf_file.print(String(":" + String(wifi_cfg.wifiChannel)));

		conf_file.print(String(":" + String(wifi_cfg.ipStaticLocal[0])));
		conf_file.print(String("." + String(wifi_cfg.ipStaticLocal[1])));
		conf_file.print(String("." + String(wifi_cfg.ipStaticLocal[2])));
		conf_file.print(String("." + String(wifi_cfg.ipStaticLocal[3])));

		conf_file.print(String(":" + String(wifi_cfg.ipSubnet[0])));
		conf_file.print(String("." + String(wifi_cfg.ipSubnet[1])));
		conf_file.print(String("." + String(wifi_cfg.ipSubnet[2])));
		conf_file.print(String("." + String(wifi_cfg.ipSubnet[3])));

		conf_file.print(String(":" + String(wifi_cfg.ipDGW[0])));
		conf_file.print(String("." + String(wifi_cfg.ipDGW[1])));
		conf_file.print(String("." + String(wifi_cfg.ipDGW[2])));
		conf_file.print(String("." + String(wifi_cfg.ipDGW[3])));

		conf_file.print(String(":" + String(wifi_cfg.ipDNS[0])));
		conf_file.print(String("." + String(wifi_cfg.ipDNS[1])));
		conf_file.print(String("." + String(wifi_cfg.ipDNS[2])));
		conf_file.print(String("." + String(wifi_cfg.ipDNS[3])));

		conf_file.print(String(":" + String(wifi_cfg.ntp_fqdn)));
		conf_file.print(String(":" + String(wifi_cfg.wifiChannel)));

		

		

		conf_file.println("] ");
		conf_file.close();

		 debugMe("Wifi wrote conf");
	}	// end open conf file

	
} // end FS_wifi_write()

boolean FS_wifi_read()
{
	// read the wifi config
	
	String addr = String("/conf/wifi.txt");
	File conf_file = selectedFS.open(addr, "r");
	
	if (!conf_file) {
    debugMe("There was an error opening the file for writing");
	}
	else debugMe("conf file here ?");

  
	
	delay(100);


	if (conf_file && !conf_file.isDirectory())
	{
		 debugMe("Loading Wifi conf " + addr);
		char character;
		String settingValue;
		char type;
		debugMe("File-opened");
		
		memset(wifi_cfg.APname, 0, sizeof(wifi_cfg.APname));  // reset them to 0
		memset(wifi_cfg.APpassword, 0, sizeof(wifi_cfg.APpassword));
		memset(wifi_cfg.ssid, 0, sizeof(wifi_cfg.ssid));
		memset(wifi_cfg.pwd, 0, sizeof(wifi_cfg.pwd));
		memset(wifi_cfg.ntp_fqdn, 0, sizeof(wifi_cfg.ntp_fqdn));
		
		while (conf_file.available())
		{	
			

			character = conf_file.read();
			
			delay(100);
			while ((conf_file.available()) && (character != '[')) {  // Go to first setting
				character = conf_file.read();
			}
			
			type = conf_file.read();
			character = conf_file.read(); // go past the first ":" after the type

			if (type == 'b')   // wifi booleans

			{
				write_bool(WIFI_POWER, get_bool_conf_value(conf_file, &character));  		
				write_bool(WIFI_MODE_TPM, get_bool_conf_value(conf_file, &character));				
				write_bool(STATIC_IP_ENABLED, get_bool_conf_value(conf_file, &character));		
				write_bool(OTA_SERVER, get_bool_conf_value(conf_file, &character));			
				write_bool(HTTP_ENABLED, get_bool_conf_value(conf_file, &character));	




				
			}

			else if (type == 'w')   // Changed to 'w' from W' beacuse of new bools settings
			{
				
				settingValue = get_string_conf_value(conf_file, &character);
				settingValue.toCharArray(wifi_cfg.APname, settingValue.length() + 1);

				settingValue = get_string_conf_value(conf_file, &character);
				settingValue.toCharArray(wifi_cfg.APpassword, settingValue.length() + 1);

				settingValue = get_string_conf_value(conf_file, &character);
				settingValue.toCharArray(wifi_cfg.ssid, settingValue.length() + 1);

				settingValue = get_string_conf_value(conf_file, &character);
				settingValue.toCharArray(wifi_cfg.pwd, settingValue.length() + 1);

				uint8_t in_int = get_int_conf_value(conf_file, &character);	wifi_cfg.wifiChannel = uint8_t(constrain(in_int, 1, 12));

				for (uint8_t i = 0; i < 4; i++) wifi_cfg.ipStaticLocal[i] = get_IP_conf_value(conf_file, &character);
				for (uint8_t i = 0; i < 4; i++) wifi_cfg.ipSubnet[i] = get_IP_conf_value(conf_file, &character);
				for (uint8_t i = 0; i < 4; i++) wifi_cfg.ipDGW[i] = get_IP_conf_value(conf_file, &character);
				for (uint8_t i = 0; i < 4; i++) wifi_cfg.ipDNS[i] = get_IP_conf_value(conf_file, &character);
			
				settingValue = get_string_conf_value(conf_file, &character);
				settingValue.toCharArray(wifi_cfg.ntp_fqdn, settingValue.length() + 1);
				
				
				
								
			}
			while ((conf_file.available()) && (character != ']')) character = conf_file.read();   // goto End

		}
		
		conf_file.close();
		debugMe("done wifi load " + addr);
		return true;
	}	// end open conf file
	else  debugMe("error opening " + addr);
	
	conf_file.close();
	return false;
} // end FS_wifi_read()

//Artnet
#ifndef ARTNET_DISABLED
void	FS_artnet_write()
{
	// write the artnet config to disk

	String addr = String("/conf/artnet.txt");
	//String title = "Main Config for ESP.";
	File conf_file = selectedFS.open(addr, "w");

	if (!conf_file  && !conf_file.isDirectory())
	{
		 debugMe("Cant write  artnet Conf file");
	}
	else  // it opens
	{
		conf_file.println("Artnet Config for ESP.");
		conf_file.print(String("[A:" + String(get_bool(ARTNET_SEND))));
		conf_file.print(String(":" + String(artnet_cfg.startU)));
		conf_file.print(String(":" + String(artnet_cfg.numU)));
		conf_file.print(String(":" + String(get_bool(ARTNET_RECIVE))));

		conf_file.println("] ");

		conf_file.println("Artnet node Config for , Node Nr:StartU: Number Universe: IP.");
		for (uint8_t node = 0 ; node < ARTNET_NR_NODES_TPM; node++)
		{
			conf_file.print(String("[N:" + String(node)));
			conf_file.print(String(":" + String(artnetNode[node].startU)));
			conf_file.print(String(":" + String(artnetNode[node].numU)));
			conf_file.print(String(":" + String(artnetNode[node].IP[0])));
			conf_file.print(String("." + String(artnetNode[node].IP[1])));
			conf_file.print(String("." + String(artnetNode[node].IP[2])));
			conf_file.print(String("." + String(artnetNode[node].IP[3])));
			conf_file.println("] ");
		}

		
		conf_file.close();

		 debugMe("artnet conf wrote");
	}	// end open conf file


}

boolean FS_artnet_read()
{
	// read the artnet config from disk

	String addr = String("/conf/artnet.txt");
	File conf_file = selectedFS.open(addr, "r");
	delay(100);
	if (!conf_file && !conf_file.isDirectory()) {
		 debugMe("artnet file read failed");
	}
	else 
	{
		 debugMe("Opening " + addr);
		char character;
		//String settingName;
		//String settingValue;
		char type;


		while (conf_file.available())
		{
			character = conf_file.read();

			while ((conf_file.available()) && (character != '[')) {  // Go to first setting
				character = conf_file.read();
			}

			type = conf_file.read();
			character = conf_file.read(); // go past the first ":" after the type

			if (type == 'A')
			{
				write_bool(ARTNET_SEND,	get_bool_conf_value(conf_file, &character));
				artnet_cfg.startU =			get_int_conf_value(conf_file, &character);
				artnet_cfg.numU =			get_int_conf_value(conf_file, &character);
				write_bool(ARTNET_RECIVE,	get_bool_conf_value(conf_file, &character));

							
			}
			if (type == 'N')
			{
				uint8_t nodeNr = get_int_conf_value(conf_file, &character);
				artnetNode[nodeNr].startU = get_int_conf_value(conf_file, &character);
				artnetNode[nodeNr].numU   = get_int_conf_value(conf_file, &character);
				for (uint8_t i = 0; i < 4; i++) artnetNode[nodeNr].IP[i] = get_IP_conf_value(conf_file, &character);
			}
							
			




			while ((conf_file.available()) && (character != ']')) character = conf_file.read();
		

		}
		conf_file.close();
		return true;
	}	// end open conf file
	

	return false;
}
#endif



void FS_play_conf_clear(uint8_t conf_nr) 
{
	String addr = String("/conf/"+ String(conf_nr) + ".playConf.txt");
	debugMe("deleted save " + addr);
	{	if( selectedFS.remove(addr.c_str()))  debugMe("deleted save ok"); else debugMe("haha delete did not work"); }

	uint8_t byte_nr = 0;
	uint8_t bit_nr = conf_nr;
	while (bit_nr > 7)
	{
		byte_nr++;
		bit_nr = bit_nr - 8;
	}

	bitWrite(SaveConf.confStatus[byte_nr], bit_nr, false);

	osc_queu_MSG_rgb(String("/ostc/master/conf/l/"+String(conf_nr)), 255,0,0);
	osc_queu_MSG_VAL_STRING("/ostc/master/savename/" + String(conf_nr) , "-" ) ;

}	


void FS_pal_save(uint8_t save_no, uint8_t pal_no)
{
	String addr = String("/conf/" + String(save_no) + ".pal.txt");

	File conf_file = selectedFS.open(addr, "w");

	if (!conf_file && !conf_file.isDirectory()) {
		 debugMe("pallete file creation failed");
	}
		conf_file.println("Pallete Config.");
		conf_file.println("holds the 16 colors for a pallete");
		conf_file.println("p = pallete Config : R . G . B : R . G . B ... ");
		
		conf_file.print(String("[p:" + String(LEDS_pal_read( pal_no, 0, 0))));   
		conf_file.print(String("." + String(LEDS_pal_read(pal_no, 0, 1))));
		conf_file.print(String("." + String(LEDS_pal_read(pal_no, 0, 2))));
			

		for (uint8_t color = 1; color < 16; color++) 
		{
			conf_file.print(String(":" + String(LEDS_pal_read( pal_no, color, 0))));   
			conf_file.print(String("." + String(LEDS_pal_read(pal_no, color, 1))));
			conf_file.print(String("." + String(LEDS_pal_read(pal_no, color, 2))));
		}
		conf_file.println("] ");
		

}

void FS_pal_load(uint8_t load_nr,uint8_t pal_no)
{
String addr = String("/conf/" + String(load_nr) + ".pal.txt");
	 debugMe("READ Conf " + addr);
	File conf_file = selectedFS.open(addr, "r");
	delay(100);
	if (conf_file && !conf_file.isDirectory())
	{
		char character;
		char type;

		while (conf_file.available()) 
		{

			character = conf_file.read();
			while ((conf_file.available()) && (character != '[')) 
			{  // Go to first setting
				character = conf_file.read();
			}

			type = conf_file.read();
			character = conf_file.read(); // go past the first ":"
			
			int  in_int = 0;
			if (type == 'p')
			{
				
				for (uint8_t color = 0; color < 16; color++) {
					in_int = get_int_conf_value(conf_file, &character); 	LEDS_pal_write(pal_no, color, 0, in_int) ; // targetPalette[pal_no][color].r =
					in_int = get_int_conf_value(conf_file, &character); 	LEDS_pal_write(pal_no, color, 1, in_int);
					in_int = get_int_conf_value(conf_file, &character); 	LEDS_pal_write(pal_no, color, 2, in_int);

				}


			} 
		}


		conf_file.close();
	}

}


void FS_write_Strip_Config(uint8_t val)
{
	uint8_t selectedDeckNo = 0;
	String addr = String("/conf/" + String(val) + ".LampConf.txt");
	//String title = "Main Config for ESP.";
	
	yield();
	File conf_file = selectedFS.open(addr, "w");

	if (!conf_file && !conf_file.isDirectory()) {
		 debugMe("play file creation failed");
	}
	else 
	{   // yeah its open
		debugMe("Write Conf File");

		conf_file.print(String("[NM:"+ String(led_cfg.Led_Setup_confname))); 		
		conf_file.println("] ");

		for (uint8_t form = 0; form < NR_FORM_PARTS; form++) 
		{
			
			conf_file.print(String("[LD:" + String(form)));
			conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.form_cfg[form].start_led)));
			conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.form_cfg[form].nr_leds)));
			conf_file.println("] ");

		} 
	conf_file.close();

	}
}






void	FS_play_conf_write1(uint8_t val)
{
	// write the artnet config to disk
	uint8_t selectedDeckNo = 0;
	String addr = String("/conf/" + String(val) + ".playConf.txt");
	//String title = "Main Config for ESP.";
	
	yield();
	File conf_file = selectedFS.open(addr, "w");

	if (!conf_file && !conf_file.isDirectory()) {
		 debugMe("play file creation failed");
	}
	else {   // yeah its open
		debugMe("Write Conf File");
	
		conf_file.print(String("[NM:"+ String(deck[0].cfg.confname))); 		
		conf_file.println("] ");	
					
		//conf_file.println("Play Config.");
		//conf_file.println("LS = LED DEVICE Settings : Fire Cooling : Fire Sparking : Red : Green : Blue : Pallete Bri: Pallete FPS: Blend Invert : SPARE : fft scale : Global Bri");

			conf_file.print(String("[LS:" + String(deck[selectedDeckNo].cfg.led_master_cfg.r)));
			conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.led_master_cfg.g)));
			conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.led_master_cfg.b)));
			conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.led_master_cfg.pal_bri)));
			conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.led_master_cfg.pal_fps)));	
			
			conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.led_master_cfg.bri)));
			conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.layer.clear_start_led)));
			conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.layer.clear_Nr_leds)));
			conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.fft_config.Scale)));
			conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.fft_config.fftAutoMin)));
			conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.fft_config.fftAutoMax)));
			conf_file.println("] ");

		//conf_file.println("FC = form Config : Start Led : Nr Leds : Fade  ");
/*		for (uint8_t form = 0; form < NR_FORM_PARTS; form++) 
		{
			
			conf_file.print(String("[FC:" + String(form)));
			conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.form_cfg[form].start_led)));
			conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.form_cfg[form].nr_leds)));
			conf_file.println("] ");

		} 
*/		//conf_file.println("ly = Layers 0 to 47, ");
		conf_file.print("[ly:" + String(deck[selectedDeckNo].cfg.layer.select[0]) )	;
		for (uint8_t layer = 1 ; layer < MAX_LAYERS_SELECT ; layer++)
		{
				conf_file.print(":" + String(deck[selectedDeckNo].cfg.layer.select[layer]) )	;
		}
		conf_file.println("] ");

		
		for (uint8_t layer = 0 ; layer < NO_OF_SAVE_LAYERS ; layer++)
		{
			conf_file.print("[LC:" 	+ String(layer) )	;
			
			conf_file.print(":" 	+ String(deck[selectedDeckNo].cfg.layer.save_startLed[layer]) )	;
			conf_file.print(":" 	+ String(deck[selectedDeckNo].cfg.layer.save_NrLeds[layer]) )	;
			conf_file.print(":" 	+ String(deck[selectedDeckNo].cfg.layer.save_lvl[layer]) )	;
			conf_file.print(":" 	+ String(deck[selectedDeckNo].cfg.layer.save_mix[layer]) )	;

			conf_file.println("] ");
		}

		//conf_file.println("PF - Pallete FX Form ");
		for (uint8_t form = 0; form < NR_FORM_PARTS; form++) 
		{
			if (deck[selectedDeckNo].cfg.form_cfg[form].nr_leds > 0)
			{
				conf_file.print(String("[PF:" + String(form)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.form_fx_pal[form].level)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.form_fx_pal[form].index_start)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.form_fx_pal[form].index_add_led)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.form_fx_pal[form].index_add_frame)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.form_fx_pal[form].autoPalMode)));
				conf_file.println("] ");
			}
		}

		for (uint8_t form = 0; form < _M_NR_FORM_BYTES_; form++) 
		{
			if (deck[selectedDeckNo].cfg.form_cfg[form].nr_leds > 0)
			{
				conf_file.print(String("[PS:" + String(form)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.form_fx_pal_singles[form].pal)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.form_fx_pal_singles[form].mix_mode)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.form_fx_pal_singles[form].palSpeedBin)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.form_fx_pal_singles[form].triggerBin)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.form_fx_pal_singles[form].lvl_bin)));
				for (uint8_t setting = 0; setting < _M_NR_FORM_PAL_OPTIONS_; setting++) conf_file.print((String(":" + String(deck[selectedDeckNo].cfg.form_menu_pal[form][setting]))));
				conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.form_fx_pal_singles[form].master_lvl)));
				conf_file.println("] ");
			}
		}

		//conf_file.println("TF form fft  ");
		for (uint8_t form = 0; form < NR_FORM_PARTS; form++) 
		{
			if (deck[selectedDeckNo].cfg.form_cfg[form].nr_leds > 0)
			{
				conf_file.print(String("[TF:" + String(form)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.form_fx_fft[form].level)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.form_fx_fft[form].offset)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.form_fx_fft[form].extend)));
				conf_file.println("] ");
			}
		}
		for (uint8_t form = 0; form < _M_NR_FORM_BYTES_; form++) 
		{
			if (deck[selectedDeckNo].cfg.form_cfg[form].nr_leds > 0)
			{
				conf_file.print(String("[TC:" + String(form)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.form_fx_fft_signles[form].mix_mode)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.form_fx_fft_signles[form].triggerBin)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.form_fx_fft_signles[form].lvl_bin)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.form_fx_fft_signles[form].color)));
				for (uint8_t setting = 0; setting < _M_NR_FORM_FFT_OPTIONS_; setting++) conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.form_menu_fft[form][setting])));
				conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.form_fx_fft_signles[form].master_lvl)));
				conf_file.println("] ");
			}
		}

		for (uint8_t form = 0; form < NR_FX_PARTS; form++) 
		{
			if (deck[selectedDeckNo].cfg.form_cfg[form].nr_leds > 0)
			{
				conf_file.print(String("[DF:" + String(form)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_dots[form].nr_dots)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_dots[form].speed)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_dots[form].index_add)));

				conf_file.println("] ");
			}
		}
		
		for (uint8_t form = 0; form < NR_FX_BYTES; form++) 
		{
			if (deck[selectedDeckNo].cfg.form_cfg[form].nr_leds > 0)
			{
				conf_file.print(String("[DC:" + String(form)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_dots_bytes[form].pal)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_dots_bytes[form].level)));
				for (uint8_t setting = 0; setting < _M_NR_FORM_DOT_OPTIONS_; setting++) conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_menu_dot[form][setting])));
				conf_file.println("] ");
			}
		}
		
		//conf_file.println("SF shimmer ");
		for (uint8_t form = 0; form < NR_FX_PARTS; form++) 
		{
			if (deck[selectedDeckNo].cfg.form_cfg[form].nr_leds > 0)
			{
				conf_file.print(String("[SF:" + String(form)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_shim[form].xscale)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_shim[form].yscale)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_shim[form].beater)));


				conf_file.println("] ");
			}
		}
		for (uint8_t form = 0; form < NR_FX_BYTES; form++) 
		{
			if (deck[selectedDeckNo].cfg.form_cfg[form].nr_leds > 0)
			{
				conf_file.print(String("[SC:" + String(form)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_shim_bytes[form].pal)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_shim_bytes[form].mix_mode)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_shim_bytes[form].level)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_shim_bytes[form].triggerBin)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_shim_bytes[form].lvl_bin)));
				for (uint8_t setting = 0; setting < _M_NR_FORM_SHIMMER_OPTIONS_; setting++) conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_menu_shimmer[form][setting])));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_shim_bytes[form].master_lvl)));
				conf_file.println("] ");
			}
		}

		for (uint8_t form = 0; form < NR_FX_PARTS; form++) 
		{
			if (deck[selectedDeckNo].cfg.form_cfg[form].nr_leds > 0)
			{
				conf_file.print(String("[CK:" + String(form)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_clock[form].color )));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_clock[form].level )));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_clock[form].pal_speed )));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_clock[form].pal_compression )));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_clock[form].offset )));


				conf_file.println("] ");
			}
		}
		for (uint8_t form = 0; form < NR_FX_BYTES; form++) 
		{
			if (deck[selectedDeckNo].cfg.form_cfg[form].nr_leds > 0)
			{
				conf_file.print(String("[CB:" + String(form)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_clock_bytes[form].mix_mode)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_clock_bytes[form].master_lvl)));
				for (uint8_t setting = 0; setting < _M_NR_FORM_CLOCK_OPTIONS_; setting++) conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_menu_clock[form][setting])));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_clock_bytes[form].type )));
				
				conf_file.println("] ");
			}
		}



		//conf_file.println("IF form Fire  ");
		for (uint8_t form = 0; form < NR_FX_BYTES; form++) 
		{
			if (deck[selectedDeckNo].cfg.form_cfg[form].nr_leds > 0)
			{
				conf_file.print(String("[IF:" + String(form)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_fire_bytes[form].pal)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_fire_bytes[form].mix_mode)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_fire_bytes[form].level)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_fire_bytes[form].cooling)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_fire_bytes[form].sparking)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_fire_bytes[form].triggerBin)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_fire_bytes[form].lvl_bin)));
				for (uint8_t setting = 0; setting < _M_NR_FORM_FIRE_OPTIONS_; setting++) conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_menu_fire[form][setting])));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_fire_bytes[form].master_lvl)));
				conf_file.println("] ");
			}
		}



		//conf_file.println("EF form eyes  ");
		for (uint8_t form = 0; form < NR_FX_BYTES; form++) 
		{
			if (deck[selectedDeckNo].cfg.form_cfg[form].nr_leds > 0)
			{
				conf_file.print(String("[EF:" + String(form)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_eyes_bytes[form].color)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_eyes_bytes[form].mix_mode)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_eyes_bytes[form].level)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_eyes_bytes[form].triggerBin)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_eyes_bytes[form].lvl_bin)));

				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_eyes_bytes[form].on_frames)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_eyes_bytes[form].EyeWidth)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_eyes_bytes[form].EyeSpace)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_eyes_bytes[form].fadeval)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_eyes_bytes[form].pause_frames)));
				for (uint8_t setting = 0; setting < _M_NR_FORM_EYES_OPTIONS_; setting++) conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_menu_eyes[form][setting])));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_eyes_bytes[form].master_lvl)));
				conf_file.println("] ");
			}
		}


		//conf_file.println("TF form eyes  ");
		for (uint8_t form = 0; form < NR_FX_BYTES; form++) 
		{
			if (deck[selectedDeckNo].cfg.form_cfg[form].nr_leds > 0)
			{
				conf_file.print(String("[OF:" + String(form)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_strobe_bytes[form].pal)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_strobe_bytes[form].mix_mode)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_strobe_bytes[form].level)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_strobe_bytes[form].triggerBin)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_strobe_bytes[form].lvl_bin)));

				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_strobe_bytes[form].on_frames)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_strobe_bytes[form].off_frames)));
				for (uint8_t setting = 0; setting < _M_NR_FORM_STROBE_OPTIONS_; setting++) conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_menu_strobe[form][setting])));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_strobe_bytes[form].master_lvl)));
				conf_file.println("] ");
			}
		}



		//conf_file.println("TF form eyes  ");
		for (uint8_t form = 0; form < NR_FX_PARTS; form++) 
		{
			if (deck[selectedDeckNo].cfg.form_cfg[form].nr_leds > 0)
			{
				conf_file.print(String("[MF:" + String(form)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_meteor[form].meteorSize)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_meteor[form].meteorTrailDecay)));

				conf_file.println("] ");
			}
		}
		for (uint8_t form = 0; form < NR_FX_BYTES; form++) 
		{
			if (deck[selectedDeckNo].cfg.form_cfg[form].nr_leds > 0)
			{
				conf_file.print(String("[MC:" + String(form)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_meteor_bytes[form].color)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_meteor_bytes[form].level)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_meteor_bytes[form].triggerBin)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_meteor_bytes[form].lvl_bin)));
				for (uint8_t setting = 0; setting < _M_NR_FORM_METEOR_OPTIONS_; setting++) conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_menu_meteor[form][setting])));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_meteor_bytes[form].master_lvl)));
				conf_file.println("] ");
			}
		}

		//conf_file.println("GF Glitter ");
		for (uint8_t form = 0; form < NR_FX_PARTS; form++) 
		{
			if (deck[selectedDeckNo].cfg.form_cfg[form].nr_leds > 0)
			{
				conf_file.print(String("[GF:" + String(form)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_glitter[form].value)));


				conf_file.println("] ");
			}
		}
		for (uint8_t form = 0; form < NR_FX_BYTES; form++) 
		{
			if (deck[selectedDeckNo].cfg.form_cfg[form].nr_leds > 0)
			{
				conf_file.print(String("[GC:" + String(form)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_glitter_bytes[form].pal)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_glitter_bytes[form].level)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx_glitter_bytes[form].glit_bin)));
				for (uint8_t setting = 0; setting < _M_NR_FORM_GLITTER_OPTIONS_; setting++) conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_menu_glitter[form][setting])));
				


				conf_file.println("] ");
			}
		}


		//conf_file.println("XF  FX1 form ");
		for (uint8_t form = 0; form < NR_FX_BYTES; form++) 
		{
			if (deck[selectedDeckNo].cfg.form_cfg[form].nr_leds > 0)
			{

				
				conf_file.print(String("[XC:" + String(form)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx1[form].level )));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx1[form].mix_mode)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx1[form].fade)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx1[form].triggerBin)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx1[form].lvl_bin)));
				for (uint8_t setting = 0; setting < _M_NR_FORM_FX1_OPTIONS_; setting++) conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_menu_fx1[form][setting])));
				conf_file.print(String(":" + String(deck[selectedDeckNo].fx1_cfg.form_fx1[form].master_lvl)));
				conf_file.println("] ");
			}
		}


#ifdef USE_SD  // when writing to SD we need to split the file into appends since it writes to mem and then to disk 
							// When writing to SPIFFS its faster to do it all in one go and not append it since here we write traigt to the SPIFFS
		
				conf_file.close();
		
	}


}


void  FS_play_conf_write_append1(uint8_t val)
{
	uint8_t selectedDeckNo = 0;
	String addr = String("/conf/" + String(val) + ".playConf.txt");
	//String title = "Main Config for ESP.";
	
	yield();
	File conf_file = selectedFS.open(addr, "a");

	if (!conf_file && !conf_file.isDirectory()) {
		 debugMe("play file creation failed");
	}
	else {   // yeah its open
		debugMe("Write Conf File");


#endif   // SD


		//conf_file.println("Modify ");
		for (uint8_t form = 0; form < NR_FORM_PARTS; form++) 
		{
			if (deck[selectedDeckNo].cfg.form_cfg[form].nr_leds > 0)
			{
				conf_file.print(String("[YF:" + String(form)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.form_fx_modify[form].RotateFixed)));
				conf_file.println("] ");
			}
		}
		for (uint8_t form = 0; form < _M_NR_FORM_BYTES_; form++) 
		{
			if (deck[selectedDeckNo].cfg.form_cfg[form].nr_leds > 0)
			{
				conf_file.print(String("[YC:" + String(form)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.form_fx_modify_bytes[form].RotateFullFrames)));
				conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.form_fx_modify_bytes[form].RotateTriggerBin)));
				for (uint8_t setting = 0; setting < _M_NR_FORM_MODIFY_OPTIONS_; setting++) conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.form_menu_modify[form][setting])));
				conf_file.println("] ");
			}
		}

		
	
		//conf_file.println("PA = pallete Config : Pal Nr : R : G :B : R : G : B ... ");
		for (uint8_t pal = 0; pal < NR_PALETTS; pal++) {
			conf_file.print(String("[PA:" + String(pal)));

			for (uint8_t color = 0; color < 16; color++) {
				conf_file.print(String(":" + String(LEDS_pal_read( pal, color, 0))));   // targetPalette[pal][color].r)));
				conf_file.print(String("." + String(LEDS_pal_read(pal, color, 1))));
				conf_file.print(String("." + String(LEDS_pal_read(pal, color, 2))));
			}
			conf_file.println("] ");
		}
		
		//conf_file.println("A = FFT Bin Config : Pal Nr : BIN Nr : Trigger : Into R : Into G : into B ");
		//for (int bin = 0; bin < 7; bin++) 
		{							// Save FFT settings   

			conf_file.print(String("[AM:" 	+ String(uint8_t(deck[selectedDeckNo].cfg.fft_config.fft_menu_bri))));
			conf_file.print(String(":" 		+ String(uint8_t(deck[selectedDeckNo].cfg.fft_config.fft_bin_autoTrigger))));
			conf_file.print(String(":" 		+ String(uint8_t(deck[selectedDeckNo].cfg.fft_config.fft_menu_fps))));

			for (int color = 0; color < 3; color++)		conf_file.print(String(":" + String(uint8_t(deck[selectedDeckNo].cfg.fft_config.fft_menu[color]))));
			for (int bin = 0; bin < 7; bin++)  	conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.fft_config.trigger[bin])));
			
			conf_file.println("] ");
		}
		
		for (int fxbin = 0; fxbin < FFT_FX_NR_OF_BINS; fxbin++) 	
		{
			//if (deck[selectedDeckNo].cfg.fft_config.fft_fxbin[fxbin].set_val != 0 || deck[selectedDeckNo].cfg.fft_config.fft_fxbin[fxbin].menu_select != 0 || deck[selectedDeckNo].cfg.fft_config.fft_fxbin[fxbin].trrig_val != 0)
			{
			conf_file.print(String("[AF:" + String(fxbin)));
			conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.fft_config.fft_fxbin[fxbin].set_val)));
			conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.fft_config.fft_fxbin[fxbin].trrig_val)));
			conf_file.print(String(":" + String(deck[selectedDeckNo].cfg.fft_config.fft_fxbin[fxbin].menu_select)));
			conf_file.println("] ");
			}
		}
		//*/
		conf_file.close();
		
	}


}





//play conf
void FS_play_conf_write(uint8_t conf_nr) 
{

	FS_play_conf_write1(conf_nr);

	#ifdef USE_SD   // when writing to SD we need to split the file into appends since it writes to mem and then to disk 
		FS_play_conf_write_append1(conf_nr);
	#endif
	FS_write_Conf_status(conf_nr, true);
	osc_queu_MSG_rgb(String("/ostc/master/conf/l/"+String(conf_nr)), 0,255,0);
	osc_queu_MSG_VAL_STRING("/ostc/master/savename/" + String(conf_nr)  , deck[0].cfg.confname);
}





void FS_play_conf_readSendSavenames( ) 
{
	// Read the Play config NR
	
	String addrList = String("/conf/Savelist.txt");
	File List_conf_file = selectedFS.open(addrList, "w");
	List_conf_file.println( "Nr:Name");
	uint8_t Bundle_Counter = 0;

	for (uint8_t confNo = 0; confNo < MAX_NR_SAVES ; confNo++)
	{

		String addr = String("/conf/" + String(confNo) + ".playConf.txt");
		debugMe("READ Conf " + addr);
		File conf_file = selectedFS.open(addr, "r");
		String settingValue;
		String OSCAddress = "/ostc/master/savename/" + String(confNo);
		

		delay(100);
		if (conf_file && !conf_file.isDirectory())
		{

			char Confname[32];
			char character;
			char type;
			char typeb;
			// debugMe("File-opened");
			FS_write_Conf_status(confNo,true);
			while (conf_file.available()) 
			{

				character = conf_file.read();
				while ((conf_file.available()) && (character != '[')) 
				{  // Go to first setting
					character = conf_file.read();
				}

				type = conf_file.read();
				typeb = conf_file.read();
				character = conf_file.read(); // go past the first ":"
				
				int  in_int = 0;

				

				if ((type == 'N') && (typeb == 'M'))
				{
					
					
					memset(Confname, 0, sizeof(Confname));
					settingValue = get_string_conf_value(conf_file, &character);
					settingValue.toCharArray(Confname, settingValue.length() + 1);
					debugMe("checking Conf :", false);
					debugMe(String(Confname));

					
					osc_queu_MSG_VAL_STRING(OSCAddress, Confname);
					//osc_StC_Send_CharArray(OSCAddress, Confname) ;
					osc_queu_MSG_rgb(String("/ostc/master/conf/l/"+String(confNo)), 0,255,0);
					
					break;
				}
				
				
			}
			// close the file:
			conf_file.close();
			debugMe("play-File-Closed");

			//String addrList = String("/conf/Savelist.txt");
			//File List_conf_file = selectedFS.open(addrList, "a");
			List_conf_file.println(String(confNo) +  ":" + settingValue);
			
			Bundle_Counter = Bundle_Counter +1;
			if (Bundle_Counter > 8 )
			{
				Bundle_Counter = 0;
				osc_send_out_float_MSG_buffer() ;
				debugMe("one Set");
			}

			
			
		}
		else
		{
			FS_write_Conf_status(confNo,false);
			osc_queu_MSG_VAL_STRING(OSCAddress, " - ") ;
			osc_queu_MSG_rgb(String("/ostc/master/conf/l/"+String(confNo)), 0,255,0);

			//List_conf_file.println(String(confNo) +  ": - " );
		}
		
	}

	List_conf_file.close();

	
}


void FS_play_conf_custom_readSendSavenames( ) 
{
	// Read the Play config NR
	String addrList = String("/conf/CSavelist.txt");
	File List_conf_file = selectedFS.open(addrList, "w");
	List_conf_file.println( "Nr:Name");

	for (uint8_t confNo = 0; confNo < Nr_CustomConfs ; confNo++)
	{
		//SaveConf.confCustomLoadNrs[confNo]
		String OSCAddress = "/ostc/master/custsave/" + String(confNo);

		String addr = String("/conf/" + String(SaveConf.confCustomLoadNrs[confNo]) + ".playConf.txt");
		debugMe("READ Conf " + addr);
		File conf_file = selectedFS.open(addr, "r");
		String settingValue;

		osc_queu_MSG_int(String("/ostc/master/cnum/"+String(confNo)),SaveConf.confCustomLoadNrs[confNo]);


		delay(100);
		if (conf_file && !conf_file.isDirectory())
		{

			char Confname[32];
			char character;
			char type;
			char typeb;
			// debugMe("File-opened");


			FS_write_Custom_Conf_status(confNo,true);
			//bitWrite(SaveConf.confCustomStatus[byte_nr], bit_nr, true);





			while (conf_file.available()) 
			{

				character = conf_file.read();
				while ((conf_file.available()) && (character != '[')) 
				{  // Go to first setting
					character = conf_file.read();
				}

				type = conf_file.read();
				typeb = conf_file.read();
				character = conf_file.read(); // go past the first ":"
				
				int  in_int = 0;

				

				if ((type == 'N') && (typeb == 'M'))
				{
					
					
					memset(Confname, 0, sizeof(Confname));
					settingValue = get_string_conf_value(conf_file, &character);
					settingValue.toCharArray(Confname, settingValue.length() + 1);
					debugMe("checking Conf :", false);
					debugMe(String(Confname));
					

					osc_queu_MSG_VAL_STRING(OSCAddress, Confname) ;
					osc_queu_MSG_rgb(String("/ostc/master/conf/l/"+String(confNo)), 0,255,0);
					
					break;
				}
				
				
			}
			// close the file:
			conf_file.close();
			debugMe("custom play-File-Closed");
			List_conf_file.println(String(SaveConf.confCustomLoadNrs[confNo]) +  ":" + settingValue);
			
			
		}
		else
		{
			// if the file didn't open, print an error:
			osc_queu_MSG_VAL_STRING(OSCAddress, " - ") ;
			FS_write_Custom_Conf_status(confNo,false);
			osc_queu_MSG_rgb(String("/ostc/master/conf/l/"+String(confNo)), 255,0,0);
		}
		
	}

	List_conf_file.close();
}

void FS_get_Strip_Config_list( )
{

	// Read the Play config NR
	String addrList = String("/conf/lamps.txt");
	File List_conf_file = selectedFS.open(addrList, "w");
	List_conf_file.print( "{ ");
	
	String ExportString ; 

	for (uint8_t confNo = 0; confNo < MAX_LAMP_CONFIGS ; confNo++)
	{
		//SaveConf.confCustomLoadNrs[confNo]
		//String OSCAddress = "/ostc/master/custsave/" + String(confNo);

		String addr = String("/conf/" + String(confNo)  + ".LampConf.txt" );
		debugMe("READ Conf " + addr);
		File conf_file = selectedFS.open(addr, "r");

		String settingValue;

		//osc_queu_MSG_int(String("/ostc/master/cnum/"+String(confNo)),SaveConf.confCustomLoadNrs[confNo]);


		delay(100);
		if (conf_file && !conf_file.isDirectory())
		{

			char Confname[24];
			char character;
			char type;
			char typeb;
			// debugMe("File-opened");


			//FS_write_Custom_Conf_status(confNo,true);
			//bitWrite(SaveConf.confCustomStatus[byte_nr], bit_nr, true);





			while (conf_file.available()) 
			{

				character = conf_file.read();
				while ((conf_file.available()) && (character != '[')) 
				{  // Go to first setting
					character = conf_file.read();
				}

				type = conf_file.read();
				typeb = conf_file.read();
				character = conf_file.read(); // go past the first ":"
				
			//	int  in_int = 0;

				

				if ((type == 'N') && (typeb == 'M'))
				{
					
					
					memset(Confname, 0, sizeof(Confname));
					settingValue = get_string_conf_value(conf_file, &character);
					settingValue.toCharArray(Confname, settingValue.length() + 1);
					debugMe("checking Conf :", false);
					debugMe(String(Confname));
					
					if (confNo > 0) {List_conf_file.print(", "); ExportString = ExportString + ", " ; }
					List_conf_file.println(String("\""+ String(confNo) + " : " + settingValue + "\" : " + String(confNo) ) )   ;
					ExportString = ExportString + String("\"" + String(confNo) + " : "  + settingValue + "\" : " + String(confNo) ) ; 
					//osc_queu_MSG_VAL_STRING(OSCAddress, Confname) ;
					//osc_queu_MSG_rgb(String("/ostc/master/conf/l/"+String(confNo)), 0,255,0);
					
					
				}
				break;
				
			}
			// close the file:
			conf_file.close();
			//debugMe("custom play-File-Closed");
			
			
			
		}
		else
		{
			// if the file didn't open, print an error:
			//osc_queu_MSG_VAL_STRING(OSCAddress, " - ") ;
			//FS_write_Custom_Conf_status(confNo,false);
			//osc_queu_MSG_rgb(String("/ostc/master/conf/l/"+String(confNo)), 255,0,0);
		}
		
	}
	List_conf_file.println( " }");
	List_conf_file.close();

	ExportString = "{ "  + ExportString + " }";
	debugMe(ExportString);
	//osc_queu_MSG_VAL_STRING("/ostc/master/Lamp/Versions", ExportString);
	osc_Send_String("/ostc/master/Lamp/Versions" , ExportString);
}





boolean FS_read_Strip_Config(uint8_t conf_nr, deck_cfg_struct* DeckConf , led_cfg_struct* LedConf )
{

	String addr = String("/conf/" + String(conf_nr) + ".LampConf.txt");
	 debugMe("READ Conf " + addr);
	File conf_file = selectedFS.open(addr, "r");
	String settingValue;

	delay(100);
	if (conf_file && !conf_file.isDirectory())
	{

		char character;
		char type;
		char typeb;
		int strip_no = 0;
		debugMe("File-opened");
		while (conf_file.available()) 
		{

			character = conf_file.read();
			while ((conf_file.available()) && (character != '[')) 
			{  // Go to first setting
				character = conf_file.read();
			}

			type = conf_file.read();
			typeb = conf_file.read();
			character = conf_file.read(); // go past the first ":"
			
			int  in_int = 0;

			if ((type == 'N') && (typeb == 'M'))
			{
				
				memset(LedConf->Led_Setup_confname, 0, sizeof(LedConf->Led_Setup_confname));
				settingValue = get_string_conf_value(conf_file, &character);
				settingValue.toCharArray(LedConf->Led_Setup_confname, settingValue.length() + 1);
				debugMe("Loading Conf :", false);
				debugMe(String(LedConf->Led_Setup_confname));
			}
				else if ((type == 'L') && (typeb == 'D'))  
			{
				strip_no = get_int_conf_value(conf_file, &character);
				if(AnotherSetting(&character)) {in_int = get_int_conf_value(conf_file, &character); DeckConf->form_cfg[strip_no].start_led = constrain(in_int, 0, led_cfg.NrLeds);}
				if(AnotherSetting(&character)) {in_int = get_int_conf_value(conf_file, &character); DeckConf->form_cfg[strip_no].nr_leds = constrain(in_int, 0, led_cfg.NrLeds - DeckConf->form_cfg[strip_no].start_led);}
			}


			while ((conf_file.available()) && (character != ']')) 
			{ 
				character = conf_file.read();
			}


			
		}
		// close the file:
		conf_file.close();

		
		
		return true;
	}
	else
	{
		// if the file didn't open, print an error:
		Serial.println("error opening " + addr);
	}
	 debugMe("play-File-Closed");


	return false;
}


boolean FS_play_conf_read(uint8_t conf_nr, deck_cfg_struct* targetConf  ,deck_fx1_struct* targetFXConf ) 
{
	// Read the Play config NR

	/*
		LS LED DEvice Settings
		FC Form Config

		PF Palette FX
		PB Palette FX Booleans
		PS

		DF FX Dots
		DB FX Dots Bools

		SF FX Shimm
		SB FX Shimm bools

		TF FX FFT
		TC
		TB

		IF FX Fire
		IB 

		MF Meteor
		MB 

		OF Strobe
		OB

		EF FX Eye
		EB

		GF Glitter
		GB

		CK	Clock
		CB

		XF FX01
		XB

		YF  MOdify
		YB 

		CL Copy Leds

		PA Palette

		AM Audio FFT 
		AF FX Bins

		ly Layers
		LM Layer save Mix
		LL Layer Save Level

	*/

	String addr = String("/conf/" + String(conf_nr) + ".playConf.txt");
	 debugMe("READ Conf " + addr);
	File conf_file = selectedFS.open(addr, "r");
	String settingValue;

	delay(100);
	if (conf_file && !conf_file.isDirectory())
	{

		char character;
		char type;
		char typeb;
		int strip_no = 0;
		debugMe("File-opened");
		while (conf_file.available()) 
		{

			character = conf_file.read();
			while ((conf_file.available()) && (character != '[')) 
			{  // Go to first setting
				character = conf_file.read();
			}

			type = conf_file.read();
			typeb = conf_file.read();
			character = conf_file.read(); // go past the first ":"
			
			int  in_int = 0;

			

			if ((type == 'N') && (typeb == 'M'))
			{
				
				memset(targetConf->confname, 0, sizeof(targetConf->confname));
				settingValue = get_string_conf_value(conf_file, &character);
				settingValue.toCharArray(targetConf->confname, settingValue.length() + 1);
				debugMe("Loading Conf :", false);
				debugMe(String(targetConf->confname));
			}
			

			else if ((type == 'L') && (typeb == 'S'))
			{
												in_int = get_int_conf_value(conf_file, &character);	targetConf->led_master_cfg.r					= uint8_t(constrain(in_int, 0, 255));
				if(AnotherSetting(&character)) {in_int = get_int_conf_value(conf_file, &character);	targetConf->led_master_cfg.g					= uint8_t(constrain(in_int, 0, 255));}
				if(AnotherSetting(&character)) {in_int = get_int_conf_value(conf_file, &character);	targetConf->led_master_cfg.b					= uint8_t(constrain(in_int, 0, 255));}
				if(AnotherSetting(&character)) {in_int = get_int_conf_value(conf_file, &character);	targetConf->led_master_cfg.pal_bri				= uint8_t(constrain(in_int, 0, 255));}
				if(AnotherSetting(&character)) {in_int = get_int_conf_value(conf_file, &character);	if (!get_bool(POTS_LVL_MASTER)) targetConf->led_master_cfg.pal_fps     			= uint8_t(constrain(in_int, 1, MAX_PAL_FPS));}
				
				if(AnotherSetting(&character)) {in_int = get_int_conf_value(conf_file, &character);	if (!get_bool(POTS_LVL_MASTER))   targetConf->led_master_cfg.bri					= uint8_t(constrain( in_int, 0, 255));}
				if(AnotherSetting(&character)) {in_int = get_int_conf_value(conf_file, &character);	targetConf->layer.clear_start_led				= uint16_t(constrain(in_int, 0, led_cfg.NrLeds)); }
				if(AnotherSetting(&character)) {in_int = get_int_conf_value(conf_file, &character);	targetConf->layer.clear_Nr_leds					= uint16_t(constrain( in_int, 0, led_cfg.NrLeds ));  }
				if(AnotherSetting(&character)) {in_int = get_int_conf_value(conf_file, &character);	targetConf->fft_config.Scale  					= uint8_t(constrain( in_int, 1, 100)); }
				if(AnotherSetting(&character)) {in_int = get_int_conf_value(conf_file, &character);	targetConf->fft_config.fftAutoMin 				= uint8_t(constrain(in_int, 0,255));}
				if(AnotherSetting(&character)) {in_int = get_int_conf_value(conf_file, &character);	targetConf->fft_config.fftAutoMax 				= uint8_t(constrain(in_int, 0,255));}

			}
#ifdef LOAD_LEDS_FROM_PLAYCONFIG	
			else if ((type == 'F') && (typeb == 'C'))  
			{
				strip_no = get_int_conf_value(conf_file, &character);
				if(AnotherSetting(&character)) {in_int = get_int_conf_value(conf_file, &character); targetConf->form_cfg[strip_no].start_led = constrain(in_int, 0, led_cfg.NrLeds);}
				if(AnotherSetting(&character)) {in_int = get_int_conf_value(conf_file, &character); targetConf->form_cfg[strip_no].nr_leds = constrain(in_int, 0, led_cfg.NrLeds - targetConf->form_cfg[strip_no].start_led);}
			}
#endif			
			else if ((type == 'P') && (typeb == 'F'))
			{
				strip_no = get_int_conf_value(conf_file, &character);
				if(AnotherSetting(&character)) {in_int = get_int_conf_value(conf_file, &character); targetConf->form_fx_pal[strip_no].level = in_int; }
				if(AnotherSetting(&character)) {in_int = get_int_conf_value(conf_file, &character); targetConf->form_fx_pal[strip_no].index_start = in_int ;}
				if(AnotherSetting(&character)) {in_int = get_int_conf_value(conf_file, &character); targetConf->form_fx_pal[strip_no].index_add_led = in_int;}
				if(AnotherSetting(&character)) {in_int = get_int_conf_value(conf_file, &character); targetConf->form_fx_pal[strip_no].index_add_frame = in_int; }
				if(AnotherSetting(&character)) {in_int = get_int_conf_value(conf_file, &character); targetConf->form_fx_pal[strip_no].autoPalMode = in_int;   LEDS_G_AutoCalcPal(0,strip_no);    }



				
			}	
			else if ((type == 'P') && (typeb == 'S'))
			{
				strip_no = get_int_conf_value(conf_file, &character);
				if(AnotherSetting(&character)) {in_int = get_int_conf_value(conf_file, &character); targetConf->form_fx_pal_singles[strip_no].pal 			= in_int; }
				if(AnotherSetting(&character)) {in_int = get_int_conf_value(conf_file, &character); targetConf->form_fx_pal_singles[strip_no].mix_mode 		= in_int; }
				if(AnotherSetting(&character)) {in_int = get_int_conf_value(conf_file, &character); targetConf->form_fx_pal_singles[strip_no].palSpeedBin 	= in_int; }
				if(AnotherSetting(&character)) {in_int = get_int_conf_value(conf_file, &character); targetConf->form_fx_pal_singles[strip_no].triggerBin 	= in_int; }
				if(AnotherSetting(&character)) {in_int = get_int_conf_value(conf_file, &character); targetConf->form_fx_pal_singles[strip_no].lvl_bin 		= in_int; 	}
				if(AnotherSetting(&character)) {for (uint8_t setting = 0; setting < _M_NR_FORM_PAL_OPTIONS_; setting++) if(AnotherSetting(&character)) {in_int = get_int_conf_value(conf_file, &character);   targetConf->form_menu_pal[strip_no][setting] = in_int; } }
				if(AnotherSetting(&character)) {in_int = get_int_conf_value(conf_file, &character); targetConf->form_fx_pal_singles[strip_no].master_lvl 		= in_int; 	} else targetConf->form_fx_pal_singles[strip_no].master_lvl  = 255 ;
			}
			else if ((type == 'P') && (typeb == 'B'))
			{
				strip_no = get_int_conf_value(conf_file, &character);
				for (uint8_t setting = 0; setting < _M_NR_FORM_PAL_OPTIONS_; setting++) 
					if(AnotherSetting(&character)) { bitWrite(targetConf->form_menu_pal[get_strip_menu_bit(strip_no)][setting], striptobit(strip_no), get_bool_conf_value(conf_file, &character)); }
			}
			
			
			else if ((type == 'T') && (typeb == 'F'))	
			{
				strip_no = get_int_conf_value(conf_file, &character);
				if(AnotherSetting(&character)) {in_int = get_int_conf_value(conf_file, &character); targetConf->form_fx_fft[strip_no].level = in_int; }
				if(AnotherSetting(&character)) {in_int = get_int_conf_value(conf_file, &character); targetConf->form_fx_fft[strip_no].offset = in_int;}
				if(AnotherSetting(&character)) {if (character != ']')  {in_int = get_int_conf_value(conf_file, &character); targetConf->form_fx_fft[strip_no].extend = in_int;} 	else {targetConf->form_fx_fft[strip_no].extend = 0;} }
			}
			else if ((type == 'T') && (typeb == 'C'))	
			{
				strip_no = get_int_conf_value(conf_file, &character);
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetConf->form_fx_fft_signles[strip_no].mix_mode = in_int; }
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetConf->form_fx_fft_signles[strip_no].triggerBin = in_int; }	else  targetConf->form_fx_fft_signles[strip_no].triggerBin  = 255;
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetConf->form_fx_fft_signles[strip_no].lvl_bin = in_int; }   	else  targetConf->form_fx_fft_signles[strip_no].lvl_bin  = 255;
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetConf->form_fx_fft_signles[strip_no].color = in_int; }   		else  targetConf->form_fx_fft_signles[strip_no].color  = 0; 
				for (uint8_t setting = 0; setting < _M_NR_FORM_FFT_OPTIONS_; setting++)  if(AnotherSetting(&character)) {in_int = get_int_conf_value(conf_file, &character); targetConf->form_menu_fft[strip_no][setting] = in_int;  }
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetConf->form_fx_fft_signles[strip_no].master_lvl = in_int; }  	else  targetConf->form_fx_fft_signles[strip_no].master_lvl  = 255; 
			}
			else if ((type == 'D') && (typeb == 'F'))	
			{
				strip_no = get_int_conf_value(conf_file, &character);
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_dots[strip_no].nr_dots = in_int;}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_dots[strip_no].speed = constrain(in_int,1,MAX_JD_SPEED_VALUE);}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_dots[strip_no].index_add = constrain(in_int,1,MAX_JD_SPEED_VALUE);}
			}
			else if ((type == 'D') && (typeb == 'C'))	
			{
				strip_no = get_int_conf_value(conf_file, &character);
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_dots_bytes[strip_no].pal = in_int; }
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_dots_bytes[strip_no].level = in_int; }
				for (uint8_t setting = 0; setting < _M_NR_FORM_DOT_OPTIONS_; setting++)  if(AnotherSetting(&character))  { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_menu_dot[strip_no][setting] = in_int; }
			}

			else if ((type == 'S') && (typeb == 'F'))
			{
				strip_no = get_int_conf_value(conf_file, &character);

				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_shim[strip_no].xscale = in_int; }
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_shim[strip_no].yscale = in_int; }
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_shim[strip_no].beater = in_int; }

			}
			else if ((type == 'S') && (typeb == 'C'))
			{
				strip_no = get_int_conf_value(conf_file, &character);
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_shim_bytes[strip_no].pal = in_int; }
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_shim_bytes[strip_no].mix_mode = in_int; }
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_shim_bytes[strip_no].level = in_int; }
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_shim_bytes[strip_no].triggerBin = in_int;} 
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_shim_bytes[strip_no].lvl_bin = in_int; }
				for (uint8_t setting = 0; setting < _M_NR_FORM_SHIMMER_OPTIONS_; setting++)  if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character);  targetFXConf->form_menu_shimmer[strip_no][setting] =in_int; }
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_shim_bytes[strip_no].master_lvl = in_int; }
			}

			else if ((type == 'C') && (typeb == 'K'))
			{
				strip_no = get_int_conf_value(conf_file, &character);
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_clock[strip_no].color = in_int; }
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_clock[strip_no].level = in_int; }
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_clock[strip_no].pal_speed = in_int; }
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_clock[strip_no].pal_compression = in_int; }
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_clock[strip_no].offset = in_int; }
			}
			else if ((type == 'C') && (typeb == 'B'))
			{
				strip_no = get_int_conf_value(conf_file, &character);
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_clock_bytes[strip_no].mix_mode = in_int; }
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_clock_bytes[strip_no].master_lvl = in_int; }
				for (uint8_t setting = 0; setting < _M_NR_FORM_CLOCK_OPTIONS_; setting++)  if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character);  targetFXConf->form_menu_clock[strip_no][setting] =in_int; }
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_clock_bytes[strip_no].type = in_int; }
			}

			else if ((type == 'I') && (typeb == 'F'))	
			{
				strip_no = constrain(get_int_conf_value(conf_file, &character),0,NR_FX_BYTES);
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_fire_bytes[strip_no].pal = in_int;}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_fire_bytes[strip_no].mix_mode = in_int;}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_fire_bytes[strip_no].level = in_int;}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_fire_bytes[strip_no].cooling = in_int;}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_fire_bytes[strip_no].sparking = in_int; }
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_fire_bytes[strip_no].triggerBin = in_int;} else targetFXConf->form_fx_fire_bytes[strip_no].triggerBin = 255;
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_fire_bytes[strip_no].lvl_bin = in_int;}    else targetFXConf->form_fx_fire_bytes[strip_no].lvl_bin = 255;
				for (uint8_t setting = 0; setting < _M_NR_FORM_FIRE_OPTIONS_; setting++)  if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_menu_fire[strip_no][setting] = in_int;}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_fire_bytes[strip_no].master_lvl = in_int;}    else targetFXConf->form_fx_fire_bytes[strip_no].master_lvl = 255;

			}

			else if ((type == 'G') && (typeb == 'F'))	
			{
				strip_no = get_int_conf_value(conf_file, &character);
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_glitter[strip_no].value = in_int;}

			}
			else if ((type == 'G') && (typeb == 'C'))	
			{
				strip_no = get_int_conf_value(conf_file, &character);
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_glitter_bytes[strip_no].pal = in_int;}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_glitter_bytes[strip_no].level = in_int;}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_glitter_bytes[strip_no].glit_bin = in_int;}
				for (uint8_t setting = 0; setting < _M_NR_FORM_GLITTER_OPTIONS_; setting++)  if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_menu_glitter[strip_no][setting] =in_int;}
			}
			else if ((type == 'X') && (typeb == 'C'))	
			{
												 in_int = get_int_conf_value(conf_file, &character); strip_no = constrain(in_int,0,NR_FX_BYTES);
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx1[strip_no].level = in_int; }
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx1[strip_no].mix_mode = in_int; }
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx1[strip_no].fade = in_int; }
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx1[strip_no].triggerBin = in_int; } else targetFXConf->form_fx1[strip_no].triggerBin = 255;
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx1[strip_no].lvl_bin = in_int; }    else targetFXConf->form_fx1[strip_no].lvl_bin = 255;
				for (uint8_t setting = 0; setting < _M_NR_FORM_FX1_OPTIONS_; setting++)  if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_menu_fx1[strip_no][setting] = in_int ; }
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx1[strip_no].master_lvl = in_int; }    else targetFXConf->form_fx1[strip_no].master_lvl = 255;
				
			}

			else if ((type == 'M') && (typeb == 'F'))	
			{
				strip_no = get_int_conf_value(conf_file, &character);
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_meteor[strip_no].meteorSize = in_int;}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_meteor[strip_no].meteorTrailDecay = in_int;}
				
			}
			else if ((type == 'M') && (typeb == 'C'))	
			{
				strip_no = get_int_conf_value(conf_file, &character);
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_meteor_bytes[strip_no].color = in_int;}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_meteor_bytes[strip_no].level = in_int;}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_meteor_bytes[strip_no].triggerBin = in_int;}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_meteor_bytes[strip_no].lvl_bin = in_int;}
				for (uint8_t setting = 0; setting < _M_NR_FORM_METEOR_OPTIONS_; setting++)  if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_menu_meteor[strip_no][setting] = in_int;}
			}

			else if ((type == 'O') && (typeb == 'F'))	
			{
				strip_no = constrain(get_int_conf_value(conf_file, &character),0,NR_FX_BYTES);
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_strobe_bytes[strip_no].pal = in_int;}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_strobe_bytes[strip_no].mix_mode = in_int;}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_strobe_bytes[strip_no].level = in_int;}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_strobe_bytes[strip_no].triggerBin = in_int;}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_strobe_bytes[strip_no].lvl_bin = in_int;}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_strobe_bytes[strip_no].on_frames = in_int;}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_strobe_bytes[strip_no].off_frames = in_int;}
				for (uint8_t setting = 0; setting < _M_NR_FORM_STROBE_OPTIONS_; setting++)  if(AnotherSetting(&character))  { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_menu_strobe[strip_no][setting] = in_int; }
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_strobe_bytes[strip_no].master_lvl = in_int;}
			}
		
		   else if ((type == 'E') && (typeb == 'F'))	
			{
				strip_no = get_int_conf_value(conf_file, &character);
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_eyes_bytes[strip_no].color = in_int;}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_eyes_bytes[strip_no].mix_mode = in_int;}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_eyes_bytes[strip_no].level = in_int;}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_eyes_bytes[strip_no].triggerBin = in_int;}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_eyes_bytes[strip_no].lvl_bin = in_int;}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_eyes_bytes[strip_no].on_frames = in_int;}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_eyes_bytes[strip_no].EyeWidth = in_int;}

				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_eyes_bytes[strip_no].EyeSpace = in_int;}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_eyes_bytes[strip_no].fadeval = in_int;}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_eyes_bytes[strip_no].pause_frames = in_int;}
				for (uint8_t setting = 0; setting < _M_NR_FORM_EYES_OPTIONS_; setting++)  if(AnotherSetting(&character))  { in_int = get_int_conf_value(conf_file, &character);  targetFXConf->form_menu_eyes[strip_no][setting] = in_int; }
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetFXConf->form_fx_eyes_bytes[strip_no].master_lvl = in_int;}
			}
			else if ((type == 'Y') && (typeb == 'F'))	
			{
				strip_no = get_int_conf_value(conf_file, &character);
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character, LEDS_DEF_MODIFY_ROTATEFIXED); 		targetConf->form_fx_modify[strip_no].RotateFixed = in_int;}
				
			}
			else if ((type == 'Y') && (typeb == 'C'))	
			{
				strip_no = get_int_conf_value(conf_file, &character);
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character, LEDS_DEF_MODIFY_ROTATEFULLFRAMES); 	targetConf->form_fx_modify_bytes[strip_no].RotateFullFrames = in_int;}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character, LEDS_DEF_MODIFY_ROTATETRIGGERBIN); 	targetConf->form_fx_modify_bytes[strip_no].RotateTriggerBin = in_int;}
				for (uint8_t setting = 0; setting < _M_NR_FORM_MODIFY_OPTIONS_; setting++)  if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character);  targetConf->form_menu_modify[strip_no][setting] = in_int; }
				
			}

			else if  ((type == 'P') && (typeb == 'A'))	
			{
				uint8_t pal_no = get_int_conf_value(conf_file, &character);
				for (uint8_t color = 0; color < 16; color++) {
					if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); 		LEDS_pal_write(&targetConf->LEDS_pal_cur[pal_no],pal_no, color, 0, in_int) ;} // targetPalette[pal_no][color].r =
					if(AnotherSetting(&character)) { in_int = get_IP_conf_value(conf_file, &character); 		LEDS_pal_write(&targetConf->LEDS_pal_cur[pal_no],pal_no, color, 1, in_int);}
					if(AnotherSetting(&character)) { in_int = get_IP_conf_value(conf_file, &character); 		LEDS_pal_write(&targetConf->LEDS_pal_cur[pal_no],pal_no, color, 2, in_int);}
				}


			} 
			
			else if  ((type == 'A') && (typeb == 'M'))	
			{
				
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetConf->fft_config.fft_menu_bri 			= in_int;}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character);targetConf->fft_config.fft_bin_autoTrigger 		= in_int;}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character);targetConf->fft_config.fft_menu_fps 			= in_int;}

				for (uint8_t color = 0; color < 3; color++)   				if(AnotherSetting(&character))  { in_int = get_int_conf_value(conf_file, &character);targetConf->fft_config.fft_menu[color]  	= in_int;}
				for (uint8_t bin = 0; bin < 7 ; bin++) 		if(AnotherSetting(&character))  { in_int = get_int_conf_value(conf_file, &character);targetConf->fft_config.trigger[bin] 		= in_int;}
				
	
			}
			else if ((type == 'A') && (typeb == 'F'))
			{	
				uint8_t fxbin = constrain(get_int_conf_value(conf_file, &character),0,FFT_FX_NR_OF_BINS-1);
				if(AnotherSetting(&character))  { in_int = get_int_conf_value(conf_file, &character); targetConf->fft_config.fft_fxbin[fxbin].set_val = in_int;}
				if(AnotherSetting(&character))	{ in_int = get_int_conf_value(conf_file, &character); targetConf->fft_config.fft_fxbin[fxbin].trrig_val = in_int;}
				if(AnotherSetting(&character)) 	{ in_int = get_int_conf_value(conf_file, &character); targetConf->fft_config.fft_fxbin[fxbin].menu_select = in_int; }
			}
			
			else if  ((type == 'l') && (typeb == 'y'))	
			{
					for (uint8_t layer = 0 ; layer < MAX_LAYERS_SELECT ; layer++)
					{
					targetConf->layer.select[layer] = get_int_conf_value(conf_file, &character)	;
					}
			}
			else if  ((type == 'L') && (typeb == 'C'))	
			{
				uint8_t layerNo = get_int_conf_value(conf_file, &character);					
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetConf->layer.save_startLed[layerNo] 	= in_int	;}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetConf->layer.save_NrLeds[layerNo] 	= constrain(in_int,0,led_cfg.NrLeds)	;}
				if(AnotherSetting(&character)) { in_int = get_int_conf_value(conf_file, &character); targetConf->layer.save_lvl[layerNo] 		= in_int	;}
				if(AnotherSetting(&character)) {in_int = get_int_conf_value(conf_file, &character);  targetConf->layer.save_mix[layerNo] 		= in_int	;}

			}



			while ((conf_file.available()) && (character != ']')) 
			{ 
				character = conf_file.read();
			}


			
		}
		// close the file:
		conf_file.close();

		led_cfg.Play_Nr = conf_nr; 
		LEDS_pal_reset_index();
		
		return true;
	}
	else
	{
		// if the file didn't open, print an error:
		Serial.println("error opening " + addr);
	}
	 debugMe("play-File-Closed");


	return false;
}


// Global bools
void FS_Bools_write(uint8_t conf_nr)
{
	// Write the global config out to disk (DS settings tab)

	String addr = String("/conf/" + String(conf_nr) + ".device.txt");
	//String title = "Main Config for ESP.";
	File conf_file = selectedFS.open(addr, "w");

	if (!conf_file && !conf_file.isDirectory())
	{
		 debugMe("Cant write bool Conf file");
	}
	else  // it opens
	{
		conf_file.println(F("Main Config for ESP. 0 = off,  1 = on"));
		conf_file.println(F("D = Device Config :Led Mode: max bri : Startup bri : Nr of Leds (Max 680): Data1 Nr Leds: Data1 StartLed: Data2 ....-> data4 Startled"));
		conf_file.print(String("[D:"	+ String(led_cfg.ledMode)));
		conf_file.print(String(":"		+ String(led_cfg.max_bri)));
		conf_file.print(String(":"		+ String(led_cfg.startup_bri)));
		conf_file.print(String(":"		+ String(led_cfg.NrLeds)));
		conf_file.print(String(":"		+ String(led_cfg.DataNR_leds[0])));
		conf_file.print(String(":"		+ String(led_cfg.DataStart_leds[0] )));
		conf_file.print(String(":"		+ String(led_cfg.DataNR_leds[1])));
		conf_file.print(String(":"		+ String(led_cfg.DataStart_leds[1] )));
		conf_file.print(String(":"		+ String(led_cfg.DataNR_leds[2])));
		conf_file.print(String(":"		+ String(led_cfg.DataStart_leds[2] )));
		conf_file.print(String(":"		+ String(led_cfg.DataNR_leds[3])));
		conf_file.print(String(":"		+ String(led_cfg.DataStart_leds[3] )));
		conf_file.print(String(":"		+ String(led_cfg.apa102data_rate)));
		conf_file.print(String(":"		+ String(led_cfg.bootCFG)));
		conf_file.print(String(":"		+ String(led_cfg.PotSens)));
		conf_file.print(String(":"		+ String(led_cfg.Led_Setup_ConfNr)));
		conf_file.println("] ");

		conf_file.println(F("b = Device Bool Config 0=false 1= true : Debug Telnet: FFT enabled : FFT Master : FFT Auto : FFT Master Send out UDP MC : DATA1_ENABLE : DATA2_ENABLE :DATA3_ENABLE :DATA4_ENABLE : Disable FPS&BRI on HW "));
		conf_file.print(String("[b:" + String(get_bool(DEBUG_OUT))));	
		conf_file.print(String(":" + String(get_bool(DEBUG_TELNET))));
		conf_file.print(String(":" + String(get_bool(FFT_ENABLE))));
		conf_file.print(String(":" + String(get_bool(FFT_MASTER))));	
		conf_file.print(String(":" + String(0 )));  // SPARE!!!!		
		conf_file.print(String(":" + String(get_bool(FFT_MASTER_SEND))));
		conf_file.print(String(":" + String(get_bool(DATA1_ENABLE))));
		conf_file.print(String(":" + String(get_bool(DATA2_ENABLE))));
		conf_file.print(String(":" + String(get_bool(DATA3_ENABLE))));
		conf_file.print(String(":" + String(get_bool(DATA4_ENABLE))));
		conf_file.print(String(":" + String(get_bool(POT_DISABLE))));
		conf_file.print(String(":" + String(get_bool(POTS_LVL_MASTER))));
		
		conf_file.println("] ");


		conf_file.println(F("S = Sequencer on: Conf ON . time in min : ... to conf 15 "));
		conf_file.print(String("[S:" + String(get_bool(SEQUENCER_ON))));
		
		for(uint8_t confNr = 0; confNr < MAX_NR_SAVES; confNr++)
		{
			conf_file.print(String(":" + String(LEDS_get_sequencer(confNr ))));
			conf_file.print(String("." + String(play_conf_time_min[confNr])));
		}

		conf_file.println(F("] "));
		
		conf_file.println(F("C = Custom Save Nrs "));
		conf_file.print(String("[C:" + String(get_bool(SEQUENCER_ON))));
		
		for(uint8_t confNr = 0; confNr < Nr_CustomConfs; confNr++)
		{
			conf_file.print(String(":" + String(SaveConf.confCustomLoadNrs[confNr])));
		}

		conf_file.println(F("] "));



		
		 
		conf_file.close();
		
		 debugMe(F("Bool conf wrote"));
	}	// end open conf file


}


boolean FS_mqtt_read()
{
	// read the device config and bools
	
	String addr = String("/conf/mqtt.txt");

	if (selectedFS.exists(addr))
	{ 
		debugMe("Reading mqtt file");
		File conf_file = selectedFS.open(addr, "r");
		

		if (conf_file&& !conf_file.isDirectory())
		{
			char character;
			String settingValue;
			char type;


			memset(mqtt_cfg.username, 0, sizeof(mqtt_cfg.username)); 
			memset(mqtt_cfg.password, 0, sizeof(mqtt_cfg.password)); 

			while (conf_file.available())
			{
				//debugMe("pre read");	
				character = conf_file.read();
				delay(10);
				//debugMe("1");
				while ((conf_file.available()) && (character != '[')) {  // Go to first setting
					character = conf_file.read();
				}
				
				


				type = conf_file.read();
				character = conf_file.read(); // go past the first ":" after the type
				
				if (type == 'M')
				{
					int in_int = 0;
					write_bool(MQTT_ON, get_bool_conf_value(conf_file, &character));
					for (uint8_t i = 0; i < 4; i++) mqtt_cfg.mqttIP[i] = get_IP_conf_value(conf_file, &character);
					in_int = get_int_conf_value(conf_file, &character);		mqtt_cfg.mqttPort = uint16_t(in_int);

					settingValue = get_string_conf_value(conf_file, &character); settingValue.toCharArray(mqtt_cfg.username, settingValue.length() + 1);
					settingValue = get_string_conf_value(conf_file, &character); settingValue.toCharArray(mqtt_cfg.password, settingValue.length() + 1);

					if (character  != ']')  {in_int = get_int_conf_value(conf_file, &character);		mqtt_cfg.publishSec = uint16_t(in_int); } else {mqtt_cfg.publishSec = 60 ; };
					
				}
				else
					debugMe("NO_TYPE");

				while ((conf_file.available()) && (character != ']')) character = conf_file.read();   // goto End	
																									 
			}
			conf_file.close();
			return true;
		}	// end open conf file

		else
		{
			debugMe("error opening " + addr + " Loading defaults ");
			
		}

		conf_file.close();
	}
	else
	{
		debugMe("NO mqtt File");
	}

	return false;
}

void FS_mqtt_write()
{
	// write out the wifi config
	String addr = String("/conf/mqtt.txt");
	//String title = "Main Config for ESP.";
	File conf_file = selectedFS.open(addr, "w");

	if (!conf_file && !conf_file.isDirectory())
	{
		 debugMe("Cant write MQTT Conf file");
	}
	else  // it opens
	{
		conf_file.println("M = MQTT :ON =1 off = 0: IP  : PORT  : Username  : Password : Publish timeout in seconds");
		conf_file.print(String("[M:" + String(get_bool(MQTT_ON))));
		conf_file.print(String(":" + String(mqtt_cfg.mqttIP[0])));
		conf_file.print(String("." + String(mqtt_cfg.mqttIP[1])));
		conf_file.print(String("." + String(mqtt_cfg.mqttIP[2])));
		conf_file.print(String("." + String(mqtt_cfg.mqttIP[3])));
		conf_file.print(String(":" + String(mqtt_cfg.mqttPort)));
		conf_file.print(String(":" + String(mqtt_cfg.username)));
		conf_file.print(String(":" + String(mqtt_cfg.password)));
		conf_file.print(String(":" + String(mqtt_cfg.publishSec)));

		conf_file.println("] ");
		conf_file.close();

		 debugMe("MQTT wrote conf");
	}	// end open conf file

	
} // end FS_mqtt_write()



boolean FS_Bools_read(uint8_t conf_nr)
{
	// read the device config and bools
	
	String addr = String("/conf/" + String(conf_nr) + ".device.txt");
	
	
	if (selectedFS.exists(addr))
	{ 
		debugMe("Reading bools file");
		File conf_file = selectedFS.open(addr, "r");
		

		if (conf_file&& !conf_file.isDirectory())
		{
			//debugMe("in file");
			char character;
			String settingValue;
			char type;
			while (conf_file.available())
			{

				character = conf_file.read();
				delay(10);

				while ((conf_file.available()) && (character != '[')) {  // Go to first setting
					character = conf_file.read();
				}
				
				type = conf_file.read();
				character = conf_file.read(); // go past the first ":" after the type


				if (type == 'D')
				{
					
					int in_int = 0;
					in_int = get_int_conf_value(conf_file, &character);		led_cfg.ledMode 			= uint8_t(constrain(in_int, 0, 5));
					in_int = get_int_conf_value(conf_file, &character);		led_cfg.max_bri 			= uint8_t(constrain(in_int, 0, 255));
					in_int = get_int_conf_value(conf_file, &character);		led_cfg.startup_bri 		= uint8_t(constrain(in_int, 0, 255));
					in_int = get_int_conf_value(conf_file, &character);		led_cfg.NrLeds 				= uint16_t(constrain(in_int, 1,MAX_NUM_LEDS));
					in_int = get_int_conf_value(conf_file, &character);		led_cfg.DataNR_leds[0] 		= uint16_t(constrain(in_int, 0,led_cfg.NrLeds  ));
					in_int = get_int_conf_value(conf_file, &character);		led_cfg.DataStart_leds[0]  	= uint16_t(constrain(in_int, 0,led_cfg.NrLeds));
					in_int = get_int_conf_value(conf_file, &character);		led_cfg.DataNR_leds[1] 		= uint16_t(constrain(in_int, 0,led_cfg.NrLeds  ));
					in_int = get_int_conf_value(conf_file, &character);		led_cfg.DataStart_leds[1]  	= uint16_t(constrain(in_int, 0,led_cfg.NrLeds));
					in_int = get_int_conf_value(conf_file, &character);		led_cfg.DataNR_leds[2] 		= uint16_t(constrain(in_int, 0,led_cfg.NrLeds  ));
					in_int = get_int_conf_value(conf_file, &character);		led_cfg.DataStart_leds[2]  	= uint16_t(constrain(in_int, 0,led_cfg.NrLeds));
					in_int = get_int_conf_value(conf_file, &character);		led_cfg.DataNR_leds[3] 		= uint16_t(constrain(in_int, 0,led_cfg.NrLeds ));
					in_int = get_int_conf_value(conf_file, &character);		led_cfg.DataStart_leds[3]  	= uint16_t(constrain(in_int, 0,led_cfg.NrLeds));
					in_int = get_int_conf_value(conf_file, &character);		led_cfg.apa102data_rate 	= uint8_t(constrain(in_int, 1,24));
					if(AnotherSetting(&character))  {in_int = get_int_conf_value(conf_file, &character);		led_cfg.bootCFG 			= uint8_t(constrain(in_int, 0,MAX_NR_SAVES));} else led_cfg.bootCFG  = MAX_NR_SAVES;
					if(AnotherSetting(&character))  {in_int = get_int_conf_value(conf_file, &character);		led_cfg.PotSens 			= uint8_t(constrain(in_int, 0,255));} 		   else led_cfg.PotSens  = POT_SENSE_DEF;
					if(AnotherSetting(&character))  {in_int = get_int_conf_value(conf_file, &character);		led_cfg.Led_Setup_ConfNr 	= uint8_t(constrain(in_int, 0,MAX_LAMP_CONFIGS));}  else led_cfg.Led_Setup_ConfNr  = 0;
					
						

				}
				else if (type == 'b')
				{
					//debugMe("in B");
					write_bool(DEBUG_OUT, get_bool_conf_value(conf_file, &character));
					write_bool(DEBUG_TELNET, get_bool_conf_value(conf_file, &character));
					write_bool(FFT_ENABLE, get_bool_conf_value(conf_file, &character));
					write_bool(FFT_MASTER, get_bool_conf_value(conf_file, &character));
					get_bool_conf_value(conf_file, &character) ; // SPARE!!!!!!!!!!!      ///write_bool(FFT_AUTO, get_bool_conf_value(conf_file, &character));					
					write_bool(FFT_MASTER_SEND, get_bool_conf_value(conf_file, &character));
					write_bool(DATA1_ENABLE, get_bool_conf_value(conf_file, &character));
					write_bool(DATA2_ENABLE, get_bool_conf_value(conf_file, &character));
					write_bool(DATA3_ENABLE, get_bool_conf_value(conf_file, &character));
					write_bool(DATA4_ENABLE, get_bool_conf_value(conf_file, &character));
					write_bool(POT_DISABLE, get_bool_conf_value(conf_file, &character));
					if(AnotherSetting(&character))  write_bool(POTS_LVL_MASTER, get_bool_conf_value(conf_file, &character));
					
					
					

				}
				else if (type == 'S')
				{
					write_bool(SEQUENCER_ON, get_bool_conf_value(conf_file, &character));
					for(uint8_t confNr = 0; confNr < MAX_NR_SAVES; confNr++)
					{
						int in_int = 0;

						LEDS_write_sequencer(confNr , get_int_conf_value	(conf_file, &character)) ;
						in_int = get_int_conf_value(conf_file, &character);	 	play_conf_time_min[confNr] = in_int;		
					}
				}
				else if (type == 'C')
				{
					
					for(uint8_t confNr = 0; confNr < Nr_CustomConfs; confNr++)
					{
						int in_int = 0;
						in_int = get_int_conf_value(conf_file, &character); 
						SaveConf.confCustomLoadNrs[confNr] = in_int;
	
					}
				}
				else
					debugMe("NO_TYPE");

				while ((conf_file.available()) && (character != ']')) character = conf_file.read();   // goto End	
																									 
			}
			conf_file.close();
			return true;
		}	// end open conf file

		else
		{
			debugMe("error opening " + addr + " Loading defaults ");
			
		}

		conf_file.close();
	}
	else
	{
		debugMe("NO BOOLS File");
	}

	return false;
}


// delete
void FS_osc_delete_all_saves()
{
		String address;

		if (selectedFS.exists("/conf/0.wifi.txt"))		selectedFS.remove("/conf/0.wifi.txt");
		if (selectedFS.exists("/conf/0.Bool.txt"))		selectedFS.remove("/conf/0.Bool.txt");
		if (selectedFS.exists("/conf/0.artnet.txt"))	selectedFS.remove("/conf/0.artnet.txt");

		for (uint8_t play_mode_int = 0; play_mode_int < 16; play_mode_int++) 
		{
			//memset(address, 0, sizeof(address));
			address = String("/conf/" + String(play_mode_int) + ".playConf.txt");
			if (selectedFS.exists(address)) selectedFS.remove(address);
			
		}

		for (uint8_t fft_mode_int = 1; fft_mode_int <= 5; fft_mode_int++) 
		{
			//memset(address, 0, sizeof(address));
			address = String("/conf/" + String(fft_mode_int) + ".fft.txt");
			if (selectedFS.exists(address))  selectedFS.remove(address);
		}

}



void FS_listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
	debugMe("Listing directory: "+  String(dirname));

	File root = fs.open(dirname);
	if (!root) {
		debugMe("Failed to open directory");
		return;
	}
	if (!root.isDirectory()) {
		debugMe("Not a directory");
		return;
	}

	File file = root.openNextFile();
	while (file) {
		if (file.isDirectory()) {
			debugMe("  DIR : ", false);
			debugMe(file.name());
			if (levels) {
				FS_listDir(fs, file.name(), levels - 1);
			}
		}
		else {
			debugMe("  FILE: ", false);
			debugMe(file.name(),false);
			debugMe("  SIZE: ",false);
			debugMe(String(file.size()));
		}
		file = root.openNextFile();
	}
	file.close();
}


//Setup
void FS_setup()
{
	debugMe("Start selectedFS");
	if (selectedFS.begin())   // true = format on fail
	{
		debugMe("Started selectedFS");
		FS_listDir(selectedFS, "/", 0);
	} else{

		debugMe("FAILED toStart FS");

	}
	delay(100);
	load_bool();
	FS_load_PlayConf_status();
	FS_artnet_read();
	


}
