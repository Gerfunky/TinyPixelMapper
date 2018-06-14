// wifi-ota.h

#ifndef _WIFI_OTA_h
#define _WIFI_OTA_h



#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"


#include "IPAddress.h"

										// Wifi IP settings data stucture 
	struct wifi_Struct 
	{
		char		pwd[32];			// the Wifi client Password
		char		ssid[32];			// the Wifi client SSID
		char		APname[32];			// the Hostname & AP mode SSID
		char		APpassword[32];		// The AP password
		IPAddress	ipStaticLocal;		// the configured Static IP-address
		IPAddress	ipSubnet;			// for Static IP the Subnet
		IPAddress	ipDGW;				// for Static IP the DGW
		IPAddress	ipDNS;				// for Static IP the  DNS server 
		IPAddress	ipNTP;				// What NTP server to use in IP 
		char		ntp_fqdn[24];		// what NTP server to use in FQDN#

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






 // Functions

 void wifi_loop();   // The main wifi loop
 void wifi_setup();  // The wifi setup function



 void WIFI_FFT_toggle_master(boolean value);			// osc.cpp
 void WIFI_FFT_toggle(boolean mode_value);				//osc.cpp

 //void WIFI_FFT_master_send();   // Comms

 void WiFi_telnet_print(String input, boolean line);
 void WiFi_telnet_print(tm input, boolean line);
 void WiFi_telnet_print(float input, boolean line);
 void WiFi_telnet_print(uint8_t input, boolean line);
 void WiFi_telnet_print(int input, boolean line);
 void WiFi_telnet_print(IPAddress input, boolean line);


#ifndef ARTNET_DISABLED
	void WiFi_artnet_enable();
	//void WiFi_artnet_setup();
	void WiFi_artnet_loop();
#endif


#endif
#endif

