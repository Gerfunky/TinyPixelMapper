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
const char* mqtt_server = MMQT_BROKER ;

WiFiClient espClient;

long lastMsg = 0;
char msg[50];
int value = 0;


/*
char MMQTopicBrightness[30];
char MMQTopicBrightnessControll[30];

char MMQTopicStatusSet[30];
char MMQTopicStatus[30];

char MMQTopicPlaySet[30];
char MMQTopicPlayStatus[30];
*/
//float temperature = 0;
//float humidity = 0;


// from leds.cpp
extern void LEDS_set_bri(uint8_t bri);
extern uint8_t LEDS_get_bri();

extern uint8_t LEDS_get_playNr(); 
extern	void LEDS_set_playNr(uint8_t setNr);


// from wifi
extern String WiFi_Get_Name();

PubSubClient client(espClient);
//PubSubClient client(server, 1883, callback, ethClient);



void callback(char* topic, byte* message, unsigned int length) {
  debugMe("Message arrived on topic: ");
  debugMe(topic);
  
   String messageTemp;
  
  for (int i = 0; i < length; i++) {
   // debugMe((char)message[i]);
    messageTemp += (char)message[i];
  }
   debugMe(". Message: ");
   debugMe(messageTemp);

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
    String deviceName = WiFi_Get_Name() ;
    String MMQTopicDeviceStatusSet = deviceName + MMQT_TOPIC_FUNCTION_DEVICE + MMQT_TOPIC_RECIVE ;
    String MMQTopicBrightnessControll =  deviceName + MMQT_TOPIC_FUNCTION_BRI + MMQT_TOPIC_RECIVE ;  
    String MMQTopicPlaySet =  deviceName + MMQT_TOPIC_FUNCTION_PLAY + MMQT_TOPIC_RECIVE ; 



 if (String(topic) == MMQTopicDeviceStatusSet) 
    {
        debugMe("Changing state ");
        
        
         if (message[0] == 'o' &&  message[1] == 'n' )
            { 
                //LEDS_set_bri(255);
                debugMe("on");
            }
        else if (message[0] == 'o' &&  message[1] == 'f' )
            { 
                LEDS_set_bri(0);
                debugMe("off");
            }

        }
else 
  if (String(topic) == MMQTopicBrightnessControll) 
  {
    debugMe("Changing BRI to ");
        uint8_t inval = messageTemp.toInt();
      
      LEDS_set_bri(inval);
      debugMe(inval);

    }
else 
  if (String(topic) == MMQTopicPlaySet) 
  {
    debugMe("Changing play to ");
        
        String short_msg = String(message[0]+message[1]);
        uint8_t inval = short_msg.toInt();
      
     


      LEDS_set_playNr(inval);
      debugMe(inval);

    }
    
  
}
void MMQT_subscribe()
{
    debugMe("subscribing");
    
    String deviceName = WiFi_Get_Name() ;

    String INTopicString = deviceName + MMQT_TOPIC_FUNCTION_BRI + MMQT_TOPIC_RECIVE ;    client.subscribe(INTopicString.c_str());
           INTopicString = deviceName + MMQT_TOPIC_FUNCTION_PLAY + MMQT_TOPIC_RECIVE ;   client.subscribe(INTopicString.c_str());
           INTopicString = deviceName + MMQT_TOPIC_FUNCTION_DEVICE + MMQT_TOPIC_RECIVE ; client.subscribe(INTopicString.c_str());

    
    /*client.loop();
    client.subscribe(MMQTopicBrightnessControll);
    client.loop();
    client.subscribe(MMQTopicPlaySet);
    client.loop(); */
    
}

void MMQT_publish()
{
    //char  OutTopic[30];
    //memset(OutTopic, 0, sizeof(OutTopic));  

    String deviceName = WiFi_Get_Name() ;

    String OutTopicString = deviceName + MMQT_TOPIC_FUNCTION_BRI + MMQT_TOPIC_SEND ;    client.publish(OutTopicString.c_str(), String(LEDS_get_bri()).c_str()  );  

           OutTopicString = deviceName + MMQT_TOPIC_FUNCTION_PLAY + MMQT_TOPIC_SEND ;   client.publish(OutTopicString.c_str(), String(LEDS_get_playNr()).c_str() );  

           OutTopicString = deviceName + MMQT_TOPIC_FUNCTION_DEVICE + MMQT_TOPIC_SEND ; if (LEDS_get_bri() != 0) client.publish(OutTopicString.c_str(), "on") ; else client.publish(OutTopicString.c_str(), "off");  



    //client.publish(MMQTopicBrightness, String(LEDS_get_bri()).c_str());  
 //if (LEDS_get_bri() != 0)  client.publish(MMQTopicStatus, "on"); 
 //else  client.publish(MMQTopicStatus, "off");

 //client.publish(MMQTopicPlayStatus, String(LEDS_get_playNr()).c_str());  


}

 

boolean reconnect() {
  if (client.connect(MMQT_DEF_NAME, MMQT_DEF_USER, MMQT_DEF_PASSWORD ))
  {
      debugMe("MMQT connecting");
    // Once connected, publish an announcement...
    MMQT_publish();
    // ... and resubscribe
    MMQT_subscribe();
  }
  return client.connected();
}




void MMQT_loop()   
{

    if (MMQT_ENABLED == true)
    {
        long now = millis();
        if (now - lastMsg > 60000 ) 
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
/*
    // init the topics.
    String def_topic_bri = MMQT_DEF_BRI_TOPIC_SEND  ;
    String def_topic_bri_controll =  MMQT_DEF_BRI_TOPIC_RECIVE;

    String def_topic_set = MMQT_DEF_STATUS_TOPIC_RECIVE  ;
    String def_topic_status =  MMQT_DEF_STATUS_TOPIC_SEND ;

    String def_topic_play_set = MMQT_DEF_PLAY_TOPIC_RECIVE  ;
    String def_topic_play_status =  MMQT_DEF_PLAY_TOPIC_SEND ;

    memset(MMQTopicBrightness, 0, sizeof(MMQTopicBrightness));
    memset(MMQTopicBrightnessControll, 0, sizeof(MMQTopicBrightnessControll));

    memset(MMQTopicStatus, 0, sizeof(MMQTopicStatus));
    memset(MMQTopicStatusSet, 0, sizeof(MMQTopicStatusSet));

    memset(MMQTopicPlayStatus, 0, sizeof(MMQTopicPlayStatus));
    memset(MMQTopicPlaySet, 0, sizeof(MMQTopicPlaySet));
    
    def_topic_bri.toCharArray(MMQTopicBrightness, def_topic_bri.length() + 1);
    def_topic_bri_controll.toCharArray(MMQTopicBrightnessControll, def_topic_bri_controll.length() + 1);

    def_topic_set.toCharArray(MMQTopicStatusSet, def_topic_set.length() + 1);
    def_topic_status.toCharArray(MMQTopicStatus, def_topic_status.length() + 1);
    
    def_topic_play_set.toCharArray(MMQTopicPlaySet, def_topic_play_set.length() + 1);
    def_topic_play_status.toCharArray(MMQTopicPlayStatus, def_topic_play_status.length() + 1);
*/

  if (MMQT_ENABLED == true) 
  {
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
    lastMsg = 0;

    reconnect();
  }
}