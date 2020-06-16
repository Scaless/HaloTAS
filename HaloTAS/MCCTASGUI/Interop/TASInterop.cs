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

namespace MCCTASGUI.Interop
{
    public class InteropRequest
    {
        public InteropRequestHeader header;
        public byte[] requestData;
        public InteropResponse response;

        public InteropRequest()
        {
            header = new InteropRequestHeader();
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
        public static async Task<InteropResponse?> MakeRequestAsync(InteropRequest request)
        {
            using (NamedPipeClientStream pipeClient = new NamedPipeClientStream(".", "MCCTAS", PipeDirection.InOut))
            {
                try
                {
                    await pipeClient.ConnectAsync(500);
                }
                catch (TimeoutException e)
                {
                    return null;
                }
                if (pipeClient.IsConnected)
                {
                    try
                    {
                        InteropDataStream ids = new InteropDataStream(pipeClient);
                        var headerData = MarshalObjectToArray(request.header);
                        ids.WriteData(headerData);

                        if (request.requestData != null)
                        {
                            ids.WriteData(request.requestData);
                        }

                        await ids.FlushRequest();

                        return await ids.GetResponse();
                    }
                    catch (Exception e)
                    {
                        return null;
                    }
                }
            }

            return null;
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

        private class InteropDataStream
        {
            private Stream DataStream;
            private byte[] WriteBuffer;
            private int WrittenBytes;

            public InteropDataStream(Stream stream)
            {
                DataStream = stream;
                WriteBuffer = new byte[4 * 1024 * 1024];
            }

            public void WriteData(byte[] data)
            {
                Buffer.BlockCopy(data, 0, WriteBuffer, WrittenBytes, data.Length);
                WrittenBytes += data.Length;
            }

            public async Task FlushRequest()
            {
                await DataStream.WriteAsync(WriteBuffer, 0, WrittenBytes);
                DataStream.Flush();
                WrittenBytes = 0;
            }

            public async Task<InteropResponse> GetResponse()
            {
                InteropResponse response = new InteropResponse();
                response.header = new InteropResponseHeader();

                byte[] responseHeaderArray = new byte[Marshal.SizeOf(response.header)];
                await DataStream.ReadAsync(responseHeaderArray, 0, responseHeaderArray.Length);

                MarshalArrayToObject(ref response.header, responseHeaderArray);

                response.responseData = new byte[response.header.ResponsePayloadSize];
                if (response.header.ResponsePayloadSize > 0)
                {
                    await DataStream.ReadAsync(response.responseData, 0, response.header.ResponsePayloadSize);
                }

                return response;
            }
        }
    }



}


