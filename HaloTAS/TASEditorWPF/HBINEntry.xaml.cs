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

namespace TASEditorWPF
{
    /// <summary>
    /// Interaction logic for HBINEntry.xaml
    /// </summary>
    public partial class HBINEntry : UserControl
    {
        private int AbsoluteFrame = 0;
        private InputMoment Value;

        public HBINEntry()
        {
            InitializeComponent();
        }

        public void SetValues(int absoluteFrame, InputMoment value)
        {
            AbsoluteFrame = absoluteFrame;
            Value = value;

            TBFrameCounter.Text = AbsoluteFrame.ToString();
            TxtCheckpoint.Text = Value.CheckpointId.ToString();
            TxtTick.Text = Value.Tick.ToString();
            TxtMouseYaw.Text = Value.MouseYaw.ToString();
            TxtMousePitch.Text = Value.MousePitch.ToString();
        }
    }
}
