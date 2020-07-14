using System;
using System.Threading.Tasks;

namespace MCCTASGUI.Interop
{
    public class TASStatus
    {
        public bool Connected { get; set; }

        public bool H1DLLLoaded { get; set; }
        public bool H2DLLLoaded { get; set; }
        public bool H3DLLLoaded { get; set; }
        public bool ReachDLLLoaded { get; set; }
        public bool ODSTDLLLoaded { get; set; }
        public bool H4DLLLoaded { get; set; }

        public Halo1Status Halo1 { get; set; }

        public TASStatus()
        {
            Halo1 = new Halo1Status();
        }
    }

    class GlobalInterop
    {
        public static TASStatus Status { get; protected set; }

        public static async Task RefreshStatus()
        {
            InteropRequest request = new InteropRequest();
            request.header.RequestType = InteropRequestType.GetGameInformation;
            var response = await TASInterop.MakeRequestAsync(request);

            Status = new TASStatus();
            if (response?.header.ResponseType == InteropResponseType.Success)
            {
                Status.Connected = true;

                GetGameInformationResponse responseData = new GetGameInformationResponse();
                TASInterop.MarshalArrayToObject(ref responseData, response?.responseData);
                Status.H1DLLLoaded = responseData.Halo1Loaded;
                Status.H2DLLLoaded = responseData.Halo2Loaded;
                Status.H3DLLLoaded = responseData.Halo3Loaded;
                Status.ODSTDLLLoaded = responseData.ODSTLoaded;
                Status.ReachDLLLoaded = responseData.ReachLoaded;
                Status.H4DLLLoaded = responseData.Halo4Loaded;

                if (Status.H1DLLLoaded)
                {
                    Status.Halo1.CheatsEnabled = responseData.Halo1.CheatsEnabled;
                }
            }
            else
            {
                Status.Connected = false;
            }
        }
    }
}
