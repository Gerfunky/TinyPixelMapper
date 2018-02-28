
#include "config_TPM.h"

#ifdef _MSC_VER   
		#include <WiFi\src\WiFi.h>
		#include <ArduinoOTA\src\ArduinoOTA.h>
		#include<WiFi\src\WiFiUdp.h>
		#include <RemoteDebug\RemoteDebug.h>
		//#include <Time\TimeLib.h>
		#include <time.h>

	#ifndef ARTNET_DISABLED 
		#include <Artnet\Artnet.h>
	#endif
	
#else 
	#include <WiFi.h>	
	#include <WiFiUdp.h>
	//#include <WiFiAP.h>
	#include <ArduinoOTA.h>
	//#include <TimeLib.h>
	#include "time.h"
	#include <RemoteDebug.h> 
	#ifndef ARTNET_DISABLED 
		#include <Artnet.h>
	#endif
	
#endif




#include "tools.h"			// include the Tools for reading and writing bools and DebugMe
#include "wifi-ota.h"		// needs Wifi and co for data stuctures!!!
#include "leds.h"			// include for led data structures
#include "config_fs.h"
#include "httpd.h"
#include "osc.h"



RemoteDebug TelnetDebug;


// wifi
wifi_Struct wifi_cfg;

// Artnet
#ifndef ARTNET_DISABLED
	Artnet artnet;		// make the Artnet server
#endif

	artnet_struct artnet_cfg = { DEF_ARTNET_STAT_UNIVERSE, DEF_ARTNET_NUMBER_OF_UNIVERSES };


// ntp
#define NTP_PACKET_SIZE 48		// the ntp packetsize
#define NTP_LOCAL_PORT	2843	// 2 Invinity 4 Ever

//the ntp deamon
WiFiUDP ntp_udp;


// FFT data 
WiFiUDP  FFT_master;
WiFiUDP  FFT_slave;

fft_ip_cfg_struct fft_ip_cfg;



// from leds

extern fft_led_cfg_struct fft_led_cfg;



//OTA 
void setup_OTA()
{
	// configure and setup the OTA process 
	//  so that its possible to compile and send
	// out of arduino 
	// Port defaults to 8266
	// ArduinoOTA.setPort(8266);

	// Hostname defaults to esp8266-[ChipID]
	ArduinoOTA.setHostname(wifi_cfg.APname);

	// authentication by default Password = love
	//ArduinoOTA.setPassword((const char *) "love" );

	if (get_bool(DEBUG_OUT) == true) 
	{
			ArduinoOTA.onStart([]()												{ debugMe("Start OTA"); });
			ArduinoOTA.onEnd([]()												{ debugMe("End OTA");   });
			ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) { debugMe(String("Progress:" + String(progress / (total / 100)))); });
			ArduinoOTA.onError([](ota_error_t error)							{ debugMe(String("Error: " + String(error)), false);

			if (error == OTA_AUTH_ERROR)			debugMe("Auth Failed");
			else if (error == OTA_BEGIN_ERROR)		debugMe("Begin Failed");
			else if (error == OTA_CONNECT_ERROR)	debugMe("Connect Failed");
			else if (error == OTA_RECEIVE_ERROR)	debugMe("Receive Failed");
			else if (error == OTA_END_ERROR)		debugMe("End Failed");
			});
			debugMe("OTA Ready");
	}

	ArduinoOTA.begin();
}



void NTPprintTime() {
	
	struct tm timeinfo;
	
	if (!getLocalTime(&timeinfo)) {
		debugMe("Failed to obtain time");
		return;
	}
	debugMe("***********************");
	//Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
	//sprintf(&timeinfo, "%A, %B %d %Y %H:%M:%S");
	//timeinfo.tm_hour
		//String XXX = String(&timeinfo, "%A, %B %d %Y %H:%M:%S");
		debugMe(timeinfo);
	/*
	// digital clock display of the time
	// print the time
	if (get_bool(DEBUG_OUT) == true)
	{	
		debugMe("***************  ", false);
		debugMe(timeinfo.tm_hour,false);
		debugMe(":", false);
		debugMe(minute(), false);
		debugMe(":", false);
		debugMe(second(), false);
		debugMe(" ", false);
		debugMe(day(), false);
		debugMe(".", false);
		debugMe(month(), false);
		debugMe(".", false);
		debugMe(year(), false);
		debugMe("  ***************");
		//debugMe();
	}
	//*/
}

