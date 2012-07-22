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
using System.IO;
using System.ServiceProcess;
using System.Windows.Automation.Peers;
using System.Windows.Automation.Provider;
using System.Diagnostics;
using System.Threading;
namespace Configuration_Utility
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow: MetroWindow
    {
        [DllImport("kernel32.dll")]
        static extern IntPtr GetConsoleWindow();

        [DllImport("user32.dll")]
        static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);

        const int SW_HIDE = 0;
        const int SW_SHOW = 5;

        [DllImport("user32.dll")]
        static extern bool EnumDisplayDevices(string lpDevice, uint iDevNum, byte[] lpDisplayDevice, uint dwFlags);
        //private object cursorSync = new object();
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
            Console.WriteLine("Adding Displays");
            adddisplays();
            Console.WriteLine("Adding Touch sensor for every Display");
            addsensors();

            //uncommment if you don't want to show the console .
            //var handle = GetConsoleWindow();
            //// Hide
            //ShowWindow(handle, SW_HIDE);

            Properties.Settings.Default.NumberOfMonitors = ScreenUtil.Screens.Count();
            Properties.Settings.Default.Save();
            Monitor1.border.BorderBrush = new SolidColorBrush(Colors.SteelBlue);
            try
            {
                Console.WriteLine("0 >" + ScreenUtil.Screens[0].WorkingArea.X + " " + ScreenUtil.Screens[0].WorkingArea.Y + " " + ScreenUtil.Screens[0].WorkingArea.Top + " " + ScreenUtil.Screens[0].WorkingArea.Left + " " + ScreenUtil.Screens[0].WorkingArea.Bottom + " " + ScreenUtil.Screens[0].WorkingArea.Right);
            }
            catch { }   //  Console.WriteLine("1 >" + ScreenUtil.Screens[1].WorkingArea.X + " " + ScreenUtil.Screens[1].WorkingArea.Y + " " + ScreenUtil.Screens[1].WorkingArea.Top + " " + ScreenUtil.Screens[1].WorkingArea.Left + " " + ScreenUtil.Screens[1].WorkingArea.Bottom + " " + ScreenUtil.Screens[1].WorkingArea.Right);
       
        }

        public void addsensors()
        {
            sensor_stackpanel.Children.Clear();
            Sensor1 = new Sensor();
            Sensor2 = new Sensor();
            Sensor3 = new Sensor();
            Sensor4 = new Sensor();
            Sensor5 = new Sensor();
           
            Sensor1.service_name="Tuio-to-Vmulti-Service-1.exe";
            Sensor2.service_name = "Tuio-to-Vmulti-Service-2.exe";
            Sensor3.service_name = "Tuio-to-Vmulti-Service-3.exe";
            Sensor4.service_name = "Tuio-to-Vmulti-Service-4.exe";
            Sensor5.service_name = "Tuio-to-Vmulti-Service-5.exe";

            load_values_from_config_files();
        
            Sensor1.service.Click +=new RoutedEventHandler(service_Click1);
            Sensor2.service.Click += new RoutedEventHandler(service_Click2);
            Sensor3.service.Click += new RoutedEventHandler(service_Click3);
            Sensor4.service.Click += new RoutedEventHandler(service_Click4);
            Sensor5.service.Click += new RoutedEventHandler(service_Click5);
            //Get service status and display it in the sesnor area . 
            Sensor1.service_status.Content = get_service_status("Tuio-To-vmulti-Device1");
            Sensor2.service_status.Content = get_service_status("Tuio-To-vmulti-Device2");
            Sensor3.service_status.Content = get_service_status("Tuio-To-vmulti-Device3");
            Sensor4.service_status.Content = get_service_status("Tuio-To-vmulti-Device4");
            Sensor5.service_status.Content = get_service_status("Tuio-To-vmulti-Device5");
    
            sensor_stackpanel.Children.Insert(0, Sensor1);
            sensor_stackpanel.Children.Insert(1, Sensor2);
            sensor_stackpanel.Children.Insert(2, Sensor3);
            sensor_stackpanel.Children.Insert(3, Sensor4);
            sensor_stackpanel.Children.Insert(4, Sensor5);
            Sensor1.Visibility = Visibility.Visible;
            
        }

        private void install_service(string service_name,string file_name,string port)
        {
                Process Process_Remove = new Process();
                ProcessStartInfo startInfo = new ProcessStartInfo();
                startInfo.FileName = file_name;
                startInfo.Arguments = "remove " + port;
                startInfo.CreateNoWindow = true;
                startInfo.UseShellExecute = true;
              
                startInfo.WindowStyle = ProcessWindowStyle.Hidden;
            
                using (Process exeProcess = Process.Start(startInfo))
                {
                    exeProcess.WaitForExit();
                }
                          
              
                //status = sc.Status.ToString();
                ProcessStartInfo startInfo2 = new ProcessStartInfo();
                startInfo2.FileName = file_name;
                startInfo2.Arguments = "install " + port;
                startInfo2.UseShellExecute = true;
                startInfo2.WindowStyle = ProcessWindowStyle.Hidden;
                startInfo2.CreateNoWindow = true;
                using (Process exeProcess2= Process.Start(startInfo2))
                {
                    exeProcess2.WaitForExit();
                }

                if (System.IO.File.ReadAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\service1.txt") == "The Service is Running")
                {
                    start_service("Tuio-To-vmulti-Device1");
                }
                if (System.IO.File.ReadAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\service2.txt") == "The Service is Running")
                {
                    start_service("Tuio-To-vmulti-Device2");
                }
                if (System.IO.File.ReadAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\service3.txt") == "The Service is Running")
                {
                    start_service("Tuio-To-vmulti-Device3");
                }
                if (System.IO.File.ReadAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\service4.txt") == "The Service is Running")
                {
                    start_service("Tuio-To-vmulti-Device4");
                }
                if (System.IO.File.ReadAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\service5.txt") == "The Service is Running")
                {
                    start_service("Tuio-To-vmulti-Device5");
                }

         
            
                Sensor1.service_status.Content = get_service_status("Tuio-To-vmulti-Device1");
                Sensor2.service_status.Content = get_service_status("Tuio-To-vmulti-Device2");
                Sensor3.service_status.Content = get_service_status("Tuio-To-vmulti-Device3");
                Sensor4.service_status.Content = get_service_status("Tuio-To-vmulti-Device4");
                Sensor5.service_status.Content = get_service_status("Tuio-To-vmulti-Device5");
    
        }

        private void start_feedback(Sensor sensor, string service_name)
        {

            ServiceController sc = new ServiceController();
            sc.ServiceName = service_name;
            string status = "";
            try
            {

                status = sc.Status.ToString();
            }
            catch
            { status = "Does not exist"; }

            if (status == "Stopped")
            {
                sensor.addtuioclient();
                sensor.service_status.Content = get_service_status("Tuio-To-vmulti-Device1");
            }

            if (status == "Running")
            {
                sensor.service_status.Content = get_service_status("Tuio-To-vmulti-Device1");
            }


        }
       
        private void service_Click1(object sender, RoutedEventArgs e)
        {
            try
            {
                Console.WriteLine("Button Clicked ");
            }
            catch { }
            ServiceController sc = new ServiceController();
            sc.ServiceName = "Tuio-To-vmulti-Device1";
            string status = "";
            try
            {
              status =sc.Status.ToString();
            }
            catch
            { status = "Does not exist";
            Sensor1.service_status.Content = "Does not exist";

            try
            {
                Console.WriteLine("Does not exist");
            }
            catch { }
            }
            if (status == "Running")
            {
                stop_service(sc.ServiceName);
                Sensor1.service_status.Content = get_service_status(sc.ServiceName);
                Sensor1.addtuioclient();
                
            }
            if (status == "Stopped")
            {
                Sensor1.removetuioclinet();
                start_service(sc.ServiceName);
                Sensor1.service_status.Content = get_service_status(sc.ServiceName);
                
            }
        }
        private void service_Click2(object sender, RoutedEventArgs e)
        {
            ServiceController sc = new ServiceController();
            sc.ServiceName = "Tuio-To-vmulti-Device2";
            string status = "";
            try
            {
                status = sc.Status.ToString();
            }
            catch
            { status = "Does not exist"; }
            if (status == "Running")
            {
                stop_service(sc.ServiceName);
                Sensor2.service_status.Content = get_service_status(sc.ServiceName);
            }
            if (status == "Stopped")
            {
                start_service(sc.ServiceName);
                Sensor2.service_status.Content = get_service_status(sc.ServiceName);
            }
        }
        private void service_Click3(object sender, RoutedEventArgs e)
        {
            ServiceController sc = new ServiceController();
            sc.ServiceName = "Tuio-To-vmulti-Device3";
            string status = "";
            try
            {
                status = sc.Status.ToString();
            }
            catch
            { status = "Does not exist"; }
            if (status == "Running")
            {
                stop_service(sc.ServiceName);
                Sensor3.service_status.Content = get_service_status(sc.ServiceName);
            }
            if (status == "Stopped")
            {
                start_service(sc.ServiceName);
                Sensor3.service_status.Content = get_service_status(sc.ServiceName);
            }
        }

        private void service_Click4(object sender, RoutedEventArgs e)
        {
            ServiceController sc = new ServiceController();
            sc.ServiceName = "Tuio-To-vmulti-Device4";
            string status = "";
            try
            {
                status = sc.Status.ToString();
            }
            catch
            { status = "Does not exist"; }
            if (status == "Running")
            {
                stop_service(sc.ServiceName);
                Sensor4.service_status.Content = get_service_status(sc.ServiceName);
            }
            if (status == "Stopped")
            {
                start_service(sc.ServiceName);
                Sensor4.service_status.Content = get_service_status(sc.ServiceName);
            }
        }

        private void service_Click5(object sender, RoutedEventArgs e)
        {
            ServiceController sc = new ServiceController();
            sc.ServiceName = "Tuio-To-vmulti-Device5";
            string status = "";
            try
            {
                status = sc.Status.ToString();
            }
            catch
            { status = "Does not exist"; }
            if (status == "Running")
            {
                stop_service(sc.ServiceName);
                Sensor5.service_status.Content = get_service_status(sc.ServiceName);
            }
            if (status == "Stopped")
            {
                start_service(sc.ServiceName);
                Sensor5.service_status.Content = get_service_status(sc.ServiceName);
            }
        }

        public string get_service_status(string service_name)
        {
            ServiceController sc = new ServiceController();
            sc.ServiceName = service_name;
            try
            {
                return "The Service is " + sc.Status.ToString();
            }
            catch
            { return service_name + "Does not exist"; }
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

        private void apply_Click(object sender, RoutedEventArgs e){
            do_apply_stuff();
        }

        private void do_apply_stuff()
        {

            System.IO.Directory.CreateDirectory("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data");

            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\tuioport1.txt", Sensor1.tuio_port.Text);
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\tuioport2.txt", Sensor2.tuio_port.Text);
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\tuioport3.txt", Sensor3.tuio_port.Text);
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\tuioport4.txt", Sensor4.tuio_port.Text);
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\tuioport5.txt", Sensor5.tuio_port.Text);
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\inverthorizontal1.txt", Sensor1.invert_horizontal.IsChecked.ToString());
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\inverthorizontal2.txt", Sensor2.invert_horizontal.IsChecked.ToString());
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\inverthorizontal3.txt", Sensor3.invert_horizontal.IsChecked.ToString());
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\inverthorizontal4.txt", Sensor4.invert_horizontal.IsChecked.ToString());
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\inverthorizontal5.txt", Sensor5.invert_horizontal.IsChecked.ToString());
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\invertverticle1.txt", Sensor1.invert_verticle.IsChecked.ToString());
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\invertverticle2.txt", Sensor2.invert_verticle.IsChecked.ToString());
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\invertverticle3.txt", Sensor3.invert_verticle.IsChecked.ToString());
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\invertverticle4.txt", Sensor4.invert_verticle.IsChecked.ToString());
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\invertverticle5.txt", Sensor5.invert_verticle.IsChecked.ToString());
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\x01.txt", Sensor1.x_offset.Text);
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\x02.txt", Sensor2.x_offset.Text);
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\x03.txt", Sensor3.x_offset.Text);
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\x04.txt", Sensor4.x_offset.Text);
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\x05.txt", Sensor5.x_offset.Text);
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\y01.txt", Sensor1.y_offset.Text);
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\y02.txt", Sensor2.y_offset.Text);
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\y03.txt", Sensor3.y_offset.Text);
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\y04.txt", Sensor4.y_offset.Text);
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\y05.txt", Sensor5.y_offset.Text);
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\service1.txt",  get_service_status("Tuio-To-vmulti-Device1"));
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\service2.txt",  get_service_status("Tuio-To-vmulti-Device2"));
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\service3.txt", get_service_status("Tuio-To-vmulti-Device3"));
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\service4.txt",  get_service_status("Tuio-To-vmulti-Device4"));
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\service5.txt",  get_service_status("Tuio-To-vmulti-Device5"));
            
            
            //Installs service if it's not already installed . 
            install_service("Tuio-To-vmulti-Device1", Sensor1.service_name, Sensor1.tuio_port.Text);
            install_service("Tuio-To-vmulti-Device2", Sensor2.service_name, Sensor2.tuio_port.Text);
            install_service("Tuio-To-vmulti-Device3", Sensor3.service_name, Sensor3.tuio_port.Text);
            install_service("Tuio-To-vmulti-Device4", Sensor4.service_name, Sensor4.tuio_port.Text);
            install_service("Tuio-To-vmulti-Device5", Sensor5.service_name, Sensor5.tuio_port.Text);

            start_feedback(Sensor1, "Tuio-To-vmulti-Device1");
            start_feedback(Sensor2, "Tuio-To-vmulti-Device2");
            start_feedback(Sensor3, "Tuio-To-vmulti-Device3");
            start_feedback(Sensor4, "Tuio-To-vmulti-Device4");
            start_feedback(Sensor5, "Tuio-To-vmulti-Device5");
        }

        private void load_values_from_config_files()
        {
            if(System.IO.File.Exists("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\tuioport1.txt")==true)
            {
            Sensor1.tuio_port.Text = System.IO.File.ReadAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\tuioport1.txt");
            Sensor2.tuio_port.Text = System.IO.File.ReadAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\tuioport2.txt");
            Sensor3.tuio_port.Text = System.IO.File.ReadAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\tuioport3.txt");
            Sensor4.tuio_port.Text = System.IO.File.ReadAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\tuioport4.txt");
            Sensor5.tuio_port.Text = System.IO.File.ReadAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\tuioport5.txt");

            if (System.IO.File.ReadAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\inverthorizontal1.txt") == "True") 
                Sensor1.invert_horizontal.IsChecked = true;

            if (System.IO.File.ReadAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\inverthorizontal2.txt") == "True")
                Sensor2.invert_horizontal.IsChecked = true;

            if (System.IO.File.ReadAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\inverthorizontal3.txt") == "True")
                Sensor3.invert_horizontal.IsChecked = true;

            if(System.IO.File.ReadAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\inverthorizontal4.txt")== "True")
                Sensor4.invert_horizontal.IsChecked = true;

            if(System.IO.File.ReadAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\inverthorizontal5.txt")=="True")
                Sensor5.invert_horizontal.IsChecked=true;

            if(System.IO.File.ReadAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\invertverticle1.txt")=="True")
                Sensor1.invert_verticle.IsChecked=true;

            if(System.IO.File.ReadAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\invertverticle2.txt")=="True")
                Sensor2.invert_verticle.IsChecked=true;

            if(System.IO.File.ReadAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\invertverticle3.txt")=="True")
                Sensor3.invert_verticle.IsChecked=true;

           if( System.IO.File.ReadAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\invertverticle4.txt")=="True")
               Sensor4.invert_verticle.IsChecked=true;

            if(System.IO.File.ReadAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\invertverticle5.txt")=="True")
                Sensor5.invert_verticle.IsChecked=true;

            Sensor1.x_offset.Text=System.IO.File.ReadAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\x01.txt");
            Sensor2.x_offset.Text = System.IO.File.ReadAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\x02.txt");
            Sensor3.x_offset.Text = System.IO.File.ReadAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\x03.txt");
            Sensor4.x_offset.Text = System.IO.File.ReadAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\x04.txt");
            Sensor5.x_offset.Text = System.IO.File.ReadAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\x05.txt");
            Sensor1.y_offset.Text = System.IO.File.ReadAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\y01.txt");
            Sensor2.y_offset.Text = System.IO.File.ReadAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\y02.txt");
            Sensor3.y_offset.Text = System.IO.File.ReadAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\y03.txt");
            Sensor4.y_offset.Text = System.IO.File.ReadAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\y04.txt");
            Sensor5.y_offset.Text = System.IO.File.ReadAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\y05.txt");
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\service1.txt", get_service_status("Tuio-To-vmulti-Device1"));
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\service2.txt", get_service_status("Tuio-To-vmulti-Device2"));
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\service3.txt", get_service_status("Tuio-To-vmulti-Device3"));
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\service4.txt", get_service_status("Tuio-To-vmulti-Device4"));
            System.IO.File.WriteAllText("C:\\Users\\AppData\\TUIO-To-Vmulti\\Data\\service5.txt", get_service_status("Tuio-To-vmulti-Device5"));
            
                
            install_service("Tuio-To-vmulti-Device1", Sensor1.service_name, Sensor1.tuio_port.Text);
            install_service("Tuio-To-vmulti-Device2", Sensor2.service_name, Sensor2.tuio_port.Text);
            install_service("Tuio-To-vmulti-Device3", Sensor3.service_name, Sensor3.tuio_port.Text);
            install_service("Tuio-To-vmulti-Device4", Sensor4.service_name, Sensor4.tuio_port.Text);
            install_service("Tuio-To-vmulti-Device5", Sensor5.service_name, Sensor5.tuio_port.Text);


            start_feedback(Sensor1, "Tuio-To-vmulti-Device1");
            start_feedback(Sensor2, "Tuio-To-vmulti-Device2");
            start_feedback(Sensor3, "Tuio-To-vmulti-Device3");
            start_feedback(Sensor4, "Tuio-To-vmulti-Device4");
            start_feedback(Sensor5, "Tuio-To-vmulti-Device5");
            
            }
        }

        public string start_service(string service_name)
        {

            ServiceController sc = new ServiceController();
            sc.ServiceName = service_name;
            try
            {
                Console.WriteLine("The " + service_name + " service status is currently set to {0}",
                                   sc.Status.ToString());
            }
            catch { }
            if (sc.Status == ServiceControllerStatus.Stopped)
            {
                // Start the service if the current status is stopped.
                try
                {
                    Console.WriteLine("Starting the " + service_name + " service...");
                }
                catch { }
                try
                {
                    // Start the service, and wait until its status is "Running".
                    sc.Start();
                    sc.WaitForStatus(ServiceControllerStatus.Running);

                    // Display the current service status.
                    try
                    {
                        Console.WriteLine("The " + service_name + " service status is now set to {0}.",
                                           sc.Status.ToString());
                    }
                    catch { }
                    return "The " + service_name + " service status is now set to " + sc.Status.ToString();
                }
                catch (InvalidOperationException)
                {
                    try
                    {
                        Console.WriteLine("Could not start the " + service_name + " service.");
                    }
                    catch { }
                    return "Could not start the " + service_name + " service.";
                }
            }
            return "Service is already running";
        }

        public string stop_service(string service_name)
        {

            ServiceController sc = new ServiceController();
            sc.ServiceName = service_name;
            try
            {
                Console.WriteLine("The " + service_name + " service status is currently set to {0}",
                                   sc.Status.ToString());
            }
            catch { }
            if (sc.Status == ServiceControllerStatus.Running)
            {
                // Start the service if the current status is stopped.
                try
                {
                    Console.WriteLine("Starting the " + service_name + " service...");
                }
                catch { }
                try
                {
                    // Start the service, and wait until its status is "Running".
                    sc.Stop();
                    sc.WaitForStatus(ServiceControllerStatus.Stopped);

                    // Display the current service status.
                    try
                    {
                        Console.WriteLine("The " + service_name + " service status is now set to {0}.",
                                           sc.Status.ToString());
                    }
                    catch { }
                    return "The " + service_name + " service status is now set to " + sc.Status.ToString();
                }
                catch (InvalidOperationException)
                {
                    try
                    {
                        Console.WriteLine("Could not Stop the " + service_name + " service.");
                    }
                    catch { }
                    return "Could not Stop the " + service_name + " service.";
                }
            }
            return "Service is already Stopped";
        }

        private void ok_Click(object sender, RoutedEventArgs e)
        {
            do_apply_stuff();
            Process.GetCurrentProcess().Kill();

        }

        private void cancel_Click(object sender, RoutedEventArgs e)
        {
            Process.GetCurrentProcess().Kill(); ;
        }

        private void MetroWindow_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            Process.GetCurrentProcess().Kill();
        }
   }
}
 

   