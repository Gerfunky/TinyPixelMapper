REM config Download Script for Tinypixelmapper.com
REM requres CURL


echo OFF
IF  "%~1" == "/?"  GOTO SYNTAX 
IF  "%~1" == ""  GOTO MISSINGIP 


echo **************************************************
echo DOWNLOADING all Configs 0-15 to %1
echo **************************************************

echo Downloading wifi.txt
curl  --fail "http://%1/conf/wifi.txt" -o wifi.txt
echo Downloading mqtt.txt
curl  --fail "http://%1/conf/mqtt.txt" -o mqtt.txt
echo Downloading 0.device.txt
curl  --fail "http://%1/conf/0.device.txt" -o 0.device.txt
echo Downloading artnet.txt
curl  --fail "http://%1/conf/artnet.txt" -o artnet.txt
FOR /L %%c IN (0 1 15) DO  (
echo Downloading %%c.LampConf.txt
curl --fail "http://%1/conf/%%c.LampConf.txt" -o %%c.LampConf.txt
)
echo Downloading lamps.txt
curl --fail "http://%1/conf/lamps.txt" -o lamps.txt

GOTO DONEs



:SYNTAX
echo  Device Config Downloder for Tinypixelmapper
echo  TPM-DL.DevConf  IPADDRESS  				 			  = download (wifi.txt, mqtt.txt, 0.device.txt, artnet.txt)



GOTO DONEs

:MISSINGIP
ECHO IP not specified !   Add IP after command or /? for help


:DONEs
Echo Finished
