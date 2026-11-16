# Integration Guide: SecuGen HU20 Biometric Reader with GLPI Agent

## 1. Overview
This document specifies the procedure for extracting the serial number from the SecuGen HU20 biometric reader via a C++ utility and making it available for automatic collection by the **GLPI Agent** through the Windows Registry (`HKLM`).

---

## 2. PowerShell Capture Script (`Update-HU20Serial.ps1`)

save `Update-HU20Serial.ps1` -> `C:\Program Files\LeitoraHU20`

---

## 3. Automation via Windows Task Scheduler

Run the command below in PowerShell (as Administrator) to schedule automatic execution of the script at system startup (`AtStartup`) with elevated privileges (`NT AUTHORITY\SYSTEM`):

```powershell
$Trigger = New-ScheduledTaskTrigger -AtStartup
$User    = "NT AUTHORITY\SYSTEM"
$Action  = New-ScheduledTaskAction -Execute "PowerShell.exe" -Argument "-ExecutionPolicy Bypass -WindowStyle Hidden -File C:\Program Files\LeitoraHU20\Update-HU20Serial.ps1"

Register-ScheduledTask -TaskName "GLPI_HU20_SerialUpdate" -Trigger $Trigger -User $User -Action $Action -RunLevel Highest -Force
```

---

## 4. Change USB.pm

See [glpi-agent-usb-pm-guide.md(https://github.com/wjs67/LeitoraHU20/blob/main/glpi_integration/glpi-agent-usb-pm-guide.md)
  
