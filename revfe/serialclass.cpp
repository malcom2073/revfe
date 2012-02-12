/***************************************************************************
*   Copyright (C) 2009 by Michael Carpenter (malcom2073)                  *
*   mcarpenter@interforcesystems.com                                      *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#include "serialclass.h"

#include <QDebug>
SerialClass::SerialClass()
{

}

int SerialClass::openPort(QString portName)
{
	return openPort(portName,9600);
}
int SerialClass::openPort(QString portName,int baudrate)
{
#ifdef Q_OS_WIN32
	portHandle=CreateFileA(portName.toStdString().c_str(), GENERIC_READ|GENERIC_WRITE,0, NULL, OPEN_EXISTING, 0, NULL);
	if (portHandle == INVALID_HANDLE_VALUE)
	{
		return -1;
	}
    COMMCONFIG Win_CommConfig;
	COMMTIMEOUTS Win_CommTimeouts;
	unsigned long confSize = sizeof(COMMCONFIG);
	Win_CommConfig.dwSize = confSize;
	GetCommConfig(portHandle, &Win_CommConfig, &confSize);
	Win_CommConfig.dcb.Parity = 0;
	Win_CommConfig.dcb.fRtsControl = RTS_CONTROL_DISABLE;
	Win_CommConfig.dcb.fOutxCtsFlow = FALSE;
	Win_CommConfig.dcb.fOutxDsrFlow = FALSE;
	//Win_CommConfig.dcb.fDtrControl = DTR_CONTROL_DISABLE;
	Win_CommConfig.dcb.fDtrControl = DTR_CONTROL_ENABLE;
	Win_CommConfig.dcb.fDsrSensitivity = FALSE;
	Win_CommConfig.dcb.fNull=FALSE;
	Win_CommConfig.dcb.fTXContinueOnXoff = FALSE;
	Win_CommConfig.dcb.fInX=FALSE;
    Win_CommConfig.dcb.fOutX=FALSE;
    Win_CommConfig.dcb.fBinary=TRUE;
	Win_CommConfig.dcb.DCBlength = sizeof(DCB);
	Win_CommConfig.dcb.BaudRate = baudrate;
	Win_CommConfig.dcb.ByteSize = 8;
	Win_CommTimeouts.ReadIntervalTimeout = 50;
	Win_CommTimeouts.ReadTotalTimeoutMultiplier = 0;
	Win_CommTimeouts.ReadTotalTimeoutConstant = 110;
	Win_CommTimeouts.WriteTotalTimeoutMultiplier = 0;
	Win_CommTimeouts.WriteTotalTimeoutConstant = 110;
	SetCommConfig(portHandle, &Win_CommConfig, sizeof(COMMCONFIG));
	SetCommTimeouts(portHandle,&Win_CommTimeouts);
	#else
	#ifdef Q_OS_LINUX
	//printf("Attempting to open COM port\n");
	portHandle = open(portName.toStdString().c_str(),O_RDWR | O_NOCTTY);
	if (portHandle < 0)
	{
		printf("Error opening Com\n");
		return -1;
	}
	printf("Com Port Opened %i\n",portHandle);
	struct termios oldtio;
	struct termios newtio;
	tcgetattr(portHandle,&oldtio);
	bzero(&newtio,sizeof(newtio));

	long BAUD = B9600;  
	switch (baudrate)
	{
		case 19200:
			BAUD  = B19200;
			break;
		case 9600:
			BAUD  = B9600;
			break;
		case 4800:
			BAUD  = B4800;
			break;
		case 38400:
		default:
			BAUD = B38400;
			break;

	}  //end of switch baud_rate
	printf("Baud: %i\n",baudrate);
	newtio.c_cflag = BAUD | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR | ICRNL;
	newtio.c_oflag = 0;
	newtio.c_lflag &= ~ICANON;
	
	tcflush(portHandle,TCIFLUSH);
	tcsetattr(portHandle,TCSANOW,&newtio);
	fcntl(portHandle,F_SETFL,FNDELAY);
	return 0;
#endif
#endif



	return -1;
}
void SerialClass::closePort()
{
	#ifdef Q_OS_WIN32
	CloseHandle(portHandle);
	#else
	#ifdef Q_OS_LINUX
	close(portHandle);
	#endif
	#endif
}
int SerialClass::read(char *arr,int len)
{
	int length;
	#ifdef Q_OS_WIN32
	::ReadFile(portHandle,(void*)arr,len,(LPDWORD)&length,NULL);
	#else
	#ifdef Q_OS_LINUX
	#endif
	#endif
	return length;
}
int SerialClass::write(QByteArray arr)
{
	int len;
	#ifdef Q_OS_WIN32
	::WriteFile(portHandle,(void*)arr.data(),(DWORD)arr.length(),(LPDWORD)&len,NULL);
	#else
	#ifdef Q_OS_LINUX
	#endif
	#endif
	return len;
}
int SerialClass::writeLine(QString line)
{
	int len;
	#ifdef Q_OS_WIN32
	::WriteFile(portHandle,(void*)line.toStdString().c_str(),(DWORD)line.length(),(LPDWORD)&len,NULL);
	#else
	#ifdef Q_OS_LINUX
	len = ::write(portHandle,line.toStdString().c_str(),line.length());
	#endif
	#endif
	return len;
}
QString SerialClass::readLine()
{
	QString retVal = "";
	char buffer[1024];
	int readBytes = 0;
	bool endLine = false;
	int count = 0;
	while (!endLine)
	{
		char tmp;
		//ReadFile(portHandle,(void*)buffer,1024,(LPDWORD)&readBytes,NULL);
		#ifdef Q_OS_WIN32
		ReadFile(portHandle,(void*)&tmp,1,(LPDWORD)&readBytes,NULL);
		#else
		#ifdef Q_OS_LINUX
		readBytes = ::read(portHandle,&buffer,1024);
		#endif
		#endif
		if (readBytes == 0)
		{
			#ifdef Q_OS_WIN32
			Sleep(100);
			#else
			#ifdef Q_OS_LINUX
			usleep(100000);
			#endif
			#endif
			if (count++ > 10)
			{
				return "";
			}
			//return "";
		}
		else if (readBytes != -1)
		{
			for (int i=0;i<readBytes;i++)
			{
				if (buffer[i] == '\n')
				{
					lineList.append(bufferString);
					bufferString = "";
					//endLine = true;
				}
				else
				{
					bufferString.append(QString(buffer[i]));
				}
			}
			endLine = true;
			/*
			for (int i=0;i<readBytes;i++)
			{
				if (buffer[i] == '\r')
				{
					endLine = true;
					break;
				}
				else
				{
					retVal.append(QString(buffer[i]));
				}
			}
			printf("%s %i\n",buffer,readBytes);
			*/
			/*if (tmp == '\r')
			{
				endLine = true;
				break;
			}
			//printf("%i\n",readBytes);
			retVal.append(QString(tmp));
			printf("Line: %s\n",retVal.toStdString().c_str());
			*/
		}
		else
		{
			printf("ERROR in Read!\n");
			perror("Wee:\n");
			return "";
		}
	}
	//tcflush(serial_fd,TCIFLUSH);
	if (lineList.count() > 0)
	{
		QString line = lineList[0];
		lineList.removeAt(0);
		return line;
	}
	return "";
}
