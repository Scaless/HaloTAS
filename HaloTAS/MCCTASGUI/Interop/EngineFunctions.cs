using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace MCCTASGUI.Interop
{
    public enum Halo1Cheat
    {
        DeathlessPlayer,
        BumpPosession,
        SuperJump,
        ReflectDamageHits,
        Medusa,
        OneShotKill,
        BottomlessClip,

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
        Sputnik,
        BootsOffTheGround
    }

    public enum Halo2Skull
    {
        // Scoring
        Anger,
        Assassins,
        BlackEye,
        Blind,
        Catch,
        EyePatch,
        Famine,
        Fog,
        Iron,
        Jacked,
        MasterBlaster,
        Mythic,
        Recession,
        SoAngry,
        Streaking,
        Swarm,
        ThatsJustWrong,
        TheyComeBack,
        Thunderstorm,

        // Non-scoring
        Bandanna,
        BondedPair,
        Boom,
        Envy,
        Feather,
        Ghost,
        GruntBirthdayParty,
        GruntFuneral,
        IWHBYD,
        Malfunction,
        Pinata,
        ProphetBirthdayParty,
        Scarab,
        Sputnik
    }

    public class Halo1Status
    {
        public bool[] CheatsEnabled;

        public Halo1Status()
        {
            CheatsEnabled = new bool[EnumUtils<Halo1Cheat>.Count];
        }
    }

    public class Halo2Status
    {
        public bool[] SkullsEnabled;

        public Halo2Status()
        {
            SkullsEnabled = new bool[EnumUtils<Halo2Skull>.Count];
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

        public static async Task SetCheatEnabled(Halo1Cheat cheat, bool enabled)
        {
            InteropRequest request = new InteropRequest();
            request.header.RequestType = InteropRequestType.Halo1SetCheatEnabled;
            request.header.RequestPayloadSize = Marshal.SizeOf(typeof(Halo1SetCheatEnabledRequest));

            var payload = new Halo1SetCheatEnabledRequest();
            payload.Cheat = (int)cheat;
            payload.Enabled = enabled;
            request.requestData = TASInterop.MarshalObjectToArray(payload);

            var response = await TASInterop.MakeRequestAsync(request);

            if (response?.header.ResponseType != InteropResponseType.Success)
            {
                // Something went wrong
            }
        }
    }

    class H2EngineFunctions
    {
        public static async Task SetSkullEnabled(Halo2Skull skull, bool enabled)
        {
            InteropRequest request = new InteropRequest();
            request.header.RequestType = InteropRequestType.Halo2SetCheatEnabled;
            request.header.RequestPayloadSize = Marshal.SizeOf(typeof(Halo2SetSkullEnabledRequest));

            var payload = new Halo2SetSkullEnabledRequest();
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
