using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using InterfaceClasses;

public class Plugin
{
    
    public Plugin()
    {

    }
    InterfaceClasses.IPluginHost _hostPlugin;
    public String Name()
    {
        return "SamplePlugin";
    }
    public String Author()
    {
        return "Michael Carpenter";
    }
    public String Description()
    {
        return "A small sample plugin";
    }
    public String Version()
    {
        return "1.0.0";
    }
    public void Initialize(InterfaceClasses.IPluginHost host)
    {
        _hostPlugin = host;
    }
    public void passPluginMessage(String sender, String message)
    {
        try
        {
            IPCMessage ipcMessage = new IPCMessage(message);
            if (ipcMessage.getClass() == "test")
            {
                if (ipcMessage.getMethod() == "play")
                {
                    _hostPlugin.passCoreMessage("SamplePlugin", "MediaEngine:media play");
                }
            }
        }
        catch (Exception ex)
        {
            MessageBox.Show(ex.Message);
        }

        
    }

}