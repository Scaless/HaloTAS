using System;
using System.Collections.Generic;
using System.Collections.Concurrent;
using System.Text;
using System.IO;
using System.IO.Pipes;
using System.Threading;
using System.Runtime.InteropServices;
using System.Drawing;
using System.Runtime.CompilerServices;
using System.Threading.Tasks;

namespace MCCTASGUI
{
    public enum InteropRequestType
    {
        Ping = 0,
        GetDLLInformation = 1,
        SetCameraDetails = 2,

        Invalid = -1
    }

    public enum InteropResponseType
    {
        Success = 0,

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

    public struct TASStatus
    {
        public bool Connected { get; set; }
        public bool KillConnection { get; set; }
    }

    public class InteropRequest
    {
        public InteropRequestHeader header;
        public byte[] requestData;
        public TaskCompletionSource<bool> Completed;
        public InteropResponse response;

        public InteropRequest()
        {
            header = new InteropRequestHeader();
            Completed = new TaskCompletionSource<bool>();
            response = new InteropResponse();
            requestData = null;
        }
    }

    public struct InteropResponse
    {
        public InteropResponseHeader header;
        public byte[] responseData;
    }

    public class TASInterop
    {
        private Thread PipeServerThread;
        private TASStatus CurrentStatus;

        private static ConcurrentQueue<InteropRequest> RequestQueue = new ConcurrentQueue<InteropRequest>();

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

        public static async Task<InteropResponse> MakeRequestAsync(InteropRequest request)
        {
            RequestQueue.Enqueue(request);
            await request.Completed.Task;
            return request.response;
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
                        if (status.KillConnection)
                        {
                            status.KillConnection = false;
                            break;
                        }

                        status.Connected = true;
                        Thread.Sleep(20);
                        try
                        {
                            InteropRequest request;
                            while (RequestQueue.TryDequeue(out request))
                            {
                                var headerData = MarshalObjectToArray(request.header);
                                ids.WriteData(headerData);
                                
                                if(request.requestData != null)
                                {
                                    ids.WriteData(request.requestData);
                                }

                                ids.FlushRequest();

                                request.response = ids.GetResponse();
                                request.Completed.SetResult(true);
                            }
                        }   
                        catch(Exception e) {
                            var f = e.ToString();
                        }
                    }

                    status.Connected = false;
                }
            }

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

        public unsafe InteropResponse GetResponse()
        {
            InteropResponse response = new InteropResponse();
            response.header = new InteropResponseHeader();

            byte[] responseHeaderArray = new byte[sizeof(InteropResponseHeader)];
            DataStream.Read(responseHeaderArray, 0, responseHeaderArray.Length);

            TASInterop.MarshalArrayToObject(ref response.header, responseHeaderArray);

            response.responseData = new byte[response.header.ResponsePayloadSize];
            if (response.header.ResponsePayloadSize > 0)
            {
                DataStream.Read(response.responseData, 0, response.header.ResponsePayloadSize);
            }

            return response;
        }
    }

}