/*
void NTP_requestTime(IPAddress& address)		// taken from the arduino NTP example
{
	// send out a ntp request to get the actual time
	byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

	debugMe("sending NTP request...");
	// set all bytes in the buffer to 0
	memset(packetBuffer, 0, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	packetBuffer[0] = 0b11100011;   // LI, Version, Mode
	packetBuffer[1] = 0;     // Stratum, or type of clock
	packetBuffer[2] = 6;     // Polling Interval
	packetBuffer[3] = 0xEC;  // Peer Clock Precision
							 // 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12] = 49;
	packetBuffer[13] = 0x4E;
	packetBuffer[14] = 49;
	packetBuffer[15] = 52;

	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp:

	ntp_udp.beginPacket(address, 123); //NTP requests are to port 123
	ntp_udp.write(packetBuffer, NTP_PACKET_SIZE);
	ntp_udp.endPacket();
}


void NTP_parse_response() 
{
		// analyse the ntp response packet to extract the time
		byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
		memset(packetBuffer, 0, NTP_PACKET_SIZE); // reset to 0

		int packetsize = ntp_udp.parsePacket();
		if (packetsize >= NTP_PACKET_SIZE)  
		//else
		{
			debugMe("Packetsize : " + String(packetsize));
			ntp_udp.read(packetBuffer, NTP_PACKET_SIZE);

			debugMe("Receive NTP Response");
			ntp_udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
			unsigned long secsSince1900;
			// convert four bytes starting at location 40 to a long integer
			secsSince1900 = (unsigned long)packetBuffer[40] << 24;
			secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
			secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
			secsSince1900 |= (unsigned long)packetBuffer[43];
			setTime(secsSince1900 - 2208988800UL + DEF_TIMEZONE * SECS_PER_HOUR);
		}
}
*/
void setup_NTP()
{
	const long  gmtOffset_sec = 3600;
	const int   daylightOffset_sec = 3600;

	configTime(gmtOffset_sec, daylightOffset_sec, wifi_cfg.ntp_fqdn);
	NTPprintTime();

	/*
	// setup the NTP client
	ntp_udp.begin(NTP_LOCAL_PORT);
	//get the IP from the NTP server / pool
	WiFi.hostByName(wifi_cfg.ntp_fqdn , wifi_cfg.ipNTP);

	NTP_requestTime(wifi_cfg.ipNTP);
	uint32_t beginWait = millis();
	while (millis() - beginWait < 1500) 
	{
		NTP_parse_response();
		if (timeStatus() == timeSet)
			break;

	}
	
	NTPprintTime();
	//if (year() == 1970 && get_bool(WIFI_MODE) == 1 && get_bool(DEBUG_OUT) == true) {debugMe("******wifi client no connect no time resetting *****");ESP.restart();}

	// */
}



// artnet

#ifndef ARTNET_DISABLED
	void wifi_artnet_loop()
	{
		// the main artnet loop  calback set to leds function with show
		if (get_bool(ARTNET_ENABLE)== true) artnet.read();
	}

	void wifi_artnet_enable() 
	{
		// enable artnet, This is a exclusice mode other settings dont apply

		// byte loc_ip = WiFi.localIP();
		// debugMe("enable artnet");
		byte artnet_mac[] = DEF_ARTNET_MAC ;
		artnet.begin(artnet_mac, 0);   // mac and ip setting useless since were setting ip for the esp8266 
		artnet.setArtDmxCallback(LEDS_artnet_in);  // function in leds with schow

	}

	void setup_artnet_Vars()
	{
		// configure the Artnet vaiables 
		// from disk or load the defaults.

		if (FS_artnet_read(0) == false)
		{
			write_bool(ARTNET_ENABLE, DEF_ARTNET_ENABLE);
			artnet_cfg.startU	= DEF_ARTNET_STAT_UNIVERSE;
			artnet_cfg.numU		= DEF_ARTNET_NUMBER_OF_UNIVERSES;

		}

	}

	void setup_artnet()
	{
		//the Artnet setup 
		if (get_bool(ARTNET_ENABLE)== true)	wifi_artnet_enable();


	}
