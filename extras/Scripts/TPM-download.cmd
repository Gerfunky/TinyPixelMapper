REM config Download Script for Tinypixelmapper.com
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
echo DOWNLOADING all Configs 0-15 to %1
echo **************************************************
FOR /L %%c IN (0 1 15) DO  (
echo Downloading %%c.playConf.txt
curl "http://%1/conf/%%c.playConf.txt" > %%c.playConf.txt
)

GOTO DONEs

:DOONE
echo **************************************************
echo DOWNLOADING Config %2 to %1
echo **************************************************
echo Downloading %2.playConf.txt
curl "http://%1/conf/%2.playConf.txt" > %2.playConf.txt



GOTO DONEs

:DOSOME
echo **************************************************
echo DOWNLOADING Configs %2 - %3 to %1
echo **************************************************

FOR /L %%c IN (%2 1 %3) DO  (
echo Downloading %%c.playConf.txt
curl "http://%1/conf/%%c.playConf.txt" > %%c.playConf.txt
)
GOTO DONEs



:SYNTAX
echo  tpm-download  IPADDRESS  				 			  = download all 0-15
echo  tpm-download  IPADDRESS ConfNR        			   = download one specific config  
echo  tpm-download  IPADDRESS ConfNRStart ConfNrEnd        = download a batch of configs  


GOTO DONEs

:MISSINGIP
ECHO IP not specified !   Add IP after command or /? for help


:DONEs
Echo Finished
