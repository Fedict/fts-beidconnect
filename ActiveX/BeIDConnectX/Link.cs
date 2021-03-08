using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.Diagnostics;
using System.IO;
using System.Net;


namespace BeIDConnectX
{
    [ProgId("be.bosa.beidconnect")]
    [ClassInterface(ClassInterfaceType.AutoDual)]
    [Guid("B08A638E-66C3-4586-823A-5B89A0301920")]
    [ComVisible(true)]
    public class Link : IObjectSafetyImpl
    {
        [ComVisible(true)]
        public String sendNativeMessage(String message)
        {
              Process process = new Process();

#if ADMIN
            string programFilesfolder = Environment.GetFolderPath(Environment.SpecialFolder.ProgramFilesX86);
            if (programFilesfolder == string.Empty)
            {
                programFilesfolder = Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles);
            }
#else
            string programFilesfolder = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);
#endif            

            process.StartInfo.FileName = programFilesfolder + "\\BOSA\\BeIDConnect\\beidconnect.exe";

            process.StartInfo.Arguments = "";
            process.StartInfo.UseShellExecute = false;
            process.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;
            process.StartInfo.CreateNoWindow = true;
            process.StartInfo.RedirectStandardInput = true;
            process.StartInfo.RedirectStandardOutput = true;

            process.Start();

            StreamWriter streamWriter = process.StandardInput;
            StreamReader streamReader = process.StandardOutput;

            streamWriter.AutoFlush = false;

            BinaryWriter binaryWriter = new BinaryWriter(streamWriter.BaseStream);

            binaryWriter.Write(message.Length);
            binaryWriter.Write(System.Text.Encoding.ASCII.GetBytes(message));

            streamWriter.Flush();
            binaryWriter.Close();

            BinaryReader binaryReader = new BinaryReader(streamReader.BaseStream, Encoding.ASCII);

            int lengthToRead = binaryReader.ReadInt32();

            String response = Encoding.ASCII.GetString(binaryReader.ReadBytes(lengthToRead));

            binaryReader.Close();

            process.WaitForExit();
            process.Close();

            return response;

        }

        [ComVisible(true)]
        public bool IsPresent()
        {
            return true;
        }
    }
}
