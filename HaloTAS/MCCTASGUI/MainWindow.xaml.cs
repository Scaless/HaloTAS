using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
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

namespace MCCTASGUI
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        TASInterop TASInterop;
        LiveMapDataViewer liveMapDataViewer = null;
        AboutWindow aboutWindow = null;
        TASInputEditor inputEditorWindow = null;

        public MainWindow()
        {
            InitializeComponent();

            Application.Current.MainWindow = this;
            Application.Current.ShutdownMode = ShutdownMode.OnMainWindowClose;
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            TASInterop = new TASInterop();
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
                if(liveMapDataViewer.WindowState == WindowState.Minimized)
                    liveMapDataViewer.WindowState = WindowState.Normal;
                liveMapDataViewer.Focus();
            }
        }
        private void LiveMapDataViewer_Closed(object sender, EventArgs e)
        {
            liveMapDataViewer = null;
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
                if (aboutWindow.WindowState == WindowState.Minimized)
                    aboutWindow.WindowState = WindowState.Normal;
                aboutWindow.Focus();
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
                if (inputEditorWindow.WindowState == WindowState.Minimized)
                    inputEditorWindow.WindowState = WindowState.Normal;
                inputEditorWindow.Focus();
            }
        }
        private void InputEditorWindow_Closed(object sender, EventArgs e)
        {
            inputEditorWindow = null;
        }

    }
}
