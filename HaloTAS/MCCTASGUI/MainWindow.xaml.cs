using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;

namespace MCCTASGUI
{
    public class Halo1Skull
    {
        public string Name { get; set; }
        public bool Enabled { get; set; }
    }

    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private TASInterop TASInterop;
        private LiveMapDataViewer liveMapDataViewer = null;
        private AboutWindow aboutWindow = null;
        private TASInputEditor inputEditorWindow = null;
        private H1CinematicCameraWindow H1CinematicCameraWindow = null;

        public List<Halo1Skull> Halo1Skulls = new List<Halo1Skull>() {
            new Halo1Skull(){Name = "Anger", Enabled = false},
            new Halo1Skull(){Name = "Blind", Enabled = false},
            new Halo1Skull(){Name = "Black Eye", Enabled = false},
            new Halo1Skull(){Name = "Catch", Enabled = false},
            new Halo1Skull(){Name = "Eye Patch", Enabled = false},
        };

        public MainWindow()
        {
            InitializeComponent();

            Application.Current.MainWindow = this;
            Application.Current.ShutdownMode = ShutdownMode.OnMainWindowClose;

            TASInterop = new TASInterop();

            tblkStatusConnected.Text = Directory.GetCurrentDirectory();
        }

        private async void Window_Loaded(object sender, RoutedEventArgs e)
        {
            await UpdatePingTimer();
        }

        private async Task UpdatePingTimer()
        {
            while (true)
            {
                await Task.Delay(1000);

                await Dispatcher.Invoke(async () =>
                {
                    var Status = TASInterop.GetCurrentStatus();

                    InteropRequest request = new InteropRequest();
                    request.header.RequestType = InteropRequestType.GetGameInformation;
                    var response = await TASInterop.MakeRequestAsync(request);

                    tblkStatusConnected.Text = Status.Connected ? "CONNECTED" : "NOT CONNECTED";

                    if (response.header.ResponseType == InteropResponseType.Success)
                    {
                        GetGameInformationResponse responseData = new GetGameInformationResponse();
                        TASInterop.MarshalArrayToObject(ref responseData, response.responseData);
                        Status.H1DLLLoaded = responseData.Halo1Loaded;
                        Status.H2DLLLoaded = responseData.Halo2Loaded;
                        Status.H3DLLLoaded = responseData.Halo3Loaded;
                        Status.ODSTDLLLoaded = responseData.ODSTLoaded;
                        Status.ReachDLLLoaded = responseData.ReachLoaded;
                        Status.H4DLLLoaded = responseData.Halo4Loaded;
                    }

                    iconHalo1Loaded.Fill = Status.H1DLLLoaded ? Brushes.Green : Brushes.Red;
                    iconHalo2Loaded.Fill = Status.H2DLLLoaded ? Brushes.Green : Brushes.Red;
                    iconHalo3Loaded.Fill = Status.H3DLLLoaded ? Brushes.Green : Brushes.Red;
                    iconHaloReachLoaded.Fill = Status.ReachDLLLoaded ? Brushes.Green : Brushes.Red;
                });
            }
        }

        private void MenuOpenMapData(object sender, RoutedEventArgs e)
        {
            if (liveMapDataViewer == null)
            {
                liveMapDataViewer = new LiveMapDataViewer();
                liveMapDataViewer.Closed += LiveMapDataViewer_Closed;
                liveMapDataViewer.Show();
            } 
            else
            {
                FocusWindow(liveMapDataViewer);
            }
        }
        private void LiveMapDataViewer_Closed(object sender, EventArgs e)
        {
            liveMapDataViewer = null;
        }

        private void MenuOpenH1CinematicCamera(object sender, RoutedEventArgs e)
        {
            if(H1CinematicCameraWindow == null)
            {
                H1CinematicCameraWindow = new H1CinematicCameraWindow();
                H1CinematicCameraWindow.Closed += H1CinematicCamera_Closed;
                H1CinematicCameraWindow.Show();
            } 
            else
            {
                FocusWindow(H1CinematicCameraWindow);
            }
        }
        private void H1CinematicCamera_Closed(object sender, EventArgs e)
        {
            H1CinematicCameraWindow = null;
        }

        private void MenuOpenAbout(object sender, RoutedEventArgs e)
        {
            if (aboutWindow == null)
            {
                aboutWindow = new AboutWindow();
                aboutWindow.Closed += AboutWindow_Closed;
                aboutWindow.Show();
            }
            else
            {
                FocusWindow(aboutWindow);
            }
        }
        private void AboutWindow_Closed(object sender, EventArgs e)
        {
            aboutWindow = null;
        }

        private void MenuOpenInputEditor(object sender, RoutedEventArgs e)
        {
            if (inputEditorWindow == null)
            {
                inputEditorWindow = new TASInputEditor();
                inputEditorWindow.Closed += InputEditorWindow_Closed;
                inputEditorWindow.Show();
            }
            else
            {
                FocusWindow(inputEditorWindow);
            }
        }
        private void InputEditorWindow_Closed(object sender, EventArgs e)
        {
            inputEditorWindow = null;
        }

        private void FocusWindow(Window window)
        {
            if (window == null)
                return;

            if (window.WindowState == WindowState.Minimized)
                window.WindowState = WindowState.Normal;
            window.Focus();
        }

        private void btnInject_Click(object sender, RoutedEventArgs e)
        {
            DLLInjector.Inject();
        }

        private void btnKillConnection_Click(object sender, RoutedEventArgs e)
        {
            TASInterop.KillConnection();
        }

        private async void btnTestRequest_Click(object sender, RoutedEventArgs e)
        {
            //InteropRequest request = new InteropRequest();
            //request.header.RequestType = InteropRequestType.GetDLLInformation;
            //request.header.RequestPayloadSize = Marshal.SizeOf(typeof(DLLInformationRequest));

            //var payload = new DLLInformationRequest();
            //payload.DLLName = "halo1.dll";
            //request.requestData = TASInterop.MarshalObjectToArray(payload);

            //var response = await TASInterop.MakeRequestAsync(request);

            //if(response.header.ResponseType == InteropResponseType.DLLInformationFound)
            //{
            //    DLLInformationResponse responseData = new DLLInformationResponse();
            //    TASInterop.MarshalArrayToObject(ref responseData, response.responseData);
            //    System.Diagnostics.Debug.WriteLine($"0x{responseData.DLLAddress:X}");
            //}

            await Task.Delay(0);
        }

        private async Task ExecuteCommand()
        {
            InteropRequest request = new InteropRequest();
            request.header.RequestType = InteropRequestType.ExecuteCommand;
            request.header.RequestPayloadSize = Marshal.SizeOf(typeof(ExecuteCommandRequest));


            var payload = new ExecuteCommandRequest();
            payload.Command = tbCommand.Text;
            request.requestData = TASInterop.MarshalObjectToArray(payload);

            tbCommand.Text = "";
            
            var response = await TASInterop.MakeRequestAsync(request);
        }

        private async void tbCommand_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Return)
            {
                await ExecuteCommand();
            }
        }

        private async void btnSendCommand_Click(object sender, RoutedEventArgs e)
        {
            await ExecuteCommand();
        }

       
    }
}
