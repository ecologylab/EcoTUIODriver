#include <windows.h>
#include <hidsdi.h>
#include <setupapi.h>
#include <stdio.h>
#include <stdlib.h>

#include "vmulticclient.h"

#if __GNUC__
    #define __in
    #define __in_ecount(x)
    typedef void* PVOID;
    typedef PVOID HDEVINFO;
    WINHIDSDI BOOL WINAPI HidD_SetOutputReport(HANDLE, PVOID, ULONG);
#endif

typedef struct _vmultic_client_t
{
    HANDLE hControl;
    HANDLE hMessage;
    BYTE controlReport[CONTROL_REPORT_SIZE];
} vmultic_client_t;

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

pvmultic_client vmultic_alloc(void)
{
    return (pvmultic_client)malloc(sizeof(vmultic_client_t));
}

void vmultic_free(pvmultic_client vmultic)
{
    free(vmultic);
}

BOOL vmultic_connect(pvmultic_client vmultic)
{
    //
    // Find the HID devices
    //

    vmultic->hControl = SearchMatchingHwID(0xff00, 0x0001);
    if (vmultic->hControl == INVALID_HANDLE_VALUE || vmultic->hControl == NULL)
        return FALSE;
    vmultic->hMessage = SearchMatchingHwID(0xff00, 0x0002);
    if (vmultic->hMessage == INVALID_HANDLE_VALUE || vmultic->hMessage == NULL)
    {
        vmultic_disconnect(vmultic);
        return FALSE;
    }

    //
    // Set the buffer count to 10 on the message HID
    //

    if (!HidD_SetNumInputBuffers(vmultic->hMessage, 10))
    {
        printf("failed HidD_SetNumInputBuffers %d\n", GetLastError());
        vmultic_disconnect(vmultic);
        return FALSE;
    }

    return TRUE;
}

void vmultic_disconnect(pvmultic_client vmultic)
{
    if (vmultic->hControl != NULL)
        CloseHandle(vmultic->hControl);
    if (vmultic->hMessage != NULL)
        CloseHandle(vmultic->hMessage);
    vmultic->hControl = NULL;
    vmultic->hMessage = NULL;
}

BOOL vmultic_update_mouse(pvmultic_client vmultic, BYTE button, USHORT x, USHORT y, BYTE wheelPosition)
{
    vmulticControlReportHeader* pReport = NULL;
    vmulticMouseReport* pMouseReport = NULL;

    if (CONTROL_REPORT_SIZE <= sizeof(vmulticControlReportHeader) + sizeof(vmulticMouseReport))
    {
        return FALSE;
    }

    //
    // Set the report header
    //

    pReport = (vmulticControlReportHeader*)vmultic->controlReport;
    pReport->ReportID = REPORTID_CONTROL;
    pReport->ReportLength = sizeof(vmulticMouseReport);

    //
    // Set the input report
    //

    pMouseReport = (vmulticMouseReport*)(vmultic->controlReport + sizeof(vmulticControlReportHeader));
    pMouseReport->ReportID = REPORTID_MOUSE;
    pMouseReport->Button = button;
    pMouseReport->XValue = x;
    pMouseReport->YValue = y;
    pMouseReport->WheelPosition = wheelPosition;

    // Send the report
    return HidOutput(FALSE, vmultic->hControl, (PCHAR)vmultic->controlReport, CONTROL_REPORT_SIZE);
}

BOOL vmultic_update_relative_mouse(pvmultic_client vmultic, BYTE button,
BYTE x, BYTE y, BYTE wheelPosition)
{
    vmulticControlReportHeader* pReport = NULL;
    vmulticRelativeMouseReport* pMouseReport = NULL;

    if (CONTROL_REPORT_SIZE <= sizeof(vmulticControlReportHeader) + sizeof(vmulticRelativeMouseReport))
    {
        return FALSE;
    }

    //
    // Set the report header
    //

    pReport = (vmulticControlReportHeader*)vmultic->controlReport;
    pReport->ReportID = REPORTID_CONTROL;
    pReport->ReportLength = sizeof(vmulticRelativeMouseReport);

    //
    // Set the input report
    //

    pMouseReport = (vmulticRelativeMouseReport*)(vmultic->controlReport + sizeof(vmulticControlReportHeader));
    pMouseReport->ReportID = REPORTID_RELATIVE_MOUSE;
    pMouseReport->Button = button;
    pMouseReport->XValue = x;
    pMouseReport->YValue = y;
    pMouseReport->WheelPosition = wheelPosition;

    // Send the report
    return HidOutput(FALSE, vmultic->hControl, (PCHAR)vmultic->controlReport, CONTROL_REPORT_SIZE);
}

BOOL vmultic_update_digi(pvmultic_client vmultic, BYTE status, USHORT x, USHORT y)
{
    vmulticControlReportHeader* pReport = NULL;
    vmulticDigiReport* pDigiReport = NULL;

    if (CONTROL_REPORT_SIZE <= sizeof(vmulticControlReportHeader) + sizeof(vmulticDigiReport))
    {
        return FALSE;
    }

    //
    // Set the report header
    //

    pReport = (vmulticControlReportHeader*)vmultic->controlReport;
    pReport->ReportID = REPORTID_CONTROL;
    pReport->ReportLength = sizeof(vmulticDigiReport);

    //
    // Set the input report
    //

    pDigiReport = (vmulticDigiReport*)(vmultic->controlReport + sizeof(vmulticControlReportHeader));
    pDigiReport->ReportID = REPORTID_DIGI;
    pDigiReport->Status = status;
    pDigiReport->XValue = x;
    pDigiReport->YValue = y;

    // Send the report
    return HidOutput(FALSE, vmultic->hControl, (PCHAR)vmultic->controlReport, CONTROL_REPORT_SIZE);
}

