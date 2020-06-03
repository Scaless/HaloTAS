using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.IO.Pipes;
using System.Threading;

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
        GetDLLInformation = 1,

        Invalid = -1
    }

    public struct InteropRequest
    {
        public InteropRequestType RequestType { get; set; }
        public int RequestPayloadSize { get; set; }
        public byte[] RequestPayload { get; set; }
    }

    public struct InteropResponse
    {
        public InteropResponseType ResponseType { get; set; }
        public int ResponsePayloadSize { get; set; }
        public byte[] ResponsePayload { get; set; }
    }

    public struct TASStatus
    {
        public bool Connected { get; set; }
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

        private static void PipeClientThread(ref TASStatus status)
        {
            while (true)
            {
                using (NamedPipeClientStream pipeClient = new NamedPipeClientStream(".", "MCCTAS", PipeDirection.InOut))
                {
                    pipeClient.Connect();

                    while (pipeClient.IsConnected)
                    {
                        status.Connected = true;
                        Thread.Sleep(1000);
                        try
                        {
                            InteropDataStream ids = new InteropDataStream(pipeClient);

                            InteropRequest ireq = new InteropRequest();
                            ireq.RequestType = InteropRequestType.Ping;
                            ireq.RequestPayloadSize = 0;
                            ireq.RequestPayload = new byte[0];

                            ids.SendRequest(ireq);

                            var irep = ids.ReadResponse();

                            if (irep.HasValue)
                            {
                                System.Diagnostics.Debug.WriteLine(irep.Value.ResponseType);
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
        private Stream stream;

        public InteropDataStream(Stream stream)
        {
            this.stream = stream;
        }

        public void SendRequest(InteropRequest request)
        {
            stream.WriteByte((byte)request.RequestType);

            byte[] requestSizeBytes = BitConverter.GetBytes(request.RequestPayloadSize);
            stream.Write(requestSizeBytes, 0, 4);

            stream.Write(request.RequestPayload);

            stream.Flush();
        }

        public InteropResponse? ReadResponse()
        {
            InteropResponse response;

            InteropResponseType responseCode = (InteropResponseType)stream.ReadByte();
            if (responseCode == InteropResponseType.Invalid)
            {
                return null;
            }

            response = new InteropResponse();
            response.ResponseType = responseCode;

            byte[] responseSizeBytes = new byte[4];
            stream.Read(responseSizeBytes, 0, 4);
            response.ResponsePayloadSize = BitConverter.ToInt32(responseSizeBytes, 0);

            if (response.ResponsePayloadSize > 0)
            {
                response.ResponsePayload = new byte[response.ResponsePayloadSize];
                stream.Read(response.ResponsePayload, 0, response.ResponsePayloadSize);
            }

            return response;
        }
    }

    // Defines the data protocol for reading and writing strings on our stream
    public class StreamString
    {
        private Stream ioStream;
        private ASCIIEncoding streamEncoding;

        public StreamString(Stream ioStream)
        {
            this.ioStream = ioStream;
            streamEncoding = new ASCIIEncoding();
        }

        public string ReadString()
        {
            int len = ioStream.ReadByte();
            if (len == -1)
            {
                return string.Empty;
            }

            byte[] inBuffer = new byte[len];
            ioStream.Read(inBuffer, 0, len);

            return streamEncoding.GetString(inBuffer);
        }

        public int WriteString(string outString)
        {
            ioStream.WriteByte(128);
            ioStream.Flush();
            return 1;

            //byte[] outBuffer = streamEncoding.GetBytes(outString);
            //int len = outBuffer.Length;
            //if (len > UInt16.MaxValue)
            //{
            //    len = (int)UInt16.MaxValue;
            //}
            //ioStream.WriteByte((byte)(len / 256));
            //ioStream.WriteByte((byte)(len & 255));
            //ioStream.Write(outBuffer, 0, len);
            //ioStream.Flush();

            //return outBuffer.Length + 2;
        }
    }

}


