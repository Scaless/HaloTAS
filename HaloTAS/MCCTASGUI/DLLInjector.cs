using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows;

namespace MCCTASGUI
{
    class DLLInjector
    {
        const string CURRENT_SUPPORTED_VERSION = "1.2282.0.0";

        [DllImport("kernel32.dll")]
        static extern IntPtr OpenProcess(int dwDesiredAccess, bool bInheritHandle, int dwProcessId);

        [DllImport("kernel32.dll", CharSet = CharSet.Auto)]
        static extern IntPtr GetModuleHandle(string lpModuleName);

        [DllImport("kernel32", CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
        static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

        [DllImport("kernel32.dll", SetLastError = true, ExactSpelling = true)]
        static extern IntPtr VirtualAllocEx(IntPtr hProcess, IntPtr lpAddress,
            uint dwSize, uint flAllocationType, uint flProtect);

        [DllImport("kernel32.dll", SetLastError = true)]
        static extern bool WriteProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, byte[] lpBuffer, uint nSize, out UIntPtr lpNumberOfBytesWritten);

        [DllImport("kernel32.dll")]
        static extern IntPtr CreateRemoteThread(IntPtr hProcess,
            IntPtr lpThreadAttributes, uint dwStackSize, IntPtr lpStartAddress, IntPtr lpParameter, uint dwCreationFlags, IntPtr lpThreadId);

        // privileges
        const int PROCESS_CREATE_THREAD = 0x0002;
        const int PROCESS_QUERY_INFORMATION = 0x0400;
        const int PROCESS_VM_OPERATION = 0x0008;
        const int PROCESS_VM_WRITE = 0x0020;
        const int PROCESS_VM_READ = 0x0010;

        // used for memory allocation
        const uint MEM_COMMIT = 0x00001000;
        const uint MEM_RESERVE = 0x00002000;
        const uint PAGE_READWRITE = 4;

        public static void Inject()
        {
            Process eacProcess = Process.GetProcessesByName("EasyAntiCheat.exe").FirstOrDefault();
            if(eacProcess != null)
            {
                string caption = "Injection Failed - EAC is running";
                string message = "The EasyAntiCheat process is running. MCCTAS will not work with anti-cheat enabled, please re-launch the game with EAC disabled.";
                MessageBox.Show(message, caption, MessageBoxButton.OK);
                return;
            }

            Process targetProcess = Process.GetProcessesByName("HaloInfinite").FirstOrDefault();

            if(targetProcess == null)
            {
                string caption = "Injection Failed - MCC not running";
                string message = "Couldn't find MCC-Win64-Shipping.exe. Are you sure the game is running?";
                MessageBox.Show(message, caption, MessageBoxButton.OK);
                return;
            }

            // Version Checking
            //var fileVersionInfo = FileVersionInfo.GetVersionInfo(targetProcess.MainModule.FileName);
            //if(fileVersionInfo.FileVersion != CURRENT_SUPPORTED_VERSION)
            //{
            //    string caption = "Injection Failed - Version Mismatch";
            //    string message = $"The version of Master Chief Collection that is running ({fileVersionInfo.FileVersion}) does not match the version supported by MCCTAS ({CURRENT_SUPPORTED_VERSION}). Launch Anyways?";
            //    MessageBoxResult result = MessageBox.Show(message, caption, MessageBoxButton.YesNo);
            //    if(result != MessageBoxResult.Yes)
            //    {
            //        return;
            //    }
            //}

            // geting the handle of the process - with required privileges
            IntPtr procHandle = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, false, targetProcess.Id);

            // searching for the address of LoadLibraryA and storing it in a pointer
            IntPtr loadLibraryAddr = GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");

            // full path of the dll we want to inject
            string dllFullPath = Directory.GetFiles(".", "MCCTAS.dll").FirstOrDefault();

            if (string.IsNullOrWhiteSpace(dllFullPath))
            {
                string caption = "Injection Failed - MCCTAS.dll Not Found";
                string message = "Couldn't find MCCTAS.dll. It should be in the folder alongside this program.";
                MessageBox.Show(message, caption, MessageBoxButton.OK);
                return;
            }

            dllFullPath = Path.GetFullPath(dllFullPath);

            // alocating some memory on the target process - enough to store the name of the dll
            // and storing its address in a pointer
            IntPtr allocMemAddress = VirtualAllocEx(procHandle, IntPtr.Zero, (uint)((dllFullPath.Length + 1) * Marshal.SizeOf(typeof(char))), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

            // writing the name of the dll there
            UIntPtr bytesWritten;
            WriteProcessMemory(procHandle, allocMemAddress, Encoding.Default.GetBytes(dllFullPath), (uint)((dllFullPath.Length + 1) * Marshal.SizeOf(typeof(char))), out bytesWritten);

            // creating a thread that will call LoadLibraryA with allocMemAddress as argument
            CreateRemoteThread(procHandle, IntPtr.Zero, 0, loadLibraryAddr, allocMemAddress, 0, IntPtr.Zero);
        }

    }
}

