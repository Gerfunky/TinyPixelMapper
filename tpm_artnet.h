



#ifndef _TPM_ARTNET_h
#define _TPM_ARTNET_h
	//#ifndef ARTNET_DISABLED 
    #if defined(ARDUINO) && ARDUINO >= 100
        #include "arduino.h"
    #endif 
	
#include "IPAddress.h"


    
	#define ARTNET_NR_NODES_TPM  8
	#define ARTNET_NR_NODE_SETTINGS  6

	struct artnet_node_struct	// Arnet config Data stucture
	{
		byte	    startU;		// Arnet Start universe
		byte	    numU;		// Arnetnet Nunmber of universes
        IPAddress   IP;         // IPaddress of the node
        uint16_t    StartLed;   // where to start at in the led array         		
	};




	struct artnet_struct	// Arnet config Data stucture
	{
		byte	startU;		// Arnet Start universe
		byte	numU;		// Arnetnet Nunmber of universes
        		
	};




  // FUNCTIONS

	void WiFi_artnet_rc_enable();
	//void WiFi_artnet_setup();
	void WiFi_artnet_recive_loop();
    void WiFi_artnet_enable() ;

    void ARNET_set_pixel(uint16_t pixel, uint8_t redColor, uint8_t greenColor, uint8_t blueColor);      // Set one pixel in the universe (max = 170)
    void ARTNET_set_node(uint8_t node_Nr, uint8_t universe  ); // Set the OUT ip and universe
    void ARTNET_send_node(uint8_t node_Nr);                     // Send the data to the Artnet node that was set
    



#endif