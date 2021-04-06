using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Windows;
using System.Diagnostics;
using Newtonsoft.Json;
using System.Net.Http;
using System.IO;

namespace HRPatcherGUI
{
    
    public class Version
    {
        public string version { get; set; }
        public string file { get; set; }
    }

    public class MCCPatcherVersions
    {
        public List<Version> versions { get; set; }
    }

    class Program
    {
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

        public static string GetRunningMCCVersion()
        {
            Process targetProcess = Process.GetProcessesByName("MCC-Win64-Shipping").FirstOrDefault();
            if(targetProcess == null)
            {
                return "";
            }
            var fileVersionInfo = FileVersionInfo.GetVersionInfo(targetProcess.MainModule.FileName);
            if(fileVersionInfo == null)
            {
                return "";
            }
            return fileVersionInfo.FileVersion;
        }

        public static bool Inject(string DLLName)
        {
            Process eacProcess = Process.GetProcessesByName("EasyAntiCheat.exe").FirstOrDefault();
            if (eacProcess != null)
            {
                string caption = "Injection Failed - EAC is running";
                string message = "The EasyAntiCheat process is running. Patches cannot be applied with EAC enabled, please re-launch the game with EAC disabled.";
                MessageBox.Show(message, caption, MessageBoxButton.OK);
                return false;
            }

            Process targetProcess = Process.GetProcessesByName("MCC-Win64-Shipping").FirstOrDefault();
            if (targetProcess == null)
            {
                string caption = "Injection Failed - MCC not running";
                string message = "Couldn't find MCC-Win64-Shipping.exe. Are you sure the game is running?";
                MessageBox.Show(message, caption, MessageBoxButton.OK);
                return false;
            }

            // geting the handle of the process - with required privileges
            IntPtr procHandle = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, false, targetProcess.Id);

            // searching for the address of LoadLibraryA and storing it in a pointer
            IntPtr loadLibraryAddr = GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");

            // full path of the dll we want to inject
            string dllFullPath = System.IO.Directory.GetFiles(".", DLLName).FirstOrDefault();

            if (string.IsNullOrWhiteSpace(dllFullPath))
            {
                string caption = $"Injection Failed - {DLLName} Not Found";
                string message = $"Couldn't find {DLLName}. It should be in the folder alongside this program.";
                MessageBox.Show(message, caption, MessageBoxButton.OK);
                return false;
            }

            dllFullPath = System.IO.Path.GetFullPath(dllFullPath);

            // alocating some memory on the target process - enough to store the name of the dll
            // and storing its address in a pointer
            IntPtr allocMemAddress = VirtualAllocEx(procHandle, IntPtr.Zero, (uint)((dllFullPath.Length + 1) * Marshal.SizeOf(typeof(char))), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

            // writing the name of the dll there
            UIntPtr bytesWritten;
            WriteProcessMemory(procHandle, allocMemAddress, Encoding.Default.GetBytes(dllFullPath), (uint)((dllFullPath.Length + 1) * Marshal.SizeOf(typeof(char))), out bytesWritten);

            // creating a thread that will call LoadLibraryA with allocMemAddress as argument
            CreateRemoteThread(procHandle, IntPtr.Zero, 0, loadLibraryAddr, allocMemAddress, 0, IntPtr.Zero);

            return true;
        }

        static async Task Main(string[] args)
        {
            const string patch_file_url_root = "https://github.com/Scaless/HRPatcherReleases/raw/main/";
            const string manifest_url = "https://raw.githubusercontent.com/Scaless/HRPatcherReleases/main/manifest.json";

            HttpClient client = new HttpClient();
            HttpResponseMessage manifest_response = await client.GetAsync(manifest_url);
            if (!manifest_response.IsSuccessStatusCode)
            {
                string caption = "HRPatcher Manifest Could Not Be Downloaded";
                string message = "Couldn't download the patch manifest. No patches were applied.";
                MessageBox.Show(message, caption, MessageBoxButton.OK);
                return;
            }

            MCCPatcherVersions PatcherVersions = JsonConvert.DeserializeObject<MCCPatcherVersions>(await manifest_response.Content.ReadAsStringAsync());
            if (PatcherVersions == null)
            {
                string caption = "HRPatcher Manifest Corrupted";
                string message = "Failed to read the patch manifest. No patches were applied.";
                MessageBox.Show(message, caption, MessageBoxButton.OK);
                return;
            }

            string running_mcc = GetRunningMCCVersion();
            if (string.IsNullOrWhiteSpace(running_mcc))
            {
                string caption = "Couldn't Determine MCC Version";
                string message = "Failed to get the version of MCC. Is MCC running with EAC disabled? No patches were applied.";
                MessageBox.Show(message, caption, MessageBoxButton.OK);
                return;
            }

            var found_version = PatcherVersions.versions.FirstOrDefault(x => x.version == running_mcc);
            if (found_version == null)
            {
                string caption = $"No Patcher Available For MCC Version {running_mcc}";
                string message = $"There are no patches available for the version of MCC currently running ({running_mcc}). No patches were applied.";
                MessageBox.Show(message, caption, MessageBoxButton.OK);
                return;
            }

            string patch_url = patch_file_url_root + found_version.file;
            HttpResponseMessage patch_response = await client.GetAsync(patch_url);
            if (!patch_response.IsSuccessStatusCode)
            {
                string caption = "HRPatcher Patch File Could Not Be Downloaded";
                string message = "Couldn't download the patch file. No patches were applied.";
                MessageBox.Show(message, caption, MessageBoxButton.OK);
                return;
            }

            try
            {
                using (Stream output = File.OpenWrite(found_version.file))
                    await patch_response.Content.CopyToAsync(output);
            }
            catch (Exception e)
            {
                string caption = "HRPatcher Patch File Failed To Save";
                string message = $"Couldn't save the patch file to disk. No patches were applied. ::: {e}";
                MessageBox.Show(message, caption, MessageBoxButton.OK);
                return;
            }

            // Validate file was saved correctly
            if (!System.IO.File.Exists(found_version.file))
            {
                string caption = "HRPatcher Patch File Failed To Save";
                string message = "Couldn't save the patch file to disk. No patches were applied.";
                MessageBox.Show(message, caption, MessageBoxButton.OK);
                return;
            }

            if (Inject(found_version.file))
            {
                string caption = "HRPatcher: Patch Applied!";
                string message = $"Patching was successful. To stop patches from being active, close out of the game completely and relaunch MCC.";
                MessageBox.Show(message, caption, MessageBoxButton.OK);
                return;
            } 
            else
            {
                string caption = "HRPatcher: Inject Failed";
                string message = $"Injection failed. No patches were applied.";
                MessageBox.Show(message, caption, MessageBoxButton.OK);
                return;
            }
        }
    }
}
