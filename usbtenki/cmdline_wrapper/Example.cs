using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


namespace DracalCSexample
{
    class Program
    {
        static void Main(string[] args)
        {
            Process usbtenki = new Process();
            usbtenki.StartInfo.FileName = "usbtenkiget";
            usbtenki.StartInfo.Arguments = "-i 0,1,2";
            usbtenki.StartInfo.UseShellExecute = false;
            usbtenki.StartInfo.RedirectStandardOutput = true;

            try
            {
                usbtenki.Start();
            }
            catch (Exception e)
            {
                Console.Error.WriteLine("could not run usbtenkiget: " + e);
                return;
            }

            usbtenki.WaitForExit();

            String output = usbtenki.StandardOutput.ReadLine();

            if (output == null)
            {
                Console.Error.WriteLine("usbtenkiget did not return data");
                return;
            }

            float[] fields = output.Split(',').Select(field => float.Parse(field.Trim())).ToArray();

            if (fields.Length != 3)
            {
                Console.Error.WriteLine("usbtenkiget returned an incorrect number of fields");
                return;
            }

            Console.WriteLine("Temperature (C): " + fields[0]);
            Console.WriteLine("RH......... (%): " + fields[1]);
            Console.WriteLine("Pressure. (kPa): " + fields[2]);
            Console.WriteLine("Temperature (F): " + (fields[0] * 9 / 5 + 32));
        }
    }
}
