using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace TASInterface
{
    public enum KEYS_INTERNAL
    {
        ESC = 0,
        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        PrntScrn,
        ScrollLock,
        PauseBreak,
        Tilde,
        NUM_1,
        NUM_2,
        NUM_3,
        NUM_4,
        NUM_5,
        NUM_6,
        NUM_7,
        NUM_8,
        NUM_9,
        NUM_0,
        Minus,
        Equal,
        Backspace,
        Tab,
        Q,
        W,
        E,
        R,
        T,
        Y,
        U,
        I,
        O,
        P,
        LeftBracket,
        RightBracket,
        Pipe,
        CapsLock,
        A,
        S,
        D,
        F,
        G,
        H,
        J,
        K,
        L,
        Colon,
        Quote,
        Enter,
        LShift,
        Z,
        X,
        C,
        V,
        B,
        N,
        M,
        Comma,
        Period,
        ForwardSlash,
        RShift,
        LCtrl,
        LWin,
        LAlt,
        Space,
        RAlt,
        RWin,
        KeyThatLiterallyNoOneHasEverUsed,
        RightCtrl,
        Up,
        Down,
        Left,
        Right
    };

    struct InputMoment
    {
        public byte[] InputBuf;
        public float MouseUpDown, MouseLeftRight;
        public byte LeftMouseDown, RightMouseDown;
    };

    [StructLayout(LayoutKind.Explicit)]
    struct InputKey
    {
        [FieldOffset(0)]
        public UInt64 fullKey;

        [FieldOffset(0)]
        public int sublevel;
        [FieldOffset(4)]
        public int inputCounter;
    }

    struct GameState
    {
        public string LastStatus;
        public bool Running;
        public string MapString;
        public int InputCounter;
        public int SubLevel;
        public uint FrameCounter;
        public uint Counter;
        public Dictionary<UInt64, InputMoment> FullInputMap;
    }

    class HaloGameInterface
    {
        public bool IsGameRunning { get; private set; }

        IntPtr _readHandle, _writeHandle;

        byte[] inputBuffer = new byte[104];

        const int PROCESS_WM_READ = 0x0010;

        [DllImport("kernel32.dll")]
        static extern IntPtr OpenProcess(int dwDesiredAccess, bool bInheritHandle, int dwProcessId);
        [DllImport("kernel32.dll")]
        static extern bool ReadProcessMemory(int hProcess, int lpBaseAddress, byte[] lpBuffer, int dwSize, ref int lpNumberOfBytesRead);
        [DllImport("kernel32.dll", SetLastError = true)]
        static extern bool WriteProcessMemory(int hProcess, int lpBaseAddress, byte[] lpBuffer, int dwSize, ref int lpNumberOfBytesWritten);

        public bool IsRunning()
        {
            var processes = Process.GetProcessesByName("halo");
            if (processes.Length > 0) {
                var process = processes[0];
                _readHandle = OpenProcess(PROCESS_WM_READ, false, process.Id);
                _writeHandle = OpenProcess(0x1F0FFF, false, process.Id);
                IsGameRunning = true;
            }
            else
            {
                IsGameRunning = false;
                _readHandle = IntPtr.Zero;
                _writeHandle = IntPtr.Zero;
            }
            
            return IsGameRunning;
        }

        public void ReadMemory(int addr, ref byte[] buffer)
        {
            if (_readHandle == IntPtr.Zero) return;
            int bytesRead = 0;
            ReadProcessMemory((int)_readHandle, addr, buffer, buffer.Length, ref bytesRead);
        }

        public void WriteMemory(int addr, byte[] writeBytes)
        {
            if (_writeHandle == IntPtr.Zero) return;
            int bytesWritten = 0;
            WriteProcessMemory((int)_writeHandle, addr, writeBytes, writeBytes.Length, ref bytesWritten);
        }
    }
}
