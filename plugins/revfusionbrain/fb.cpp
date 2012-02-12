#include "fb.h"

FB::FB()
{
	libusb_init(&context);
	libusb_set_debug(context,3);
}
FB::~FB()
{
	libusb_close(fusionBrainHandle);
	libusb_free_device_list(devs,1);
	libusb_exit(context);
}
void FB::init()
{
	libusb_device *dev;
	libusb_get_device_list(NULL,&devs);
	int i=0;
	while ((dev = devs[i++]) != NULL)
	{
		struct libusb_device_descriptor desc;
		libusb_get_device_descriptor(dev,&desc);
		if ((desc.idVendor == 0x04d8) && (desc.idProduct == 0x000E))
		{
			printf("Found fusion brain v4\n");
			fusionBrain = dev;
			return;
		}
	}

}
bool FB::findFB()
{
	libusb_device *dev;
	libusb_get_device_list(NULL,&devs);
	int i=0;
	while ((dev = devs[i++]) != NULL)
	{
		struct libusb_device_descriptor desc;
		libusb_get_device_descriptor(dev,&desc);
		if ((desc.idVendor == 0x04d8) && (desc.idProduct == 0x000E))
		{
			printf("Found fusion brain v4\n");
			fusionBrain = dev;
			return true;
		}
	}
	return false;
}

void FB::connect()
{
	printf("Trying to open...\n");
	if (libusb_open(fusionBrain,&fusionBrainHandle))
	{
		printf("ERROR\n");
		return;
	}
	printf("Opened... trying to claim...\n");
	libusb_claim_interface(fusionBrainHandle,0);
}
void FB::setSingleOutput(int ionum, bool active)
{
	ionum--;
	unsigned char data_send[64];
	for (int i=0;i<64;i++)
	{
		data_send[i] = 0;
	}
	if (active)
	{
		data_send[ionum*2] = 0b00000001;
	}
	else
	{
		data_send[ionum*2] = 0b00000000;
	}
	//data_send[60] = 0;
	int actualSent;
	unsigned char EP_OUT = 0x01;
	unsigned char EP_IN = 0x81;
	unsigned char data_recv[64];
	for (int i=0;i<64;i++)
	{
		data_recv[i]=0;
	}
	//data_in[0] = 1;
	int actualReceived;
	int dataSendResult = 0;
	int dataRecvResult=0;
	//while (true)
	//{
	bool stop = false;
	while (!stop)
	{
		dataRecvResult = libusb_bulk_transfer(fusionBrainHandle,EP_IN,data_recv,64,&actualReceived,500);
		printf("RX: %i : %i\n",dataRecvResult,actualReceived);
		printf("1: %i\n",data_recv[0]);
		if (active)
		{
			data_recv[ionum*2] = 0b00000001;
		}
		else
		{
			data_recv[ionum*2] = 0b00000000;
		}
		dataSendResult = libusb_bulk_transfer(fusionBrainHandle,EP_OUT,data_recv,64,&actualSent,500);
		printf("TX: %i : %i\n",dataSendResult,actualSent);
		dataRecvResult = libusb_bulk_transfer(fusionBrainHandle,EP_IN,data_recv,64,&actualReceived,500);
		if (active)
		{
			if (data_recv[ionum*2] == 1)
			{
				stop = true;
			}
		}
		else
		{
			if (data_recv[ionum*2] == 0)
			{
				stop = true;
			}
		}
	}
	//printf("1: %i\n",data_in[4]);
}
void FB::setAllOutput(unsigned char low, unsigned char high)
{
	unsigned char data_send[64];
	int actualSent;
	unsigned char EP_OUT = 0x01;
	unsigned char EP_IN = 0x81;
	unsigned char data_recv[64];
	for (int i=0;i<64;i++)
	{
		data_recv[i]=0;
	}
	//data_in[0] = 1;
	int actualReceived;
	int dataSendResult = 0;
	int dataRecvResult=0;
	//while (true)
	//{
	bool stop = false;
	while (!stop)
	{
		dataRecvResult = libusb_bulk_transfer(fusionBrainHandle,EP_IN,data_recv,64,&actualReceived,500);
		printf("RX: %i : %i\n",dataRecvResult,actualReceived);
		printf("1: %i\n",data_recv[0]);
	for (int i=0;i<8;i++)
	{
		data_recv[i * 2] = (low << i) & 1;
		data_recv[(i+8)*2] = (high << i) & 1;
	}
		dataSendResult = libusb_bulk_transfer(fusionBrainHandle,EP_OUT,data_recv,64,&actualSent,500);
		printf("TX: %i : %i\n",dataSendResult,actualSent);
		dataRecvResult = libusb_bulk_transfer(fusionBrainHandle,EP_IN,data_recv,64,&actualReceived,500);
		stop = true;
		for (int i=0;i<8;i++)
		{
			if ((data_recv[i * 2] != (low << i) & 1) || (data_recv[(i + 8) * 2] != (high << i) & 1))
			{
				stop = false;
			}
		}
	}
}
int FB::getAllOutputStatus()
{
	unsigned char data_send[64];
	int actualSent;
	unsigned char EP_OUT = 0x01;
	unsigned char EP_IN = 0x81;
	unsigned char data_recv[64];
	for (int i=0;i<64;i++)
	{
		data_recv[i]=0;
	}
	//data_in[0] = 1;
	int actualReceived;
	int dataSendResult = 0;
	int dataRecvResult=0;
	//while (true)
	//{
	bool stop = false;
	dataRecvResult = libusb_bulk_transfer(fusionBrainHandle,EP_IN,data_recv,64,&actualReceived,500);
	int retVar=0;
	for (int i=0;i<16;i++)
	{
		retVar |= (1 * data_recv[i*2]) << i;
	}
	return retVar;
}
