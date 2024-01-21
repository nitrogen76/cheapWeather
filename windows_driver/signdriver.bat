@echo off
setlocal


SET AC="c:\certs\godaddy.crt"
SET TS_SERVER="http://tsa.starfieldtech.com"
SET CERTNAME="Dracal"

inf2cat /v /driver:.\ /os:2000,XP_X86,XP_X64,Vista_X86,Vista_X64,7_X86,7_X64
SignTool sign /v /ac %AC% /n %CERTNAME% /t %TS_SERVER% usbtenki.cat
