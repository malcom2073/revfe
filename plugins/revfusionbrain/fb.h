#include <stdio.h>
//#include <usb.h>
#include <libusb.h>

class FB
{
public:
	FB();
	~FB();
	void init();
	bool findFB();
	void connect();
	void setSingleOutput(int ionum, bool active);
	void setAllOutput(unsigned char low, unsigned char high);
	int getAllOutputStatus();
private:
	libusb_device *fusionBrain;
	libusb_device_handle *fusionBrainHandle;
	libusb_device **devs;
	libusb_context *context;
};

