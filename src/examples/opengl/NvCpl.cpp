
#include <stdio.h>
#include <conio.h>
#include "NvCpl.h"

/*******************************************************************************

    Main

*******************************************************************************/

static void InitInfo(HINSTANCE);
static void DesktopConfiguration(HINSTANCE);
static void GPUConfiguration(HINSTANCE);

int main(int argc, char* argv[])
{
	printf("-- NVCPL Code Sample --\n\n");
    // Load NVCPL library
    HINSTANCE hLib = ::LoadLibrary(L"NVCPL.dll");
    if (hLib == 0) {
        printf("Unable to load NVCPL.dll\n");
        return -1;
    }
    // MMW: It's possible to load nvcpl even when current the graphics 
    // card is not Nvidia: therefore also need to check for Nvidia 
    // card to make sure the results of these queries reflect reality.

	InitInfo(hLib);

    printf("\nDo you want to test the GPU configuration functions? [y|n]");
    char c = getch();
    printf("\n");
    if (c == 'y')
    {
        // Use the GPU configuration functions
        GPUConfiguration(hLib);
    }

    printf("\nDo you want to test the Desktop configuration functions? [y|n]");
    c = getch();
    printf("\n");
    if (c == 'y')
    {
        // Use the desktop configuration functions if requested
        DesktopConfiguration(hLib);
    }

    // Free NVCPL library
    ::FreeLibrary(hLib);

	return 0;
}

/*******************************************************************************

    Initial Info Display

*******************************************************************************/

static void InitInfo(HINSTANCE hLib)
{	
	// Get the NvGetDisplayInfo function pointer from the library
	NVDISPLAYINFO displayInfo = {0};
    fNvGetDisplayInfo pfNvGetDisplayInfo = (fNvGetDisplayInfo)::GetProcAddress(hLib, "NvGetDisplayInfo");
    if (pfNvGetDisplayInfo == NULL)
        printf("Unable to get a pointer to NvGetDisplayInfo\n");
    else {
		// displayInfo.cbSize must be set to size of structure
		// displayInfo.dwInputFields1 must be set before call to indicate which fields to retrieve
		// displayInfo.dwOutputFields1 will be set on return to indicate which fields were successfully retrived
		// see NVDISPLAYINFO1_* bit definitions for field information, use 0xffffffff to retrieve all fields
		memset(&displayInfo, 0, sizeof(displayInfo));
		displayInfo.cbSize = sizeof(displayInfo);
		displayInfo.dwInputFields1 = 0xffffffff; // 0xffffffff means all fields should be retrieved
		displayInfo.dwInputFields2 = 0xffffffff; // 0xffffffff means all fields should be retrieved
		if (!pfNvGetDisplayInfo("0", &displayInfo))
			printf("Unable to retrieve display info\n");
        else {
			printf("Display Adapter : %s\n", displayInfo.szAdapterName);
			if ((displayInfo.dwOutputFields1 & NVDISPLAYINFO1_BOARDTYPE) != 0) // not supported by all drivers
			{
				printf("Display Board : ");
				switch (displayInfo.dwBoardType)
				{
					case NVBOARDTYPE_GEFORCE:
						printf("GeForce");
						break;
					case NVBOARDTYPE_QUADRO:
						printf("Quadro");
						break;
					case NVBOARDTYPE_NVS:
						printf("NVS");
						break;
					default:
						printf("0x%08lX", displayInfo.dwBoardType);
						break;
				}
				printf("\n");
			}
			printf("Display Driver : %s\n", displayInfo.szDriverVersion);
		}
	}
}

/*******************************************************************************

    GPU configuration

*******************************************************************************/

