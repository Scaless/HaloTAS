using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace MCCTASGUI.Interop
{
    public enum Halo1Skull
    {
        Anger,
        Blind,
        BlackEye,
        Catch,
        EyePatch,
        Famine,
        Fog,
        Foreign,
        Iron,
        Mythic,
        Recession,
        ThatsJustWrong,
        Thunderstorm,
        ToughLuck,
        Bandana,
        Boom,
        Ghost,
        GruntBirthdayParty,
        GruntFuneral,
        Malfunction,
        Pinata,
        Sputnik
    }

    public class Halo1Status
    {
        public bool[] SkullsEnabled;

        public Halo1Status()
        {
            SkullsEnabled = new bool[22];
        }
    }

    class H1EngineFunctions
    {
        public static async Task ExecuteCommand(string command)
        {
            if (string.IsNullOrWhiteSpace(command))
                return;

            InteropRequest request = new InteropRequest();
            request.header.RequestType = InteropRequestType.ExecuteCommand;
            request.header.RequestPayloadSize = Marshal.SizeOf(typeof(ExecuteCommandRequest));

            var payload = new ExecuteCommandRequest();
            payload.Command = command;
            request.requestData = TASInterop.MarshalObjectToArray(payload);

            var response = await TASInterop.MakeRequestAsync(request);

            if(response?.header.ResponseType != InteropResponseType.Success)
            {
                // Something went wrong
            }
        }

        public static async Task SetSkullEnabled(Halo1Skull skull, bool enabled)
        {
            InteropRequest request = new InteropRequest();
            request.header.RequestType = InteropRequestType.Halo1SetSkullEnabled;
            request.header.RequestPayloadSize = Marshal.SizeOf(typeof(Halo1SetSkullEnabledRequest));

            var payload = new Halo1SetSkullEnabledRequest();
            payload.Skull = (int)skull;
            payload.Enabled = enabled;
            request.requestData = TASInterop.MarshalObjectToArray(payload);

            var response = await TASInterop.MakeRequestAsync(request);

            if (response?.header.ResponseType != InteropResponseType.Success)
            {
                // Something went wrong
            }
        }
    }
}
