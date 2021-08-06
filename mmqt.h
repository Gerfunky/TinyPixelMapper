#ifndef _MMQT_h
#define _MMQT_h


#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#endif

#include "config_TPM.h"	
/*
            Home Assistant config
 configuration.yaml

light:
  - platform: mqtt
    name: "WZ gate"
    state_topic: "TinyPixelMapperWZ/device/status"
    command_topic: "TinyPixelMapperWZ/device/set"
    #brightness_state_topic: 'TinyPixelMapperWZ/bri/status'
    #brightness_command_topic: 'TinyPixelMapperWZ/bri/set'
    qos: 0
    payload_on: "on"
    payload_off: "off"
    optimistic: false

input_select:
  tinxpixelmapper:
    name: TinxPixelMapper
    options:
      - '0 Startup'
      - '1'
      - '2'
      - '3'

input_number:
  tinypixelmapper_fps:
    name: TinyPixelMapper FPS
    min: 1
    max: 90
    step: 1
    unit_of_measurement: fps
    icon: mdi:target
  tinypixelmapper_bri:
    name: TinyPixelMapper Bri
    min: 0
    max: 255
    step: 1
    #unit_of_measurement: fps
    #icon: mdi:target



automation.yaml

- id: tpm_play_select
  alias: Set Play mode
  description: ''
  trigger:
  - entity_id: input_select.tinxpixelmapper
    platform: state
  condition: []
  action:
    service: mqtt.publish
    data:
      topic: TinyPixelMapperWZ/play/set
      payload_template: "{% if is_state(\"input_select.TinxPixelmapper\", \"0 Startup\"\
        ) %}\n  0          \n{% elif is_state(\"input_select.TinxPixelmapper\", \"\
        1\") %}\n  1\n{% elif is_state(\"input_select.TinxPixelmapper\", \"2\") %}\n\
        \  2\n{% elif is_state(\"input_select.TinxPixelmapper\", \"3\") %}\n  3\n\
        {% endif %}"
- id: tpm_get_fps
  alias: Set FPS slider
  trigger:
    platform: mqtt
    topic: TinyPixelMapperWZ/fps/status
  action:
    service: input_number.set_value
    data_template:
      entity_id: input_number.tinypixelmapper_fps
      value: '{{ trigger.payload }}'
- id: tpm_set_fps
  alias: FPS slider moved
  trigger:
  - entity_id: input_number.tinypixelmapper_fps
    platform: state
  action:
  - data:
      payload_template: '{{ states(''input_number.tinypixelmapper_fps'') | int }}'
      topic: TinyPixelMapperWZ/fps/set
    service: mqtt.publish

- id: tpm_get_bri
  alias: Set bri slider
  trigger:
    platform: mqtt
    topic: TinyPixelMapperWZ/bri/status
  action:
    service: input_number.set_value
    data_template:
      entity_id: input_number.tinypixelmapper_bri
      value: '{{ trigger.payload }}'
- id: tpm_set_bri
  alias: bri slider moved
  trigger:
  - entity_id: input_number.tinypixelmapper_bri
    platform: state
  action:
  - data:
      payload_template: '{{ states(''input_number.tinypixelmapper_bri'') | int }}'
      topic: TinyPixelMapperWZ/bri/set
    service: mqtt.publish




*/

// MMQT Topics
// 
// The topics are automatically generated
// deviceName/Function/action
//
// device:  APname / deviceName
// Action: /set = subsribe topic to set something in the TPM
//         /status = publish the status of the Function 
// Functions : /device = Is it on or off
//              /bri   = Master fader
//              /play  = Play nummer / config
//              /fps   = Speed
//



// try 2 

#define MMQT_TOPIC_SEND "/status"
#define MMQT_TOPIC_RECIVE "/set"

#define MMQT_TOPIC_FUNCTION_PLAY    "/play"
#define MMQT_TOPIC_FUNCTION_BRI     "/bri"
#define MMQT_TOPIC_FUNCTION_DEVICE  "/device"
#define MMQT_TOPIC_FUNCTION_FPS     "/fps"
#define MMQT_TOPIC_FUNCTION_TEMP    "/temp"

//#define MMQT_DEF_NAME           "TPM-WZ"
#define MMQT_DEF_USER          "lights"
#define MMQT_DEF_PASSWORD      "lights123"
#define MQTT_DEF_PORT           1883
#define MQTT_DEF_PUBLISH_TIMEOUT   60 // in seconds
//#define MMQT_ENABLED  false


void MMQT_setup();
void MMQT_loop()  ;

struct MMQT_config_struct
{
    boolean   enabled;

};


struct mqtt_Struct 
	{           
        char		username[16];		// The mqtt username
        char		password[16];		// The mqtt password
        uint8_t     mqttIP[4];            // mqtt ip address
        //uint8_t     mqttIP2;
        //uint8_t     mqttIP3;
        //uint8_t     mqttIP4;
        uint16_t  mqttPort;              // mqtt port
        uint16_t publishSec;

    };




#endif 