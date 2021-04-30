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
using System.Security.Cryptography;
using System.Windows.Forms;

namespace HRPatcherGUI
{
    
    public class Version
    {
        public string version { get; set; }
        public string file { get; set; }
        public string sha1 { get; set; }
    }

    public class MCCPatcherVersions
    {
        public List<Version> versions { get; set; }
        public int current_patcher_version { get; set; }
        public string current_patcher_link { get; set; }
    }

    static class Program
    {
        [DllImport("kernel32.dll")]
        static extern IntPtr OpenProcess(int dwDesiredAccess, bool bInheritHandle, int dwProcessId);
        [DllImport("kernel32.dll", CharSet = CharSet.Auto)]
        static extern IntPtr GetModuleHandle(string lpModuleName);
        [DllImport("kernel32", CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
        static extern IntPtr GetProcAddress(IntPtr hModule, string procName);
        [DllImport("kernel32.dll", SetLastError = true, ExactSpelling = true)]
        static extern IntPtr VirtualAllocEx(IntPtr hProcess, IntPtr lpAddress, uint dwSize, uint flAllocationType, uint flProtect);
        [DllImport("kernel32.dll", SetLastError = true)]
        static extern bool WriteProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, byte[] lpBuffer, uint nSize, out UIntPtr lpNumberOfBytesWritten);
        [DllImport("kernel32.dll")]
        static extern IntPtr CreateRemoteThread(IntPtr hProcess, IntPtr lpThreadAttributes, uint dwStackSize, IntPtr lpStartAddress, IntPtr lpParameter, uint dwCreationFlags, IntPtr lpThreadId);

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
                System.Windows.MessageBox.Show(message, caption, MessageBoxButton.OK);
                return false;
            }

