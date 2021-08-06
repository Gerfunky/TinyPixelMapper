/*********
  Based on : 
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/
#include "mmqt.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "tools.h"


// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.1.144";
//const char* mqtt_server = MMQT_BROKER ;

WiFiClient espClient;

long lastMsg = 0;
char msg[50];
int value = 0;
 

 mqtt_Struct mqtt_cfg = { 
        MMQT_DEF_USER,
        MMQT_DEF_PASSWORD,
        {172,16,222,19},
        MQTT_DEF_PORT,
        MQTT_DEF_PUBLISH_TIMEOUT
        };



// from leds.cpp
extern void LEDS_set_bri(uint8_t bri);
extern uint8_t LEDS_get_bri();
extern uint8_t LEDS_get_FPS_setting();
extern void LEDS_set_FPS(uint8_t fps_setting);

extern uint8_t LEDS_get_playNr(); 
extern	void LEDS_set_playNr(uint8_t setNr);


// from wifi
extern String WiFi_Get_Name();


// from FS
extern boolean FS_mqtt_read();
extern void FS_mqtt_write();

PubSubClient client(espClient);



void MMQT_publish()
{


    String deviceName = WiFi_Get_Name() ;

    String OutTopicString = deviceName + MMQT_TOPIC_FUNCTION_BRI + MMQT_TOPIC_SEND ;    client.publish(OutTopicString.c_str(), String(LEDS_get_bri()).c_str()  );  
           OutTopicString = deviceName + MMQT_TOPIC_FUNCTION_FPS + MMQT_TOPIC_SEND ;   client.publish(OutTopicString.c_str(), String(LEDS_get_FPS_setting()).c_str() );

           OutTopicString = deviceName + MMQT_TOPIC_FUNCTION_PLAY + MMQT_TOPIC_SEND ;   client.publish(OutTopicString.c_str(), String(LEDS_get_playNr()).c_str() );  

           OutTopicString = deviceName + MMQT_TOPIC_FUNCTION_DEVICE + MMQT_TOPIC_SEND ; if (LEDS_get_bri() != 0) client.publish(OutTopicString.c_str(), "on") ; else client.publish(OutTopicString.c_str(), "off");  
           OutTopicString = deviceName + MMQT_TOPIC_FUNCTION_TEMP + MMQT_TOPIC_SEND ;   client.publish(OutTopicString.c_str(), String(temperatureRead()).c_str() );  


}


void callback(char* topic, byte* message, unsigned int length) {
  debugMe("Message arrived on topic: ");
  debugMe(topic);
  
   String messageTemp;
  
  for (int i = 0; i < length; i++) {
   // debugMe((char)message[i]);
    messageTemp += (char)message[i];
  }
   debugMe("Message: ", false); debugMe(messageTemp);
   

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
    String deviceName = WiFi_Get_Name() ;
    String MMQTopicDeviceStatusSet = deviceName + MMQT_TOPIC_FUNCTION_DEVICE + MMQT_TOPIC_RECIVE ;
    String MMQTopicFPSStatusSet = deviceName + MMQT_TOPIC_FUNCTION_FPS + MMQT_TOPIC_RECIVE ;
    String MMQTopicBrightnessControll =  deviceName + MMQT_TOPIC_FUNCTION_BRI + MMQT_TOPIC_RECIVE ;  
    String MMQTopicPlaySet =  deviceName + MMQT_TOPIC_FUNCTION_PLAY + MMQT_TOPIC_RECIVE ; 



 if (String(topic) == MMQTopicDeviceStatusSet) 
    {
        debugMe("Changing state ");
        
        
         if (message[0] == 'o' &&  message[1] == 'n' )
            { 
                if (LEDS_get_bri() == 0 ) LEDS_set_bri(255);
                debugMe("mqtt on");
                MMQT_publish();
            }
        else if (message[0] == 'o' &&  message[1] == 'f' )
            { 
                LEDS_set_bri(0);
                debugMe("mqtt off");
                MMQT_publish();
            }

        }
else if (String(topic) == MMQTopicBrightnessControll) 
  {
      uint8_t inval = messageTemp.toInt();
      debugMe("Changing BRI to ", false); debugMe(inval);
      LEDS_set_bri(inval);
     

    }
else if (String(topic) == MMQTopicPlaySet) 
  {
    uint8_t inval = messageTemp.toInt()   ; //short_msg.toInt();
    debugMe("Changing play to ", false); debugMe(inval);
    LEDS_set_playNr(inval);


    }
else if (String(topic) == MMQTopicFPSStatusSet) 
  {
    
        
        //String short_msg = String(message[0]+message[1]);
        uint8_t inval = messageTemp.toInt()   ; //short_msg.toInt();
      
      debugMe("Changing fps to ", false); debugMe(inval);
      LEDS_set_FPS(inval);

    }
   else
   {
     { debugMe("unknown mqtt topic : ", false) ;  debugMe(topic) ;
    
     }
   }
    
  
}
void MMQT_subscribe()
{
    debugMe("subscribing");
    
    String deviceName = WiFi_Get_Name() ;

    String INTopicString = deviceName + MMQT_TOPIC_FUNCTION_BRI + MMQT_TOPIC_RECIVE ;    client.subscribe(INTopicString.c_str());
           INTopicString = deviceName + MMQT_TOPIC_FUNCTION_FPS + MMQT_TOPIC_RECIVE ;    client.subscribe(INTopicString.c_str());
           INTopicString = deviceName + MMQT_TOPIC_FUNCTION_PLAY + MMQT_TOPIC_RECIVE ;   client.subscribe(INTopicString.c_str());
           INTopicString = deviceName + MMQT_TOPIC_FUNCTION_DEVICE + MMQT_TOPIC_RECIVE ; client.subscribe(INTopicString.c_str());


    
}

 

boolean reconnect() {
  
  if (client.connect(WiFi_Get_Name().c_str() , mqtt_cfg.username, mqtt_cfg.password ))
  {
      debugMe(client.state( ));
      debugMe("MMQT connecting");
    // Once connected, publish an announcement...
    MMQT_publish();
    // ... and resubscribe
    MMQT_subscribe();
  }
  debugMe(client.state( ));
  return client.connected();
}




void MMQT_loop()   
{

    if (get_bool(MQTT_ON) == true)
    {
        long now = millis();
        if (now - lastMsg > (mqtt_cfg.publishSec * 1000  )) 
        {
            lastMsg = now;
            if (!client.connected()) 
            {
                debugMe("MMQT Not connected");
                if (reconnect()) 
                {
                    lastMsg = 0;
                }
                
            } else 
            {
                debugMe("MMQT Connected");
                MMQT_publish();

                
            }

        
        }

    client.loop();
    }
}







void MMQT_setup() {

  if (FS_mqtt_read() == false )   FS_mqtt_write();

   client.setServer(mqtt_cfg.mqttIP, mqtt_cfg.mqttPort);
   client.setCallback(callback);
    lastMsg = 0;


  if (get_bool(MQTT_ON) == true) 
  {
    reconnect();
  }
}