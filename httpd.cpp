/*
		Here we Have the Web Server (httpd) and the update client over http 

		Credit where credit is due: Some parts of the code taken from : .... TODO 

		the html files are located in the data subfolder and need to be sent in with 
		ESP8266 Sketch Data Upload howto = HTTP:// ....... TODO


*/

#include "config_TPM.h"



#ifdef _MSC_VER  

	#ifdef ESP32
		#include<WiFi\src\WiFi.h>
		//#include <HTTPClient.h>
		#include<ESPmDNS\src\ESPmDNS.h>
		#include<SPIFFS\src\SPIFFS.h>
		#include<FS\src\FS.h>
		#include<ESP8266WebServer\WebServer.h>
	#endif


#else

	#ifdef ESP32
		#include <WiFi.h>	
		//#include <HTTPClient.h>
		#include <ESPmDNS.h>
		#include <WebServer.h>
		#include <FS.h>	
		#include<SPIFFS.h>
	#endif


#endif
 

// ********* Externals
	#include "tools.h"						// for bools reading/writing
	#include "config_fs.h"					
	#include "httpd.h"

// *********** External Variables 
	#include "wifi-ota.h"					// get the wifi structures
	extern wifi_Struct wifi_cfg;			// link to wifi variable wifi_cfg


// Variables
	WebServer  httpd(80);					// The Web Server 




String httpd_getContentType(String filename) {
	if (httpd.hasArg("download")) return "application/octet-stream";
	else if (filename.endsWith(".htm")) return "text/html";
	else if (filename.endsWith(".html")) return "text/html";
	else if (filename.endsWith(".css")) return "text/css";
	else if (filename.endsWith(".js")) return "application/javascript";
	else if (filename.endsWith(".png")) return "image/png";
	else if (filename.endsWith(".gif")) return "image/gif";
	else if (filename.endsWith(".jpg")) return "image/jpeg";
	else if (filename.endsWith(".ico")) return "image/x-icon";
	else if (filename.endsWith(".xml")) return "text/xml";
	else if (filename.endsWith(".pdf")) return "application/x-pdf";
	else if (filename.endsWith(".zip")) return "application/x-zip";
	else if (filename.endsWith(".gz")) return "application/x-gzip";
	return "text/plain";
}


bool httpd_handleFileRead(String path) {

	 
      
	if (path.endsWith("/")) path += "index.html";
	debugMe("handleFileRead: " + path);

	String contentType = httpd_getContentType(path);
	String pathWithGz = path + ".gz";
	if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
		if (SPIFFS.exists(pathWithGz))
			path += ".gz";
		File file = SPIFFS.open(path, "r");
		size_t sent = httpd.streamFile(file, contentType);
		file.close();
		debugMe(path + " closed");
		return true;
	}
	return false;
}

void httpd_handleFileUpload() {
	File fsUploadFile;							// Variable to hold a file upload
	if (httpd.uri() != "/edit") return;
	HTTPUpload& upload = httpd.upload();
	if (upload.status == UPLOAD_FILE_START) {
		String filename = upload.filename;
		if (!filename.startsWith("/")) filename = "/" + filename;
              
		debugMe("handleFileUpload Name: ",false);
		debugMe(filename);
            
		fsUploadFile = SPIFFS.open(filename, "w");
		filename = String();
	}
	else if (upload.status == UPLOAD_FILE_WRITE) {
		//DBG_OUTPUT_PORT.print("handleFileUpload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
		if (fsUploadFile)
			fsUploadFile.write(upload.buf, upload.currentSize);
	}
	else if (upload.status == UPLOAD_FILE_END) {
		if (fsUploadFile)
			fsUploadFile.close();

                 
		debugMe("handleFileUpload Size: ",false);
		debugMe(String(upload.totalSize));
           
	}
}



