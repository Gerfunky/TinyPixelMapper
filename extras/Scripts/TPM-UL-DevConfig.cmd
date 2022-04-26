REM config Upload Script for Tinypixelmapper.com
REM requres CURL




echo OFF
IF  "%~1" == "/?"  GOTO SYNTAX 
IF  "%~1" == ""  GOTO MISSINGIP 



echo **************************************************
echo Uploading all Device Configs  to %1
echo **************************************************

echo Uploading wifi.txt
curl -F "file=@wifi.txt;filename=conf/wifi.txt"  "http://%1/edit"
echo Uploading mqtt.txt
curl -F "file=@mqtt.txt;filename=conf/mqtt.txt"  "http://%1/edit"
echo Uploading artnet.txt
curl -F "file=@artnet.txt;filename=conf/artnet.txt"  "http://%1/edit"
echo Uploading 0.device.txt
curl -F "file=@0.device.txt;filename=conf/0.device.txt"  "http://%1/edit"

echo **************************************************
echo Uploading Lamp Configs 0-15 if available to %1
echo **************************************************

FOR /L %%c IN (0 1 15) DO  (
IF EXIST %%c.LampConf.txt (
echo Uploading %%c.LampConf.txt
curl -F "file=@%%c.LampConf.txt;filename=conf/%%c.LampConf.txt"  "http://%1/edit"
)
)



GOTO DONEs



:SYNTAX
echo  Device Config Uploader for Tinypixelmapper
echo  tpm-upload  IPADDRESS		= upload (wifi.txt, mqtt.txt, 0.device.txt, artnet.txt)
 


GOTO DONEs

:MISSINGIP
ECHO IP not specified !   Add IP after command or /? for help


:DONEs
Echo Finished





