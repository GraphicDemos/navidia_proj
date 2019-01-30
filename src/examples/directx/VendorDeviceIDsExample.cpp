/*****************************************************************************

File:  VendorDeviceIDsExample.cpp

Copyright NVIDIA Corporation 2005
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

******************************************************************************/

// Example code to retrieve vendor and device ID's for the primary display
// device.
//

#include <windows.h>

#include <string>
#include <iostream>
#include <sstream>
#include <conio.h>

#include "stdlib.h"
#include "NVIDIA_DEVICE_INFO.h"

using namespace std;

bool GetDeviceIdentification(string &vendorID, string &deviceID)
{
	DISPLAY_DEVICEA dd;
    dd.cb = sizeof(DISPLAY_DEVICEA);

    int i = 0;
    string id;

    // locate primary display device
    while (EnumDisplayDevicesA(NULL, i, &dd, 0))
    {
        if (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
        {
            id = dd.DeviceID;
            break;
        }

        i++;
    }

    if (id == "") return false;

    // get vendor ID
    vendorID = id.substr(8, 4);

    // get device ID
    deviceID = id.substr(17, 4);

    return true;
}

int main(void)
{
    string vendorID;
    string deviceID;
	stringstream SS;

	GetDeviceIdentification(vendorID, deviceID);

	char vendorIDChar[6], deviceIDChar[6];
	
	SS << vendorID; 
	SS >> vendorIDChar;
	SS.clear();
	SS << deviceID; 
	SS >> deviceIDChar;
	int base = 16;

    unsigned long vendorIDLong;
    unsigned long deviceIDLong;

	vendorIDLong = strtoul(vendorIDChar, NULL, 16);
	deviceIDLong = strtoul(deviceIDChar, NULL, 16);
	
    if(vendorIDLong == 0x12D2)
    {
		cout << "Vendor: NVIDIA" << endl;
		switch(deviceIDLong)
        {
			case 0x0018:
			case 0x0019:
			cout << "Device: " << "RIVA 128" << endl;
        }
    }
	else if(vendorIDLong == DT_NVIDIA_VENDOR_ID)        
    {
		bool findNVIDIADevice = false;
		cout << "Vendor: NVIDIA" << endl << endl;
		for (int i = 0; i<NVIDIA_DEVICE_TOTAL;i++) {
			if (NVIDIA_DEVICE_INFO[i].deviceID == deviceIDLong)
			{
				findNVIDIADevice = true;
				cout << "Device ID: 0x" << deviceIDChar << endl;
				cout << "Chip Name: " << NVIDIA_DEVICE_INFO[i].chipIDName << endl;
				i = NVIDIA_DEVICE_TOTAL;
			}
		}
		if (!findNVIDIADevice) 
			cout << "Device: Unable to locate device information" << endl;
    } else {
		cout << "Vendor: Unknown" << endl;
		cout << "Device: Unknown" << endl;
	}

	cout << endl << "- Hit any key to end - " << endl;
    getch();

    return 0;
}