void httpd_handleFileDelete() {
	if (httpd.args() == 0) return httpd.send(500, "text/plain", "BAD ARGS");
	String path = httpd.arg(0);

             
	 debugMe("handleFileDelete: " + path);

	if (path == "/")
		return httpd.send(500, "text/plain", "BAD PATH");
	if (!SPIFFS.exists(path))
		return httpd.send(404, "text/plain", "FileNotFound");
	SPIFFS.remove(path);
	httpd.send(200, "text/plain", "");
	path = String();
}

void httpd_handleFileCreate() {
	if (httpd.args() == 0)
		return httpd.send(500, "text/plain", "BAD ARGS");
	String path = httpd.arg(0);

	 debugMe("handleFileCreate: " + path);

	if (path == "/")
		return httpd.send(500, "text/plain", "BAD PATH");
	if (SPIFFS.exists(path))
		return httpd.send(500, "text/plain", "FILE EXISTS");
	File file = SPIFFS.open(path, "w");
	if (file)
		file.close();
	else
		return httpd.send(500, "text/plain", "CREATE FAILED");
	httpd.send(200, "text/plain", "");
	path = String();
}

void httpd_handleFileList() {
	if (!httpd.hasArg("dir")) { httpd.send(500, "text/plain", "BAD ARGS"); return; }
	String path = httpd.arg("dir");
 
	 debugMe("handleFileList: " + path);

	File dir = SPIFFS.open(path);

	//File file = dir.openNextFile();

	path = String();

	String output = "[";

	File fileX = dir.openNextFile();

	debugMe(String(fileX.name()));

	while (fileX) {
		//File entry = dir.open("r");
		if (output != "[") output += ',';
		bool isDir = fileX.isDirectory();
		//bool isDir = false;
		output += "{\"type\":\"";
		output += (isDir) ? "dir" : "file";
		output += "\",\"name\":\"";
		output += String(fileX.name());
		output += "\"}";
		//debugMe(String(fileX.name()));
		fileX.close();
		fileX = dir.openNextFile();
		debugMe(String(fileX.name()));
		//dir.close();
	}

	dir.close();


	output += "]";
	httpd.send(200, "text/json", output);
	//debugMe(output);
}

void httpd_handle_default_args()
{

	if (httpd.args() > 0)
	{
    
		 debugMe("set args present");

		// set the brightness


		if (httpd.hasArg("ssid") && httpd.hasArg("password") && httpd.hasArg("wifiMode")) {
			String ssid_STR = httpd.arg("ssid");
			String PWD_STR = httpd.arg("password");
			String wifiMmode_STR = httpd.arg("wifiMode");

			ssid_STR.toCharArray(wifi_cfg.ssid, sizeof(wifi_cfg.ssid));
			PWD_STR.toCharArray(wifi_cfg.pwd, sizeof(wifi_cfg.pwd));
			write_bool(WIFI_MODE, 1);
			//wifiMode =  wifiMmode_STR[0]; 
			FS_wifi_write(0);



			debugMe("Setting ssid to ", false);
			 debugMe(wifi_cfg.ssid);

			debugMe("Setting password to ", false);
			debugMe(wifi_cfg.pwd);

			debugMe("Setting wifi_mode to ", false);
			debugMe(get_bool(WIFI_MODE));
			

		}

		if (httpd.hasArg("APname") && httpd.hasArg("wifiMode")) {
			String APname_STR = httpd.arg("APname");
			String wifiMmode_STR = httpd.arg("wifiMode");

			APname_STR.toCharArray(wifi_cfg.APname, 32);
			write_bool(WIFI_MODE, 0 );
			FS_wifi_write(0);

			debugMe("Setting APname to ", false);
			debugMe(wifi_cfg.APname);

			debugMe("Setting wifi_mode to ", false);
			debugMe(get_bool(WIFI_MODE));
			

		}

		if (httpd.hasArg("delete")) {
			String path = httpd.arg("delete");

			// path.toCharArray(ePassword,64);    
			SPIFFS.remove(path);

			debugMe("requested delte : ", false);
			debugMe(path);
			

		}
	}

}


