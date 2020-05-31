using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.IO.Pipes;
using System.Threading;

namespace MCCTASGUI
{
    class TASInterop
    {
        Thread PipeServerThread;

        public TASInterop()
        {
            PipeServerThread = new Thread(ServerThread);
            PipeServerThread.IsBackground = true;
            PipeServerThread.Start();
        }

        private static void ServerThread(object data)
        {
            using (NamedPipeServerStream pipeServer = new NamedPipeServerStream("MCCTAS", PipeDirection.InOut, 1, PipeTransmissionMode.Message))
            {
                while (true)
                {
                    pipeServer.WaitForConnection();
                    StreamString ss = new StreamString(pipeServer);
                    string filename = ss.ReadString();
                    if (!string.IsNullOrEmpty(filename))
                    {
                        System.Diagnostics.Debug.WriteLine(filename);
                    }
                    pipeServer.Disconnect();
                }
            }
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
            int len = 0;

            len = ioStream.ReadByte();

            if(len == -1)
            {
                return string.Empty;
            }

            //len = ioStream.ReadByte() * 256;
            //len += ioStream.ReadByte();
            byte[] inBuffer = new byte[len];
            ioStream.Read(inBuffer, 0, len);

            return streamEncoding.GetString(inBuffer);
        }

        //public int WriteString(string outString)
        //{
        //    byte[] outBuffer = streamEncoding.GetBytes(outString);
        //    int len = outBuffer.Length;
        //    if (len > UInt16.MaxValue)
        //    {
        //        len = (int)UInt16.MaxValue;
        //    }
        //    ioStream.WriteByte((byte)(len / 256));
        //    ioStream.WriteByte((byte)(len & 255));
        //    ioStream.Write(outBuffer, 0, len);
        //    ioStream.Flush();

        //    return outBuffer.Length + 2;
        //}
    }

}


