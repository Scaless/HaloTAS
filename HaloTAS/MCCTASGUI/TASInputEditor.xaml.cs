using MCCTASGUI.Controls;
using System;
using System.Collections.Generic;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace MCCTASGUI
{
    /// <summary>
    /// Interaction logic for TASInputEditor.xaml
    /// </summary>
    public partial class TASInputEditor : Window
    {
        
        List<TickItem> items;
        
        public TASInputEditor()
        {
            InitializeComponent();

            items = new List<TickItem>();
            const int ITEM_COUNT = 10000;
            for (int i = 0; i < ITEM_COUNT; i++)
            {
                TickItem ti = new TickItem(i);
                items.Add(ti);
            }

            lbTickItems.ItemsSource = items;
        }

        private void MenuClose_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }
    }

    public enum TickInput
    {
        None = 0,
        ForwardBegin,
        ForwardEnd,
        BackwardBegin,
        BackwardEnd,
        LeftBegin,
        LeftEnd,
        RightBegin,
        RightEnd,
        ShootBegin,
        ShootEnd,
        JumpBegin,
        JumpEnd,
        CrouchBegin,
        CrouchEnd,

        MeleeInstant,
        ZoomInInstant,
        ZoomOutInstant,
        WeaponSwapInstant,
        GrenadeSwapInstant,
        GrenadeBeginThrowInstant,
    }

    public enum TickAction
    {
        None = 0,
        MoveForward,
        MoveBackward,
        MoveLeft,
        MoveRight,
        Crouch,
        Jump,
        Shoot,
        Melee,
        ZoomIn,
        ZoomOut,
        WeaponSwap,
        GrenadeSwap,

        Info_GrenadeSpawned
    }

    public class TickItem
    {
        public TickItem(int NewTick)
        {
            Tick = NewTick;

            Inputs = new List<TickInput>();
            Actions = new List<TickAction>();

            /////// TODO-SCALES: Remove Mockup :)
            if (Tick == 4)
                Inputs.Add(TickInput.ForwardBegin);
            if (Tick == 10)
                Inputs.Add(TickInput.ForwardEnd);

            if(Tick >= 4 && Tick < 10)
            {
                Actions.Add(TickAction.MoveForward);
            }

            if (Tick == 12)
                Inputs.Add(TickInput.GrenadeBeginThrowInstant);
            if (Tick == 19)
                Actions.Add(TickAction.Info_GrenadeSpawned);
            /////////////////////////////////
        }

        public int Tick { get; set; }

        public float LookX { get; set; }
        public float LookY { get; set; }

        public List<TickInput> Inputs { get; set; }
        public List<TickAction> Actions { get; set; }
    }
}
