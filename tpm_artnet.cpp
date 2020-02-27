
// artnet
	#include "config_TPM.h"
    #include "tpm_artnet.h"
    #include "config_fs.h"
    #include "tools.h"
    #include "leds.h"
    #include <ArtnetWifi.h>
    
    





// Artnet

	ArtnetWifi artnet;		// make the Artnet server


	artnet_struct artnet_cfg = { DEF_ARTNET_STAT_UNIVERSE, DEF_ARTNET_NUMBER_OF_UNIVERSES };



    artnet_node_struct artnetNode[ARTNET_NR_NODES_TPM] = {{DEF_ARTNET_STAT_UNIVERSE,DEF_ARTNET_NUMBER_OF_UNIVERSES,{172,16,222,20},36},
                                                     {DEF_ARTNET_STAT_UNIVERSE,DEF_ARTNET_NUMBER_OF_UNIVERSES,{172,16,222,22},36},
                                                     {DEF_ARTNET_STAT_UNIVERSE,DEF_ARTNET_NUMBER_OF_UNIVERSES,{172,16,222,23},36},
                                                     {DEF_ARTNET_STAT_UNIVERSE,DEF_ARTNET_NUMBER_OF_UNIVERSES,{172,16,222,21},36}   
                                                     };







    void ARTNET_set_node(uint8_t node_Nr, uint8_t universe  )
    {
            artnet.setUniverse(universe);
            artnet.setLength(510); // 510 max


    }

    void ARTNET_send_node(uint8_t node_Nr)
    {


            artnet.write(artnetNode[node_Nr].IP);


    }

    void ARNET_set_pixel(uint16_t pixel, uint8_t redColor, uint8_t greenColor, uint8_t blueColor)
    {

        uint16_t startChannel = 0;      //
        startChannel = constrain(pixel * 3, 0, 510);  


        artnet.setByte(startChannel,        redColor);
        artnet.setByte(startChannel +1,     greenColor);
        artnet.setByte(startChannel +2,     blueColor);

    }




    void ARTNET_set_channel(uint16_t channel, uint8_t value)
    {

        artnet.setByte(channel,   value);

    }




	void WiFi_artnet_recive_loop()
	{
		// the main artnet loop  calback set to leds function with show
		artnet.read();


	
		

	}

	void WiFi_artnet_Load_Vars()
	{
		// configure the Artnet vaiables 
		// from disk or load the defaults.

		if (!FS_artnet_read())
		{
			write_bool(ARTNET_SEND, DEF_ARTNET_SEND_ENABLE);
            write_bool(ARTNET_RECIVE, DEF_ARTNET_RECIVE_ENABLE);
			artnet_cfg.startU = DEF_ARTNET_STAT_UNIVERSE;
			artnet_cfg.numU = DEF_ARTNET_NUMBER_OF_UNIVERSES;

		}

	}


 void WiFi_artnet_rc_enable()
 {
    

     if(get_bool(ARTNET_RECIVE)) 
		//artnet.begin(artnet_mac, 0);   // mac and ip setting useless since were setting ip for the esp8266 
		artnet.setArtDmxCallback(LEDS_artnet_in);  // function in leds with show


 }



	void WiFi_artnet_enable() 
	{
		// enable artnet, This is a exclusice mode other settings dont apply
		WiFi_artnet_Load_Vars();



		// byte loc_ip = WiFi.localIP();
		// debugMe("enable artnet");
		//byte artnet_mac[] = DEF_ARTNET_MAC ;
		artnet.begin(	);

		if(get_bool(ARTNET_RECIVE)) 
		//artnet.begin(artnet_mac, 0);   // mac and ip setting useless since were setting ip for the esp8266 
		artnet.setArtDmxCallback(LEDS_artnet_in);  // function in leds with show
        

	}


	/*void WiFi_artnet_setup()
	{

		//the Artnet setup 
		if (get_bool(ARTNET_ENABLE)== true)	WiFi_artnet_enable();


	}  */



// basic WiFi