BOOL vmultic_update_multitouch(pvmultic_client vmultic, PTOUCH pTouch, BYTE actualCount)
{
    vmulticControlReportHeader* pReport = NULL;
    vmulticMultiTouchReport* pMultiReport = NULL;
    int numberOfTouchesSent = 0;

    if (CONTROL_REPORT_SIZE <= sizeof(vmulticControlReportHeader) + sizeof(vmulticMultiTouchReport))
        return FALSE;

    //
    // Set the report header
    //

    pReport = (vmulticControlReportHeader*)vmultic->controlReport;
    pReport->ReportID = REPORTID_CONTROL;
    pReport->ReportLength = sizeof(vmulticMultiTouchReport);

    while (numberOfTouchesSent < actualCount)
    {

        //
        // Set the input report
        //

        pMultiReport = (vmulticMultiTouchReport*)(vmultic->controlReport + sizeof(vmulticControlReportHeader));
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
        if (!HidOutput(TRUE, vmultic->hControl, (PCHAR)vmultic->controlReport, CONTROL_REPORT_SIZE))
            return FALSE;

        numberOfTouchesSent += 2;
    }

    return TRUE;
}

BOOL vmultic_update_joystick(pvmultic_client vmultic, USHORT buttons, BYTE hat, BYTE x, BYTE y, BYTE rx, BYTE ry, BYTE throttle)
{
    vmulticControlReportHeader* pReport = NULL;
    vmulticJoystickReport* pJoystickReport = NULL;

    if (CONTROL_REPORT_SIZE <= sizeof(vmulticControlReportHeader) + sizeof(vmulticJoystickReport))
    {
        return FALSE;
    }

    //
    // Set the report header
    //

    pReport = (vmulticControlReportHeader*)vmultic->controlReport;
    pReport->ReportID = REPORTID_CONTROL;
    pReport->ReportLength = sizeof(vmulticJoystickReport);

    //
    // Set the input report
    //

    pJoystickReport = (vmulticJoystickReport*)(vmultic->controlReport + sizeof(vmulticControlReportHeader));
    pJoystickReport->ReportID = REPORTID_JOYSTICK;
    pJoystickReport->Buttons = buttons;
    pJoystickReport->Hat = hat;
    pJoystickReport->XValue = x;
    pJoystickReport->YValue = y;
    pJoystickReport->RXValue = rx;
    pJoystickReport->RYValue = ry;
    pJoystickReport->Throttle = throttle;

    // Send the report
    return HidOutput(FALSE, vmultic->hControl, (PCHAR)vmultic->controlReport, CONTROL_REPORT_SIZE);
}

BOOL vmultic_update_keyboard(pvmultic_client vmultic, BYTE shiftKeyFlags, BYTE keyCodes[KBD_KEY_CODES])
{
    vmulticControlReportHeader* pReport = NULL;
    vmulticKeyboardReport* pKeyboardReport = NULL;

    if (CONTROL_REPORT_SIZE <= sizeof(vmulticControlReportHeader) + sizeof(vmulticKeyboardReport))
    {
        return FALSE;
    }

    //
    // Set the report header
    //

    pReport = (vmulticControlReportHeader*)vmultic->controlReport;
    pReport->ReportID = REPORTID_CONTROL;
    pReport->ReportLength = sizeof(vmulticKeyboardReport);

    //
    // Set the input report
    //

    pKeyboardReport = (vmulticKeyboardReport*)(vmultic->controlReport + sizeof(vmulticControlReportHeader));
    pKeyboardReport->ReportID = REPORTID_KEYBOARD;
    pKeyboardReport->ShiftKeyFlags = shiftKeyFlags;
    memcpy(pKeyboardReport->KeyCodes, keyCodes, KBD_KEY_CODES);

    // Send the report
    return HidOutput(FALSE, vmultic->hControl, (PCHAR)vmultic->controlReport, CONTROL_REPORT_SIZE);
}

BOOL vmultic_write_message(pvmultic_client vmultic, vmulticMessageReport* pReport)
{
    vmulticControlReportHeader* pReportHeader;
    ULONG bytesWritten;

    //
    // Set the report header
    //

    pReportHeader = (vmulticControlReportHeader*)vmultic->controlReport;
    pReportHeader->ReportID = REPORTID_CONTROL;
    pReportHeader->ReportLength = sizeof(vmulticMessageReport);

    //
    // Set the body
    //

    pReport->ReportID = REPORTID_MESSAGE;
    memcpy(vmultic->controlReport + sizeof(vmulticControlReportHeader), pReport, sizeof(vmulticMessageReport));

    //
    // Write the report
    //

    if (!WriteFile(vmultic->hControl, vmultic->controlReport, CONTROL_REPORT_SIZE, &bytesWritten, NULL))
    {
        printf("failed WriteFile %d\n", GetLastError());
        return FALSE;
    }

    return TRUE;
}

BOOL vmultic_read_message(pvmultic_client vmultic, vmulticMessageReport* pReport)
{
    ULONG bytesRead;

    //
    // Read the report
    //

    if (!ReadFile(vmultic->hMessage, pReport, sizeof(vmulticMessageReport), &bytesRead, NULL))
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

    if (Attributes.VendorID == vmultic_VID && Attributes.ProductID == vmultic_PID)
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
