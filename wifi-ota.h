// wifi-ota.h

#ifndef _WIFI-OTA_h
#define _WIFI-OTA_h

#define ARTNET_DISABLED 

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



	struct artnet_struct	// Arnet config Data stucture
	{
		byte	startU;		// Arnet Start universe
		byte	numU;		// Arnetnet Nunmber of universes		
	};


	 



 struct fft_ip_cfg_struct
 {									// Config structure to hold the FFT parameters
	 IPAddress	IP_multi;			// Multicast IP address to send the FFT data to		
	 uint16_t	port_slave;			// Multicast DEST port for FFT packets
	 uint16_t	port_master;		// Multicast source port to send from
 };


#endif

