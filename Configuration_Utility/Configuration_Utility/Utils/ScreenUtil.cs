using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace Configuration_Utility.Utils
{
    static class ScreenUtil
    {

        /// <summary>
        /// Returns the Number of Screens Connected.
        /// </summary>
        public static Screen[] Screens
        {
            get
            {
                return Screen.AllScreens;
            }
        }

    }
}
