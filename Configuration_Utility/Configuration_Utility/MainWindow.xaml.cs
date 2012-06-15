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
using TUIO;
using MahApps.Metro.Controls;
using System.Drawing;
using System.Runtime.InteropServices;


namespace Configuration_Utility
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow: MetroWindow
    {
        [DllImport("user32.dll")]
        static extern bool EnumDisplayDevices(string lpDevice, uint iDevNum, byte[] lpDisplayDevice, uint dwFlags);
        private object cursorSync = new object();
        Monitor Monitor1,Monitor2,Monitor3,Monitor4,Monitor5;
        Sensor Sensor1, Sensor2, Sensor3, Sensor4, Sensor5;
        System.Windows.Controls.Button DetectDisplays;
        public double width;
        public double height;
        public MainWindow()
        {
            
            InitializeComponent();
            MinimizeToTray.Enable(this);
            width = System.Windows.Application.Current.MainWindow.Width;
            height = System.Windows.Application.Current.MainWindow.Height;
            
            adddisplays();
            addsensors();
            Properties.Settings.Default.NumberOfMonitors = ScreenUtil.Screens.Count();
            Properties.Settings.Default.Save();
            Monitor1.border.BorderBrush = new SolidColorBrush(Colors.SteelBlue);
            Console.WriteLine("0 >" + ScreenUtil.Screens[0].WorkingArea.X + " " + ScreenUtil.Screens[0].WorkingArea.Y + " " + ScreenUtil.Screens[0].WorkingArea.Top + " " +ScreenUtil.Screens[0].WorkingArea.Left + " " +ScreenUtil.Screens[0].WorkingArea.Bottom + " " + ScreenUtil.Screens[0].WorkingArea.Right);
          //  Console.WriteLine("1 >" + ScreenUtil.Screens[1].WorkingArea.X + " " + ScreenUtil.Screens[1].WorkingArea.Y + " " + ScreenUtil.Screens[1].WorkingArea.Top + " " + ScreenUtil.Screens[1].WorkingArea.Left + " " + ScreenUtil.Screens[1].WorkingArea.Bottom + " " + ScreenUtil.Screens[1].WorkingArea.Right);
      
        }

        public void addsensors()
        {

            sensor_stackpanel.Children.Clear();
            Sensor1 = new Sensor();
            Sensor2 = new Sensor();
            Sensor3 = new Sensor();
            Sensor4 = new Sensor();
            Sensor5 = new Sensor();
            sensor_stackpanel.Children.Insert(0, Sensor1);
            sensor_stackpanel.Children.Insert(1, Sensor2);
            sensor_stackpanel.Children.Insert(2, Sensor3);
            sensor_stackpanel.Children.Insert(3, Sensor4);
            sensor_stackpanel.Children.Insert(4, Sensor5);
            Sensor1.Visibility = Visibility.Visible;
            
        }

        public void adddisplays()
        {
           display_stackpanel.Children.Clear();
           if (ScreenUtil.Screens.Count()>0)
           { 
                Monitor1 = new Monitor();
                Monitor1.border.Width = ScreenUtil.Screens[0].Bounds.Width / 10;
                Monitor1.border.Height = ScreenUtil.Screens[0].Bounds.Height / 10;
                Monitor1.text.Content = ScreenUtil.Screens[0].Bounds.Width + "X" + ScreenUtil.Screens[0].Bounds.Height;
                Monitor1.device_name.Content = ScreenUtil.Screens[0].DeviceName.ToString();
               // Console.WriteLine("Primary" + ScreenUtil.Screens[1].Primary.ToString() + "Workign Area x" + ScreenUtil.Screens[1].WorkingArea.X + "Workign Area y" + ScreenUtil.Screens[1].WorkingArea.Y);
               // EnumDisplayDevices(
                if (ScreenUtil.Screens[0].Primary == true)
                {
                    Monitor1.primary.Visibility = Visibility.Visible;
                }
                   Monitor1.Active.Content = true;
                Monitor1.MouseDown += new MouseButtonEventHandler(Monitor1_MouseDown);
                display_stackpanel.Children.Insert(0, Monitor1);
                Monitor1.Margin = new Thickness(ScreenUtil.Screens[0].WorkingArea.X / 10 , ScreenUtil.Screens[0].WorkingArea.Y / 10, 0, 0);
               

                
           }
           if (ScreenUtil.Screens.Count()>1)
           {
                Monitor2 = new Monitor();
                Monitor2.border.Width = ScreenUtil.Screens[1].Bounds.Width / 10;
                Monitor2.border.Height = ScreenUtil.Screens[1].Bounds.Height / 10;
                Monitor2.text.Content = ScreenUtil.Screens[1].Bounds.Width + "X" + ScreenUtil.Screens[1].Bounds.Height;
                if (ScreenUtil.Screens[1].Primary == true)
                {
                    Monitor2.primary.Visibility = Visibility.Visible;
                } 
               Monitor2.device_name.Content = ScreenUtil.Screens[1].DeviceName.ToString();
                Monitor2.MouseDown += new MouseButtonEventHandler(Monitor2_MouseDown);
                display_stackpanel.Children.Insert(1, Monitor2); Monitor2.Margin = new Thickness(2*ScreenUtil.Screens[1].WorkingArea.X / 10,ScreenUtil.Screens[1].WorkingArea.Y / 10, 0, 0);
               
           }
           if (ScreenUtil.Screens.Count()>2)
           {
               Monitor3 = new Monitor();
               Monitor3.border.Width = ScreenUtil.Screens[2].Bounds.Width / 10;
               Monitor3.border.Height = ScreenUtil.Screens[2].Bounds.Height / 10;
               Monitor3.text.Content = ScreenUtil.Screens[2].Bounds.Width + "X" + ScreenUtil.Screens[2].Bounds.Height;
               if (ScreenUtil.Screens[2].Primary == true)
               {
                   Monitor3.primary.Visibility = Visibility.Visible;
               }
               Monitor3.MouseDown += new MouseButtonEventHandler(Monitor3_MouseDown);
               display_stackpanel.Children.Insert(2, Monitor3);
               Monitor3.Margin = new Thickness(2*ScreenUtil.Screens[2].WorkingArea.X / 10, ScreenUtil.Screens[2].WorkingArea.Y / 10, 0, 0);
           }
           if (ScreenUtil.Screens.Count()>3)
           {
               Monitor4 = new Monitor();
               Monitor4.border.Width = ScreenUtil.Screens[3].Bounds.Width / 10;
               Monitor4.border.Height = ScreenUtil.Screens[3].Bounds.Height / 10;
               Monitor4.text.Content = ScreenUtil.Screens[3].Bounds.Width + "X" + ScreenUtil.Screens[3].Bounds.Height;
               if (ScreenUtil.Screens[3].Primary == true)
               {
                   Monitor4.primary.Visibility = Visibility.Visible;
               }
               Monitor4.MouseDown += new MouseButtonEventHandler(Monitor4_MouseDown);
               display_stackpanel.Children.Insert(3, Monitor4); Monitor4.Margin = new Thickness(2*ScreenUtil.Screens[3].WorkingArea.X / 10, ScreenUtil.Screens[3].WorkingArea.Y / 10, 0, 0);
           }
           if (ScreenUtil.Screens.Count()>4)
           {
               Monitor5 = new Monitor();
               Monitor5.border.Width = ScreenUtil.Screens[4].Bounds.Width / 10;
               Monitor5.border.Height = ScreenUtil.Screens[4].Bounds.Height / 10;
               Monitor5.text.Content = ScreenUtil.Screens[4].Bounds.Width + "X" + ScreenUtil.Screens[4].Bounds.Height;
               if (ScreenUtil.Screens[4].Primary == true)
               {
                   Monitor5.primary.Visibility = Visibility.Visible;
               }
               Monitor5.MouseDown += new MouseButtonEventHandler(Monitor5_MouseDown);
               display_stackpanel.Children.Insert(4, Monitor5); Monitor5.Margin = new Thickness(2*ScreenUtil.Screens[4].WorkingArea.X / 10, ScreenUtil.Screens[4].WorkingArea.Y / 10, 0, 0);
           }

           DetectDisplays = new System.Windows.Controls.Button();
           DetectDisplays.Width = 120;
           DetectDisplays.Height = 40;
           DetectDisplays.Content = "Detect Displays";
           DetectDisplays.Click += new RoutedEventHandler(DetectDisplays_Click);
           Grid.SetColumn(DetectDisplays, 1);
           display_stackpanel.Children.Add(DetectDisplays);
          // Canvas.SetLeft(DetectDisplays, 250);
     
        }

       

        void Monitor5_MouseDown(object sender, MouseButtonEventArgs e)
        {
            adddisplays();
            Monitor5.border.BorderBrush = new SolidColorBrush(Colors.SteelBlue);
            Sensor1.Visibility = Visibility.Collapsed;
            Sensor2.Visibility = Visibility.Collapsed;
            Sensor3.Visibility = Visibility.Collapsed;
            Sensor4.Visibility = Visibility.Collapsed;
            Sensor5.Visibility = Visibility.Visible;
        }

        void Monitor4_MouseDown(object sender, MouseButtonEventArgs e)
        {
            adddisplays();
            Monitor4.border.BorderBrush = new SolidColorBrush(Colors.SteelBlue);
            Sensor1.Visibility = Visibility.Collapsed;
            Sensor2.Visibility = Visibility.Collapsed;
            Sensor3.Visibility = Visibility.Collapsed;
            Sensor4.Visibility = Visibility.Visible;
            Sensor5.Visibility = Visibility.Collapsed;
        }

        void Monitor3_MouseDown(object sender, MouseButtonEventArgs e)
        {
            adddisplays();
            Monitor3.border.BorderBrush = new SolidColorBrush(Colors.SteelBlue);
            Sensor1.Visibility = Visibility.Collapsed;
            Sensor2.Visibility = Visibility.Collapsed;
            Sensor3.Visibility = Visibility.Visible;
            Sensor4.Visibility = Visibility.Collapsed;
            Sensor5.Visibility = Visibility.Collapsed;
        }

        void Monitor2_MouseDown(object sender, MouseButtonEventArgs e)
        {
            adddisplays();
            Monitor2.border.BorderBrush = new SolidColorBrush(Colors.SteelBlue);
            Sensor1.Visibility = Visibility.Collapsed;
            Sensor2.Visibility = Visibility.Visible;
            Sensor3.Visibility = Visibility.Collapsed;
            Sensor4.Visibility = Visibility.Collapsed;
            Sensor5.Visibility = Visibility.Collapsed;
        }

        void Monitor1_MouseDown(object sender, MouseButtonEventArgs e)
        {
            adddisplays();
            Monitor1.border.BorderBrush = new SolidColorBrush(Colors.SteelBlue);
            Sensor1.Visibility = Visibility.Visible;
            Sensor2.Visibility = Visibility.Collapsed;
            Sensor3.Visibility = Visibility.Collapsed;
            Sensor4.Visibility = Visibility.Collapsed;
            Sensor5.Visibility = Visibility.Collapsed;
        }

        private void DetectDisplays_Click(object sender, RoutedEventArgs e)
        {
            adddisplays();
            Monitor1.border.BorderBrush = new SolidColorBrush(Colors.SteelBlue);
            Sensor1.Visibility = Visibility.Visible;
            Sensor2.Visibility = Visibility.Collapsed;
            Sensor3.Visibility = Visibility.Collapsed;
            Sensor4.Visibility = Visibility.Collapsed;
            Sensor5.Visibility = Visibility.Collapsed;
        }

   }
}
 

   