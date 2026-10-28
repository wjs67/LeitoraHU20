# HU20 Reader - Biometric Serial Reader (Native C++)


## 👤 Author

Wellington Silva © 2026 - swj67@protonmail.com

---
## 📝 License

See the `LICENSE` file for details.

## 📞 Support

For issues with the Secugen SDK, consult the official documentation or contact Secugen support.


## 📋 Description

This project reads the serial number of the **HU20** digital biometric reader from Secugen connected via USB.

**Version:** 1.0.1 (Native C++)  
**Compatibility:** Windows 7 SP1, 8, 10 and 11 (x64)  
**Runtime Dependencies:** None ✅

Note: This executable was developed exclusively to provide information regarding the serial number of the SecuGen - HU20 digital reader, the value of which will be passed to the IT asset management agent - GLPI Agent (https://github.com/glpi-project/glpi-agent).

Without this process, the inventory of this device on the GLPI server will show the USB port to which the reader is connected and not its serial number.

## 🎯 Requirements

### To Run
- **Windows 7 SP1** or later (any version up to Windows 11)
- **HU20 Reader connected via USB**
- **HU20 Driver installed** (provided by Secugen)



---

## 📦 Distribution Package Structure

To distribute the program, organize the files like this:

```
LeitoraHU20\
├── LeitoraHU20.exe              ← Compiled executable
├── sgfplib.dll                  ← Native DLL (REQUIRED)
├── sgbledev.dll                 ← Native DLL
├── sgfdusdax64.dll              ← Native DLL
├── sgfpamx.dll                  ← Native DLL
└── sgwsqlib.dll                 ← Native DLL
```

**Important:** All 5 DLLs must be in the same directory as the executable!


## 🚀 Installation for End User

1. **Install HU20 Driver** (provided by Secugen)
   - Connect the reader to USB
   - Run the driver installer
   - Restart if prompted

2. **Extract the ZIP File**
   - Extract LeitoraHU20_exe_v`X.X.X`.zip in any folder (example: `C:\Programs\LeitoraHU20`)

3. **Run the program in Windows PowerShell**
   - .\ `LeitoraHU20.exe`
   - The serial number will be displayed

---

### Backward Compatibility

The C++ executable is 100% compatible with:
- ✅ Windows 7 SP1
- ✅ Windows 8 / 8.1
- ✅ Windows 10 (all versions)
- ✅ Windows 11

---

## 🐛 Troubleshooting

### "HU20 Device not found"
- Verify that the reader is connected to USB
- Verify that the reader's LED is on
- Reinstall the HU20 driver

### "Cannot open sgfplib.dll"
- Make sure all 5 DLLs are in the same directory as the .exe
- Do not remove or rename the DLLs

---

## ⚠️ Understanding "ERR: Can't open the key..." Messages

### What Are These Messages?

When you run `LeitoraHU20.exe`, you may see output like:

```
ERR: Can't open the key(2) {Hardware}\DEVICEMAP\SERIALCOMM\COM1
ERR: Can't open the key(2) {some_registry_path}
H54200804821
```

### Why Do They Appear?

These messages are **diagnostic output from the Secugen SDK library (sgfplib.dll)** during initialization. The SDK attempts to read optional Windows Registry settings to detect connected serial devices and configure hardware parameters. When these registry keys don't exist (which is normal), the SDK outputs these error messages.

**Important:** These messages are **NOT errors from this program** – they are informational messages from the SDK library itself.


None of these work because the messages originate from SDK initialization code before these redirections can take effect.

### Example Output

```
Expected output with messages (program still works perfectly):
ERR: Can't open the key(2) {Hardware}\DEVICEMAP\SERIALCOMM\COM1
ERR: Can't open the key(2) {Registry path}
H54200804821
[Exit code: 0 - SUCCESS]

This is equivalent to:
H54200804821
[Exit code: 0 - SUCCESS]
```

The "ERR" messages are cosmetic output from the SDK and do not affect the result.

---



