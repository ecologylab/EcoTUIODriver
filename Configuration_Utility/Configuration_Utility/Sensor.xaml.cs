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

namespace Configuration_Utility
{
    /// <summary>
    /// Interaction logic for Sensor.xaml
    /// </summary>
    public partial class Sensor : System.Windows.Controls.UserControl ,TuioListener
    {
        int port = 3333;
        private Dictionary<long, TuioCursor> cursorList;
        private object cursorSync = new object();
        private object objectSync = new object();
        Sensor Listener;
        public Sensor()
        {
            InitializeComponent();
            Listener = this;
            addtuioclient();
         //   this.Width = System.Windows.Application.Current.MainWindow.Width;
            cursorList = new Dictionary<long, TuioCursor>(128);
            Timer timer1 = new Timer();
            timer1.Interval = 100;
            timer1.Enabled = true;
            timer1.Tick += new System.EventHandler(OnTimerEvent);
        }

        public void addtuioclient()
        {
            TuioClient client = null;

            client = new TuioClient(port);

            if (client != null)
            {
                client.addTuioListener(Listener);
                client.connect();
                Console.WriteLine("listening to TUIO messages at port " + client.getPort());

            }
            else Console.WriteLine("usage: java TuioDump [port]");
        }

        public void OnTimerEvent(object source, EventArgs e)
        {
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
                        Ellipse ellipse = new Ellipse
                        {
                            Fill = new SolidColorBrush(Colors.CadetBlue),
                            Width = 15,
                            Height = 15,
                            Opacity = 1,
                            Margin = new Thickness(current_point.getScreenX((int)VisualFeedback.Width) - VisualFeedback.Height / 100, current_point.getScreenY((int)VisualFeedback.Height) - VisualFeedback.Height / 100, 0, 0)
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
                l2.Content = "( Touch Your Touch Screen )";
                VisualFeedback.Children.Add(l2);
                Canvas.SetLeft(l2, 60);
                Canvas.SetTop(l2, 270);
                
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

        private void tuio_port_TextChanged(object sender, TextChangedEventArgs e)
        {
            
        }

        private void invert_horizontal_Click(object sender, RoutedEventArgs e)
        {

        }

        private void invert_verticle_Click(object sender, RoutedEventArgs e)
        {

        }

        private void x_offset_TextChanged(object sender, TextChangedEventArgs e)
        {

        }

        private void y_offset_TextChanged(object sender, TextChangedEventArgs e)
        {

        }
        
    }


    
}