            Process targetProcess = Process.GetProcessesByName("MCC-Win64-Shipping").FirstOrDefault();
            if (targetProcess == null)
            {
                string caption = "Injection Failed - MCC not running";
                string message = "Couldn't find MCC-Win64-Shipping.exe. Are you sure the game is running?";
                System.Windows.MessageBox.Show(message, caption, MessageBoxButton.OK);
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
                System.Windows.MessageBox.Show(message, caption, MessageBoxButton.OK);
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

        public static string GetFileSHA1(string FileName)
        {
            try
            {
                using (var sha1 = new SHA1CryptoServiceProvider())
                {
                    byte[] file = File.ReadAllBytes(FileName);
                    return BitConverter.ToString(sha1.ComputeHash(file)).Replace("-", "");
                }
            }
            catch (Exception)
            {
            }
            return "";
        }

        public static bool FileExistsWithSHA1(string FileName, String Hash)
        {
            if (File.Exists(FileName))
            {
                string sha1 = GetFileSHA1(FileName);
                if (sha1 == Hash)
                {
                    return true;
                }
            }
            return false;
        }

        const int CurrentPatcherVersion = 2;

        [STAThread]
        static void Main(string[] args)
        {
            
            foreach(var arg in args)
            {
                if(arg == "-offline")
                {
                    System.Windows.Forms.OpenFileDialog file_browser = new System.Windows.Forms.OpenFileDialog();
                    file_browser.Filter = "DLLs (*.dll)|*.dll|All files (*.*)|*.*";
                    file_browser.CheckFileExists = true;
                    if(file_browser.ShowDialog() == System.Windows.Forms.DialogResult.OK)
                    {
                        foreach (var path in file_browser.SafeFileNames)
                        {
                            Inject(path);

                            string caption = "DLL Injected";
                            string message = $"The DLL {path} was injected into MCC.";
                            System.Windows.MessageBox.Show(message, caption, MessageBoxButton.OK);
                        }
                    }
                    return;
                }
            }

            const string patch_file_url_root = "https://github.com/Scaless/HRPatcherReleases/raw/main/";
            const string manifest_url = "https://raw.githubusercontent.com/Scaless/HRPatcherReleases/main/manifest.json";

            // Get Running MCC Version
            string running_mcc = GetRunningMCCVersion();
            if (string.IsNullOrWhiteSpace(running_mcc))
            {
                string caption = "Couldn't Determine MCC Version";
                string message = "Failed to get the version of MCC. Is MCC running with EAC disabled? No patches were applied.";
                System.Windows.MessageBox.Show(message, caption, MessageBoxButton.OK);
                return;
            }

            // HTTP Get current manifest
            HttpClient client = new HttpClient();
            HttpResponseMessage manifest_response = client.GetAsync(manifest_url).Result;
            if (!manifest_response.IsSuccessStatusCode)
            {
                string caption = "HRPatcher Manifest Could Not Be Downloaded";
                string message = "Couldn't download the patch manifest. No patches were applied.";
                System.Windows.MessageBox.Show(message, caption, MessageBoxButton.OK);
                return;
            }

            // Parse Manifest
            MCCPatcherVersions PatcherVersions = JsonConvert.DeserializeObject<MCCPatcherVersions>(manifest_response.Content.ReadAsStringAsync().Result);
            if (PatcherVersions == null)
            {
                string caption = "HRPatcher Manifest Corrupted";
                string message = "Failed to read the patch manifest. No patches were applied.";
                System.Windows.MessageBox.Show(message, caption, MessageBoxButton.OK);
                return;
            }

            // Check patcher version
            if (PatcherVersions.current_patcher_version != CurrentPatcherVersion)
            {
                string caption = "HRPatcher Update Available";
                string message = "A new version of HRPatcher is available. Click Yes to open a link to the new version in your browser," +
                    " No to attempt patching with the current version, or Cancel to stop patching.";
                var update_user_result = System.Windows.MessageBox.Show(message, caption, MessageBoxButton.YesNoCancel, MessageBoxImage.Information, MessageBoxResult.Yes);
                if (update_user_result == MessageBoxResult.Yes)
                {
                    Process.Start(PatcherVersions.current_patcher_link);
                    return;
                }
                if (update_user_result == MessageBoxResult.Cancel)
                {
                    return;
                }
            }

            // Check if this version is in the manifest
            var found_version = PatcherVersions.versions.FirstOrDefault(x => x.version == running_mcc);
            if (found_version == null)
            {
                string caption = $"No Patcher Available For MCC Version {running_mcc}";
                string message = $"There are no patches available for the version of MCC currently running ({running_mcc}). No patches were applied.";
                System.Windows.MessageBox.Show(message, caption, MessageBoxButton.OK);
                return;
            }

            // Check if there is an existing file that matches the manifest
            if(!FileExistsWithSHA1(found_version.file, found_version.sha1))
            {
                // Download the patch file if it doesnt exist
                string patch_url = patch_file_url_root + found_version.file;
                HttpResponseMessage patch_response = client.GetAsync(patch_url).Result;
                if (!patch_response.IsSuccessStatusCode)
                {
                    string caption = "HRPatcher Patch File Could Not Be Downloaded";
                    string message = "Couldn't download the patch file. No patches were applied.";
                    System.Windows.MessageBox.Show(message, caption, MessageBoxButton.OK);
                    return;
                }
                try
                {
                    using (Stream output = File.OpenWrite(found_version.file))
                        patch_response.Content.CopyToAsync(output).Wait();
                }
                catch (Exception e)
                {
                    string caption = "HRPatcher Patch File Failed To Save";
                    string message = $"Couldn't save the patch file to disk. No patches were applied. ::: {e}";
                    System.Windows.MessageBox.Show(message, caption, MessageBoxButton.OK);
                    return;
                }
            }

            // Final Validation that file was saved correctly
            if (!File.Exists(found_version.file))
            {
                string caption = "HRPatcher Patch File Failed To Save";
                string message = "Couldn't save the patch file to disk. No patches were applied.";
                System.Windows.MessageBox.Show(message, caption, MessageBoxButton.OK);
                return;
            }

            // Inject
            if (Inject(found_version.file))
            {
                string caption = "HRPatcher: Patch Applied!";
                string message = $"Patching was successful. Patches will be active until you close out of MCC completely.";
                System.Windows.MessageBox.Show(message, caption, MessageBoxButton.OK);
                return;
            } 
            else
            {
                string caption = "HRPatcher: Inject Failed";
                string message = $"Injection failed. No patches were applied.";
                System.Windows.MessageBox.Show(message, caption, MessageBoxButton.OK);
                return;
            }
        }
    }
}
