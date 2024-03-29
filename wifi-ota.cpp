
#include "config_TPM.h"


#ifdef USE_ETHERNET
	#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT
	#define ETH_PHY_POWER 12

	#include <ETH.h>


	static bool eth_connected = false;



#else 
		#include <WiFiMulti.h>	
		#include <WiFiUdp.h>

#endif




	#include "AsyncUDP.h"
	//#include <WiFiAP.h>

	#include <ArduinoOTA.h>
	#include <DNSServer.h>

	#include "time.h"
	#include <RemoteDebug.h> 

	#include "tpm_artnet.h"




#include "tools.h"			// include the Tools for reading and writing bools and DebugMe
#include "wifi-ota.h"		// needs Wifi and co for data stuctures!!!
#include "leds.h"			// include for led data structures

#include "config_fs.h"	


#include "httpd.h"
#include "osc.h"



RemoteDebug TelnetDebug;
DNSServer dnsServer;

// wifi
wifi_Struct wifi_cfg;



// ntp
#define NTP_PACKET_SIZE 48		// the ntp packetsize
#define NTP_LOCAL_PORT	2843	// 2 Invinity 4 Ever

//the ntp deamon
//WiFiUDP ntp_udp;
AsyncUDP ntp_udp;

// FFT data 
WiFiUDP  FFT_master;
WiFiUDP  FFT_slave;





fft_ip_cfg_struct fft_ip_cfg;



// from leds

//extern fft_led_cfg_struct fft_led_cfg;



