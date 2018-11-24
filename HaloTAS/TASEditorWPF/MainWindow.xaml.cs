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
using System.IO;
using Microsoft.Win32;

namespace TASEditorWPF
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        OpenFileDialog ofd = new OpenFileDialog
        {
            DefaultExt = "bin",
            InitialDirectory = @"C:\Halo",
            Filter = @"Halo Binary Logs (*.hbin)|*.hbin",
            RestoreDirectory = true,
            CheckFileExists = true
        };

        public MainWindow()
        {
            InitializeComponent();
        }

        private void ButtonLoadHBIN_Click(object sender, RoutedEventArgs e)
        {
            if (!ofd.ShowDialog() ?? true)
                return;
            if (string.IsNullOrWhiteSpace(ofd.FileName))
                return;

            GridEntries.Children.Clear();
            GridEntries.RowDefinitions.Clear();

            List<InputMoment> inputMoments = LoadHBIN(ofd.OpenFile());

            int count = 0;
            foreach(var input in inputMoments)
            {
                count++;
                var rowDef = new RowDefinition();
                rowDef.Height = GridLength.Auto;
                GridEntries.RowDefinitions.Add(rowDef);

                var control = new HBINEntry();
                control.SetValues(count, input);

                GridEntries.Children.Add(control);
                Grid.SetRow(control, GridEntries.RowDefinitions.Count - 1);
            }

            //hbinEditor.Import(inputMoments);
        }

        private List<InputMoment> LoadHBIN(Stream hbinFileStream)
        {
            List<InputMoment> inputs = new List<InputMoment>();

            const int structSize = 120;
            byte[] buf = new byte[structSize];
            while (hbinFileStream.Read(buf, 0, structSize) != 0)
            {
                InputMoment im = new InputMoment
                {
                    CheckpointId = BitConverter.ToUInt32(buf, 0),
                    Tick = BitConverter.ToUInt32(buf, 4),
                    MouseYaw = BitConverter.ToSingle(buf, 112),
                    MousePitch = BitConverter.ToSingle(buf, 116),
                };

                Array.Copy(buf, 8, im.InputBuffer, 0, 104);

                inputs.Add(im);
            }

            return inputs;
        }
    }
}
