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
        private bool Initializing;    

        public H1CinematicCameraWindow()
        {
            Initializing = true;
            InitializeComponent();
            Initializing = false;
        }

        private async void SliderCameraPosition_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            if (Initializing)
                return;

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

        private void sliderLerpTime_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            if (Initializing)
                return;

            lblTimeSlider.Content = sliderLerpTime.Value;
        }

        //a/b = values, t between 0 and 1
        private float Lerp(float a, float b, float t)
        {
            return a + t * (b - a);
        }

        private async void btnLerp_Click(object sender, RoutedEventArgs e)
        {
            double time = sliderLerpTime.Value;
            for (float i = 0; i <= time; i += .01f)
            {
                float frac = i / (float)time;

                float newX = Lerp(aX, bX, frac);
                float newY = Lerp(aY, bY, frac);
                float newZ = Lerp(aZ, bZ, frac);

                await UpdateCameraPositionAsync(newX, newY, newZ);
                await Task.Delay(1);
            }
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
