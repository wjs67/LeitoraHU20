using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using SecuGen.FDxSDKPro.Windows;

namespace SecuGenSerialReader
{
    class Program
    {
        // P/Invoke para obter mais informações sobre erros de carregamento de DLL
        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
        private static extern IntPtr LoadLibrary(string lpFileName);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern int GetLastError();

        // P/Invoke para suprimir stderr (saída de erro das DLLs nativas)
        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool SetStdHandle(int nStdHandle, IntPtr hHandle);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr GetStdHandle(int nStdHandle);

        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        private static extern IntPtr CreateFile(string lpFileName, uint dwDesiredAccess, uint dwShareMode, 
            IntPtr lpSecurityAttributes, uint dwCreationDisposition, uint dwFlagsAndAttributes, IntPtr hTemplateFile);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool CloseHandle(IntPtr hObject);

        private const int STD_OUTPUT_HANDLE = -11;
        private const int STD_ERROR_HANDLE = -12;
        private const uint GENERIC_WRITE = 0x40000000;
        private const uint OPEN_ALWAYS = 4;

        private static IntPtr _originalStdOut;
        private static IntPtr _originalStdErr;
        private static IntPtr _nullHandle;

        static void SuppressConsoleOutput()
        {
            try
            {
                _originalStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
                _originalStdErr = GetStdHandle(STD_ERROR_HANDLE);
                
                _nullHandle = CreateFile("NUL", GENERIC_WRITE, 0, IntPtr.Zero, OPEN_ALWAYS, 0, IntPtr.Zero);
                if (_nullHandle != IntPtr.Zero && _nullHandle.ToInt64() != -1)
                {
                    SetStdHandle(STD_OUTPUT_HANDLE, _nullHandle);
                    SetStdHandle(STD_ERROR_HANDLE, _nullHandle);
                }
            }
            catch
            {
                // Ignore errors
            }
        }

        static void RestoreConsoleOutput()
        {
            try
            {
                if (_originalStdOut != IntPtr.Zero)
                    SetStdHandle(STD_OUTPUT_HANDLE, _originalStdOut);
                if (_originalStdErr != IntPtr.Zero)
                    SetStdHandle(STD_ERROR_HANDLE, _originalStdErr);
                if (_nullHandle != IntPtr.Zero && _nullHandle.ToInt64() != -1)
                    CloseHandle(_nullHandle);
            }
            catch
            {
                // Ignore errors
            }
        }

        static void Main(string[] args)
        {
            string pastaAtual = AppDomain.CurrentDomain.BaseDirectory;
            string caminhoSgfplib = Path.Combine(pastaAtual, "sgfplib.dll");

            try
            {
                // Adiciona a pasta atual ao PATH para resolver dependências
                string pathAtual = Environment.GetEnvironmentVariable("PATH");
                if (!pathAtual.Contains(pastaAtual))
                {
                    Environment.SetEnvironmentVariable("PATH", pastaAtual + ";" + pathAtual);
                }

                if (File.Exists(caminhoSgfplib))
                {
                    // Tenta carregar com NativeLibrary primeiro
                    try
                    {
                        NativeLibrary.Load(caminhoSgfplib);
                    }
                    catch (Exception exNative)
                    {
                        // Se falhar, tenta com LoadLibrary direto
                        IntPtr handle = LoadLibrary(caminhoSgfplib);
                        if (handle == IntPtr.Zero)
                        {
                            int erro = GetLastError();
                            Console.Error.WriteLine("Falha ao carregar sgfplib.dll via LoadLibrary. Código: " + erro);
                            Console.Error.WriteLine("Erro NativeLibrary: " + exNative.Message);
                            throw new Exception("Impossível carregar a biblioteca nativa sgfplib.dll");
                        }
                    }
                }
                else
                {
                    Console.Error.WriteLine("Erro: sgfplib.dll nao encontrada em: " + caminhoSgfplib);
                    throw new FileNotFoundException("sgfplib.dll não encontrada");
                }

                // Suprime os erros de stderr/stdout das DLLs nativas
                SuppressConsoleOutput();

                // Executa a leitura normal do hardware
                SGFingerPrintManager fpm = new SGFingerPrintManager();
                uint erroInit = (uint)fpm.Init(SGFPMDeviceName.DEV_AUTO);

                // Restaura o console para imprimir o resultado
                RestoreConsoleOutput();

                if (erroInit == (uint)SGFPMError.ERROR_NONE)
                {
                    // Tenta abrir múltiplos dispositivos (alguns sistemas têm múltiplos leitores)
                    bool dispositivoEncontrado = false;
                    
                    for (int i = 0; i < 10; i++)
                    {
                        uint erroOpen = (uint)fpm.OpenDevice(i);

                        if (erroOpen == (uint)SGFPMError.ERROR_NONE)
                        {
                            SGFPMDeviceInfoParam deviceInfo = new SGFPMDeviceInfoParam();
                            uint erroInfo = (uint)fpm.GetDeviceInfo(deviceInfo);

                            if (erroInfo == (uint)SGFPMError.ERROR_NONE)
                            {
                                string numeroDeSerie = Encoding.ASCII.GetString(deviceInfo.DeviceSN).Trim('\0', ' ');
                                Console.WriteLine(numeroDeSerie);
                                dispositivoEncontrado = true;
                                fpm.CloseDevice();
                                break;
                            }
                            
                            fpm.CloseDevice();
                        }
                    }

                    if (!dispositivoEncontrado)
                    {
                        // Se nenhum dispositivo foi encontrado, tenta apenas o primeiro
                        uint erroOpen = (uint)fpm.OpenDevice(0);
                        
                        if (erroOpen != (uint)SGFPMError.ERROR_NONE)
                        {
                            Console.Error.WriteLine("Dispositivo HU20 não encontrado ou falha ao abrir. Código de erro: " + erroOpen);
                            Console.Error.WriteLine("Verifique se:");
                            Console.Error.WriteLine("  1. O leitor HU20 está conectado à USB");
                            Console.Error.WriteLine("  2. O driver está instalado corretamente");
                            Console.Error.WriteLine("  3. Nenhum outro programa está usando o leitor");
                        }
                    }
                }
                else
                {
                    Console.Error.WriteLine("Falha ao inicializar o SDK SecuGen. Codigo: " + erroInit);
                    Console.Error.WriteLine("Dica: Verifique se o driver do HU20 está instalado corretamente.");
                }
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine("Erro critico de runtime: " + ex.Message);
            }
        }
    }
}