#endif

// basic WiFi
void setup_wifi_Vars()   // old version for ESP8266
{
	// load the wifi vaiables

	// Clean out the Wifi cha arrays
	memset(wifi_cfg.APname,		0, sizeof(wifi_cfg.APname));
	memset(wifi_cfg.ssid,		0, sizeof(wifi_cfg.ssid));
	memset(wifi_cfg.pwd,		0, sizeof(wifi_cfg.pwd));
	memset(wifi_cfg.ntp_fqdn,	0, sizeof(wifi_cfg.ntp_fqdn));

	//if (FS_wifi_read(0) == false)		// Get the config of disk,  on fail load defaults.
	if (false == false)		// Get the config of disk,  on fail load defaults.
	{
		debugMe("Loading WifiSetup Defaults");
		//load the defaults
		String def_APname = DEF_AP_NAME;
		String def_ssid = DEF_SSID;
		String def_pwd = DEF_WIFI_PWD;
		String def_ntp_fqdn = DEF_NTP_SERVER;

		def_APname.toCharArray(		wifi_cfg.APname,	def_APname.length()		+ 1);
		def_ssid.toCharArray(		wifi_cfg.ssid,		def_ssid.length()		+ 1);
		def_pwd.toCharArray(		wifi_cfg.pwd,		def_pwd.length()		+ 1);
		def_ntp_fqdn.toCharArray(	wifi_cfg.ntp_fqdn,	def_ntp_fqdn.length()	+ 1);

		//write_bool(WIFI_MODE, DEF_WIFI_MODE);
		//write_bool(STATIC_IP_ENABLED, DEF_STATIC_IP_ENABLED);
		wifi_cfg.ipStaticLocal	= DEF_IP_LOCAL;
		wifi_cfg.ipSubnet		= DEF_IP_SUBNET;
		wifi_cfg.ipDGW			= DEF_IP_DGW;
		wifi_cfg.ipDNS			= DEF_DNS;
		

	}

	//if (get_bool(STATIC_IP_ENABLED) == true) WiFi.config(wifi_cfg.ipStaticLocal, wifi_cfg.ipDGW, wifi_cfg.ipSubnet, wifi_cfg.ipDNS);
#ifdef ESP8266
	WiFi.hostname(wifi_cfg.APname);
#endif
	//else WiFi.config(wifi_cfg.ipStaticLocal, wifi_cfg.ipDGW, wifi_cfg.ipSubnet, wifi_cfg.ipDNS);
}





#ifdef ESP32

boolean wifi_load_settings()   // load the wifi settings from SPIFFS or from default Settings.
{
	// load the wifi vaiables

	// Clean out the Wifi cha arrays
	memset(wifi_cfg.APname, 0, sizeof(wifi_cfg.APname));
	memset(wifi_cfg.ssid, 0, sizeof(wifi_cfg.ssid));
	memset(wifi_cfg.pwd, 0, sizeof(wifi_cfg.pwd));
	memset(wifi_cfg.ntp_fqdn, 0, sizeof(wifi_cfg.ntp_fqdn));

	//if (FS_wifi_read(0) == false)		// Get the config of disk,  on fail load defaults.
	if (false == false)		// Get the config of disk,  on fail load defaults.
	{
		debugMe("Loading WifiSetup Defaults");
		//load the defaults
		String def_APname = DEF_AP_NAME;
		String def_ssid = DEF_SSID;
		String def_pwd = DEF_WIFI_PWD;
		String def_ntp_fqdn = DEF_NTP_SERVER;

		def_APname.toCharArray(wifi_cfg.APname, def_APname.length() + 1);
		def_ssid.toCharArray(wifi_cfg.ssid, def_ssid.length() + 1);
		def_pwd.toCharArray(wifi_cfg.pwd, def_pwd.length() + 1);
		def_ntp_fqdn.toCharArray(wifi_cfg.ntp_fqdn, def_ntp_fqdn.length() + 1);

		write_bool(WIFI_MODE, DEF_WIFI_MODE);
		write_bool(STATIC_IP_ENABLED, DEF_STATIC_IP_ENABLED);
		wifi_cfg.ipStaticLocal = DEF_IP_LOCAL;
		wifi_cfg.ipSubnet = DEF_IP_SUBNET;
		wifi_cfg.ipDGW = DEF_IP_DGW;
		wifi_cfg.ipDNS = DEF_DNS;


	}


		// Set the Static IP if static ip is selected.
		if (get_bool(STATIC_IP_ENABLED) == true)
			if (!WiFi.config(wifi_cfg.ipStaticLocal, wifi_cfg.ipDGW, wifi_cfg.ipSubnet, wifi_cfg.ipDNS))
				debugMe("WiFi: Client config Static IP FAILED ");
	
		//debugMe("setting AP settings");
		//if (!WiFi.softAPConfig(wifi_cfg.ipStaticLocal, wifi_cfg.ipDGW, wifi_cfg.ipSubnet))
		//	debugMe("WiFi: AP Config FAILED");

	
}


