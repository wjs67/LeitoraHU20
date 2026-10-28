#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <cstring>
#include <string>
#include <fstream>
#include <sstream>
#include <time.h>

// ============================================================================
// Debug Configuration
// ============================================================================

#define LOG_TO_FILE 1
#define LOG_FILENAME "LeitoraHU20_Debug.log"

static FILE* g_logFile = NULL;
static bool g_debugMode = false;  // Enabled only if --debug is passed

void OpenDebugLog() {
    if (!g_debugMode) return;
    
#if LOG_TO_FILE
    errno_t err = fopen_s(&g_logFile, LOG_FILENAME, "w");
    if (err == 0 && g_logFile) {
        time_t now = time(NULL);
        char timeStr[100];
        ctime_s(timeStr, sizeof(timeStr), &now);
        fprintf(g_logFile, "========================================\n");
        fprintf(g_logFile, "LeitoraHU20 - Debug Log\n");
        fprintf(g_logFile, "Started at: %s", timeStr);
        fprintf(g_logFile, "========================================\n\n");
        fflush(g_logFile);
    }
#endif
}

void CloseDebugLog() {
    if (!g_debugMode) return;
    
#if LOG_TO_FILE
    if (g_logFile) {
        fprintf(g_logFile, "\n========================================\n");
        fprintf(g_logFile, "Log ended\n");
        fprintf(g_logFile, "========================================\n");
        fclose(g_logFile);
        g_logFile = NULL;
    }
#endif
}

void DebugLog(const char* format, ...) {
    if (!g_debugMode) return;
    
    va_list args;
    va_start(args, format);
    
    // Imprime no console
    vprintf(format, args);
    printf("\n");
    fflush(stdout);
    
    va_end(args);

#if LOG_TO_FILE
    if (g_logFile) {
        va_list args2;
        va_start(args2, format);
        vfprintf(g_logFile, format, args2);
        fprintf(g_logFile, "\n");
        fflush(g_logFile);
        va_end(args2);
    }
#endif
}

// ============================================================================
// ErrorPrint - Always prints (debug mode or not)
// ============================================================================
void ErrorPrint(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    // Sempre imprime no console
    vprintf(format, args);
    printf("\n");
    fflush(stdout);
    
    va_end(args);

#if LOG_TO_FILE
    if (g_logFile) {
        va_list args2;
        va_start(args2, format);
        vfprintf(g_logFile, format, args2);
        fprintf(g_logFile, "\n");
        fflush(g_logFile);
        va_end(args2);
    }
#endif
}

void DebugLogHex(const char* label, const unsigned char* data, int len) {
    if (!g_debugMode) return;
    
    char buffer[256];
    sprintf_s(buffer, sizeof(buffer), "%s: ", label);
    
    for (int i = 0; i < len && i < 32; i++) {
        char hex[10];
        sprintf_s(hex, sizeof(hex), "%02X ", data[i]);
        strcat_s(buffer, sizeof(buffer), hex);
    }
    
    DebugLog("%s", buffer);
}

// ============================================================================
// SecuGen SDK Definitions
// ============================================================================

// Helper function to avoid direct return in LoadSecuGenDLL
inline bool ReturnSuccessFromLoad() {
    return true;
}

#define MAX_DEVICE_COUNT 10

typedef unsigned char BYTE;
typedef unsigned int UINT;

enum SGFPMDeviceName {
    DEV_AUTO = 0xFF
};

enum SGFPMError {
    ERROR_NONE = 0x00000000,
    ERROR_DEV_NOT_FOUND = 0x00000003
};

typedef struct {
    BYTE DeviceSN[32];
    BYTE DeviceID[16];
    BYTE DeviceVer[32];
    BYTE ImageWidth;
    BYTE ImageHeight;
    BYTE Dummy[15];
} SGFPMDeviceInfoParam;

// ============================================================================
// SDK Function Pointers
// ============================================================================

