// wifi-ota.h

#ifndef _WIFI-OTA_h
#define _WIFI-OTA_h

//#define ARTNET_DISABLED 

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "IPAddress.h"

										// Wifi IP settings data stucture 
	struct wifi_Struct 
	{
		char		pwd[32];			// the Wifi client Password
		char		ssid[32];			// the Wifi client SSID
		char		APname[32];			// the Hostname & AP mode SSID
		IPAddress	ipStaticLocal;		// the configured Static IP-address
		IPAddress	ipSubnet;			// for Static IP the Subnet
		IPAddress	ipDGW;				// for Static IP the DGW
		IPAddress	ipDNS;				// for Static IP the  DNS server 
		IPAddress	ipNTP;				// What NTP server to use in IP 
		char		ntp_fqdn[24];		// what NTP server to use in FQDN
	};

																// DEFAULT setting if no config is loaded from the SPIFFS
	#define DEF_AP_NAME			"TinyPixelMapper1"				// AP / Hostname
	#define DEF_SSID			"home"							// SSID to connect to 
	#define DEF_WIFI_PWD		"love4all"		// PW for wifi Client
	#define DEF_AP_PASSWD		"love4all"					// PW for AP mode   !!! no OSC config yet STATIC !!!!
	#define DEF_IP_LOCAL		{172,16,222,31}					// Static IP
	#define DEF_IP_SUBNET		{255,255,255,0}					// Subnet Mask
	#define DEF_IP_DGW			{172,16,222,1}					// DGW

	#define DEF_DNS				{172,16,222,1}					// DNS server
	#define DEF_NTP_SERVER		"0.at.pool.ntp.org"				//"0.at.pool.ntp.org"	 // only FQDN's  no ip!! 	
	#define DEF_TIMEZONE		2								// how much to add to UTC for the NTP client
	#define WIFI_CLIENT_CONNECT_TIMEOUT		10000				// how long to try to connect to the Wifi
	#define WIFI_CLIENT_CONNECT_TRYS		2					// how many times to try to connect to the wifi 

// Artnet
	#define DEF_ARTNET_STAT_UNIVERSE 5								// Default Artnet Start universe
	#define DEF_ARTNET_NUMBER_OF_UNIVERSES 4						// Default Arnet NR of universes MAX 4 !!!! TODO 
	#define DEF_ARTNET_MAC { 0x04, 0xE9, 0xE5, 0x00, 0x69, 0xEC }   // TODO!!!!! we dont want duplicates!!! so lets generate it!!!

	struct artnet_struct	// Arnet config Data stucture
	{
		byte	startU;		// Arnet Start universe
		byte	numU;		// Arnetnet Nunmber of universes		
	};


	 
														//  DEFAULT FFT parameters if not loaded from SPIFFS
 #define DEF_FFT_IP_MULTICAST	{239, 0, 0, 57}			//  Multicast IP address to send the FFT data to
 #define DEF_FFT_SLAVE_PORT		431						// Multicast DEST port for FFT packets
 #define DEF_FFT_MASTER_PORT	432						// Multicast source port to send from
 #define DEF_FFT_ENABLE			false					// enalbe FFT from start?
 #define DEF_FFT_MASTER			false					// set the node to Master mode?


 struct fft_ip_cfg_struct
 {									// Config structure to hold the FFT parameters
	 IPAddress	IP_multi;			// Multicast IP address to send the FFT data to		
	 uint16_t	port_slave;			// Multicast DEST port for FFT packets
	 uint16_t	port_master;		// Multicast source port to send from
 };


#endif