#endif


/*

if(!WiFi.config(IPAddress(169, 254, 1, 3), IPAddress(10, 0, 0, 1), IPAddress(255, 255, 0, 0), IPAddress(10, 0, 0, 1))){
        Serial.println("STA Failed to configure");
    }
    if(WiFi.begin(STA_SSID, STA_PASS) == WL_CONNECT_FAILED){
        Serial.println("STA Failed to start");
    }

// AP Static IP
    if(!WiFi.softAPConfig(IPAddress(192, 168, 5, 1), IPAddress(192, 168, 5, 1), IPAddress(255, 255, 255, 0))){
        Serial.println("AP Config Failed");
    }
    if(!WiFi.softAP(AP_SSID)){
        Serial.println("AP Start Failed");
    }


*/

void wifi_Event(WiFiEvent_t event, system_event_info_t info)
{
	debugMe("[WiFi-event] event:"+ String(event));
	ip4_addr_t  infoIP4;
	IPAddress infoIP;
	

	switch (event) {
	case  SYSTEM_EVENT_SCAN_DONE:					/**< 1 ESP32 finish scanning AP */
		debugMe("finish scanning AP");
		debugMe("Status: " + info.scan_done.status);
		debugMe("number: " + info.scan_done.number);
		debugMe("scan ID: " + info.scan_done.scan_id);
		break;

	case SYSTEM_EVENT_STA_START:					/**<2 ESP32 station start */
		Serial.println("STA Started");
		WiFi.setHostname(wifi_cfg.APname);
		break;

	case SYSTEM_EVENT_STA_STOP:						/**<3 ESP32 station stop */
		Serial.println("STA Stopped");
		break;

	case SYSTEM_EVENT_STA_CONNECTED:				/**<4 ESP32 station connected to AP */
		Serial.println("WIFI:STA Connected");
		debugMe("SSID = " + String(reinterpret_cast<const char*>(info.connected.ssid)));
		//debugMe("BSSID = " + String(reinterpret_cast<const char*>(info.connected.bssid)));
		debugMe("BSSID/MAC = " + String(info.connected.bssid[0], HEX) + ":" + String(info.connected.bssid[1], HEX) + ":" + String(info.connected.bssid[2], HEX) + ":" + String(info.connected.bssid[3], HEX) + ":" + String(info.connected.bssid[4], HEX) + ":" + String(info.connected.bssid[5], HEX));
		debugMe("Channel = " + String(info.connected.channel));
		debugMe("Authmode = " + String(info.connected.authmode));
		break;

	case SYSTEM_EVENT_STA_DISCONNECTED:				/**<5 ESP32 station disconnected from AP */
		Serial.println("STA Disconnected");
		debugMe("SSID = " + String(reinterpret_cast<const char*>(info.disconnected.ssid)));
		//debugMe("BSSID = " + String(reinterpret_cast<const char*>(info.disconnected.bssid)));
		debugMe("BSSID/MAC = " + String(info.disconnected.bssid[0], HEX) + ":" + String(info.disconnected.bssid[1], HEX) + ":" + String(info.disconnected.bssid[2], HEX) + ":" + String(info.disconnected.bssid[3], HEX) + ":" + String(info.disconnected.bssid[4], HEX) + ":" + String(info.disconnected.bssid[5], HEX));
		debugMe("Reason = " + String(info.disconnected.reason));
		break;

	case	SYSTEM_EVENT_STA_AUTHMODE_CHANGE:      /**<6 the auth mode of AP connected by ESP32 station changed */
		debugMe("auth mode of AP connected by ESP32 station changed");
		debugMe("Authmode New" + info.auth_change.new_mode);
		debugMe("Authmode old" + info.auth_change.old_mode);
		break;
		 
	case	SYSTEM_EVENT_STA_GOT_IP:               /**<7 ESP32 station got IP from connected AP */
		debugMe("station got IP from connected AP");
		debugMe("ON SSID :" + String(WiFi.SSID()));
		infoIP = info.got_ip.ip_info.ip.addr;
		debugMe("Got IPv4: ",false);
		debugMe(infoIP);
		infoIP = info.got_ip.ip_info.netmask.addr;
		debugMe("Got NetMask: ", false);
		debugMe(infoIP);
		infoIP = info.got_ip.ip_info.gw.addr;
		debugMe("Got DGW: ", false);
		debugMe(infoIP);
		debugMe("Changed = " + String(info.got_ip.ip_changed));
		
		break;

	case	SYSTEM_EVENT_STA_LOST_IP:              /**<8 ESP32 station lost IP and the IP is reset to 0 */
		debugMe("station lost IP and the IP is reset to 0");
		break;

	case	SYSTEM_EVENT_STA_WPS_ER_SUCCESS:       /**<9 ESP32 station wps succeeds in enrollee mode */
		debugMe("station wps succeeds in enrollee mode");
		break;

	case	SYSTEM_EVENT_STA_WPS_ER_FAILED:        /**<10 ESP32 station wps fails in enrollee mode */
		debugMe("wps fails in enrollee mode");
		break;

	case	SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:       /**<11 ESP32 station wps timeout in enrollee mode */
		debugMe("wps timeout in enrollee mode");
		break;

	case	SYSTEM_EVENT_STA_WPS_ER_PIN:           /**<12 ESP32 station wps pin code in enrollee mode */
		debugMe("wps pin code in enrollee mode ");
		break;

	case	SYSTEM_EVENT_AP_START:                 /**<13 ESP32 soft-AP start */
		//WiFi.softAPsetHostname(wifi_cfg.APname);
		debugMe("WiFi: soft-AP Started HOSTNAME = " + String(WiFi.softAPgetHostname()));
		break;

	case	SYSTEM_EVENT_AP_STOP:                  /**<14 ESP32 soft-AP stop */
		Serial.println("WiFi: soft-AP Stopped");
		break;

	case	SYSTEM_EVENT_AP_STACONNECTED:          /**<15 a station connected to ESP32 soft-AP */
		debugMe("a station connected to ESP32 soft-AP");
		debugMe("AID = " + String(info.sta_connected.aid));
		debugMe("MAC = " + String(info.sta_connected.mac[0], HEX) +":" + String(info.sta_connected.mac[1],HEX) + ":" + String(info.sta_connected.mac[2], HEX) + ":" + String(info.sta_connected.mac[3], HEX) + ":" + String(info.sta_connected.mac[4], HEX) + ":" + String(info.sta_connected.mac[5], HEX));
		break;

	case	SYSTEM_EVENT_AP_STADISCONNECTED:       /**<16 a station disconnected from ESP32 soft-AP */
		debugMe("a station disconnected from soft-AP");
		debugMe("AID = " + String(info.sta_disconnected.aid));
		debugMe("MAC = " + String(info.sta_disconnected.mac[0], HEX) + ":" + String(info.sta_disconnected.mac[1], HEX) + ":" + String(info.sta_disconnected.mac[2], HEX) + ":" + String(info.sta_disconnected.mac[3], HEX) + ":" + String(info.sta_disconnected.mac[4], HEX) + ":" + String(info.sta_disconnected.mac[5], HEX));
		break;

	case	SYSTEM_EVENT_AP_PROBEREQRECVED:        /**<17 Receive probe request packet in soft-AP interface */
		debugMe("Receive probe request packet in soft-AP interface");
		debugMe("rssi = " + String(info.ap_probereqrecved.rssi));
		debugMe("MAC = " + String(info.ap_probereqrecved.mac[0], HEX) + ":" + String(info.ap_probereqrecved.mac[1], HEX) + ":" + String(info.ap_probereqrecved.mac[2], HEX) + ":" + String(info.ap_probereqrecved.mac[3], HEX) + ":" + String(info.ap_probereqrecved.mac[4], HEX) + ":" + String(info.ap_probereqrecved.mac[5], HEX));

		break;

	case	SYSTEM_EVENT_GOT_IP6:                  /**<18 ESP32 station or ap or ethernet interface v6IP addr is preferred */
		debugMe("station or ap or ethernet interface v6IP addr is preferred");
		break;

	case	SYSTEM_EVENT_ETH_START:                /**<19 ESP32 ethernet start */
		debugMe("ethernet start");
		break;

	case	SYSTEM_EVENT_ETH_STOP:                 /**<20 ESP32 ethernet stop */
		debugMe("ethernet stop");
		break;

	case	SYSTEM_EVENT_ETH_CONNECTED:            /**<21 ESP32 ethernet phy link up */
		debugMe("ethernet phy link up");
		break;

	case	SYSTEM_EVENT_ETH_DISCONNECTED:         /**<22 ESP32 ethernet phy link down */
		debugMe("ethernet phy link down ");
		break;

	case	SYSTEM_EVENT_ETH_GOT_IP:               /**<23 ESP32 ethernet got IP from connected AP */
		debugMe("ethernet got IP from connected AP");
		break;

	default:
		debugMe("OTHER UNKNOWN EVENT");
		break;
	}
	debugMe("WiFi Event END---------");
}



