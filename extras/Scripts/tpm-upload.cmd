REM config Upload Script for Tinypixelmapper.com
REM requres CURL




echo OFF
IF  "%~1" == "/?"  GOTO SYNTAX 
IF  "%~1" == ""  GOTO MISSINGIP 


if  "%~2" == ""  GOTO DOALL
IF  "%~3" == ""  GOTO DOONE


GOTO DOSOME

goto DONEs

:DOALL
echo **************************************************
echo Uploading all Configs 0-15 to %1
echo **************************************************
FOR /L %%c IN (0 1 15) DO  (
echo Uploading %%c.playConf.txt
curl -F "file=@%%c.playConf.txt;filename=conf/%%c.playConf.txt"  "http://%1/edit"
)

GOTO DONEs

:DOONE
echo **************************************************
echo Uploading Config %2 to %1
echo **************************************************
echo Uploading %2.playConf.txt
curl -F "file=@%2.playConf.txt;filename=conf/%2.playConf.txt"  "http://%1/edit"



GOTO DONEs

:DOSOME
echo **************************************************
echo Uploading Configs %2 - %3 to %1
echo **************************************************

FOR /L %%c IN (%2 1 %3) DO  (
echo Uploading %%c.playConf.txt
curl -F "file=@%%c.playConf.txt;filename=conf/%%c.playConf.txt"  "http://%1/edit"
)
GOTO DONEs



:SYNTAX
echo  tpm-upload  IPADDRESS		= upload all 0-15
echo  tpm-upload  IPADDRESS ConfNR 	 = upload one  specific config  
echo  tpm-upload  IPADDRESS ConfNRStart ConfNrEnd 	= upload a batch of configs  


GOTO DONEs

:MISSINGIP
ECHO IP not specified !   Add IP after command or /? for help


:DONEs
Echo Finished





