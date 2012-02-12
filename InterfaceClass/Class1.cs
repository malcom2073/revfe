using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
/*namespace InterfaceClasses
{
	public interface class IPluginHost
	{
	public:
		void passCoreMessage(System::String ^sender, System::String ^message);
	};
}*/
namespace InterfaceClasses
{
    public interface IPluginHost
    {
        void passCoreMessage(String sender, String message);
    }
    public class IPCMessage
    {
        public IPCMessage()
        {
            
        }
        public IPCMessage(String message)
        {
            _args = new List<string>();
            try
            {
                if ((message.IndexOf(':') != -1) && (message.IndexOf(' ') != -1) && (message.Split(' ').Length >= 2))
                {
                    _valid = true;
                    _target = message.Split(':')[0];
                    int first = message.IndexOf(" ");
                    int second = message.IndexOf(" ", first + 1);
                    _class = message.Substring(message.IndexOf(":") + 1, first - (message.IndexOf(":") + 1));
                    _method = message.Substring(first + 1, second - (first + 1));
                    for (int i = 0; i < message.Length; i++)
                    {
                        if (message[i] == '!')
                        {
                            int index = message.IndexOf('!', i + 1);
                            if (index != -1)
                            {
                                _args.Add(message.Substring(i + 1, index - (i + 1)));
                                i = index;
                            }
                            else
                            {
                                //unknown error
                            }
                        }
                    }
                    String pluginArgs = message.Substring(message.IndexOf("!") + 1, message.LastIndexOf("!") - (message.IndexOf("!") + 1));
                    _args.Add(pluginArgs);
                    _method = _method.Substring(0, _method.IndexOf(" "));


                }
            }
            catch (Exception ex)
            {
                    
            }
        }
        public String getTarget()
        {
            return _target;
        }
        public String getClass()
        {
            return _class;
        }
        public String getMethod()
        {
            return _method;
        }
        public List<String> getArgs()
        {
            return _args;
        }
        private bool _valid;
        private String _target;
        private String _class;
        private String _method;
        private List<String> _args;
    }
}