void setup_wifi_Network_X()
{
	WiFi.mode(WIFI_STA);
	WiFi.begin(wifi_cfg.ssid, wifi_cfg.pwd);
	int x = 0;
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		debugMe("*"+ String(WiFi.status())+ "*" ,false);
		x++;
		if (x > 30) break;
	}

	debugMe("trys: " + String(x));
	debugMe("WiFi connected.");
	debugMe("IP address: ", false);
	debugMe(WiFi.localIP());
	debugMe("COOOOOOOOOOOOL");

}

void setup_wifi_Network()
{
		// setup the wifi network old version from EPS8266
		
	
		debugMe("Starting Wifi Setup");
		unsigned long currentT = millis();
		
		if (get_bool(WIFI_MODE) == false)
		{
			
			uint8_t con_try = 1;

			if (get_bool(DEBUG_OUT) == true)
			{
				debugMe(String("ssid:" + String(wifi_cfg.ssid)));
				debugMe(String("pwd:" + String(wifi_cfg.pwd)));
				debugMe(String("try : " + String(con_try) + "."));
			}




			//WiFi.mode(WIFI_STA);
			//delay(100);
			WiFi.begin(wifi_cfg.ssid, wifi_cfg.pwd);
			
			uint8_t try_led_counter = 0;
			uint8_t led_color[3] = { 255,0,0 };

			//LEDS_setall_color();

			while (WiFi.status() != WL_CONNECTED)
			{
				
						//static uint8_t led_color[3] = {0,0,0}
							//led_color[3] con_try
				//LEDS_setLED_show(try_led_counter, led_color);
				//try_led_counter++;
				delay(500);
				
				debugMe(String("." + String(con_try) + "."), false);
				if (millis() > currentT + WIFI_CLIENT_CONNECT_TIMEOUT * con_try)
					break;

			}
			


			if(WiFi.status() != WL_CONNECTED)
			{
				led_color[0] = 0;
				led_color[1] = 255; 

				while (con_try <= WIFI_CLIENT_CONNECT_TRYS)
				{
						currentT = millis();
						con_try++;
						//debugMe(".");
						debugMe(String("try : " + String(con_try)+ "."),false);
						WiFi.reconnect();
						delay(100);
						while (WiFi.status() != WL_CONNECTED)
						{
							//LEDS_setLED_show(try_led_counter, led_color);
							//try_led_counter++;

							delay(500);
							debugMe(String("."+ String(con_try) + "."),false);
							if (millis() > currentT + WIFI_CLIENT_CONNECT_TIMEOUT * con_try )
								break;
						}
						if (WiFi.status() == WL_CONNECTED) break;
				}
			}

		}
		/*	else  // wifimode AP
			{


				//WiFi.softAPConfig(wifi_cfg.ipStaticLocal, wifi_cfg.ipStaticLocal, wifi_cfg.ipSubnet);   //wifi_cfg.ipStaticLocal, wifi_cfg.ipDGW, wifi_cfg.ipSubnet, wifi_cfg.ipDNS

				WiFi.mode(WIFI_AP);
				WiFi.softAP(wifi_cfg.APname, DEF_AP_PASSWD);

				delay(50);
				debugMe("Start AP mode");
			} */
		
	

	if (WiFi.status() != WL_CONNECTED)
	{
		WiFi.disconnect();
		//WiFi.softAPConfig(wifi_cfg.ipStaticLocal, wifi_cfg.ipStaticLocal, wifi_cfg.ipSubnet);
		delay(50);
		WiFi.mode(WIFI_AP);

		WiFi.softAP(wifi_cfg.APname);// , DEF_AP_PASSWD);
		debugMe("Starting Wifi Backup no Password");
		//while(WiFi.status() != )

	}
	


		//debugMe("");
		debugMe("Wifi Signal Strength : " + String(WiFi.RSSI()));
		//debugMe(WiFi.RSSI());  // test to get the Signal strength  returns a "long"
		//debugMe(String("IP : "  + String(WiFi.localIP())) );
		debugMe(String("IP : "),false);
		debugMe(WiFi.localIP());
		


}


