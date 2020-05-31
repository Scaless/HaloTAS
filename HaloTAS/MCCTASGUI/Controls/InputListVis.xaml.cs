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
    /// Interaction logic for InputListVis.xaml
    /// </summary>
    public partial class InputListVis : UserControl
    {
        public static DependencyProperty InputsProperty =
            DependencyProperty.Register("Inputs", typeof(List<TickInput>), typeof(InputListVis),
                new PropertyMetadata(new PropertyChangedCallback(InputsPropertyChanged)));

        public List<TickInput> Inputs
        {
            get { return (List<TickInput>)GetValue(InputsProperty); }
            set { SetValue(InputsProperty, value); }
        }

        public InputListVis()
        {
            InitializeComponent();
        }

        private static void InputsPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var control = d as InputListVis;
            if (control != null)
            {
                control.Inputs = (List<TickInput>)e.NewValue;

                control.panelInputs.Children.Clear();

                foreach (var action in control.Inputs)
                {
                    Border brdr = new Border();
                    brdr.HorizontalAlignment = HorizontalAlignment.Left;

                    brdr.Margin = new Thickness(0);
                    brdr.BorderThickness = new Thickness(1);
                    brdr.BorderBrush = Brushes.Black;

                    TextBlock b = new TextBlock();
                    b.Margin = new Thickness(1);
                    b.Text = action.ToString();

                    brdr.Child = b;
                    control.panelInputs.Children.Add(brdr);
                }

            }
        }
    }
}