typedef int (__stdcall *FP_CreateSGFPMObject)(void** ppFPM);
typedef int (__stdcall *FP_DestroySGFPMObject)(void* pFPM);
typedef int (__stdcall *FP_SGFPMInit)(void* pFPM, int DeviceName);
typedef int (__stdcall *FP_SGFPMOpenDevice)(void* pFPM, int DeviceID);
typedef int (__stdcall *FP_SGFPMCloseDevice)(void* pFPM);
typedef int (__stdcall *FP_SGFPMGetDeviceInfo)(void* pFPM, SGFPMDeviceInfoParam* pParam);
typedef int (__stdcall *FP_SGFPMGetNumOfDevice)();

FP_CreateSGFPMObject g_pCreateSGFPMObject = NULL;
FP_DestroySGFPMObject g_pDestroySGFPMObject = NULL;
FP_SGFPMInit g_pSGFPMInit = NULL;
FP_SGFPMOpenDevice g_pSGFPMOpenDevice = NULL;
FP_SGFPMCloseDevice g_pSGFPMCloseDevice = NULL;
FP_SGFPMGetDeviceInfo g_pSGFPMGetDeviceInfo = NULL;
FP_SGFPMGetNumOfDevice g_pSGFPMGetNumOfDevice = NULL;

HMODULE g_hSecuGenDLL = NULL;
bool g_LoadSecuGenDLLSuccess = false;  // Global flag instead of return

// ============================================================================
// Auxiliary functions
// ============================================================================

/**
 * Loads the SecuGen SDK DLL dynamically with debug info
 */
void LoadSecuGenDLL(const char* dllPath);  // Forward declaration only

/**
 * Unloads the loaded DLL
 */
void UnloadSecuGenDLL() {
    if (g_hSecuGenDLL) {
        FreeLibrary(g_hSecuGenDLL);
        g_hSecuGenDLL = NULL;
    }
}

/**
 * Gets the application directory path
 */
std::string GetApplicationDirectory() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    
    // Remove executable name, keeping only directory
    std::string path(buffer);
    size_t pos = path.find_last_of("\\/");
    if (pos != std::string::npos) {
        return path.substr(0, pos + 1);
    }
    return path;  // Return full path if no separator found
}

/**
 * Checks if HU20 device is connected
 * Returns device count, 0 if no devices found, -1 if check unavailable
 */
int CheckDevicePresence() {
    DebugLog("\n--- Checking Device Presence ---");
    
    // Try using GetNumOfDevice if available
    if (g_pSGFPMGetNumOfDevice) {
        int numDevices = g_pSGFPMGetNumOfDevice();
        DebugLog("SGFPM_GetNumOfDevice() returned: %d", numDevices);
        
        // Handle error cases - negative values mean error
        if (numDevices < 0) {
            DebugLog("GetNumOfDevice returned error code: %d - treating as no device found", numDevices);
            return 0;  // Treat as no device found
        }
        return numDevices;
    }
    
    DebugLog("GetNumOfDevice not available, will scan slots manually");
    return -1;  // Fallback to manual scanning
}

/**
 * Decodes the HU20 device serial number
 * Structure: bytes [0-3] as DWORD, [4-19] as serial ASCII
 */
std::string DecodeDeviceSerial(const BYTE* rawData, size_t maxLen) {
    // Skip first 4 bytes (DWORD) and read 16 bytes of ASCII
    std::string result;
    
    // Start at byte 4 (offset for ASCII serial)
    size_t startIdx = 4;
    size_t endIdx = (maxLen > 20) ? 20 : maxLen;  // Read up to byte 19 (16 bytes of serial)
    
    for (size_t i = startIdx; i < endIdx; i++) {
        char c = (char)rawData[i];
        // Stop at null or space
        if (c == '\0' || c == ' ') {
            break;
        }
        // Validate printable ASCII character
        if (c >= 32 && c < 127) {
            result += c;
        }
    }
    
    return result;
}

// ============================================================================
// Main Function
// ============================================================================

