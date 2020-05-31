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
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace MCCTASGUI.Controls
{
    /// <summary>
    /// Interaction logic for ActionListVis.xaml
    /// </summary>
    public partial class ActionListVis : UserControl
    {
        public static DependencyProperty ActionsProperty =
            DependencyProperty.Register("Actions", typeof(List<TickAction>), typeof(ActionListVis),
                new PropertyMetadata(new PropertyChangedCallback(ActionsPropertyChanged)));

        public List<TickAction> Actions {
            get { return (List<TickAction>)GetValue(ActionsProperty); }
            set { SetValue(ActionsProperty, value); }
        }

        public ActionListVis()
        {
            InitializeComponent();
        }

        private static void ActionsPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var control = d as ActionListVis;
            if(control != null)
            {
                control.Actions = (List<TickAction>)e.NewValue;

                control.panelActions.Children.Clear();

                foreach(var action in control.Actions)
                {
                    Border brdr = new Border();
                    brdr.Margin  = new Thickness(0);
                    brdr.BorderThickness = new Thickness(1);
                    brdr.BorderBrush = Brushes.Black;

                    TextBlock b = new TextBlock();
                    b.Margin  = new Thickness(1);
                    b.Text = action.ToString();

                    brdr.Child = b;
                    control.panelActions.Children.Add(brdr);
                }

            }
        }
    }
}