void httpd_handleRequestSettings() 
{
	//String  output_bufferZ = "-" ;

	httpd.on("/wifiMode", []() { httpd.send(200, "text/plain", String(get_bool(WIFI_MODE)));   });
	httpd.on("/ssid", HTTP_GET, []() { httpd.send(200, "text/plain", wifi_cfg.ssid);  });
	httpd.on("/password", HTTP_GET, []() { httpd.send(200, "text/plain", wifi_cfg.pwd);   });
	httpd.on("/APname", HTTP_GET, []() { httpd.send(200, "text/plain", wifi_cfg.APname);   });
	httpd.on("/reset", HTTP_GET, []() { httpd.send(200, "text/plain", "Rebooting"); ESP.restart();   });
	



}


void httpd_toggle_webserver()
{
	if (get_bool(HTTP_ENABLED) == true)
	{
		httpd.stop();
		write_bool(HTTP_ENABLED, false);
		 debugMe("httpd turned off");
	}
	else
	{
		httpd.begin();
		write_bool(HTTP_ENABLED, true);
		debugMe("httpd turned on");
#ifdef ESP8266

		httpUpdater.setup(&httpd);
#endif		
		// debugMe("HTTP server started");
		MDNS.begin(wifi_cfg.APname);
		MDNS.addService("http", "tcp", 80);
	}


}


void httpd_setup()
{
	debugMe("HTTPd_setup");
	// Setup Handlers
	httpd.on("/list", HTTP_GET, httpd_handleFileList);
	//load editor
	httpd.on("/edit", HTTP_GET, []() { if (!httpd_handleFileRead("/edit.html")) httpd.send(404, "text/plain", "edit_FileNotFound"); });
	httpd.on("/edit", HTTP_DELETE, httpd_handleFileDelete);
	httpd.on("/edit", HTTP_POST, []() { httpd.send(200, "text/plain", ""); }, httpd_handleFileUpload);

	httpd.onNotFound([]() {if (!httpd_handleFileRead(httpd.uri()))  httpd.send(404, "text/plain", "FileNotFound im sorry check in the next 2'n dimension on the left"); });

	//get heap status, analog input value and all GPIO statuses in one json call
#ifdef ESP8266
	httpd.on("/all", HTTP_GET, []() {
		String json = "{";
		json += "\"heap\":" + String(ESP.getFreeHeap());
		json += ", \"analog\":" + String(analogRead(A0));
		json += ", \"gpio\":" + String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
		json += "}";
		httpd.send(200, "text/json", json);
		json = String();
	});
#endif
	// end FS handlers
	//httpd.serveStatic("/index.html", SPIFFS, "/index.html");  
	//httpd.on( "/set", handle_default_args ); 

	httpd.on("/settings.html", []() {   httpd_handleFileRead("/settings.html");	      httpd_handle_default_args();   });


	httpd.on("/index.html", []() {
#ifdef ARTNET_ENABLED
		if (get_bool(ARTNET_ENABLE) == true)
			handleFileRead("/artnet.html");
		else
#endif
		httpd_handleFileRead("/index.html");
		httpd_handle_default_args();
	});
	//httpd.on("/all", HTTP_GET, [](){

	httpd_handleRequestSettings();

	if (get_bool(HTTP_ENABLED) == true)
	{
		httpd.begin();					// Switch on the HTTP Server
#ifdef ESP8266
		httpUpdater.setup(&httpd);
		 debugMe("HTTP server started");
#endif
		 MDNS.begin(wifi_cfg.APname);
		MDNS.addService("http", "tcp", 80);
		debugMe("Starting HTTP");
	}
}

void http_loop()
{
	if (get_bool(HTTP_ENABLED) == true)
		httpd.handleClient();
}