static void GPUConfiguration(HINSTANCE hLib)
{
    printf("\nGPU CONFIGURATION:\n");

    // Get the NvCplGetDataInt function pointer from the library
    NvCplGetDataIntType NvCplGetDataInt = (NvCplGetDataIntType)::GetProcAddress(hLib, "NvCplGetDataInt");
    if (NvCplGetDataInt == 0)
        printf("- Unable to get a pointer to NvCplGetDataInt\n");
    else {

        // Get bus mode
        long busMode;
        printf("- Bus mode: ");
        if (NvCplGetDataInt(NVCPL_API_AGP_BUS_MODE, &busMode) == FALSE)
            printf("Unable to retrieve\n");
        else {
			// Graphics card connection to system
			//  Values are:
			//   1 = PCI
			//   4 = AGP
			//   8 = PCI Express
            switch (busMode) {
            case 1:
                printf("PCI\n");
                break;
            case 4:
                printf("AGP\n");
                break;
            case 8:
                printf("PCI Express\n");
                break;
            default:
                printf("Unknown\n");
                break;
            }
        }

        // Get bus transfer rate
        long busTransferRate;
        printf("- Bus transfer rate: ");
        if (NvCplGetDataInt(NVCPL_API_TX_RATE, &busTransferRate) == FALSE)
            printf("Unable to retrieve\n");
        else
            printf("%dX\n", busTransferRate);

        // Get AGP GART size
		if (busMode == 4) {
			long AGPGARTSize;
			printf("- AGP GART/Memory size: ");
			if (NvCplGetDataInt(NVCPL_API_AGP_LIMIT, &AGPGARTSize) == FALSE)
	            printf("Unable to retrieve\n");
			else
			{
	            // AGP GART Size is reported in Bytes: convert to MB before printing.
				printf("%d MBytes\n", AGPGARTSize/(1024*1024));
			}
		}

        // Get video memory size
        long videoRAMSize;
        printf("- Video memory size: ");
        if (NvCplGetDataInt(NVCPL_API_VIDEO_RAM_SIZE, &videoRAMSize) == FALSE)
            printf("Unable to retrieve\n");
        else
            printf("%d MBytes\n", videoRAMSize);

        // Get antialiasing mode
        long AAValue;
        printf("- Antialiasing setting: ");
        if (NvCplGetDataInt(NVCPL_API_CURRENT_AA_VALUE, &AAValue) == FALSE)
            printf("Unable to retrieve\n");
        else {
            switch (AAValue) {
            case NVCPL_API_AA_METHOD_OFF:
                printf("Off\n");
                break;
            case NVCPL_API_AA_METHOD_2X:
                printf("2X\n");
                break;
            case NVCPL_API_AA_METHOD_2XQ:
                printf("2XQ\n");
                break;
            case NVCPL_API_AA_METHOD_4X:
                printf("4X\n");
                break;
            case NVCPL_API_AA_METHOD_4X_GAUSSIAN:
                printf("4X 9T\n");
                break;
            case NVCPL_API_AA_METHOD_4XS:
                printf("4X Skewed\n");
                break;
            case NVCPL_API_AA_METHOD_6XS:
                printf("6XS\n");
                break;
            case NVCPL_API_AA_METHOD_8XS:
                printf("8XS\n");
                break;
            case NVCPL_API_AA_METHOD_16XS:
                printf("16XS\n");
                break;
            default:
                printf("Unknown\n");
                break;
            }
        }

        // Get and Modify Number of Frames Buffered
        NvCplSetDataIntType NvCplSetDataInt = (NvCplSetDataIntType)::GetProcAddress(hLib, "NvCplSetDataInt");
        if (NvCplSetDataInt == 0)
            printf("- Unable to get a pointer to NvCplSetDataInt\n");
        else 
        {
            long numFramesBuffered;
            printf("- Number of Frames Buffered: ");
            if (NvCplGetDataInt(NVCPL_API_FRAME_QUEUE_LIMIT, &numFramesBuffered) == FALSE)
                printf("Unable to retrieve\n");
            else
                printf("%d Frame(s)\n", numFramesBuffered);

            long const kForceBufferedFrames = 1;
            printf("- Setting Number of Frames Buffered to %d: ", kForceBufferedFrames);
            if (NvCplSetDataInt(NVCPL_API_FRAME_QUEUE_LIMIT, kForceBufferedFrames) == FALSE)
                printf("Unable to set\n");
            else
            {
                long    readbackValue;
                if (NvCplGetDataInt(NVCPL_API_FRAME_QUEUE_LIMIT, &readbackValue) == FALSE)
                    printf("Unable to retrieve\n");
                else
                    printf("%d Frame(s)\n", readbackValue);
            }
            printf("- Resetting Number of Frames Buffered to %d: ", numFramesBuffered);
            if (NvCplSetDataInt(NVCPL_API_FRAME_QUEUE_LIMIT, numFramesBuffered) == FALSE)
                printf("Unable to set\n");
            else
            {
                long    readbackValue;
                if (NvCplGetDataInt(NVCPL_API_FRAME_QUEUE_LIMIT, &readbackValue) == FALSE)
                    printf("Unable to retrieve\n");
                else
                    printf("%d Frame(s)\n", readbackValue);
            }
        }
		// Get number of GPUs and number of SLI GPUs
        long    numGPUs     = 0L;
        long    numSLIGPUs  = 0L;
        printf("- Number of GPUs: ");
        if (NvCplGetDataInt(NVCPL_API_NUMBER_OF_GPUS, &numGPUs) == FALSE)
            printf("Unable to retrieve\n");
        else
            printf("%ld.\n", numGPUs);
        printf("- Number of GPUs in SLI mode: ");
        if (NvCplGetDataInt(NVCPL_API_NUMBER_OF_SLI_GPUS, &numSLIGPUs) == FALSE)
            printf("Unable to retrieve\n");
        else
            printf("%ld.\n", numSLIGPUs);

        if (numSLIGPUs > 0L)
        {
            long    SLIMode = 0L;
            printf("- SLI rendering mode: ");
            if (NvCplGetDataInt(NVCPL_API_SLI_MULTI_GPU_RENDERING_MODE, &SLIMode) == FALSE)
                printf("Unable to retrieve\n");
            else
            {
                if ((SLIMode & NVCPL_API_SLI_ENABLED) == 0L)
                    printf("SLI is not enabled.\n");
                else 
                {
                    if ((SLIMode & NVCPL_API_SLI_RENDERING_MODE_AFR) != 0L)
                        printf("SLI is in AFR mode.\n");
                    else if ((SLIMode & NVCPL_API_SLI_RENDERING_MODE_SFR) != 0L)
                        printf("SLI is in SFR mode.\n");
                    else if ((SLIMode & NVCPL_API_SLI_RENDERING_MODE_SINGLE_GPU) != 0L)
                        printf("SLI is in single GPU mode.\n");
                    else 
                        printf("SLI is in auto-select mode.\n");
            
                    printf("Setting SLI to AFR mode.\n");
                    long    newSLIMode = NVCPL_API_SLI_RENDERING_MODE_AFR; 
                    if (NvCplSetDataInt(NVCPL_API_SLI_MULTI_GPU_RENDERING_MODE, newSLIMode) == FALSE)
                        printf("Unable to set\n");
                    else
                    {
                        // saving old SLI state for resetting later
                        newSLIMode = SLIMode & (~NVCPL_API_SLI_ENABLED); // turn off top bit;
                        // Query current state, should be AFR
                        if (NvCplGetDataInt(NVCPL_API_SLI_MULTI_GPU_RENDERING_MODE, &SLIMode) == FALSE)
                            printf("Unable to retrieve\n");
                        else
                        {
                            if ((SLIMode & NVCPL_API_SLI_RENDERING_MODE_AFR) != 0L)
                                printf("SLI is in AFR mode.\n");
                            else if ((SLIMode & NVCPL_API_SLI_RENDERING_MODE_SFR) != 0L)
                                printf("SLI is in SFR mode.\n");
                            else if ((SLIMode & NVCPL_API_SLI_RENDERING_MODE_SINGLE_GPU) != 0L)
                                printf("SLI is in single GPU mode.\n");
                            else 
                                printf("SLI is in auto-select mode.\n");
                        }
                        // Reset to initial SLI mode
                        if (NvCplSetDataInt(NVCPL_API_SLI_MULTI_GPU_RENDERING_MODE, newSLIMode) == FALSE)
                            printf("Unable to reset SLI mode.\n");
                        else
                            printf("Reset SLI mode to initial state.\n");
                    }
                }
            }
        }
	}
}

