using System;
using System.Collections.Generic;
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
using System.Windows.Shapes;

namespace MCCTASGUI
{
    /// <summary>
    /// Interaction logic for H1CinematicCameraWindow.xaml
    /// </summary>
    public partial class H1CinematicCameraWindow : Window
    {
        public H1CinematicCameraWindow()
        {
            InitializeComponent();
        }

        private async void SliderCameraPosition_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            await UpdateCameraPositionAsync((float)sliderCameraXPos.Value, (float)sliderCameraYPos.Value, (float)sliderCameraZPos.Value);
        }

        private async Task UpdateCameraPositionAsync(float x, float y, float z)
        {
            InteropRequest request = new InteropRequest();
            request.header.RequestType = InteropRequestType.SetCameraDetails;
            request.header.RequestPayloadSize = Marshal.SizeOf(typeof(SetCameraDetailsRequest));

            var payload = new SetCameraDetailsRequest();
            payload.PositionX = x;
            payload.PositionY = y;
            payload.PositionZ = z;

            request.requestData = TASInterop.MarshalObjectToArray(payload);

            var response = await TASInterop.MakeRequestAsync(request);

            if(response.header.ResponseType != InteropResponseType.Success)
            {
                // Something went wrong
            }

        }

        float aX, aY, aZ, bX, bY, bZ;

        //a/b = values, t between 0 and 1
        private float Lerp(float a, float b, float t)
        {
            return a + t * (b - a);
        }

        private async void btnLerp_Click(object sender, RoutedEventArgs e)
        {
            for (float i = 0; i <= 1; i += .01f)
            {
                float newX = Lerp(aX, bX, i);
                float newY = Lerp(aY, bY, i);
                float newZ = Lerp(aZ, bZ, i);

                await UpdateCameraPositionAsync(newX, newY, newZ);
                await Task.Delay(1);
            }
        }

        private void Window_KeyDown(object sender, KeyEventArgs e)
        {
            
        }

        private void btnLocA_Click(object sender, RoutedEventArgs e)
        {
            aX = (float)sliderCameraXPos.Value;
            aY = (float)sliderCameraYPos.Value;
            aZ = (float)sliderCameraZPos.Value;

            tbaX.Text = aX.ToString();
            tbaY.Text = aY.ToString();
            tbaZ.Text = aZ.ToString();
        }

        private void btnLocB_Click(object sender, RoutedEventArgs e)
        {
            bX = (float)sliderCameraXPos.Value;
            bY = (float)sliderCameraYPos.Value;
            bZ = (float)sliderCameraZPos.Value;

            tbbX.Text = bX.ToString();
            tbbY.Text = bY.ToString();
            tbbZ.Text = bZ.ToString();
        }
    }
}
