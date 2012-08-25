#include <windows.h>
#include <hidsdi.h>
#include <setupapi.h>
#include <stdio.h>
#include <stdlib.h>

#include "vmultidclient.h"

#if __GNUC__
    #define __in
    #define __in_ecount(x)
    typedef void* PVOID;
    typedef PVOID HDEVINFO;
    WINHIDSDI BOOL WINAPI HidD_SetOutputReport(HANDLE, PVOID, ULONG);
#endif

typedef struct _vmultid_client_t
{
    HANDLE hControl;
    HANDLE hMessage;
    BYTE controlReport[CONTROL_REPORT_SIZE];
} vmultid_client_t;

//
// Function prototypes
//

HANDLE
SearchMatchingHwID (
    USAGE myUsagePage,
    USAGE myUsage
    );

HANDLE
OpenDeviceInterface (
    HDEVINFO HardwareDeviceInfo,
    PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData,
    USAGE myUsagePage,
    USAGE myUsage
    );

BOOLEAN
CheckIfOurDevice(
    HANDLE file,
    USAGE myUsagePage,
    USAGE myUsage
    );

BOOL
HidOutput(
    BOOL useSetOutputReport,
    HANDLE file,
    PCHAR buffer,
    ULONG bufferSize
    );

//
// Copied this structure from hidport.h
//

typedef struct _HID_DEVICE_ATTRIBUTES {

    ULONG           Size;
    //
    // sizeof (struct _HID_DEVICE_ATTRIBUTES)
    //
    //
    // Vendor ids of this hid device
    //
    USHORT          VendorID;
    USHORT          ProductID;
    USHORT          VersionNumber;
    USHORT          Reserved[11];

} HID_DEVICE_ATTRIBUTES, * PHID_DEVICE_ATTRIBUTES;

//
// Implementation
//

pvmultid_client vmultid_alloc(void)
{
    return (pvmultid_client)malloc(sizeof(vmultid_client_t));
}

void vmultid_free(pvmultid_client vmultid)
{
    free(vmultid);
}

BOOL vmultid_connect(pvmultid_client vmultid)
{
    //
    // Find the HID devices
    //

    vmultid->hControl = SearchMatchingHwID(0xff00, 0x0001);
    if (vmultid->hControl == INVALID_HANDLE_VALUE || vmultid->hControl == NULL)
        return FALSE;
    vmultid->hMessage = SearchMatchingHwID(0xff00, 0x0002);
    if (vmultid->hMessage == INVALID_HANDLE_VALUE || vmultid->hMessage == NULL)
    {
        vmultid_disconnect(vmultid);
        return FALSE;
    }

    //
    // Set the buffer count to 10 on the message HID
    //

    if (!HidD_SetNumInputBuffers(vmultid->hMessage, 10))
    {
        printf("failed HidD_SetNumInputBuffers %d\n", GetLastError());
        vmultid_disconnect(vmultid);
        return FALSE;
    }

    return TRUE;
}

void vmultid_disconnect(pvmultid_client vmultid)
{
    if (vmultid->hControl != NULL)
        CloseHandle(vmultid->hControl);
    if (vmultid->hMessage != NULL)
        CloseHandle(vmultid->hMessage);
    vmultid->hControl = NULL;
    vmultid->hMessage = NULL;
}

BOOL vmultid_update_mouse(pvmultid_client vmultid, BYTE button, USHORT x, USHORT y, BYTE wheelPosition)
{
    vmultidControlReportHeader* pReport = NULL;
    vmultidMouseReport* pMouseReport = NULL;

    if (CONTROL_REPORT_SIZE <= sizeof(vmultidControlReportHeader) + sizeof(vmultidMouseReport))
    {
        return FALSE;
    }

    //
    // Set the report header
    //

    pReport = (vmultidControlReportHeader*)vmultid->controlReport;
    pReport->ReportID = REPORTID_CONTROL;
    pReport->ReportLength = sizeof(vmultidMouseReport);

    //
    // Set the input report
    //

    pMouseReport = (vmultidMouseReport*)(vmultid->controlReport + sizeof(vmultidControlReportHeader));
    pMouseReport->ReportID = REPORTID_MOUSE;
    pMouseReport->Button = button;
    pMouseReport->XValue = x;
    pMouseReport->YValue = y;
    pMouseReport->WheelPosition = wheelPosition;

    // Send the report
    return HidOutput(FALSE, vmultid->hControl, (PCHAR)vmultid->controlReport, CONTROL_REPORT_SIZE);
}

