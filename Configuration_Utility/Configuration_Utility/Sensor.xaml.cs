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
using TUIO;
using System.Windows.Forms;
using System.Diagnostics;
using System.ServiceProcess;
using System.Runtime.InteropServices;

namespace Configuration_Utility
{
    /// <summary>
    /// Interaction logic for Sensor.xaml
    /// </summary>
    public partial class Sensor : System.Windows.Controls.UserControl, TuioListener
    {
        [DllImport("kernel32.dll")]
        static extern IntPtr GetConsoleWindow();

        [DllImport("user32.dll")]
        static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);

        const int SW_HIDE = 0;
        const int SW_SHOW = 5;

        [DllImport("Kernel32")]
        public static extern void AllocConsole();

        [DllImport("Kernel32")]
        public static extern void FreeConsole();
        private Dictionary<long, TuioCursor> cursorList;
        private object cursorSync = new object();
        private object objectSync = new object();
        Sensor Listener;
        public string service_name = "";
        public Sensor()
        {
            InitializeComponent();
            x_offset.Text = "0";
            y_offset.Text = "0";
            tuio_port.Text = "";
            Listener = this;
            //addtuioclient();
            //this.Width = System.Windows.Application.Current.MainWindow.Width;
            cursorList = new Dictionary<long, TuioCursor>(128);

            debug.IsChecked = true;
        }
        public TuioClient client = null;
        Timer timer1;
        public void addtuioclient()
        {
            client = null;
            client = new TuioClient(Convert.ToInt32(tuio_port.Text));

            if (client != null)
            {
                client.addTuioListener(Listener);
                client.connect();
                Console.WriteLine("listening to TUIO messages at port " + client.getPort());
                timer1 = new Timer();
                timer1.Interval = 100;
                timer1.Enabled = true;
                timer1.Tick += new System.EventHandler(OnTimerEvent);
            }
            else Console.WriteLine("usage: java TuioDump [port]");
        }

        public void removetuioclinet()
        {

            if (client != null)
            {
                // Console.WriteLine("test 5.1");
                client.removeAllTuioListeners();
                // Console.WriteLine("test 5.2");
                client.disconnect();
                // Console.WriteLine("test 5.3");
            }

        }

        public void OnTimerEvent(object source, EventArgs e)
        {
            // Console.WriteLine(System.IO.Path.GetDirectoryName(System.Diagnostics.Process.GetCurrentProcess().MainModule.FileName) + "\\" + service_name);
            VisualFeedback.Children.Clear();
            if (cursorList.Count > 0)
            {
                lock (cursorSync)
                {
                    foreach (TuioCursor tcur in cursorList.Values)
                    {

                        List<TuioPoint> path = tcur.getPath();
                        TuioPoint current_point = path[0];

                        for (int i = 0; i < path.Count; i++)
                        {
                            TuioPoint next_point = path[i];
                            current_point = next_point;
                        }
                        double x;
                        double y;
                        if (invert_horizontal.IsChecked == true)
                        {
                            x = VisualFeedback.Width - (current_point.getScreenX((int)VisualFeedback.Width) - VisualFeedback.Height / 100);
                        }
                        else
                        {
                            x = current_point.getScreenX((int)VisualFeedback.Width) - VisualFeedback.Height / 100;
                        }
                        if (invert_verticle.IsChecked == true)
                        {
                            y = VisualFeedback.Height - (current_point.getScreenY((int)VisualFeedback.Height) - VisualFeedback.Height / 100);
                        }
                        else
                        {
                            y = current_point.getScreenY((int)VisualFeedback.Height) - VisualFeedback.Height / 100;
                        }
                        try
                        {
                            if (Convert.ToInt32(x_offset.Text) != 0)
                            {
                                x = x + (int)VisualFeedback.Width * Convert.ToInt32(x_offset.Text) / 100;
                            }
                            if (Convert.ToInt32(y_offset.Text) != 0)
                            {
                                y = y + (int)VisualFeedback.Height * Convert.ToInt32(y_offset.Text) / 100;
                            }
                        }
                        catch
                        {
                        }
                        Ellipse ellipse = new Ellipse
                        {
                            Fill = new SolidColorBrush(Colors.CadetBlue),
                            Width = 15,
                            Height = 15,
                            Opacity = 1,
                            Margin = new Thickness(x, y, 0, 0)
                        };


                        VisualFeedback.Children.Add(ellipse);

                        //Font font = new Font("Arial", 10.0f);
                        //      Sensor1.DrawString(tcur.getCursorID() + "", font, blackBrush, new PointF(tcur.getScreenX(width) - 10, tcur.getScreenY(height) - 10));

                    }
                }
            }
            else
            {
                l2 = new System.Windows.Controls.Label();
                l2.Content = "Touch Your Sensor";
                VisualFeedback.Children.Add(l2);
                Canvas.SetLeft(l2, 35);
                Canvas.SetTop(l2, 130);

            }
        }
        System.Windows.Controls.Label l2;
        public void addTuioObject(TuioObject tobj)
        {
            Console.WriteLine("add obj " + tobj.getSymbolID() + " " + tobj.getSessionID() + " " + tobj.getX() + " " + tobj.getY() + " " + tobj.getAngle());
        }