#ifndef FFT_SERVER_DISABLED
void WIFI_FFT_enable() 
{
	// enable the FFT mode 
	
	if (get_bool(FFT_MASTER) == true)	
	{
		FFT_master.begin(fft_ip_cfg.port_master);
		//FFT_master.setNoDelay(true); // so that we dont bundle packets together!!!
	}
	else 
	{
#ifdef ESP8266
		FFT_slave.beginMulticast(WiFi.localIP(), fft_ip_cfg.IP_multi, fft_ip_cfg.port_slave);
#endif
#ifdef ESP32
		FFT_slave.beginMulticast(fft_ip_cfg.IP_multi, fft_ip_cfg.port_slave);
#endif
		//FFT_slave.setNoDelay(true); 
		FFT_slave.flush();

	} 
}

void WIFI_FFT_disable() 
{
		// disable the FFT_client/server

	if (get_bool(FFT_MASTER) == true)
		FFT_master.stop();
	else FFT_slave.stop();
	//if (get_bool(SLAVE_MODE) == true) 
		
	

}


void WIFI_FFT_toggle_master(boolean value)
{
	// toggle FFT mode 
	// first switch it off and then 
	// write the value into the mode
	// start the FFT mode 

	if (get_bool(FFT_ENABLE) == true) WIFI_FFT_disable();
	write_bool(FFT_MASTER, value);
	WIFI_FFT_enable();
}