BOOL vmultid_update_relative_mouse(pvmultid_client vmultid, BYTE button,
BYTE x, BYTE y, BYTE wheelPosition)
{
    vmultidControlReportHeader* pReport = NULL;
    vmultidRelativeMouseReport* pMouseReport = NULL;

    if (CONTROL_REPORT_SIZE <= sizeof(vmultidControlReportHeader) + sizeof(vmultidRelativeMouseReport))
    {
        return FALSE;
    }

    //
    // Set the report header
    //

    pReport = (vmultidControlReportHeader*)vmultid->controlReport;
    pReport->ReportID = REPORTID_CONTROL;
    pReport->ReportLength = sizeof(vmultidRelativeMouseReport);

    //
    // Set the input report
    //

    pMouseReport = (vmultidRelativeMouseReport*)(vmultid->controlReport + sizeof(vmultidControlReportHeader));
    pMouseReport->ReportID = REPORTID_RELATIVE_MOUSE;
    pMouseReport->Button = button;
    pMouseReport->XValue = x;
    pMouseReport->YValue = y;
    pMouseReport->WheelPosition = wheelPosition;

    // Send the report
    return HidOutput(FALSE, vmultid->hControl, (PCHAR)vmultid->controlReport, CONTROL_REPORT_SIZE);
}

BOOL vmultid_update_digi(pvmultid_client vmultid, BYTE status, USHORT x, USHORT y)
{
    vmultidControlReportHeader* pReport = NULL;
    vmultidDigiReport* pDigiReport = NULL;

    if (CONTROL_REPORT_SIZE <= sizeof(vmultidControlReportHeader) + sizeof(vmultidDigiReport))
    {
        return FALSE;
    }

    //
    // Set the report header
    //

    pReport = (vmultidControlReportHeader*)vmultid->controlReport;
    pReport->ReportID = REPORTID_CONTROL;
    pReport->ReportLength = sizeof(vmultidDigiReport);

    //
    // Set the input report
    //

    pDigiReport = (vmultidDigiReport*)(vmultid->controlReport + sizeof(vmultidControlReportHeader));
    pDigiReport->ReportID = REPORTID_DIGI;
    pDigiReport->Status = status;
    pDigiReport->XValue = x;
    pDigiReport->YValue = y;

    // Send the report
    return HidOutput(FALSE, vmultid->hControl, (PCHAR)vmultid->controlReport, CONTROL_REPORT_SIZE);
}

BOOL vmultid_update_multitouch(pvmultid_client vmultid, PTOUCH pTouch, BYTE actualCount)
{
    vmultidControlReportHeader* pReport = NULL;
    vmultidMultiTouchReport* pMultiReport = NULL;
    int numberOfTouchesSent = 0;

    if (CONTROL_REPORT_SIZE <= sizeof(vmultidControlReportHeader) + sizeof(vmultidMultiTouchReport))
        return FALSE;

    //
    // Set the report header
    //

    pReport = (vmultidControlReportHeader*)vmultid->controlReport;
    pReport->ReportID = REPORTID_CONTROL;
    pReport->ReportLength = sizeof(vmultidMultiTouchReport);

    while (numberOfTouchesSent < actualCount)
    {

        //
        // Set the input report
        //

        pMultiReport = (vmultidMultiTouchReport*)(vmultid->controlReport + sizeof(vmultidControlReportHeader));
        pMultiReport->ReportID = REPORTID_MTOUCH;
        memcpy(pMultiReport->Touch, pTouch + numberOfTouchesSent, sizeof(TOUCH));
        if (numberOfTouchesSent <= actualCount - 2)
            memcpy(pMultiReport->Touch + 1, pTouch + numberOfTouchesSent + 1, sizeof(TOUCH));
        else
            memset(pMultiReport->Touch + 1, 0, sizeof(TOUCH));
        if (numberOfTouchesSent == 0)
            pMultiReport->ActualCount = actualCount;
        else
            pMultiReport->ActualCount = 0;

        // Send the report
        if (!HidOutput(TRUE, vmultid->hControl, (PCHAR)vmultid->controlReport, CONTROL_REPORT_SIZE))
            return FALSE;

        numberOfTouchesSent += 2;
    }

    return TRUE;
}

BOOL vmultid_update_joystick(pvmultid_client vmultid, USHORT buttons, BYTE hat, BYTE x, BYTE y, BYTE rx, BYTE ry, BYTE throttle)
{
    vmultidControlReportHeader* pReport = NULL;
    vmultidJoystickReport* pJoystickReport = NULL;

    if (CONTROL_REPORT_SIZE <= sizeof(vmultidControlReportHeader) + sizeof(vmultidJoystickReport))
    {
        return FALSE;
    }

    //
    // Set the report header
    //

    pReport = (vmultidControlReportHeader*)vmultid->controlReport;
    pReport->ReportID = REPORTID_CONTROL;
    pReport->ReportLength = sizeof(vmultidJoystickReport);

    //
    // Set the input report
    //

    pJoystickReport = (vmultidJoystickReport*)(vmultid->controlReport + sizeof(vmultidControlReportHeader));
    pJoystickReport->ReportID = REPORTID_JOYSTICK;
    pJoystickReport->Buttons = buttons;
    pJoystickReport->Hat = hat;
    pJoystickReport->XValue = x;
    pJoystickReport->YValue = y;
    pJoystickReport->RXValue = rx;
    pJoystickReport->RYValue = ry;
    pJoystickReport->Throttle = throttle;

    // Send the report
    return HidOutput(FALSE, vmultid->hControl, (PCHAR)vmultid->controlReport, CONTROL_REPORT_SIZE);
}