/*******************************************************************************

    Desktop configuration

*******************************************************************************/

static void TestSetting(fdtcfgex, const char*, const char*, const char*, float);
static void SetPrimaryDisplayState(fdtcfgex, int);

static void DesktopConfiguration(HINSTANCE hLib)
{
    printf("\nDESKTOP CONFIGURATION TEST:\n");

    // Get the NvCplGetRealConnectedDevicesString function pointer from the library
    fNvCplGetRealConnectedDevicesString pfNvCplGetRealConnectedDevicesString = (fNvCplGetRealConnectedDevicesString)::GetProcAddress(hLib, "NvCplGetRealConnectedDevicesString");
    if (pfNvCplGetRealConnectedDevicesString == NULL)
        printf("- Unable to get a pointer to NvCplGetRealConnectedDevicesString\n");
    else {
        char connectedDevices[1024];
        pfNvCplGetRealConnectedDevicesString(connectedDevices, 1024, FALSE);
        printf("- Connected devices: %s\n", connectedDevices);
        pfNvCplGetRealConnectedDevicesString(connectedDevices, 1024, TRUE);
        printf("- Active connected devices: %s\n", connectedDevices);
    }

    // Get the NvGetDisplayInfo function pointer from the library
	NVDISPLAYINFO displayInfo = {0};
    int currentDisplayState = NVDISPLAYMODE_NONE;
    int testDisplayState = NVDISPLAYMODE_NONE;
    fNvGetDisplayInfo pfNvGetDisplayInfo = (fNvGetDisplayInfo)::GetProcAddress(hLib, "NvGetDisplayInfo");
    if (pfNvGetDisplayInfo == NULL)
        printf("- Unable to get a pointer to NvGetDisplayInfo\n");
    else {
		// displayInfo.cbSize must be set to size of structure
		// displayInfo.dwInputFields1 must be set before call to indicate which fields to retrieve
		// displayInfo.dwOutputFields1 will be set on return to indicate which fields were successfully retrived
		// see NVDISPLAYINFO1_* bit definitions for field information, use 0xffffffff to retrieve all fields
		memset(&displayInfo, 0, sizeof(displayInfo));
		displayInfo.cbSize = sizeof(displayInfo);
		displayInfo.dwInputFields1 = 0xffffffff; // 0xffffffff means all fields should be retrieved
		displayInfo.dwInputFields2 = 0xffffffff; // 0xffffffff means all fields should be retrieved
		if (!pfNvGetDisplayInfo("0", &displayInfo))
			printf("- Unable to retrieve display info\n");
        else {
			printf ("- Primary Display:\n");
			// Get the current mode view and arbitrarily decide of a new test mode
			switch (displayInfo.nDisplayMode) {
			case NVDISPLAYMODE_STANDARD:
	            printf("    Display Mode: Standard\n");
				testDisplayState = NVDISPLAYMODE_CLONE;
				break;
			case NVDISPLAYMODE_DUALVIEW:
				printf("    Display Mode: Dualview\n");
				testDisplayState = NVDISPLAYMODE_CLONE;
				break;
			case NVDISPLAYMODE_CLONE:
	            printf("    Display Mode: Clone\n");
				testDisplayState = NVDISPLAYMODE_HSPAN;
				break;
			case NVDISPLAYMODE_HSPAN:
	            printf("    Display Mode: Horizontal Span\n");
				testDisplayState = NVDISPLAYMODE_VSPAN;
				break;
			case NVDISPLAYMODE_VSPAN:
	            printf("    Display Mode: Vertical Span\n");
				testDisplayState = NVDISPLAYMODE_STANDARD;
				break;
			case NVDISPLAYMODE_NONE:
			default:
	            printf("    Display Mode: None\n");
				break;
			}
			if (displayInfo.nDisplayType != NVDISPLAYTYPE_NONE)
			{
				printf("    Display Type: ");
				switch (displayInfo.nDisplayType & NVDISPLAYTYPE_CLASS_MASK)
				{
					case NVDISPLAYTYPE_CRT:
						if (displayInfo.nDisplayType == NVDISPLAYTYPE_CRT)
						{
							printf("Cathode Ray Tube (CRT)");
						}
						else
						{
							printf("Cathode Ray Tube (CRT) [subtype: 0x%04X]",
							displayInfo.nDisplayType);
						}
						break;
					case NVDISPLAYTYPE_DFP:
						if (displayInfo.nDisplayType == NVDISPLAYTYPE_DFP)
						{
							printf("Digital Flat Panel (DFP)");
						}
						else if (displayInfo.nDisplayType == NVDISPLAYTYPE_DFP_LAPTOP)
						{
							printf("Laptop Display Panel");
						}
						else
						{
							printf("Digital Flat Panel (DFP) [subtype: 0x%04X]",
							displayInfo.nDisplayType);
						}
						break;
					case NVDISPLAYTYPE_TV:
						if (displayInfo.nDisplayType == NVDISPLAYTYPE_TV)
						{
							printf("Television");
						}
						else if (displayInfo.nDisplayType == NVDISPLAYTYPE_TV_HDTV)
						{
							printf("High-Definition Television (HDTV)");
						}
						else
						{
							printf("Television [subtype: 0x%04X]", displayInfo.nDisplayType);
						}
						break;
					default:
						printf("0x%04X", displayInfo.nDisplayType);
						break;
				}
				printf("\n");
			}
			// Dump current mode information to stdout
			printf("    Current Resolution: %ld x %ld pixels\n" , displayInfo.dwVisiblePelsWidth, displayInfo.dwVisiblePelsHeight);
			printf("    Current Depth: %ld-bit\n" , displayInfo.dwBitsPerPel);
			printf("    Current Refresh Rate: %ld Hz\n" , displayInfo.dwDisplayFrequency);
			printf("    Current Rotation: %ld-degrees\n" , displayInfo.dwDegreesRotation);
			printf("    Current Pannable: %ld x %ld pixels\n" , displayInfo.dwPelsWidth, displayInfo.dwPelsHeight);
			printf("    Current Rectangle: (%ld,%ld)-(%ld,%ld)\n", displayInfo.rcDisplayRect.left, displayInfo.rcDisplayRect.top, displayInfo.rcDisplayRect.right, displayInfo.rcDisplayRect.bottom);
		}
	}

	// Get the dtcfgex function pointer from the library
    fdtcfgex NVdtcfgex = (fdtcfgex)::GetProcAddress(hLib, "dtcfgex");
    if (NVdtcfgex == 0) {
        printf("- Unable to get a pointer to NVdtcfgex\n");
        return;
    }
    else {

        // Reset any delay
        NVdtcfgex("setdelay pre 0");
        NVdtcfgex("setdelay post 0");

        // Digital vibrance
        TestSetting(NVdtcfgex, "Digital vibrance", "dvc", "", NV_DISPLAY_DIGITAL_VIBRANCE_MAX);

        // Brightness
        TestSetting(NVdtcfgex, "Brightness", "brightness", "all", NV_DISPLAY_BRIGHTNESS_MAX);

        // Contrast
        TestSetting(NVdtcfgex, "Contrast", "contrast", "all", NV_DISPLAY_CONTRAST_MAX);

        // Gamma
        TestSetting(NVdtcfgex, "Gamma", "gamma", "all", NV_DISPLAY_GAMMA_MAX);
    }

    // Get Gamma ramp
    GAMMARAMP NvGammaRamp= {0};
    fNvColorGetGammaRampEx pfNvColorGetGammaRampEx = (fNvColorGetGammaRampEx)::GetProcAddress(hLib, "NvColorGetGammaRampEx");
    if (pfNvColorGetGammaRampEx == NULL) 
        printf("- Unable to get a pointer to NvColorGetGammaRampEx\n");
    else {
		printf("    Desktop Gamma ramp:");
        if (pfNvColorGetGammaRampEx("0", &NvGammaRamp, NVCOLORAPPLY_DESKTOP) == FALSE)
            printf("- Unable to retrieve Gamma Ramp for primary display\n");
        else {
            for (unsigned int i = 0; i < 256; ++i)
                if (i < 4)
                    printf(" (%d %d %d)", NvGammaRamp.Red[i], NvGammaRamp.Green[i], NvGammaRamp.Blue[i]);
                else
                    break;
            printf("\n      Testing...\n");

            // Modify Gamma ramp
            fNvColorSetGammaRampEx pfNvColorSetGammaRampEx = (fNvColorSetGammaRampEx) GetProcAddress(hLib,"NvColorSetGammaRampEx");
            if (pfNvColorSetGammaRampEx == NULL)
                printf("- Unable to get a pointer to NvColorSetGammaRampEx\n");
            else {
                GAMMARAMP newGammaRamp;
				printf("      Setting to value/2...\n");
                for (unsigned int i = 0; i < 256; ++i) {
                    newGammaRamp.Red[i] = NvGammaRamp.Red[i] / 2;
                    newGammaRamp.Green[i] = NvGammaRamp.Green[i] / 2;
                    newGammaRamp.Blue[i] = NvGammaRamp.Blue[i] / 2;
                }
                if (pfNvColorSetGammaRampEx("0", &newGammaRamp, NVCOLORAPPLY_DESKTOP) == FALSE)
                    printf("- NvColorSetGammaRampEx: Not a valid Gamma ramp\n");
                else
                    Sleep(1000);
				printf("      Restore...\n");
                pfNvColorSetGammaRampEx("0", &NvGammaRamp, NVCOLORAPPLY_DESKTOP);
            }
        }
    }

    // Check whether the user is interested in this test
    printf("\nDo you want to test the view mode configuration functions? [y|n]");
    char c;
    c = getch();
    printf("\n");
    if (c != 'y')
        return;

    // Set the view mode to the test mode
    SetPrimaryDisplayState(NVdtcfgex, testDisplayState);
    Sleep(8000);

    // Switch back to the old mode
    SetPrimaryDisplayState(NVdtcfgex, currentDisplayState);
}