void WIFI_FFT_toggle(boolean mode_value) 
{
	// foggle the fft mode
	//debugMe("switched fft mode");

	write_bool(FFT_ENABLE, mode_value);
	if (mode_value == true) WIFI_FFT_enable();
	else  WIFI_FFT_disable();
	LEDS_fadeout();
	//PaletteMenu();
	//leds.fadeToBlackBy(255);
	//FastLED.show();


}




void WIFI_FFT_slave_handle() 
{
	// the main function to handle incomming FFT packets over wifi
	// recive the Multicast Packet from the Master FFT Sender and send it to the Leds or MC
	if (get_bool(FFT_ENABLE) == true && get_bool(FFT_MASTER) == false)			// SLAVE mode
	{	
		uint8_t packetSize = FFT_slave.parsePacket();

		//while (packetSize) 
		{
			//if (packetSize) {debugMe("F",false); } // debugMe(packetSize) ; }
			//debugMe(packetSize);
			if ((packetSize == 8) && (get_bool(FFT_ENABLE) == true) && (get_bool(FFT_MASTER) == false)) // Its a FFT packet B0 = fps , B1-7 = FFT Bins
			{
				fft_led_cfg.fps = FFT_slave.read(); // Get the speed update  first byte
				
				for (uint8_t i = 0; i < 7; i++) LEDS_FFT_enqueue(FFT_slave.read());
				//{ FFT_fifo.enqueue(FFT_slave.read()); }

	#ifdef CMD_MESSEGER_PORT    
				//Com_slave_ESP_Send_FFT();
	#endif     

			}
			FFT_slave.flush();						// flush out the open packet

			//packetSize = 0; // FFT_slave.parsePacket();		// get a new packet if available  
		} // end while packetsize !=0
	}
}