//OTA 
void WiFi_OTA_setup()
{
	// configure and setup the OTA process 
	//  so that its possible to compile and send
	// out of arduino 
	// Port defaults to 8266
	// ArduinoOTA.setPort(8266);

	// Hostname defaults to esp8266-[ChipID]
	ArduinoOTA.setHostname(wifi_cfg.APname);

	// authentication by default Password = 
	//ArduinoOTA.setPassword((const char *) "love" );

	//if (get_bool(DEBUG_OUT) == true) 
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

String WiFi_Get_Name()
{
	return String(wifi_cfg.APname);

}

// taken from https://github.com/fbiego/ESP32Time/blob/main/ESP32Time.cpp

/*!
    @brief  set the internal RTC time
    @param  epoch
            epoch time in seconds
    @param  ms
            microseconds (optional)
*/
void tpm_settime(long epoch, int ms) {
  struct timeval tv;
  tv.tv_sec = epoch;  // epoch time (seconds)
  tv.tv_usec = ms;    // microseconds
  settimeofday(&tv, NULL);
}


void tpm_settime(int sc, int mn, int hr)
{
  // seconds, minute, hour, day, month, year $ microseconds(optional)
  // ie setTime(20, 34, 8, 1, 4, 2021) = 8:34:20 1/4/2021
  struct tm t = {0};        // Initalize to all 0's
  t.tm_year = 2021 - 1900;    // This is year-1900, so 121 = 2021
  t.tm_mon = 7 - 1;
  t.tm_mday = 4;
  t.tm_hour = hr;
  t.tm_min = mn;
  t.tm_sec = sc;
  time_t timeSinceEpoch = mktime(&t);
  tpm_settime(timeSinceEpoch, 0);



}




int NTP_get_time_h()
{
	struct tm timeinfo;
	if (!getLocalTime(&timeinfo)) {
		debugMe("Failed to obtain time_H");
		return 0;
	}
	//debugMe("***********************");
	return timeinfo.tm_hour;

}
int NTP_get_time_m()
{
	struct tm timeinfo;
	if (!getLocalTime(&timeinfo)) {
		debugMe("Failed to obtain time_m");
		return 0;
	}
	//debugMe("***********************");
	return timeinfo.tm_min;

}
int NTP_get_time_s()
{
	struct tm timeinfo;
	if (!getLocalTime(&timeinfo)) {
		debugMe("Failed to obtain time_s");
		return 0;
	}
	//debugMe("***********************");
	return timeinfo.tm_sec;

}


void WiFi_NTP_printTime() {
	
	struct tm timeinfo;
	
	debugMe("Time-Setup:");
	if (get_bool(WIFI_MODE_TPM) != WIFI_ACCESSPOINT)
		{
			if (!getLocalTime(&timeinfo)) {
			debugMe("Failed to obtain time_print");
			tpm_settime(0,0,0)	;
			return;
			}
		}
	else tpm_settime(0,0,0)	;


	
	//debugMe("***********************");
	debugMe(timeinfo);

}


void WiFi_NTP_setup()
{

	
	const long  gmtOffset_sec = 3600;
	const int   daylightOffset_sec = 3600;



	{
		//debugMe("NTP DNS name resolution OK, Getting NTP Time");
		configTime(gmtOffset_sec, daylightOffset_sec, wifi_cfg.ntp_fqdn);
		WiFi_NTP_printTime();
	}

}





void WiFi_telnet_print(String input, boolean line)
{
	
	if ((TelnetDebug.isActive(TelnetDebug.VERBOSE)) && get_bool(DEBUG_TELNET))
	{
		if (line == true)
			TelnetDebug.println(input);
		else
			TelnetDebug.print(input);

	}



}
void WiFi_telnet_print(tm input, boolean line)
{
	/*
	if ((TelnetDebug.isActive(TelnetDebug.VERBOSE)) && get_bool(DEBUG_TELNET))
	{
		if (line == true)
			TelnetDebug.println(String(input));
		else
			TelnetDebug.print(String(input));

	}// */



}
void WiFi_telnet_print(float input, boolean line)
{

	if ((TelnetDebug.isActive(TelnetDebug.VERBOSE)) && get_bool(DEBUG_TELNET))
	{
		if (line == true)
			TelnetDebug.println(String(input));
		else
			TelnetDebug.print(String(input));

	}



}
void WiFi_telnet_print(uint8_t input, boolean line)
{

	if ((TelnetDebug.isActive(TelnetDebug.VERBOSE)) && get_bool(DEBUG_TELNET))
	{
		if (line == true)
			TelnetDebug.println(String(input));
		else
			TelnetDebug.print(String(input));

	}



}
void WiFi_telnet_print(int input, boolean line)
{

	if ((TelnetDebug.isActive(TelnetDebug.VERBOSE)) && get_bool(DEBUG_TELNET))
	{
		if (line == true)
			TelnetDebug.println(String(input));
		else
			TelnetDebug.print(String(input));

	}



}
void WiFi_telnet_print(IPAddress input, boolean line)
{

	if ((TelnetDebug.isActive(TelnetDebug.VERBOSE)) && get_bool(DEBUG_TELNET))
	{
		if (line == true)
			TelnetDebug.println(input);
		else
			TelnetDebug.print(input);

	}



}





void WiFi_load_settings()   // load the wifi settings from SPIFFS or from default Settings.
{
	// load the wifi vaiables
	//debugMe("Brown x1");
	// Clean out the Wifi char arrays
	memset(wifi_cfg.APname, 0, sizeof(wifi_cfg.APname));
	memset(wifi_cfg.APpassword, 0, sizeof(wifi_cfg.APpassword));
	memset(wifi_cfg.ssid, 0, sizeof(wifi_cfg.ssid));
	memset(wifi_cfg.pwd, 0, sizeof(wifi_cfg.pwd));
	memset(wifi_cfg.ntp_fqdn, 0, sizeof(wifi_cfg.ntp_fqdn));

	//debugMe("Brown x2");
	
	if (!FS_wifi_read() || OVERWRITE_INIT_CONF_ON )		// Get the config of disk,  on fail load defaults.
	//if (false == false)		// Get the config of disk,  on fail load defaults.
	{
		debugMe("Loading WifiSetup Defaults");
		//load the defaults
		String def_APname 		= DEF_AP_NAME;
		String def_APpassword 	= DEF_AP_PASSWD;
		String def_ssid 		= DEF_SSID;
		String def_pwd 			= DEF_WIFI_PWD;
		String def_ntp_fqdn 	= DEF_NTP_SERVER;

		def_APname.toCharArray(wifi_cfg.APname, def_APname.length() + 1);
		def_APpassword.toCharArray(wifi_cfg.APpassword,def_APpassword.length() +1);
		def_ssid.toCharArray(wifi_cfg.ssid, def_ssid.length() + 1);
		def_pwd.toCharArray(wifi_cfg.pwd, def_pwd.length() + 1);
		def_ntp_fqdn.toCharArray(wifi_cfg.ntp_fqdn, def_ntp_fqdn.length() + 1);

		write_bool(WIFI_POWER, DEF_WIFI_POWER);
		
		write_bool(OTA_SERVER, DEF_OTA_SERVER);
		write_bool(HTTP_ENABLED, DEF_HTTP_ENABLED);
		write_bool(WIFI_MODE_TPM, DEF_WIFI_MODE);
		write_bool(WIFI_MODE_BOOT, DEF_WIFI_MODE);
		write_bool(STATIC_IP_ENABLED, DEF_STATIC_IP_ENABLED);

		wifi_cfg.ipStaticLocal = DEF_IP_LOCAL;
		wifi_cfg.ipSubnet = DEF_IP_SUBNET;
		wifi_cfg.ipDGW = DEF_IP_DGW;
		wifi_cfg.ipDNS = DEF_DNS;
		wifi_cfg.wifiChannel = constrain(DEF_WIFI_CHANNEL,1,12);

		if (WRITE_CONF_AT_INIT || OVERWRITE_INIT_CONF_ON) FS_wifi_write();
	}

	write_bool(WIFI_POWER_ON_BOOT, get_bool(WIFI_POWER));

	
}




void WiFi_Event(WiFiEvent_t event, system_event_info_t info)
{
	//if (get_bool(WIFI_EVENTS) == true || event == 7 )   // DHCP response (7) or all
	{
		debugMe("[WiFi-event] event:"+ String(event));
		//ip4_addr_t  infoIP4;
		IPAddress infoIP;
		
		
		switch (event) {
		case  SYSTEM_EVENT_SCAN_DONE:					/**< 1 ESP32 finish scanning AP */
			debugMe("finish scanning AP");
			debugMe("Status: " + info.scan_done.status);
			debugMe("number: " + info.scan_done.number);
			debugMe("scan ID: " + info.scan_done.scan_id);
			break;

		case SYSTEM_EVENT_STA_START:					/**<2 ESP32 station start */
			debugMe("STA Started",true,true);
			WiFi.setHostname(wifi_cfg.APname);
			break;

		case SYSTEM_EVENT_STA_STOP:						/**<3 ESP32 station stop */
			debugMe("STA Stopped",true,true);
			break;

		case SYSTEM_EVENT_STA_CONNECTED:				/**<4 ESP32 station connected to AP */
			debugMe("WIFI:STA Connected");
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
			debugMe("station got IP from connected AP",true,true);
			debugMe("ON SSID :" + String(WiFi.SSID()),true,true);
			infoIP = info.got_ip.ip_info.ip.addr;
			debugMe("Got IPv4: ",false,true);
			debugMe(infoIP,true,true);
			infoIP = info.got_ip.ip_info.netmask.addr;
			debugMe("Got NetMask: ", false,true);
			debugMe(infoIP,true,true);
			infoIP = info.got_ip.ip_info.gw.addr;
			debugMe("Got DGW: ", false,true);
			debugMe(infoIP,true,true);
			//debugMe("Changed = " + String(info.got_ip.ip_changed),true,true);
			
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
			debugMe("WiFi: soft-AP Started HOSTNAME = " + String(wifi_cfg.APname)+ " PWD: " + String(wifi_cfg.APpassword),true,true);
			debugMe("IP:",false,true);
			if(get_bool(STATIC_IP_ENABLED) ) debugMe(wifi_cfg.ipStaticLocal,true,true);
				else debugMe("192.168.4.1",true,true);
				
			break;

		case	SYSTEM_EVENT_AP_STOP:                  /**<14 ESP32 soft-AP stop */
			debugMe("WiFi: soft-AP Stopped",true,true);
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

#ifdef USE_ETHERNET

		case SYSTEM_EVENT_ETH_START:
			debugMe("ETH Started",true,true);
			//set eth hostname here
			ETH.setHostname(wifi_cfg.APname);
			break;

		case	SYSTEM_EVENT_ETH_STOP:                 /**<22 ESP32 ethernet stop */
			debugMe("ETH Stopped",true,true);
      		eth_connected = false;
      break;

		case	SYSTEM_EVENT_ETH_CONNECTED:            /**<23 ESP32 ethernet phy link up */
			debugMe("ETH Connected",true,true);
			Wifi_Stop_Network();
     		break;

		case	SYSTEM_EVENT_ETH_DISCONNECTED:         /**<24 ESP32 ethernet phy link down */
			debugMe("ETH Disconnected",true,true);
			eth_connected = false;
			write_bool(WIFI_POWER_ON_BOOT, get_bool(WIFI_POWER));
			if (get_bool(WIFI_POWER) ) WiFi_Start_Network();
			break;

		case	SYSTEM_EVENT_ETH_GOT_IP:               /**<25 ESP32 ethernet got IP from connected AP */
			Wifi_Stop_Network();
			
			debugMe("ETH MAC: ",false,true);
			debugMe(ETH.macAddress(),false,true);
			debugMe(", IPv4: ",false,true);
			debugMe(ETH.localIP(),false,true);
			if (ETH.fullDuplex()) {
				debugMe(", FULL_DUPLEX",false,true);
			}
			debugMe(", ",false,true);
			debugMe(ETH.linkSpeed(),false,true);
			debugMe("Mbps",true,true);
			eth_connected = true;
			break;
#endif // OLIMEX Ethernet 

		case 	SYSTEM_EVENT_WIFI_READY:
			debugMe("WIFI-Ready");
			break;

		default:
			debugMe("OTHER UNKNOWN EVENT");
			break;
		}
		debugMe("WiFi Event END---------");
	}
}






void WiFi_Start_Network_CLIENT()
{
		debugMe("Wifi_Client_Connect : ");
		WiFi.persistent(false);
		WiFi.disconnect(true);
		WiFi.mode(WIFI_OFF);
		WiFi.mode(WIFI_STA);
		if (get_bool(STATIC_IP_ENABLED))
			WiFi.config(wifi_cfg.ipStaticLocal, wifi_cfg.ipDGW, wifi_cfg.ipSubnet,wifi_cfg.ipDNS);
		WiFi.begin(wifi_cfg.ssid, wifi_cfg.pwd);
		debugMe("SSID : ",false,true);
		debugMe(String(wifi_cfg.ssid),false,true);
		debugMe(" - PWD : ",false,true);
		debugMe(wifi_cfg.pwd,true,true);
	
	
}


void WiFi_print_settings()
{


		debugMe("");
		debugMe("WiFi connected!");
		debugMe("IP address: ",false);
		debugMe(WiFi.localIP());
		debugMe("ESP Mac Address: ",false);
		debugMe(WiFi.macAddress());
		debugMe("Subnet Mask: ",false);
		debugMe(WiFi.subnetMask());
		debugMe("Gateway IP: ",false);
		debugMe(WiFi.gatewayIP());
		debugMe("DNS: ",false);
		debugMe(WiFi.dnsIP());
}


void Wifi_Stop_Network()
{
	 WiFi.mode(WIFI_OFF);
	debugMe("Wifi is OFF",true,true);
}



void WiFi_Start_Network()
{
	//debugMe("x0");
		// setup the wifi network old version from EPS8266
	boolean btn_read = 	digitalRead(BTN_PIN); 	
	//debugMe("x00");
	if ( btn_read == BUTTON_DOWN  ||  get_bool(WIFI_MODE_TPM) == WIFI_ACCESSPOINT )			// if we push the button or wifi is set to ACCESS point
	{
		write_bool(WIFI_MODE_BOOT, WIFI_ACCESSPOINT);
		//debugMe("c0");
		LEDS_setall_color(1); FastLEDshowESP32(); delay(500);
		//WiFi.mode(WIFI_MODE_AP);
		
		//delay(500);
		

		if(btn_read == BUTTON_DOWN )
		{
			if(get_bool(STATIC_IP_ENABLED))  WiFi.softAPConfig(wifi_cfg.ipStaticLocal, wifi_cfg.ipStaticLocal, wifi_cfg.ipSubnet); 
			delay(1000);
			 WiFi.softAP(wifi_cfg.APname, DEF_AP_PASSWD, (random8(12)+1) ); //wifi_cfg.wifiChannel); 
			//write_bool(STATIC_IP_ENABLED,true);
			write_bool(WIFI_POWER,true);
			write_bool(WIFI_POWER_ON_BOOT, true);
			write_bool(HTTP_ENABLED, true);
			debugMe("Start AP mode button : " + String(wifi_cfg.APname) + " : " + String(DEF_AP_PASSWD),true,true);
		}
		else if(get_bool(WIFI_POWER))
			{
				//debugMe("c2");
					if(get_bool(STATIC_IP_ENABLED)) WiFi.softAPConfig(wifi_cfg.ipStaticLocal, wifi_cfg.ipStaticLocal, wifi_cfg.ipSubnet); // ,wifi_cfg.APname, DEF_AP_PASSWD);
					//debugMe("c2x");
					
					delay(1000);
					WiFi.softAP(wifi_cfg.APname, wifi_cfg.APpassword);

				//delay(500);
				
				//debugMe(String("Start AP mode : " + String(wifi_cfg.APname) + " : " + String(wifi_cfg.APpassword)));
			}
		//debugMe("c4");
		//delay(500);
		LEDS_setall_color(2); FastLEDshowESP32(); delay(500);


		

	}
	else  if (  get_bool(WIFI_MODE_TPM) != WIFI_ACCESSPOINT &&  get_bool(WIFI_POWER_ON_BOOT)   )	
	{	

		debugMe("Starting Wifi Client Setup", true,true);
		
		LEDS_setall_color(3); FastLEDshowESP32(); delay(500);

		write_bool(WIFI_MODE_BOOT, false);
			
		WiFi_Start_Network_CLIENT();

	
	}
	wifi_cfg.connectTimeout = millis() + 20000;
	
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
				LEDS_set_fft_fps( FFT_slave.read()); // Get the speed update  first byte
				
				for (uint8_t i = 0; i < 7; i++) LEDS_FFT_enqueue(FFT_slave.read());
				

   

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

	if (get_bool(FFT_ENABLE) == true &&  get_bool(FFT_MASTER_SEND) == true  )
	//&& (FFT_fifo.count() >= FFT_SEND_NR_PIXELS)  ) 
	{
		// Send a multicast packet to The Slave ESP servers.

		FFT_master.beginMulticastPacket();

		FFT_master.write( LEDS_get_fft_fps());
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


void WiFi_FFT_setup_vars()
{		// setup the wifi FFT variables

	if (FS_FFT_read(0) == false)
	{
		write_bool(FFT_MASTER_SEND, DEF_FFT_MASTER_SEND);
		write_bool(FFT_MASTER, DEF_FFT_MASTER);
		fft_ip_cfg.IP_multi		=  DEF_FFT_IP_MULTICAST;
		fft_ip_cfg.port_slave	=  DEF_FFT_SLAVE_PORT;
		fft_ip_cfg.port_master	=  DEF_FFT_MASTER_PORT;

	}

}

void WiFi_FFT_Setup()
{
	// setup the wifi FFT 
	WiFi_FFT_setup_vars();
	if (get_bool(FFT_ENABLE)== true) WIFI_FFT_enable();


}

void WiFi_FFT_handle_loop()
{
	// the main wifi-fft loop
	WIFI_FFT_slave_handle();
	WIFI_FFT_master_handle();

}


#endif   // FFT ENABLED


#ifdef USE_ETHERNET
void eth_load_settings()
{
		ETH.begin();
		if (get_bool(STATIC_IP_ENABLED) ) ETH.config(wifi_cfg.ipStaticLocal ,  wifi_cfg.ipDGW,wifi_cfg.ipSubnet,wifi_cfg.ipDNS,wifi_cfg.ipDNS);
		else debugMe("DHCP ON", true,true);
		debugMe("ETH Connecting:",true,true);
		yield();

}

#endif  //Olimex

boolean Network_connected_check()
{
#ifdef USE_ETHERNET
		if (eth_connected) return true;
#endif

		return	get_bool(WIFI_POWER_ON_BOOT);

}

void wifi_start_IP_services()
{	
	debugMe("start services");
#ifdef USE_ETHERNET
	yield();
	if (!eth_connected)  WiFi_Start_Network();
	if (eth_connected||get_bool(WIFI_POWER_ON_BOOT) ) ;
#else
	if (get_bool(WIFI_POWER_ON_BOOT))
#endif	
	{
		WiFi_OTA_setup();
		WiFi_NTP_setup();   //ESP32 NOK
		
		TelnetDebug.begin(wifi_cfg.APname);

		//debugMe("Hello World");



			
		WiFi_artnet_enable(); 



		OSC_setup();
		
		httpd_setup();

		//if(get_bool(STATIC_IP_ENABLED)) dnsServer.start(53, "tpm", wifi_cfg.ipStaticLocal);// WiFi.localIP());
		//else dnsServer.start(53, "tpm", IPAddress(192, 168, 4, 1));// WiFi.localIP());

		//WiFi_FFT_Setup();

		//WiFi_print_settings();
		debugMe("end services");
	}
}


// The main Wifi Setup
void wifi_setup()
{
	

	WiFi_load_settings();
	
	WiFi.onEvent(WiFi_Event); // Start event handler!
	
	//delay(5000);
	#ifdef USE_ETHERNET
	debugMe(String("OLIMEX!!!  "),true);
		eth_load_settings();
		yield();
		wifi_cfg.connectTimeout = millis() + 20000;
		yield();
//		if (!eth_connected)
//		{
//			debugMe(String("eth not connected dropping to Wifi  "),true,true);
//			//WiFi.onEvent(WiFi_Event); // Start event handler!
//			WiFi_Start_Network();

//		}
	#else
		
		WiFi_Start_Network();
	#endif

	
	
	//if (get_bool(WIFI_POWER_ON_BOOT))
	
}


void ip_services_loop()
{
		
		ArduinoOTA.handle();	// Run the main OTA loop for Wifi updating
		//yield();
		//NTP_parse_response();	// get new packets and flush if not correct.
		yield();
		OSC_loop();
		yield();
		http_loop();
		yield();
		//WiFi_FFT_handle_loop();
		yield();
		TelnetDebug.handle();



}







// the main WiFi Loop 
// making shure that all ports are handeld and flushed.
void wifi_loop()
{
		#ifdef USE_ETHERNET
			if ( (WiFi.status() != WL_CONNECTED) && (get_bool(WIFI_MODE_BOOT) != WIFI_ACCESSPOINT ) &&  (get_bool(WIFI_POWER_ON_BOOT)) && !eth_connected ) 
		#else  
			if ( (WiFi.status() != WL_CONNECTED) && (get_bool(WIFI_MODE_BOOT) != WIFI_ACCESSPOINT ) &&  (get_bool(WIFI_POWER_ON_BOOT)) ) 
		#endif 
		{	
			unsigned long currentT = millis();
			if (currentT > wifi_cfg.connectTimeout )
			{
				wifi_cfg.connectTimeout = currentT + 30000;
			//WiFi.disconnect(); 
			//WIFI_start_wificlient(); 
			//WiFi_Start_Network();
			
				WiFi_Start_Network_CLIENT();
				//WiFi_print_settings();
			}
			
		}

#ifdef USE_ETHERNET
		if  ((get_bool(WIFI_POWER_ON_BOOT)) || eth_connected)
#else  
		if  ((get_bool(WIFI_POWER_ON_BOOT)))
#endif 
		{
			ip_services_loop();

		}
		

}