BOOL vmultid_update_keyboard(pvmultid_client vmultid, BYTE shiftKeyFlags, BYTE keyCodes[KBD_KEY_CODES])
{
    vmultidControlReportHeader* pReport = NULL;
    vmultidKeyboardReport* pKeyboardReport = NULL;

    if (CONTROL_REPORT_SIZE <= sizeof(vmultidControlReportHeader) + sizeof(vmultidKeyboardReport))
    {
        return FALSE;
    }

    //
    // Set the report header
    //

    pReport = (vmultidControlReportHeader*)vmultid->controlReport;
    pReport->ReportID = REPORTID_CONTROL;
    pReport->ReportLength = sizeof(vmultidKeyboardReport);

    //
    // Set the input report
    //

    pKeyboardReport = (vmultidKeyboardReport*)(vmultid->controlReport + sizeof(vmultidControlReportHeader));
    pKeyboardReport->ReportID = REPORTID_KEYBOARD;
    pKeyboardReport->ShiftKeyFlags = shiftKeyFlags;
    memcpy(pKeyboardReport->KeyCodes, keyCodes, KBD_KEY_CODES);

    // Send the report
    return HidOutput(FALSE, vmultid->hControl, (PCHAR)vmultid->controlReport, CONTROL_REPORT_SIZE);
}

BOOL vmultid_write_message(pvmultid_client vmultid, vmultidMessageReport* pReport)
{
    vmultidControlReportHeader* pReportHeader;
    ULONG bytesWritten;

    //
    // Set the report header
    //

    pReportHeader = (vmultidControlReportHeader*)vmultid->controlReport;
    pReportHeader->ReportID = REPORTID_CONTROL;
    pReportHeader->ReportLength = sizeof(vmultidMessageReport);

    //
    // Set the body
    //

    pReport->ReportID = REPORTID_MESSAGE;
    memcpy(vmultid->controlReport + sizeof(vmultidControlReportHeader), pReport, sizeof(vmultidMessageReport));

    //
    // Write the report
    //

    if (!WriteFile(vmultid->hControl, vmultid->controlReport, CONTROL_REPORT_SIZE, &bytesWritten, NULL))
    {
        printf("failed WriteFile %d\n", GetLastError());
        return FALSE;
    }

    return TRUE;
}

BOOL vmultid_read_message(pvmultid_client vmultid, vmultidMessageReport* pReport)
{
    ULONG bytesRead;

    //
    // Read the report
    //

    if (!ReadFile(vmultid->hMessage, pReport, sizeof(vmultidMessageReport), &bytesRead, NULL))
    {
        printf("failed ReadFile %d\n", GetLastError());
        return FALSE;
    }

    return TRUE;
}

HANDLE
SearchMatchingHwID (
    USAGE myUsagePage,
    USAGE myUsage
    )
{
    HDEVINFO                  hardwareDeviceInfo;
    SP_DEVICE_INTERFACE_DATA  deviceInterfaceData;
    SP_DEVINFO_DATA           devInfoData;
    GUID                      hidguid;
    int                       i;

    HidD_GetHidGuid(&hidguid);

    hardwareDeviceInfo =
            SetupDiGetClassDevs ((LPGUID)&hidguid,
                                            NULL,
                                            NULL, // Define no
                                            (DIGCF_PRESENT |
                                            DIGCF_INTERFACEDEVICE));

    if (INVALID_HANDLE_VALUE == hardwareDeviceInfo)
    {
        printf("SetupDiGetClassDevs failed: %x\n", GetLastError());
        return INVALID_HANDLE_VALUE;
    }

    deviceInterfaceData.cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);

    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    //
    // Enumerate devices of this interface class
    //

    printf("\n....looking for our HID device (with UP=0x%x "
                "and Usage=0x%x)\n", myUsagePage, myUsage);

    for (i = 0; SetupDiEnumDeviceInterfaces (hardwareDeviceInfo,
                            0, // No care about specific PDOs
                            (LPGUID)&hidguid,
                            i, //
                            &deviceInterfaceData);
                            i++)
    {

        //
        // Open the device interface and Check if it is our device
        // by matching the Usage page and Usage from Hid_Caps.
        // If this is our device then send the hid request.
        //

        HANDLE file = OpenDeviceInterface(hardwareDeviceInfo, &deviceInterfaceData, myUsagePage, myUsage);

        if (file != INVALID_HANDLE_VALUE)
        {
            SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
            return file;
        }

        //
        //device was not found so loop around.
        //

    }

    printf("Failure: Could not find our HID device \n");

    SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);

    return INVALID_HANDLE_VALUE;
}