void WIFI_FFT_master_send() 
{
	// The main FFT master send function
	// send out the FFT multicast packets

	if (get_bool(FFT_ENABLE) == true && true == get_bool(FFT_MASTER_SEND) )
	{ // && (FFT_fifo.count() >= FFT_SEND_NR_PIXELS)  ) {
										//CRGB fft_outdata; 
										// Send a multicast packet to The Slave ESP servers.
#ifdef ESP8266
		FFT_master.beginPacketMulticast(fft_ip_cfg.IP_multi, fft_ip_cfg.port_slave, WiFi.localIP());
#endif 
#ifdef ESP32
		FFT_master.beginMulticastPacket();
#endif
		FFT_master.write(fft_led_cfg.fps);
		for (uint8_t i = 0; i < 7; i++) FFT_master.write(LEDS_FFT_get_value(i));
		FFT_master.endPacket();
	}
}


void WIFI_FFT_master_handle()
{
	// master FFT server, flusch out all incomming packets to make shure the rec buffer stays empty
	
	if (get_bool(FFT_ENABLE) == true && get_bool(FFT_MASTER)== true)
	{
		uint8_t packetSize = FFT_master.parsePacket();
		while (packetSize) { FFT_master.flush(); packetSize = FFT_master.parsePacket();} // flush out incomming all packets
	}	
}


void FFT_setup_vars()
{		// setup the wifi FFT variables

	if (FS_FFT_read(0) == false)
	{
		//write_bool(FFT_ENABLE, DEF_FFT_ENABLE);
		//write_bool(FFT_MASTER, DEF_FFT_MASTER);
		fft_ip_cfg.IP_multi		=  DEF_FFT_IP_MULTICAST;
		fft_ip_cfg.port_slave	=  DEF_FFT_SLAVE_PORT;
		fft_ip_cfg.port_master	=  DEF_FFT_MASTER_PORT;
	}

}

void FFT_Setup()
{
	// setup the wifi FFT 
	FFT_setup_vars();
	if (get_bool(FFT_ENABLE)== true) WIFI_FFT_enable();


}

void FFT_handle_loop()
{
	// the main wifi-fft loop
	WIFI_FFT_slave_handle();
	WIFI_FFT_master_handle();

}


#endif




// The main Wifi Setup
void wifi_setup()
{
	WiFi.onEvent(wifi_Event); // Start event handler!
	

	wifi_load_settings();
	
	

	setup_wifi_Network();
	setup_OTA();
	setup_NTP();   //ESP32 NOK
	TelnetDebug.begin(wifi_cfg.APname);

	debugMe("Hello World");


	#ifndef ARTNET_DISABLED
		setup_artnet_Vars();
		setup_artnet();
	#endif

	OSC_setup();
	
	httpd_setup();

	FFT_Setup();
	
}


// the main WiFi Loop 
// making shure that all ports are handeld and flushed.
void wifi_loop()
{
	ArduinoOTA.handle();	// Run the main OTA loop for Wifi updating
	//yield();
	//NTP_parse_response();	// get new packets and flush if not correct.
	yield();
	OSC_loop();
	yield();
	http_loop();
	yield();
	FFT_handle_loop();
	yield();
	TelnetDebug.handle();

}

