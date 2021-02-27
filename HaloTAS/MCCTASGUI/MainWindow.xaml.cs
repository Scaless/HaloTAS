using MCCTASGUI.Interop;
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
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private AboutWindow aboutWindow = null;
        private TASInputEditor inputEditorWindow = null;

        public MainWindow()
        {
            InitializeComponent();

            Application.Current.MainWindow = this;
            Application.Current.ShutdownMode = ShutdownMode.OnMainWindowClose;

            btnInject.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
        }

        private async void Window_Loaded(object sender, RoutedEventArgs e)
        {
            await UpdatePingTimer();
        }

        private bool PingTimerRunning = false;
        private int PingRefreshRateMilliseconds = 500;
        private async Task UpdatePingTimer()
        {
            if (PingTimerRunning)
                return;
            PingTimerRunning = true;

            while (true)
            {
                await GlobalInterop.RefreshStatus();

                var Status = GlobalInterop.Status;
                Dispatcher.Invoke(() =>
                {
                    tblkStatusConnected.Text = Status.Connected ? "CONNECTED" : "NOT CONNECTED";
                    btnInject.Visibility = Status.Connected ? Visibility.Hidden : Visibility.Visible;

                    iconHalo1Loaded.Fill = Status.H1DLLLoaded ? Brushes.Green : Brushes.Red;
                    iconHalo2Loaded.Fill = Status.H2DLLLoaded ? Brushes.Green : Brushes.Red;
                    iconHalo3Loaded.Fill = Status.H3DLLLoaded ? Brushes.Green : Brushes.Red;
                    iconHaloReachLoaded.Fill = Status.ReachDLLLoaded ? Brushes.Green : Brushes.Red;
                    iconHaloODSTLoaded.Fill = Status.ODSTDLLLoaded ? Brushes.Green : Brushes.Red;
                    iconHalo4Loaded.Fill = Status.H4DLLLoaded ? Brushes.Green : Brushes.Red;

                    panelH1Cheats.Children.Clear();
                    for(int i = 0; i < EnumUtils<Halo1Cheat>.Count; i++)
                    {
                        CheckBox cb = new CheckBox();
                        cb.Width = 150;
                        cb.IsChecked = Status.Halo1.CheatsEnabled[i];
                        cb.Content = Enum.GetName(typeof(Halo1Cheat), i).ToString();
                        cb.Margin = new Thickness(2);
                        cb.Tag = (Halo1Cheat)i;
                        cb.Checked += cbH1CheatToggleChecked;
                        cb.Unchecked += cbH1CheatToggleChecked;
                        panelH1Cheats.Children.Add(cb);
                    }

                    panelH2Skulls.Children.Clear();
                    for (int i = 0; i < EnumUtils<Halo2Skull>.Count; i++)
                    {
                        CheckBox cb = new CheckBox();
                        cb.Width = 150;
                        cb.IsChecked = Status.Halo2.SkullsEnabled[i];
                        cb.Content = Enum.GetName(typeof(Halo2Skull), i).ToString();
                        cb.Margin = new Thickness(2);
                        cb.Tag = (Halo2Skull)i;
                        cb.Checked += cbH2SkullToggleChecked;
                        cb.Unchecked += cbH2SkullToggleChecked;
                        panelH2Skulls.Children.Add(cb);
                    }
                });

                await Task.Delay(PingRefreshRateMilliseconds);
            }
        }

        private async void cbH1CheatToggleChecked(object sender, RoutedEventArgs e)
        {
            var checkbox = sender as CheckBox;
            if (checkbox == null)
                return;

            Halo1Cheat cheat = (Halo1Cheat)checkbox.Tag;
            bool newEnabled = checkbox.IsChecked ?? false;

            await H1EngineFunctions.SetCheatEnabled(cheat, newEnabled);
        }

        private async void cbH2SkullToggleChecked(object sender, RoutedEventArgs e)
        {
            var checkbox = sender as CheckBox;
            if (checkbox == null)
                return;

            Halo2Skull skull = (Halo2Skull)checkbox.Tag;
            bool newEnabled = checkbox.IsChecked ?? false;

            await H2EngineFunctions.SetSkullEnabled(skull, newEnabled);
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

        private async void tbH1SendCommand_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Return)
            {
                await H1EngineFunctions.ExecuteCommand(tbH1SendCommand.Text);
                tbH1SendCommand.Text = "";
            }
        }
        private async void btnH1SendCommand_Click(object sender, RoutedEventArgs e)
        {
            await H1EngineFunctions.ExecuteCommand(tbH1SendCommand.Text);
            tbH1SendCommand.Text = "";
        }

        private async void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            InteropRequest request = new InteropRequest();
            request.header.RequestType = InteropRequestType.KillMCCTAS;

            var result = await TASInterop.MakeRequestAsync(request);
            if (result?.header.ResponseType != InteropResponseType.Success)
            {
                // Something went wrong
            }
        }

        private void MenuFileClose(object sender, RoutedEventArgs e)
        {
            Close();
        }
    }
}