HANDLE
OpenDeviceInterface (
    HDEVINFO hardwareDeviceInfo,
    PSP_DEVICE_INTERFACE_DATA deviceInterfaceData,
    USAGE myUsagePage,
    USAGE myUsage
    )
{
    PSP_DEVICE_INTERFACE_DETAIL_DATA    deviceInterfaceDetailData = NULL;

    DWORD        predictedLength = 0;
    DWORD        requiredLength = 0;
    HANDLE       file = INVALID_HANDLE_VALUE;

    SetupDiGetDeviceInterfaceDetail(
                            hardwareDeviceInfo,
                            deviceInterfaceData,
                            NULL, // probing so no output buffer yet
                            0, // probing so output buffer length of zero
                            &requiredLength,
                            NULL
                            ); // not interested in the specific dev-node

    predictedLength = requiredLength;

    deviceInterfaceDetailData =
         (PSP_DEVICE_INTERFACE_DETAIL_DATA) malloc (predictedLength);

    if (!deviceInterfaceDetailData)
    {
        printf("Error: OpenDeviceInterface: malloc failed\n");
        goto cleanup;
    }

    deviceInterfaceDetailData->cbSize =
                    sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA);

    if (!SetupDiGetDeviceInterfaceDetail(
                            hardwareDeviceInfo,
                            deviceInterfaceData,
                            deviceInterfaceDetailData,
                            predictedLength,
                            &requiredLength,
                            NULL))
    {
        printf("Error: SetupDiGetInterfaceDeviceDetail failed\n");
        free (deviceInterfaceDetailData);
        goto cleanup;
    }

    file = CreateFile ( deviceInterfaceDetailData->DevicePath,
                            GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_READ,
                            NULL, // no SECURITY_ATTRIBUTES structure
                            OPEN_EXISTING, // No special create flags
                            0, // No special attributes
                            NULL); // No template file

    if (INVALID_HANDLE_VALUE == file) {
        printf("Error: CreateFile failed: %d\n", GetLastError());
        goto cleanup;
    }

    if (CheckIfOurDevice(file, myUsagePage, myUsage)){

        goto cleanup;

    }

    CloseHandle(file);

    file = INVALID_HANDLE_VALUE;

cleanup:

    free (deviceInterfaceDetailData);

    return file;

}


BOOLEAN
CheckIfOurDevice(
    HANDLE file,
    USAGE myUsagePage,
    USAGE myUsage)
{
    PHIDP_PREPARSED_DATA Ppd = NULL; // The opaque parser info describing this device
    HIDD_ATTRIBUTES                 Attributes; // The Attributes of this hid device.
    HIDP_CAPS                       Caps; // The Capabilities of this hid device.
    BOOLEAN                         result = FALSE;

    if (!HidD_GetPreparsedData (file, &Ppd))
    {
        printf("Error: HidD_GetPreparsedData failed \n");
        goto cleanup;
    }

    if (!HidD_GetAttributes(file, &Attributes))
    {
        printf("Error: HidD_GetAttributes failed \n");
        goto cleanup;
    }

    if (Attributes.VendorID == vmultid_VID && Attributes.ProductID == vmultid_PID)
    {
        if (!HidP_GetCaps (Ppd, &Caps))
        {
            printf("Error: HidP_GetCaps failed \n");
            goto cleanup;
        }

        if ((Caps.UsagePage == myUsagePage) && (Caps.Usage == myUsage))
        {
            printf("Success: Found my device.. \n");
            result = TRUE;
        }
    }

cleanup:

    if (Ppd != NULL)
    {
        HidD_FreePreparsedData (Ppd);
    }

    return result;
}

BOOL
HidOutput(
    BOOL useSetOutputReport,
    HANDLE file,
    PCHAR buffer,
    ULONG bufferSize
    )
{
    ULONG bytesWritten;
    if (useSetOutputReport)
    {
        //
        // Send Hid report thru HidD_SetOutputReport API
        //

        if (!HidD_SetOutputReport(file, buffer, bufferSize))
        {
            printf("failed HidD_SetOutputReport %d\n", GetLastError());
            return FALSE;
        }
    }
    else
    {
        if (!WriteFile(file, buffer, bufferSize, &bytesWritten, NULL))
        {
            printf("failed WriteFile %d\n", GetLastError());
            return FALSE;
        }
    }

    return TRUE;
}
