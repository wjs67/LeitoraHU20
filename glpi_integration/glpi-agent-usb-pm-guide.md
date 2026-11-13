# GLPI Agent Custom USB Module Deployment Guide (`USB.pm`)

## 1. Overview & Architectural Context

This guide outlines the complete operational procedure for deploying and verifying the customized **`USB.pm`** inventory module for the **GLPI Agent** on Microsoft Windows endpoints. 

Standard GLPI Agent inventories retrieve basic USB device metadata from Windows Management Instrumentation (WMI) via the `CIM_LogicalDevice` class. However, biometric USB hardware—specifically the **SecuGen HU20 Biometric Reader** (`VendorID: 1162`, `ProductID: 2200`)—frequently reports synthetic pseudo-serials containing ampersand characters (`&`) or missing serial numbers altogether.

To overcome the limitations of running hardware-level `.exe` binaries under the `NT AUTHORITY\SYSTEM` service context, the customized `USB.pm` module implements a high-reliability, 3-tier hybrid Registry lookup strategy.

### Key Features of the Custom Module
* **Early WMI Interception**: Serial numbers are injected inside `_getDevicesFromWMI` prior to object instantiation, ensuring clean single-pass reconciliation on first-time computer inventory.
* **Resilient Registry Reading**: Queries `HKLM\SOFTWARE\CustomInventory\HU20\SerialNumber` using native GLPI Agent tools (`GLPI::Agent::Tools::Win32`), safe `Win32::TieRegistry` evaluation, and `reg query` fallback (handling 64-bit and `WOW6432Node` paths).
* **Strict Validation & Fallback**: Validates retrieved serials against `/^[A-Za-z0-9]{5,}$/` and defaults to `H99999999999` if no valid Registry key exists.
* **Dual-Target Schema Reporting**: Supports reporting via both `<USBDEVICES>` (internal hardware components) and `<PERIPHERALS>` (standalone manageable assets with explicit `<MODEL>` tags).

---

## 2. Prerequisites & Requirements

Before deploying the custom module across target computers, ensure the following conditions are met:

| Requirement | Details |
| :--- | :--- |
| **Operating System** | Microsoft Windows 10 / 11 / Server 2016+ (x64 or x86) |
| **GLPI Agent Version** | GLPI Agent v1.0 or higher installed at `C:\Program Files\GLPI-Agent` |
| **Permissions** | Administrator privileges (PowerShell / Command Prompt) |
| **Registry Key** | Serial stored at `HKLM\SOFTWARE\CustomInventory\HU20\SerialNumber` |
| **GLPI Server Dropdown** | Manufacturer `SecuGen` pre-created in GLPI Server dropdowns (recommended) |

---

## 3. Step-by-Step Deployment Procedure

Follow these steps on each target Windows endpoint or automate them using a central deployment tool (e.g., Active Directory GPO, Microsoft Intune, or Ansible).

### Step 1: Backup the Original `USB.pm` File

Open PowerShell as Administrator and locate the GLPI Agent Perl library directory:

```powershell
$AgentPath = "C:\Program Files\GLPI-Agent\perl\agent\GLPI\Agent\Task\Inventory\Win32"
Copy-Item -Path "$AgentPath\USB.pm" -Destination "$AgentPath\USB.pm.bak" -Force
```

### Step 2: Deploy the Customized `USB.pm` File

Download [https://github.com/wjs67/LeitoraHU20/blob/main/glpi_integration/USB_update.pm USB_update.pm] into the GLPI Agent inventory task directory:

```powershell
Save to "C:\Program Files\GLPI-Agent\perl\agent\GLPI\Agent\Task\Inventory\Win32\USB.pm"
```

### Step 3: Verify Registry Serial Number Storage

Ensure that the target machine has its physical SecuGen HU20 serial number written to the Windows Registry. You can test and populate it via PowerShell:

```powershell
# Confirm registry value
Get-ItemProperty -Path "HKLM:\SOFTWARE\CustomInventory\HU20" -Name "SerialNumber"
```

### Step 4: Restart the GLPI Agent Service

Restart the Windows service to ensure Perl reloads the modified module from disk:

```powershell
Restart-Service -Name "glpi-agent" -WarningAction SilentlyContinue
```

### Step 5: Execute Forced Inventory Transmission

Force an immediate inventory scan and upload to the configured GLPI server:

```powershell
& "C:\Program Files\GLPI-Agent\glpi-agent.bat" --force
```

---

## 4. Local Inspection & Offline Testing

To verify what data the custom `USB.pm` module generates without sending data over the network to the GLPI server, run a local XML generation command.

### Local XML Export Command

```powershell
# Generate inventory XML to C:\Temp\inventory_debug.xml
& "C:\Program Files\GLPI-Agent\glpi-agent.bat" --local C:\Temp\inventory_debug.xml
```

### Inspecting XML Payload Content

Open `C:\Temp\inventory_debug.xml` in a text editor or via PowerShell to confirm the output structure:

```powershell
# Search XML for SecuGen USB Device entries
Select-String -Path "C:\Temp\inventory_debug.xml" -Pattern "1162", "2200", "H54200804821" -Context 2,5
```

#### Expected `<USBDEVICES>` XML Block

```xml
<USBDEVICES>
  <CAPTION>SecuGen USB Fingerprint Device</CAPTION>
  <NAME>SecuGen U20 USB FRD HU20</NAME>
  <VENDORID>1162</VENDORID>
  <PRODUCTID>2200</PRODUCTID>
  <SERIAL>H54200804821</SERIAL>
  <MANUFACTURER>SecuGen Corp.</MANUFACTURER>
</USBDEVICES>
```

## 5. GLPI Server Behavior & Component Reconciliation

Understanding how GLPI Server processes incoming inventory payloads avoids common pitfalls during initial onboarding:

1. **First-Time Computer Onboarding**:
   When a machine is inventoriated for the first time after being deleted or created anew, GLPI Server first creates the core Computer asset (`glpi_computers`), then seeds vendor/model dictionaries (`glpi_usbvendors`, `glpi_deviceusbs`), and finally connects the components. 
2. **Avoiding Duplicate Entries**:
   GLPI reconciles USB devices by matching **`Name` + `VendorID` + `ProductID`**. Modifying the device `<NAME>` string (e.g., adding dynamic suffixes) causes GLPI to treat the device as a new hardware piece, leading to duplicate entries. The updated `USB.pm` preserves the native WMI name string to ensure seamless single-entry updating.

---

## 6. Troubleshooting Guide

| Issue / Symptom | Possible Cause | Resolution |
| :--- | :--- | :--- |
| **Serial shows `H99999999999`** | Registry path `HKLM\SOFTWARE\CustomInventory\HU20\SerialNumber` does not exist or has fewer than 5 characters. | Verify Registry path and ensure value meets `/^[A-Za-z0-9]{5,}$/` pattern. |
| **Unknown configuration directive warning** | Obsolete directive like `full-biometrics` left in `agent.cfg` or in windows registry. | Remove or comment out the line in `C:\ProgramData\glpi-agent\agent.cfg` or remove in windows registry with command:  `Remove-ItemProperty -Path "HKLM:\SOFTWARE\GLPI-Agent" -Name "full-biometrics"`. |
| **Devices missing on 1st run** | GLPI Server completing initial dictionary seeding. | Run `glpi-agent.bat --force` a second time to complete relational joins. |
|
