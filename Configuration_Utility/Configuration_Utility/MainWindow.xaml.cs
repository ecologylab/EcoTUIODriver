using System;
using System.Collections.Generic;
using System.Linq;
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
using Configuration_Utility.Utils;
using System.Windows.Forms;
using MahApps.Metro;

namespace Configuration_Utility
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow 
    {
        Monitor Monitor1,Monitor2,Monitor3,Monitor4,Monitor5;
        Sensor Sensor1;
        System.Windows.Controls.Button DetectDisplays;
        public MainWindow()
        {
            InitializeComponent();
            MinimizeToTray.Enable(this);
            adddisplays();
            addsensors();
        }

        public void addsensors()
        {

            sensor_stackpanel.Children.Clear();
            Sensor1 = new Sensor();
            sensor_stackpanel.Children.Insert(0, Sensor1);


            //AddSensor = new System.Windows.Controls.Button();
            //AddSensor.Width = 90;
            //AddSensor.Height = 30;
            //AddSensor.Content = "Add Sensor";
            //AddSensor.Click +=new RoutedEventHandler(AddSensor_Click);
            //sensor_stackpanel.Children.Add(AddSensor);
        }

        public void adddisplays()
        {
           display_stackpanel.Children.Clear();
           if (ScreenUtil.Screens.Count()>0)
           { 
               
                Monitor1 = new Monitor();
                Monitor1.rectangle.Width = ScreenUtil.Screens[0].Bounds.Width / 10;
                Monitor1.rectangle.Height = ScreenUtil.Screens[0].Bounds.Height / 10;
                Monitor1.text.Content = ScreenUtil.Screens[0].Bounds.Width + "X" + ScreenUtil.Screens[0].Bounds.Height;
                display_stackpanel.Children.Insert(0, Monitor1);
              
           }
           if (ScreenUtil.Screens.Count()>1)
           {
                Monitor2 = new Monitor();
                Monitor2.rectangle.Width = ScreenUtil.Screens[1].Bounds.Width / 10;
                Monitor2.rectangle.Height = ScreenUtil.Screens[1].Bounds.Height / 10;
                Monitor2.text.Content = ScreenUtil.Screens[1].Bounds.Width + "X" + ScreenUtil.Screens[1].Bounds.Height;
                display_stackpanel.Children.Insert(1, Monitor2);
           }
           if (ScreenUtil.Screens.Count()>2)
           {
               Monitor3 = new Monitor();
               Monitor3.rectangle.Width = ScreenUtil.Screens[2].Bounds.Width / 10;
               Monitor3.rectangle.Height = ScreenUtil.Screens[2].Bounds.Height / 10;
               Monitor3.text.Content = ScreenUtil.Screens[2].Bounds.Width + "X" + ScreenUtil.Screens[2].Bounds.Height;
               display_stackpanel.Children.Insert(2, Monitor3);
           }
           if (ScreenUtil.Screens.Count()>3)
           {
               Monitor4 = new Monitor();
               Monitor4.rectangle.Width = ScreenUtil.Screens[3].Bounds.Width / 10;
               Monitor4.rectangle.Height = ScreenUtil.Screens[3].Bounds.Height / 10;
               Monitor4.text.Content = ScreenUtil.Screens[3].Bounds.Width + "X" + ScreenUtil.Screens[3].Bounds.Height;
               display_stackpanel.Children.Insert(3, Monitor4);
           }
           if (ScreenUtil.Screens.Count()>4)
           {
               Monitor5 = new Monitor();
               Monitor5.rectangle.Width = ScreenUtil.Screens[4].Bounds.Width / 10;
               Monitor5.rectangle.Height = ScreenUtil.Screens[4].Bounds.Height / 10;
               Monitor5.text.Content = ScreenUtil.Screens[4].Bounds.Width + "X" + ScreenUtil.Screens[4].Bounds.Height;
               display_stackpanel.Children.Insert(4, Monitor5);
           }

           DetectDisplays = new System.Windows.Controls.Button();
           DetectDisplays.Width = 90;
           DetectDisplays.Height = 30;
           DetectDisplays.Content = "Detect Displays";
           DetectDisplays.Click +=new RoutedEventHandler(DetectDisplays_Click);
           display_stackpanel.Children.Add(DetectDisplays);
        }

        private void DetectDisplays_Click(object sender, RoutedEventArgs e)
        {
            adddisplays();
        }

        private void AddSensor_Click(object sender, RoutedEventArgs e)
        {
           
        }

      
   }
}
 

   