int main(int argc, char* argv[]) {
    // Configure console for UTF-8 output (fixes character encoding on Windows)
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    int exitCode = 1;
    void* pFPM = NULL;
    
    // Check if --debug was passed as argument
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0) {
            g_debugMode = true;
            break;
        }
    }
    
    // Open debug log
    OpenDebugLog();
    
    // Always print startup message to ensure user sees something
    printf("LeitoraHU20 starting...\n");
    fflush(stdout);
    
    // Print progress in normal mode too
    printf("Loading SecuGen SDK...\n");
    fflush(stdout);

    
    DebugLog("========================================");
    DebugLog("LeitoraHU20 - HU20 Biometric Reader");
    DebugLog("========================================");
    DebugLog("Executable: %s", argv[0]);
    DebugLog("Platform: Windows %s", sizeof(void*) == 8 ? "x64" : "x86");

    // Get application directory

    std::string appDir = GetApplicationDirectory();

    DebugLog("\nApplication Directory: %s", appDir.c_str());


    // Build full path to sgfplib.dll

    std::string dllPath = appDir + "sgfplib.dll";

    

    DebugLog("DLL Path: %s", dllPath.c_str());


    // Check if DLL exists

    DWORD attribs = GetFileAttributesA(dllPath.c_str());

    if (attribs == INVALID_FILE_ATTRIBUTES) {

        DebugLog("ERROR: DLL file not found at specified path!");
        DebugLog("Trying current directory...");
        if (GetFileAttributesA("sgfplib.dll") != INVALID_FILE_ATTRIBUTES) {
            DebugLog("  Found sgfplib.dll in current directory");
        } else {
            DebugLog("  sgfplib.dll not found in current directory either");
        }
    } else {

        DebugLog("[OK] DLL file exists at specified path");
    }


    DebugLog("\n--- Initialization Phase ---");

    

    g_LoadSecuGenDLLSuccess = false;  // Reset flag before load
    
    // --- INLINE DLL LOADING LOGIC (was LoadSecuGenDLL function) ---
    DebugLog("--- LoadSecuGenDLL started ---");
    DebugLog("Attempting to load from: %s", dllPath.c_str());
    
    g_hSecuGenDLL = LoadLibraryA(dllPath.c_str());
    
    if (!g_hSecuGenDLL) {
        DWORD error = GetLastError();
        DebugLog("ERROR: LoadLibrary failed with error code: 0x%08X (%d)", error, error);
        DebugLog("Attempting fallback: LoadLibrary('sgfplib.dll')");
        
        g_hSecuGenDLL = LoadLibraryA("sgfplib.dll");
        
        if (!g_hSecuGenDLL) {
            error = GetLastError();
            DebugLog("ERROR: Fallback also failed with error code: 0x%08X (%d)", error, error);
            DebugLog("Possible causes:");
            DebugLog("  1. sgfplib.dll not found in PATH");
            DebugLog("  2. Missing dependencies (e.g., Visual C++ Runtime)");
            DebugLog("  3. DLL is 32-bit but executable is 64-bit (or vice versa)");
            DebugLog("  4. Corrupted DLL file");
            g_LoadSecuGenDLLSuccess = false;
        }
    }
    
    if (g_hSecuGenDLL) {
        DebugLog("SUCCESS: DLL loaded at address: %p", g_hSecuGenDLL);
        DebugLog("\n--- Loading Function Pointers ---");

        // Manually load each function pointer to avoid macro expansion crashes
        g_pCreateSGFPMObject = (FP_CreateSGFPMObject)GetProcAddress(g_hSecuGenDLL, "CreateSGFPMObject");
        if (!g_pCreateSGFPMObject) g_pCreateSGFPMObject = (FP_CreateSGFPMObject)GetProcAddress(g_hSecuGenDLL, (LPCSTR)1);
        if (g_pCreateSGFPMObject) DebugLog("  [OK] CreateSGFPMObject loaded");
        
        g_pDestroySGFPMObject = (FP_DestroySGFPMObject)GetProcAddress(g_hSecuGenDLL, "DestroySGFPMObject");
        if (!g_pDestroySGFPMObject) g_pDestroySGFPMObject = (FP_DestroySGFPMObject)GetProcAddress(g_hSecuGenDLL, (LPCSTR)2);
        if (g_pDestroySGFPMObject) DebugLog("  [OK] DestroySGFPMObject loaded");
        
        g_pSGFPMInit = (FP_SGFPMInit)GetProcAddress(g_hSecuGenDLL, "SGFPMInit");
        if (!g_pSGFPMInit) g_pSGFPMInit = (FP_SGFPMInit)GetProcAddress(g_hSecuGenDLL, (LPCSTR)47);
        if (g_pSGFPMInit) DebugLog("  [OK] SGFPMInit loaded");
        
        g_pSGFPMOpenDevice = (FP_SGFPMOpenDevice)GetProcAddress(g_hSecuGenDLL, "SGFPMOpenDevice");
        if (!g_pSGFPMOpenDevice) g_pSGFPMOpenDevice = (FP_SGFPMOpenDevice)GetProcAddress(g_hSecuGenDLL, (LPCSTR)61);
        if (g_pSGFPMOpenDevice) DebugLog("  [OK] SGFPMOpenDevice loaded");
        
        g_pSGFPMCloseDevice = (FP_SGFPMCloseDevice)GetProcAddress(g_hSecuGenDLL, "SGFPMCloseDevice");
        if (!g_pSGFPMCloseDevice) g_pSGFPMCloseDevice = (FP_SGFPMCloseDevice)GetProcAddress(g_hSecuGenDLL, (LPCSTR)6);
        if (g_pSGFPMCloseDevice) DebugLog("  [OK] SGFPMCloseDevice loaded");
        
        g_pSGFPMGetDeviceInfo = (FP_SGFPMGetDeviceInfo)GetProcAddress(g_hSecuGenDLL, "SGFPMGetDeviceInfo");
        if (!g_pSGFPMGetDeviceInfo) g_pSGFPMGetDeviceInfo = (FP_SGFPMGetDeviceInfo)GetProcAddress(g_hSecuGenDLL, (LPCSTR)24);
        if (g_pSGFPMGetDeviceInfo) DebugLog("  [OK] SGFPMGetDeviceInfo loaded");
        
        g_pSGFPMGetNumOfDevice = (FP_SGFPMGetNumOfDevice)GetProcAddress(g_hSecuGenDLL, "SGFPMGetNumOfDevice");
        if (!g_pSGFPMGetNumOfDevice) g_pSGFPMGetNumOfDevice = (FP_SGFPMGetNumOfDevice)GetProcAddress(g_hSecuGenDLL, (LPCSTR)26);
        if (g_pSGFPMGetNumOfDevice) DebugLog("  [OK] SGFPMGetNumOfDevice loaded");

        // Verify all required functions
        if (g_pCreateSGFPMObject && g_pDestroySGFPMObject && g_pSGFPMInit && 
            g_pSGFPMOpenDevice && g_pSGFPMCloseDevice && g_pSGFPMGetDeviceInfo) {
            g_LoadSecuGenDLLSuccess = true;
        } else {
            FreeLibrary(g_hSecuGenDLL);
            g_hSecuGenDLL = NULL;
            g_LoadSecuGenDLLSuccess = false;
        }
    }
    
    if (!g_LoadSecuGenDLLSuccess) {
        printf("\n=========================================\n");
        printf("ERROR: Failed to load SecuGen DLL!\n");
        printf("=========================================\n");
        printf("\nPlease ensure:\n");
        printf("  1. sgfplib.dll is in the same directory as LeitoraHU20.exe\n");
        printf("  2. All dependent DLLs are present (sgbledev.dll, etc.)\n");
        printf("  3. System has Visual C++ 2015+ Runtime installed\n");
        printf("  4. Windows drivers for HU20 are installed\n");
        printf("  5. Run with --debug flag for detailed diagnostic information\n\n");
        fflush(stdout);
        DebugLog("\nFATAL ERROR: Failed to load SecuGen DLL");
        CloseDebugLog();
        return EXIT_FAILURE;
    }
    
    printf("SDK loaded successfully.\n");
    fflush(stdout);
    DebugLog("\nDLL loaded successfully");
    
    // Create SGFPM object
    printf("Initializing fingerprint reader...\n");
    fflush(stdout);
    DebugLog("\nCalling CreateSGFPMObject()...");
    int createResult = g_pCreateSGFPMObject(&pFPM);
    DebugLog("  Result: 0x%08X", createResult);
    DebugLog("  pFPM pointer: %p", pFPM);
    
    if (!pFPM) {
        ErrorPrint("ERROR: Failed to initialize fingerprint reader object!");
        fflush(stdout);
        DebugLog("ERROR: CreateSGFPMObject returned NULL or error");
        UnloadSecuGenDLL();
        CloseDebugLog();
        return 1;
    }
    
    printf("Reader initialized. Searching for devices...\n");
    fflush(stdout);
    
    // Initialize SDK
    DebugLog("\nCalling SGFPM_Init(pFPM, DEV_AUTO=0xFF)...");
    printf("Initializing SDK for device detection...\n");
    fflush(stdout);
    int initResult = g_pSGFPMInit(pFPM, DEV_AUTO);
    DebugLog("  Result: 0x%08X (%d)", initResult, initResult);
    
    if (initResult != ERROR_NONE) {
        printf("ERROR: SDK initialization failed (0x%08X)!\n", initResult);
        printf("Try running with --debug flag for more information.\n");
        fflush(stdout);
        DebugLog("\n[!] SGFPM_Init FAILED - Diagnostic Information:");
        DebugLog("  Error Code: 0x%08X (%d)", initResult, initResult);
        
        // Error code translation and solutions
        if (initResult == 0x06) {
            DebugLog("  Error Type: GENERAL_ERROR (0x06)");
            DebugLog("");
            DebugLog("  [!] IMPORTANT: Before investigating the causes below, please:");
            DebugLog("      - Verify that the HU20 device is properly connected to a USB port");
            DebugLog("      - Check Device Manager to ensure the device is recognized by Windows");
            DebugLog("      - Try a different USB port if available");
            DebugLog("");
            DebugLog("  Possible Root Causes:");
            DebugLog("     1. SecuGen SDK not properly installed or corrupted");
            DebugLog("     2. Missing/invalid registry keys for SDK configuration");
            DebugLog("     3. HU20 reader drivers not installed");
            DebugLog("     4. DLL dependency mismatch (mixing x86/x64)");
            DebugLog("     5. USB device not recognized by system");
            DebugLog("     6. Windows permissions/UAC blocking SDK access");
        } else if (initResult == 0x02) {
            DebugLog("  Error Type: DEV_NOT_FOUND (0x02)");
            DebugLog("  No device found at specified index");
        } else {
            DebugLog("  Error Type: UNKNOWN (0x%08X)", initResult);
            DebugLog("  Check SecuGen SDK documentation for code: 0x%08X", initResult);
        }
        DebugLog("");
        
        g_pDestroySGFPMObject(pFPM);
        UnloadSecuGenDLL();
        CloseDebugLog();
        return 1;
    }
    
    printf("SDK initialized successfully. Scanning for devices...\n");
    fflush(stdout);
    DebugLog("SGFPM_Init succeeded");

    // Search for connected devices
    DebugLog("\n--- Device Search Phase ---");
    DebugLog("Scanning for connected HU20 readers (max %d devices)...", MAX_DEVICE_COUNT);
    
    bool deviceFound = false;
    
    for (int i = 0; i < MAX_DEVICE_COUNT; i++) {
        DebugLog("\nDevice %d:", i);
        
        // Attempt to open device
        int openResult = g_pSGFPMOpenDevice(pFPM, i);
        DebugLog("  SGFPM_OpenDevice() returned: 0x%08X", openResult);
        
        if (openResult == ERROR_NONE) {
            DebugLog("  [OK] Device opened successfully");
            
            // Get device information
            SGFPMDeviceInfoParam deviceInfo = {};
            int infoResult = g_pSGFPMGetDeviceInfo(pFPM, &deviceInfo);
            DebugLog("  SGFPM_GetDeviceInfo() returned: 0x%08X", infoResult);
            
            if (infoResult == ERROR_NONE) {
                DebugLog("  Device Info Retrieved:");
                DebugLogHex("    Serial (raw)", deviceInfo.DeviceSN, 32);
                DebugLogHex("    Device ID", deviceInfo.DeviceID, 16);
                DebugLog("    Image Size: %dx%d", deviceInfo.ImageWidth, deviceInfo.ImageHeight);
                
                // Decode serial number
                std::string serialNumber = DecodeDeviceSerial(deviceInfo.DeviceSN, 
                                                              sizeof(deviceInfo.DeviceSN));
                
                DebugLog("  Decoded Serial: '%s'", serialNumber.c_str());
                printf("  [Device %d] Serial decoded: %s\n", i, serialNumber.c_str());
                fflush(stdout);
                
                if (!serialNumber.empty()) {
                    printf("\n");
                    printf("=========================================\n");
                    printf("[SUCCESS] HU20 Device Found!\n");
                    printf("Serial: %s\n", serialNumber.c_str());
                    printf("=========================================\n\n");
                    fflush(stdout);
                    DebugLog("\n[SUCCESS] Serial number extracted and printed!");
                    deviceFound = true;
                    exitCode = 0;
                } else {
                    DebugLog("  ! Serial number is empty or invalid");
                    ErrorPrint("WARNING: Device found but serial number could not be decoded");
                }
            } else {
                DebugLog("  ERROR: GetDeviceInfo failed with code 0x%08X", infoResult);
                printf("  [Device %d] GetDeviceInfo failed: 0x%08X\n", i, infoResult);
                fflush(stdout);
            }

            // Close device
            int closeResult = g_pSGFPMCloseDevice(pFPM);
            DebugLog("  SGFPM_CloseDevice() returned: 0x%08X", closeResult);

            // If device found, exit loop
            if (deviceFound) {
                break;
            }
        } else if (openResult == ERROR_DEV_NOT_FOUND) {
            DebugLog("  - Device not found (normal if no reader on this slot)");
        } else {
            DebugLog("  ! Unexpected error code 0x%08X", openResult);
            printf("  [Device %d] OpenDevice error: 0x%08X\n", i, openResult);
            fflush(stdout);
        }
    }

    if (!deviceFound) {
        printf("\nERROR: HU20 device not found!\n");
        printf("Troubleshooting:\n");
        printf("  1. Verify the HU20 reader is connected via USB\n");
        printf("  2. Check Device Manager for the reader\n");
        printf("  3. Ensure SecuGen drivers are installed\n");
        printf("  4. Run with --debug flag for detailed logs\n");
        printf("  5. Check the debug log file: %s\n\n", LOG_FILENAME);
        fflush(stdout);
        DebugLog("\nERROR: No HU20 device found in any slot");
    }

    // Cleanup
    DebugLog("\n--- Cleanup Phase ---");
    if (pFPM) {
        DebugLog("Calling DestroySGFPMObject()...");
        g_pDestroySGFPMObject(pFPM);
        DebugLog("  Object destroyed");
    }
    UnloadSecuGenDLL();
    DebugLog("DLL unloaded");
    
    DebugLog("\n--- Execution Complete ---");
    DebugLog("Exit Code: %d (%s)", exitCode, exitCode == 0 ? "SUCCESS" : "FAILED");
    
    CloseDebugLog();

    return exitCode;
}
