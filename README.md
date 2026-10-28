# HU20 Reader - Compilation Instructions (Native C++)
## 👤 Author

Wellington Silva © 2026 - swj67@protonmail.com

---
## 📝 License

See the `LICENSE` file for details.

## 📞 Support

For issues with the Secugen SDK, consult the official documentation or contact Secugen support.
## 📋 Requirements

### Prerequisites
- **Visual Studio Build Tools** (https://aka.ms/vs/stable/vs_BuildTools.exe)
- **Windows 10/11** (or Windows 7/8 with updates)
- **Windows PowerShell** or **Command Prompt**

### Project Files
- `LeitoraHU20.cpp` (source code)
- `compile_direct.bat` (compilation script - included in this project)
- Native DLLs from SecuGen SDK (same directory as the script):
  - `sgfplib.dll`
  - `sgbledev.dll`
  - `sgfdusdax64.dll`
  - `sgfpamx.dll`
  - `sgwsqlib.dll`

---

## 🚀 Compilation Script (The Easy Way)

**The easiest way to compile!** Just run the script and it handles everything automatically.

### Advantages
✅ **Automatic Visual Studio Tools detection** - finds VS 2022 or 2019 automatically  
✅ **No CMake needed** - one-click compilation  
✅ **No manual configuration** - handles all compiler settings  
✅ **DLL verification** - checks if required DLLs are present  
✅ **Error feedback** - clear messages if something goes wrong  
✅ **Fastest option** - minimal setup time  

### How to Use

#### Step 1: Open Terminal in VS Code

1. Open your project folder in VS Code
2. Press `Ctrl + Backtick` (or go to Terminal → New Terminal)
3. A terminal window will open at the bottom of VS Code

**Alternative:** Open PowerShell or Command Prompt and navigate to the project directory

#### Step 2: Run the Script

```cmd
.\compile_direct.bat
```


#### Step 3: Wait for Compilation

The script will:
1. Locate Visual Studio Tools automatically
2. Set up the compiler environment
3. Compile the source code
4. Verify the DLLs are present
5. Display the result

#### ✅ Result

If successful, you'll see:
```
============================================================================
  COMPILATION COMPLETED SUCCESSFULLY!
============================================================================

Executable: C:\Path\To\Leitora HU20\LeitoraHU20.exe

Next steps:
   1. Make sure all 5 DLLs are in the same directory
   2. Run: LeitoraHU20.exe
   3. If it works, prepare for distribution as ZIP
```

The executable **LeitoraHU20.exe** will be created in the current directory.

#### ⚠️ Troubleshooting

**Error: "Visual Studio not found"**
- Install Visual Studio Build Tools 2022
- Download: https://visualstudio.microsoft.com/downloads/
- Select: "Desktop development with C++"
- Restart your computer

**Error: "LeitoraHU20.cpp not found"**
- Make sure you're running the script from the project root directory
- The script must be in the same directory as LeitoraHU20.cpp

**Warning: "DLL not found"**
- Copy the missing DLLs from the SecuGen SDK to the same directory as the script
- Run the script again

---

## ✅ Verify the Compilation

After compiling, verify that the `.exe` file was created successfully:

```cmd
dir LeitoraHU20.exe
```

---

## 🏃 Running the Application

### Normal Mode (Clean Output)

Run the executable without parameters for clean, production-ready output:

```cmd
LeitoraHU20.exe
```

**Expected Output:**
```
LeitoraHU20 starting...
Loading SecuGen SDK...
SDK loaded successfully.
Initializing fingerprint reader...
Reader initialized. Searching for devices...
Initializing SDK for device detection...
ERR: Can't open the key(2).
ERR: Can't get dword value of "LiveFingerLogEnable" (6)
ERR: Can't get dword value of "LiveFingerLevel" (6)
SDK initialized successfully. Scanning for devices...
  [Device 0] OpenDevice error: 0x00000002
  [Device 1] OpenDevice error: 0x00000002
ERR: Can't open the key(2).
ERR: Can't get dword value of "LiveFingerLogEnable" (6)
ERR: Can't get dword value of "LiveFingerLevel" (6)
  [Device 2] Serial decoded: H54200804821

=========================================
[SUCCESS] HU20 Device Found!
Serial: H54200804821
=========================================
```

### Debug Mode (Detailed Logging)

Run with the `--debug` parameter to enable verbose logging for troubleshooting:

```cmd
LeitoraHU20.exe --debug
```

**What the Debug Mode Does:**
- ✅ Displays detailed initialization steps
- ✅ Shows all device detection attempts
- ✅ Creates a log file: `LeitoraHU20_Debug.log`
- ✅ Helps diagnose connection issues
- ✅ Shows SDK initialization details

**Expected Output (Debug Mode):**
```
Device 2:
ERR: Can't open the key(2).
ERR: Can't get dword value of "LiveFingerLogEnable" (6)
ERR: Can't get dword value of "LiveFingerLevel" (6)
  SGFPM_OpenDevice() returned: 0x00000000
  [OK] Device opened successfully
  SGFPM_GetDeviceInfo() returned: 0x00000000
  Device Info Retrieved:
    Serial (raw): 02 00 00 00 48 35 34 32 30 30 38 30 34 38 32 31 20 20 20 00 BD 03 00 00 00 00 00 00 2C 01 00 00
    Device ID: 90 01 00 00 00 00 00 00 32 00 00 00 00 00 00 00
    Image Size: 0x0
  Decoded Serial: 'H54200804821'
  [Device 2] Serial decoded: H54200804821

=========================================
[SUCCESS] HU20 Device Found!
Serial: H54200804821
=========================================


[SUCCESS] Serial number extracted and printed!
  SGFPM_CloseDevice() returned: 0x00000000

--- Cleanup Phase ---
Calling DestroySGFPMObject()...
  Object destroyed
DLL unloaded

--- Execution Complete ---
Exit Code: 0 (SUCCESS)
```

**When to Use Debug Mode:**
- The reader is not being detected
- You want to see detailed device enumeration
- Troubleshooting connection issues
- Generating logs for technical support

---

## 📦 Prepare for Distribution

### 1. Copy the Executable and DLLs

Organize the files like this:

```
LeitoraHU20\
├── README_for_End_User.md
├── LICENSE
├── LeitoraHU20.exe          ← Compiled executable
├── sgfplib.dll              ← Native DLL
├── sgbledev.dll
├── sgfdusdax64.dll
├── sgfpamx.dll
└── sgwsqlib.dll
```

### 2. Create a ZIP for Distribution

```cmd
# In Windows Explorer:
# 1. Select the files above
# 2. Right-click → "Send to" → "Compressed (zipped) Folder"
```

Or via PowerShell:

```powershell
Compress-Archive -Path README_for_End_User.md, LICENSE, LeitoraHU20.exe, sgfplib.dll, sgbledev.dll, sgfdusdax64.dll, sgfpamx.dll, sgwsqlib.dll -DestinationPath LeitoraHU20-v1.0.x.zip
```

### 3. Instructions for End Users

See `README_for_End_User.md` file.

---

## 🐛 Common Issues

### Error: "Cannot find msbuild.exe"

**Solution:** Make sure Visual Studio Build Tools is installed with "Desktop development with C++" workload.

### Error: "LINK: fatal error LNK1104: cannot open file 'kernel32.lib'"

**Solution:** Reinstall Visual Studio Build Tools and make sure the Windows SDK is selected during installation.

### Error: "Cannot open sgfplib.dll"

**Solution:** Make sure the DLLs are in the same directory as the .exe

### The script runs but no .exe is created

**Solution:** 
1. Check the output messages carefully
2. Verify LeitoraHU20.cpp exists in the same directory
3. Try running in a new PowerShell window
4. Restart your computer if Visual Studio was just installed

---

## 📊 Build Information

| Aspect | Value |
|--------|-------|
| Version | 1.0.1 |
| Language | C++17 |
| Platform | Windows x64 |
| .exe Size | ~100-150 KB |
| Runtime Dependencies | None (Windows native) |
| Compatibility | Windows 7 SP1+ to Windows 11 |

---

## 🎯 Next steps after reading this README.

1. ✅ Compile the project using the script
2. ✅ Test the executable on your machine
3. ✅ Test on target machines (Windows 7-11)
4. ✅ Distribute the ZIP to end users

**Ready!** The executable is completely self-contained and can be run on any Windows machine with the HU20 driver installed.

---
---
---

## 📝 General Notes

### About "Desktop development with C++" Workload

**What is it?**

"Desktop development with C++" is a **workload** (collection of tools) in Visual Studio Build Tools that contains everything required to compile C++ code.

**Why do we need it?**

This workload provides the essential components that `compile_direct.bat` requires to build the project:

| Component | Function | Used By |
|-----------|----------|---------|
| **MSVC Compiler** (`cl.exe`) | Compiles the C++ source code | The script executes: `cl.exe LeitoraHU20.cpp` |
| **vcvars64.bat** | Configures compiler environment variables | The script executes: `vcvars64.bat` before compilation |
| **Windows SDK** | Windows system libraries and headers | `kernel32.lib`, `user32.lib` are linked |
| **Linker** (`link.exe`) | Links compiled code into the executable | Creates the final `.exe` file |

**Analogy:**

Imagine you need to **bake a cake** (create the executable):

- 🎂 **Recipe** = `compile_direct.bat` (the instructions)
- 🔪 **Tools needed** = "Desktop development with C++" (knife, bowl, spoon, oven)
- ❌ **Without the tools** = You have the recipe but can't make anything

**What we don't need:**

| Tool | Why Not Needed |
|------|----------------|
| **Full Visual Studio IDE** | ❌ Too large (~50 GB). Build Tools (500 MB) is sufficient |
| **CMake** | ❌ Not necessary (direct compilation with `cl.exe`) |
| **Ninja** | ❌ Not necessary (direct compilation) |

**Summary:**

```
Visual Studio Build Tools
    ├── "Desktop development with C++" workload
    │   ├── MSVC Compiler (cl.exe)       ← REQUIRED
    │   ├── vcvars64.bat                 ← REQUIRED
    │   ├── Windows SDK                  ← REQUIRED
    │   └── Linker (link.exe)            ← REQUIRED
    │
    └── (Optional tools not needed for this project)
```

**Result:** With only the "Desktop development with C++" workload installed, you have **everything** needed to compile! ✅
