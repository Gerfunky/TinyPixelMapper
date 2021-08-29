REM config Download Script for Tinypixelmapper.com
REM requres CURL


echo OFF
IF  "%~1" == "/?"  GOTO SYNTAX 
IF  "%~1" == ""  GOTO MISSINGIP 


echo **************************************************
echo DOWNLOADING all Configs 0-15 to %1
echo **************************************************

echo Downloading wifi.txt
curl "http://%1/conf/wifi.txt" > wifi.txt
echo Downloading mqtt.txt
curl "http://%1/conf/mqtt.txt" > mqtt.txt
echo Downloading 0.device.txt
curl "http://%1/conf/0.device.txt" > 0.device.txt
echo Downloading artnet.txt
curl "http://%1/conf/artnet.txt" > artnet.txt


GOTO DONEs



:SYNTAX
echo  Device Config Downloder for Tinypixelmapper
echo  TPM-DL.DevConf  IPADDRESS  				 			  = download (wifi.txt, mqtt.txt, 0.device.txt, artnet.txt)



GOTO DONEs

:MISSINGIP
ECHO IP not specified !   Add IP after command or /? for help


:DONEs
Echo Finished