        public void updateTuioObject(TuioObject tobj)
        {
            Console.WriteLine("set obj " + tobj.getSymbolID() + " " + tobj.getSessionID() + " " + tobj.getX() + " " + tobj.getY() + " " + tobj.getAngle() + " " + tobj.getMotionSpeed() + " " + tobj.getRotationSpeed() + " " + tobj.getMotionAccel() + " " + tobj.getRotationAccel());
        }

        public void removeTuioObject(TuioObject tobj)
        {
            Console.WriteLine("del obj " + tobj.getSymbolID() + " " + tobj.getSessionID());
        }

        public void addTuioCursor(TuioCursor tcur)
        {
            lock (cursorSync)
            {
                cursorList.Add(tcur.getSessionID(), tcur);
            }

            Console.WriteLine("add cur " + tcur.getCursorID() + " (" + tcur.getSessionID() + ") " + tcur.getX() + " " + tcur.getY());
            //  VisualFeedback.InvalidateVisual();
        }

        public void updateTuioCursor(TuioCursor tcur)
        {
            Console.WriteLine("set cur " + tcur.getCursorID() + " (" + tcur.getSessionID() + ") " + tcur.getX() + " " + tcur.getY() + " " + tcur.getMotionSpeed() + " " + tcur.getMotionAccel());
        }

        public void removeTuioCursor(TuioCursor tcur)
        {
            lock (cursorSync)
            {
                cursorList.Remove(tcur.getSessionID());
            }
            Console.WriteLine("del cur " + tcur.getCursorID() + " (" + tcur.getSessionID() + ")");
        }

        public void refresh(TuioTime frameTime)
        {
            //Console.WriteLine("refresh "+frameTime.getTotalMilliseconds());
        }
        public delegate void UpdateStatusBarEventHandler(bool message);

        public event UpdateStatusBarEventHandler UpdateStatusBar;
        bool t1 = true, t2 = true, t3 = true, t4 = true, t5 = true;
        private void tuio_port_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (IsNumeric(tuio_port.Text) != true)
            {
                port_test.Content = "Only Numeric Values are allowed";
                t1 = false;
                if (null != UpdateStatusBar)
                {
                    UpdateStatusBar(false);
                }
            }
            else
            {
                t1 = true;
                port_test.Content = "";
                if (null != UpdateStatusBar && t1 == true && t2 == true && t3 == true && t4 == true && t5 == true)
                {
                    UpdateStatusBar(true);
                }
            }

        }

        private void invert_horizontal_Click(object sender, RoutedEventArgs e)
        {

        }

        private void invert_verticle_Click(object sender, RoutedEventArgs e)
        {

        }

        private void x_offset_TextChanged(object sender, TextChangedEventArgs e)
        {
            //    if (x_offset.Text != null)
            //    {
            if (IsNumeric(x_offset.Text) != true)
            {
                t2 = false;
                xoffset_test.Content = "Only Numeric Value between -100 to 100 is allowed";
                if (null != UpdateStatusBar)
                {
                    UpdateStatusBar(false);
                }
            }
            else
            {
                if (Convert.ToInt32(x_offset.Text) >= -100 && Convert.ToInt32(x_offset.Text) <= 100)
                {
                    t2 = true;
                    xoffset_test.Content = "";
                    if (null != UpdateStatusBar && t1 == true && t2 == true && t3 == true && t4 == true && t5 == true)
                    {
                        UpdateStatusBar(true);
                    }
                }
                else
                {
                    t2 = false;
                    xoffset_test.Content = "Only Numeric Value between -100 to 100 is allowed";
                    if (null != UpdateStatusBar)
                    {
                        UpdateStatusBar(false);
                    }
                }
            }


            //  }
        }

        private void y_offset_TextChanged(object sender, TextChangedEventArgs e)
        {
            //    if (y_offset.Text != null && y_offset.Text != "")
            //    {
            if (IsNumeric(y_offset.Text) != true)
            {
                t2 = false;
                yoffset_test.Content = "Only Numeric Value betweem -100 to 100 is allowed";
                if (null != UpdateStatusBar)
                {
                    UpdateStatusBar(false);
                }

            }
            else
            {
                if (Convert.ToInt32(y_offset.Text) >= -100 && Convert.ToInt32(y_offset.Text) <= 100)
                {
                    t2 = true;
                    yoffset_test.Content = "";
                    if (null != UpdateStatusBar && t1 == true && t2 == true && t3 == true && t4 == true && t5 == true)
                    {
                        UpdateStatusBar(true);
                    }
                }
                else
                {
                    t2 = false;
                    yoffset_test.Content = "Only Numeric Value betweem -100 to 100 is allowed";
                    if (null != UpdateStatusBar)
                    {
                        UpdateStatusBar(false);
                    }
                }
            }
            //   }
        }

        private void UserControl_Loaded(object sender, RoutedEventArgs e)
        {

        }

        private void debug_Click(object sender, RoutedEventArgs e)
        {
            var handle = GetConsoleWindow();

            if (debug.IsChecked == true)
            {
                // Show
                ShowWindow(handle, SW_SHOW);
            }
            if (debug.IsChecked == false)
            {
                // Hide
                ShowWindow(handle, SW_HIDE);
            }
        }

        private bool IsNumeric(string chkNumeric)
        {
            int intOutVal;
            bool isValidNumeric = false;
            isValidNumeric = int.TryParse(chkNumeric, out intOutVal);
            return isValidNumeric;
        }




    }



}
