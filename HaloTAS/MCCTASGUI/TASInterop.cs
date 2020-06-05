using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.IO.Pipes;
using System.Threading;
using System.Runtime.InteropServices;
using System.Drawing;
using System.Runtime.CompilerServices;

namespace MCCTASGUI
{
    public enum InteropRequestType
    {
        Ping = 0,
        GetDLLInformation = 1,

        Invalid = -1
    }

    public enum InteropResponseType
    {
        Pong = 0,

        DLLInformationFound = 1,
        DLLInformationNotFound = 2,

        Invalid = -1
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

    public struct TASStatus
    {
        public bool Connected { get; set; }
        public bool KillConnection { get; set; }
    }

    public class TASInterop
    {
        private Thread PipeServerThread;
        private TASStatus CurrentStatus;

        public TASInterop()
        {
            PipeServerThread = new Thread(() => PipeClientThread(ref CurrentStatus));
            PipeServerThread.IsBackground = true;
            PipeServerThread.Start();
        }

        public TASStatus GetCurrentStatus()
        {
            return CurrentStatus;
        }

        public void KillConnection()
        {
            CurrentStatus.KillConnection = true;
        }

        private static void PipeClientThread(ref TASStatus status)
        {
            while (true)
            {
                using (NamedPipeClientStream pipeClient = new NamedPipeClientStream(".", "MCCTAS", PipeDirection.InOut))
                {
                    pipeClient.Connect();

                    InteropDataStream ids = new InteropDataStream(pipeClient);
                    
                    while (pipeClient.IsConnected)
                    {
                        status.Connected = true;
                        Thread.Sleep(1000);
                        try
                        {
                            InteropRequestHeader ireq = new InteropRequestHeader();
                            ireq.RequestType = InteropRequestType.GetDLLInformation;
                            ireq.RequestPayloadSize = 0;

                            var headerData = InteropDataStream.MarshalObjectToArray(ireq);
                            ids.WriteData(headerData);

                            DLLInformationRequest ireqPayload = new DLLInformationRequest();
                            ireqPayload.DLLName = "halo1.dll";
                            var payloadData = InteropDataStream.MarshalObjectToArray(ireqPayload);
                            ids.WriteData(payloadData);

                            ids.FlushRequest();

                            ids.HandleResponse();
                           
                            if (status.KillConnection)
                            {
                                status.KillConnection = false;
                                pipeClient.Dispose();
                            }
                        }
                        catch (Exception e)
                        {
                            var f = e.ToString();
                        }

                    }

                    status.Connected = false;
                }
            }

        }

    }

    public class InteropDataStream
    {
        private Stream DataStream;
        private byte[] WriteBuffer;
        private int WrittenBytes;

        public InteropDataStream(Stream stream)
        {
            this.DataStream = stream;
            WriteBuffer = new byte[4 * 1024 * 1024];
        }

        public void WriteData(byte[] data)
        {
            Buffer.BlockCopy(data, 0, WriteBuffer, WrittenBytes, data.Length);
            WrittenBytes += data.Length;
        }

        public void FlushRequest()
        {
            DataStream.Write(WriteBuffer, 0, WrittenBytes);
            DataStream.Flush();
            WrittenBytes = 0;
        }

        public static unsafe void MarshalArrayToObject<T>(ref T obj, byte[] rawObjectData)
        {
            int size = Marshal.SizeOf(obj);
            IntPtr ptr = Marshal.AllocHGlobal(size);
            Marshal.Copy(rawObjectData, 0, ptr, size);
            obj = (T)Marshal.PtrToStructure(ptr, obj.GetType());
            Marshal.FreeHGlobal(ptr);
        }

        public static unsafe byte[] MarshalObjectToArray<T>(T obj)
        {
            int size = Marshal.SizeOf(obj);
            byte[] arr = new byte[size];

            IntPtr ptr = Marshal.AllocHGlobal(size);
            Marshal.StructureToPtr(obj, ptr, true);
            Marshal.Copy(ptr, arr, 0, size);
            Marshal.FreeHGlobal(ptr);
            return arr;
        }

        public unsafe void HandleResponse()
        {
            InteropResponseHeader response;

            byte[] responseHeaderArray = new byte[sizeof(InteropResponseHeader)];
            DataStream.Read(responseHeaderArray, 0, responseHeaderArray.Length);

            response = new InteropResponseHeader();

            MarshalArrayToObject(ref response, responseHeaderArray);

            switch (response.ResponseType)
            {
                case InteropResponseType.Pong:
                    System.Diagnostics.Debug.WriteLine($"{response.ResponseType} - {response.ResponsePayloadSize}");
                    break;
                case InteropResponseType.DLLInformationFound:
                    if (response.ResponsePayloadSize > 0)
                    {
                        DLLInformationResponse responsePayload = new DLLInformationResponse();
                        byte[] responsePayloadBytes = new byte[response.ResponsePayloadSize];
                        DataStream.Read(responsePayloadBytes, 0, response.ResponsePayloadSize);

                        MarshalArrayToObject(ref responsePayload, responsePayloadBytes);

                        System.Diagnostics.Debug.WriteLine($"0x{responsePayload.DLLAddress:X}");
                    }
                    break;
                default:
                    break;
            }

        }
    }

}


