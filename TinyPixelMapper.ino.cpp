# 1 "c:\\users\\ger\\appdata\\local\\temp\\tmpcbgkbc"
#include <Arduino.h>
# 1 "D:/Github/TPM-PIO/src/TinyPixelMapper.ino"
# 15 "D:/Github/TPM-PIO/src/TinyPixelMapper.ino"
#include "config_TPM.h"
#include "tools.h"
#include "config_fs.h"
#include "wifi-ota.h"
#include "leds.h"
#include "mmqt.h"
void setup();
void loop();
#line 23 "D:/Github/TPM-PIO/src/TinyPixelMapper.ino"
void setup()
{

  DEF_SERIAL_PORT.begin(DEF_SERIAL_SPEED);

 if (DEF_BOOT_DEBUGING == true)
 {
  write_bool(DEBUG_OUT, DEF_BOOT_DEBUGING);

  DEF_SERIAL_PORT.setDebugOutput(true);

  debugMe(debug_ResetReason(0));
  debugMe(debug_ResetReason(1));

  debugMe("Starting Setup - Light Fractal");
 }

 setup_controlls();
 FS_setup_SPIFFS();

 LEDS_setup();

 wifi_setup();

 MMQT_setup() ;

 debugMe("DONE Setup");


}

void loop()
{

 if (get_bool(WIFI_POWER_ON_BOOT)) wifi_loop();
 LEDS_loop();

 MMQT_loop();

}