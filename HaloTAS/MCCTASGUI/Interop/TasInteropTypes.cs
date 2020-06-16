using System;
using System.Runtime.InteropServices;

namespace MCCTASGUI.Interop
{
    public enum InteropRequestType
    {
        Invalid = -1,
        Ping = 0,
        GetDLLInformation = 1,
        SetCameraDetails = 2,
        ExecuteCommand = 3,
        GetGameInformation = 4,
        Halo1SetSkullEnabled = 5,
    }

    public enum InteropResponseType
    {
        InvalidRequest = -2,
        Failure = -1,
        Success = 0,
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct InteropRequestHeader
    {
        [MarshalAs(UnmanagedType.I4)]
        public InteropRequestType RequestType;
        [MarshalAs(UnmanagedType.I4)]
        public int RequestPayloadSize;
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct InteropResponseHeader
    {
        [MarshalAs(UnmanagedType.I4)]
        public InteropResponseType ResponseType;
        [MarshalAs(UnmanagedType.I4)]
        public int ResponsePayloadSize;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public unsafe struct DLLInformationRequest
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string DLLName;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public unsafe struct DLLInformationResponse
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string DLLName;
        [MarshalAs(UnmanagedType.U8)]
        public UInt64 DLLAddress;
        [MarshalAs(UnmanagedType.U8)]
        public UInt64 EntryPoint;
        [MarshalAs(UnmanagedType.U8)]
        public UInt64 ImageSize;
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct SetCameraDetailsRequest
    {
        [MarshalAs(UnmanagedType.R4)]
        public float PositionX;
        [MarshalAs(UnmanagedType.R4)]
        public float PositionY;
        [MarshalAs(UnmanagedType.R4)]
        public float PositionZ;
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct ExecuteCommandRequest
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string Command;
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct Halo1GameInformation
    {
        [MarshalAs(UnmanagedType.I4)]
        public int Tick;

        // Skulls
        [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.Bool, SizeConst = 22)]
        public bool[] SkullsEnabled;
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct Halo1SetSkullEnabledRequest
    {
        [MarshalAs(UnmanagedType.I4)]
        public int Skull;

        [MarshalAs(UnmanagedType.Bool)]
        public bool Enabled;
    }

    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct GetGameInformationResponse
    {
        // All Game Information
        [MarshalAs(UnmanagedType.Bool)]
        public bool Halo1Loaded;
        [MarshalAs(UnmanagedType.Bool)]
        public bool Halo2Loaded;
        [MarshalAs(UnmanagedType.Bool)]
        public bool Halo3Loaded;
        [MarshalAs(UnmanagedType.Bool)]
        public bool ODSTLoaded;
        [MarshalAs(UnmanagedType.Bool)]
        public bool ReachLoaded;
        [MarshalAs(UnmanagedType.Bool)]
        public bool Halo4Loaded;

        // Halo 1 Information
        public Halo1GameInformation Halo1;
    }

}