static void TestSetting(fdtcfgex NVdtcfgex, const char* name, const char* command, const char* color, float max)
{
    // Get value for primary device
    char get[1024];
    sprintf(get, "get%svalue", command);
    NVdtcfgex(get);
    printf("    %s: %s \n      Testing...\n", name, get);

    // Set value to max and wait a bit
    char set[1024];
	printf("      Set to max...\n");
    sprintf(set, "set%s 0 %s %f", command, color, max);
    NVdtcfgex(set);
    Sleep(2000);

    // Restore value
	printf("      Restore...\n");
    sprintf(set, "set%s 0 %s %s", command, color, get);
    NVdtcfgex(set);
}

static void SetPrimaryDisplayState(fdtcfgex NVdtcfgex, int state)
{
    switch (state) {
    case NVDISPLAYMODE_STANDARD:
        printf("- Setting the view mode for primary display to standard mode...\n");
        NVdtcfgex("setview 0 standard");
        break;
    case NVDISPLAYMODE_CLONE:
        printf("- Setting the view mode for primary display to clone mode...\n");
        NVdtcfgex("setview 0 clone");
        break;
    case NVDISPLAYMODE_HSPAN:
        printf("- Setting the view mode for primary display to horizontal span mode...\n");
        NVdtcfgex("setview 0 hspan");
        break;
    case NVDISPLAYMODE_VSPAN:
        printf("- Setting the view mode for primary display to vertical span mode...\n");
        NVdtcfgex("setview 0 vspan");
        break;
    default:
        break;
    }
}
