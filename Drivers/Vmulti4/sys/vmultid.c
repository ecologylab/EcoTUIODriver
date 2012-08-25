#include <vmultid.h>

//
// Globals
//

ULONG vmultidDebugLevel      = 100;
ULONG vmultidDebugCatagories = DBG_INIT || DBG_PNP || DBG_IOCTL;


NTSTATUS
DriverEntry (
    __in PDRIVER_OBJECT  DriverObject,
    __in PUNICODE_STRING RegistryPath
    )
{
    NTSTATUS               status = STATUS_SUCCESS;
    WDF_DRIVER_CONFIG      config;
    WDF_OBJECT_ATTRIBUTES  attributes;

    vmultidPrint(DEBUG_LEVEL_INFO, DBG_INIT,
        "Driver Entry: Built %s %s\n", __DATE__, __TIME__);

    WDF_DRIVER_CONFIG_INIT(&config, vmultidEvtDeviceAdd);

    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);

    //
    // Create a framework driver object to represent our driver.
    //

    status = WdfDriverCreate(DriverObject,
                             RegistryPath,
                             &attributes,      
                             &config,         
                             WDF_NO_HANDLE
                             );

    if (!NT_SUCCESS(status)) 
    {
        vmultidPrint(DEBUG_LEVEL_ERROR, DBG_INIT,
            "WdfDriverCreate failed with status 0x%x\n", status);
    }

    return status;
}

NTSTATUS
vmultidEvtDeviceAdd(
    IN WDFDRIVER       Driver,
    IN PWDFDEVICE_INIT DeviceInit
    )
{
    NTSTATUS                      status = STATUS_SUCCESS;
    WDF_IO_QUEUE_CONFIG           queueConfig;
    WDF_OBJECT_ATTRIBUTES         attributes;
    WDFDEVICE                     device;
    WDFQUEUE                      queue;
    UCHAR                         minorFunction;
    Pvmultid_CONTEXT               devContext;

    UNREFERENCED_PARAMETER(Driver);

    PAGED_CODE();

    vmultidPrint(DEBUG_LEVEL_INFO, DBG_PNP,
        "vmultidEvtDeviceAdd called\n");

    //
    // Tell framework this is a filter driver. Filter drivers by default are  
    // not power policy owners. This works well for this driver because
    // HIDclass driver is the power policy owner for HID minidrivers.
    //
    
    WdfFdoInitSetFilter(DeviceInit);

    //
    // Because we are a virtual device the root enumerator would just put null values 
    // in response to IRP_MN_QUERY_ID. Lets override that.
    //
    
    minorFunction = IRP_MN_QUERY_ID;

    status = WdfDeviceInitAssignWdmIrpPreprocessCallback(
                                             DeviceInit,
                                             vmultidEvtWdmPreprocessMnQueryId,
                                             IRP_MJ_PNP,
                                             &minorFunction, 
                                             1 
                                             ); 
    if (!NT_SUCCESS(status)) 
    {
        vmultidPrint(DEBUG_LEVEL_ERROR, DBG_PNP,
                "WdfDeviceInitAssignWdmIrpPreprocessCallback failed Status 0x%x\n", status);

        return status;
    }
    
    //
    // Setup the device context
    //

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, vmultid_CONTEXT);

    //
    // Create a framework device object.This call will in turn create
    // a WDM device object, attach to the lower stack, and set the
    // appropriate flags and attributes.
    //
    
    status = WdfDeviceCreate(&DeviceInit, &attributes, &device);

    if (!NT_SUCCESS(status)) 
    {
        vmultidPrint(DEBUG_LEVEL_ERROR, DBG_PNP,
            "WdfDeviceCreate failed with status code 0x%x\n", status);

        return status;
    }

    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queueConfig, WdfIoQueueDispatchParallel);

    queueConfig.EvtIoInternalDeviceControl = vmultidEvtInternalDeviceControl;

    status = WdfIoQueueCreate(device,
                              &queueConfig,
                              WDF_NO_OBJECT_ATTRIBUTES,
                              &queue
                              );

    if (!NT_SUCCESS (status)) 
    {
        vmultidPrint(DEBUG_LEVEL_ERROR, DBG_PNP,
            "WdfIoQueueCreate failed 0x%x\n", status);

        return status;
    }

    //
    // Create manual I/O queue to take care of hid report read requests
    //

    devContext = vmultidGetDeviceContext(device);

    WDF_IO_QUEUE_CONFIG_INIT(&queueConfig, WdfIoQueueDispatchManual);

    queueConfig.PowerManaged = WdfFalse;

    status = WdfIoQueueCreate(device,
            &queueConfig,
            WDF_NO_OBJECT_ATTRIBUTES,
            &devContext->ReportQueue
            );

    if (!NT_SUCCESS(status)) 
    {
        vmultidPrint(DEBUG_LEVEL_ERROR, DBG_PNP,
            "WdfIoQueueCreate failed 0x%x\n", status);

        return status;
    }

    //
    // Initialize DeviceMode
    //

    devContext->DeviceMode = DEVICE_MODE_MOUSE;

    return status;
}

NTSTATUS
vmultidEvtWdmPreprocessMnQueryId(
    WDFDEVICE Device,
    PIRP Irp
    )
{
    NTSTATUS            status;
    PIO_STACK_LOCATION  IrpStack, previousSp;
    PDEVICE_OBJECT      DeviceObject;
    PWCHAR              buffer;

    PAGED_CODE();

    //
    // Get a pointer to the current location in the Irp
    //

    IrpStack = IoGetCurrentIrpStackLocation (Irp);

    //
    // Get the device object
    //
    DeviceObject = WdfDeviceWdmGetDeviceObject(Device); 


    vmultidPrint(DEBUG_LEVEL_VERBOSE, DBG_PNP,
            "vmultidEvtWdmPreprocessMnQueryId Entry\n");

    //
    // This check is required to filter out QUERY_IDs forwarded
    // by the HIDCLASS for the parent FDO. These IDs are sent
    // by PNP manager for the parent FDO if you root-enumerate this driver.
    //
    previousSp = ((PIO_STACK_LOCATION) ((UCHAR *) (IrpStack) +
                sizeof(IO_STACK_LOCATION)));

    if (previousSp->DeviceObject == DeviceObject) 
    {
        //
        // Filtering out this basically prevents the Found New Hardware
        // popup for the root-enumerated vmultid on reboot.
        //
        status = Irp->IoStatus.Status;
    }
    else
    {
        switch (IrpStack->Parameters.QueryId.IdType)
        {
            case BusQueryDeviceID:
            case BusQueryHardwareIDs:
                //
                // HIDClass is asking for child deviceid & hardwareids.
                // Let us just make up some id for our child device.
                //
                buffer = (PWCHAR)ExAllocatePoolWithTag(
                        NonPagedPool,
                        vmultid_HARDWARE_IDS_LENGTH,
                        vmultid_POOL_TAG
                        );

                if (buffer) 
                {
                    //
                    // Do the copy, store the buffer in the Irp
                    //
                    RtlCopyMemory(buffer,
                            vmultid_HARDWARE_IDS,
                            vmultid_HARDWARE_IDS_LENGTH
                            );

                    Irp->IoStatus.Information = (ULONG_PTR)buffer;
                    status = STATUS_SUCCESS;
                }
                else 
                {
                    //
                    //  No memory
                    //
                    status = STATUS_INSUFFICIENT_RESOURCES;
                }

                Irp->IoStatus.Status = status;
                //
                // We don't need to forward this to our bus. This query
                // is for our child so we should complete it right here.
                // fallthru.
                //
                IoCompleteRequest (Irp, IO_NO_INCREMENT);

                break;

            default:
                status = Irp->IoStatus.Status;
                IoCompleteRequest (Irp, IO_NO_INCREMENT);
                break;
        }
    }

    vmultidPrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultidEvtWdmPreprocessMnQueryId Exit = 0x%x\n", status);

    return status;
}

VOID
vmultidEvtInternalDeviceControl(
    IN WDFQUEUE     Queue,
    IN WDFREQUEST   Request,
    IN size_t       OutputBufferLength,
    IN size_t       InputBufferLength,
    IN ULONG        IoControlCode
    )
{
    NTSTATUS            status = STATUS_SUCCESS;
    WDFDEVICE           device;
    Pvmultid_CONTEXT     devContext;
    BOOLEAN             completeRequest = TRUE;

    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(InputBufferLength);

    device = WdfIoQueueGetDevice(Queue);
    devContext = vmultidGetDeviceContext(device);

    vmultidPrint(DEBUG_LEVEL_INFO, DBG_IOCTL,
        "%s, Queue:0x%p, Request:0x%p\n",
        DbgHidInternalIoctlString(IoControlCode),
        Queue, 
        Request
        );

    //
    // Please note that HIDCLASS provides the buffer in the Irp->UserBuffer
    // field irrespective of the ioctl buffer type. However, framework is very
    // strict about type checking. You cannot get Irp->UserBuffer by using
    // WdfRequestRetrieveOutputMemory if the ioctl is not a METHOD_NEITHER
    // internal ioctl. So depending on the ioctl code, we will either
    // use retreive function or escape to WDM to get the UserBuffer.
    //

    switch(IoControlCode) 
    {

    case IOCTL_HID_GET_DEVICE_DESCRIPTOR:
        //
        // Retrieves the device's HID descriptor.
        //
        status = vmultidGetHidDescriptor(device, Request);
        break;

    case IOCTL_HID_GET_DEVICE_ATTRIBUTES:
        //
        //Retrieves a device's attributes in a HID_DEVICE_ATTRIBUTES structure.
        //
        status = vmultidGetDeviceAttributes(Request);
        break;

    case IOCTL_HID_GET_REPORT_DESCRIPTOR:
        //
        //Obtains the report descriptor for the HID device.
        //
        status = vmultidGetReportDescriptor(device, Request);
        break;

    case IOCTL_HID_GET_STRING:
        //
        // Requests that the HID minidriver retrieve a human-readable string
        // for either the manufacturer ID, the product ID, or the serial number
        // from the string descriptor of the device. The minidriver must send
        // a Get String Descriptor request to the device, in order to retrieve
        // the string descriptor, then it must extract the string at the
        // appropriate index from the string descriptor and return it in the
        // output buffer indicated by the IRP. Before sending the Get String
        // Descriptor request, the minidriver must retrieve the appropriate
        // index for the manufacturer ID, the product ID or the serial number
        // from the device extension of a top level collection associated with
        // the device.
        //
        status = vmultidGetString(Request);
        break;

    case IOCTL_HID_WRITE_REPORT:
    case IOCTL_HID_SET_OUTPUT_REPORT:
        //
        //Transmits a class driver-supplied report to the device.
        //
        status = vmultidWriteReport(devContext, Request);
        break;
 
    case IOCTL_HID_READ_REPORT:
    case IOCTL_HID_GET_INPUT_REPORT:
        //
        // Returns a report from the device into a class driver-supplied buffer.
        // 
        status = vmultidReadReport(devContext, Request, &completeRequest);
        break;

    case IOCTL_HID_SET_FEATURE:
        //
        // This sends a HID class feature report to a top-level collection of
        // a HID class device.
        //
        status = vmultidSetFeature(devContext, Request, &completeRequest);
        break;

    case IOCTL_HID_GET_FEATURE:
        //
        // returns a feature report associated with a top-level collection
        //
        status = vmultidGetFeature(devContext, Request, &completeRequest);
        break;

    case IOCTL_HID_ACTIVATE_DEVICE:
        //
        // Makes the device ready for I/O operations.
        //
    case IOCTL_HID_DEACTIVATE_DEVICE:
        //
        // Causes the device to cease operations and terminate all outstanding
        // I/O requests.
        //
    default:
        status = STATUS_NOT_SUPPORTED;
        break;
    }

    if (completeRequest)
    {
        WdfRequestComplete(Request, status);

        vmultidPrint(DEBUG_LEVEL_INFO, DBG_IOCTL,
                "%s completed, Queue:0x%p, Request:0x%p\n",
                DbgHidInternalIoctlString(IoControlCode),
                Queue, 
                Request
        );
    }
    else
    {
        vmultidPrint(DEBUG_LEVEL_INFO, DBG_IOCTL,
                "%s deferred, Queue:0x%p, Request:0x%p\n",
                DbgHidInternalIoctlString(IoControlCode),
                Queue, 
                Request
        );
    }

    return;
}

NTSTATUS
vmultidGetHidDescriptor(
    IN WDFDEVICE Device,
    IN WDFREQUEST Request
    )
{
    NTSTATUS            status = STATUS_SUCCESS;
    size_t              bytesToCopy = 0;
    WDFMEMORY           memory;

    UNREFERENCED_PARAMETER(Device);

    vmultidPrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultidGetHidDescriptor Entry\n");

    //
    // This IOCTL is METHOD_NEITHER so WdfRequestRetrieveOutputMemory
    // will correctly retrieve buffer from Irp->UserBuffer. 
    // Remember that HIDCLASS provides the buffer in the Irp->UserBuffer
    // field irrespective of the ioctl buffer type. However, framework is very
    // strict about type checking. You cannot get Irp->UserBuffer by using
    // WdfRequestRetrieveOutputMemory if the ioctl is not a METHOD_NEITHER
    // internal ioctl.
    //
    status = WdfRequestRetrieveOutputMemory(Request, &memory);

    if (!NT_SUCCESS(status)) 
    {
        vmultidPrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
            "WdfRequestRetrieveOutputMemory failed 0x%x\n", status);

        return status;
    }

    //
    // Use hardcoded "HID Descriptor" 
    //
    bytesToCopy = DefaultHidDescriptor.bLength;

    if (bytesToCopy == 0) 
    {
        status = STATUS_INVALID_DEVICE_STATE;

        vmultidPrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
            "DefaultHidDescriptor is zero, 0x%x\n", status);

        return status;        
    }
    
    status = WdfMemoryCopyFromBuffer(memory,
                            0, // Offset
                            (PVOID) &DefaultHidDescriptor,
                            bytesToCopy);

    if (!NT_SUCCESS(status)) 
    {
        vmultidPrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
            "WdfMemoryCopyFromBuffer failed 0x%x\n", status);

        return status;
    }

    //
    // Report how many bytes were copied
    //
    WdfRequestSetInformation(Request, bytesToCopy);

    vmultidPrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultidGetHidDescriptor Exit = 0x%x\n", status);

    return status;
}

NTSTATUS
vmultidGetReportDescriptor(
    IN WDFDEVICE Device,
    IN WDFREQUEST Request
    )
{
    NTSTATUS            status = STATUS_SUCCESS;
    ULONG_PTR           bytesToCopy;
    WDFMEMORY           memory;

    UNREFERENCED_PARAMETER(Device);

    vmultidPrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultidGetReportDescriptor Entry\n");

    //
    // This IOCTL is METHOD_NEITHER so WdfRequestRetrieveOutputMemory
    // will correctly retrieve buffer from Irp->UserBuffer. 
    // Remember that HIDCLASS provides the buffer in the Irp->UserBuffer
    // field irrespective of the ioctl buffer type. However, framework is very
    // strict about type checking. You cannot get Irp->UserBuffer by using
    // WdfRequestRetrieveOutputMemory if the ioctl is not a METHOD_NEITHER
    // internal ioctl.
    //
    status = WdfRequestRetrieveOutputMemory(Request, &memory);
    if (!NT_SUCCESS(status)) 
    {
        vmultidPrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
            "WdfRequestRetrieveOutputMemory failed 0x%x\n", status);

        return status;
    }

    //
    // Use hardcoded Report descriptor
    //
    bytesToCopy = DefaultHidDescriptor.DescriptorList[0].wReportLength;

    if (bytesToCopy == 0) 
    {
        status = STATUS_INVALID_DEVICE_STATE;

        vmultidPrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
            "DefaultHidDescriptor's reportLength is zero, 0x%x\n", status);

        return status;        
    }
    
    status = WdfMemoryCopyFromBuffer(memory,
                            0,
                            (PVOID) DefaultReportDescriptor,
                            bytesToCopy);
    if (!NT_SUCCESS(status)) 
    {
        vmultidPrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
            "WdfMemoryCopyFromBuffer failed 0x%x\n", status);

        return status;
    }

    //
    // Report how many bytes were copied
    //
    WdfRequestSetInformation(Request, bytesToCopy);

    vmultidPrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultidGetReportDescriptor Exit = 0x%x\n", status);

    return status;
}


NTSTATUS
vmultidGetDeviceAttributes(
    IN WDFREQUEST Request
    )
{
    NTSTATUS                 status = STATUS_SUCCESS;
    PHID_DEVICE_ATTRIBUTES   deviceAttributes = NULL;

    vmultidPrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultidGetDeviceAttributes Entry\n");

    //
    // This IOCTL is METHOD_NEITHER so WdfRequestRetrieveOutputMemory
    // will correctly retrieve buffer from Irp->UserBuffer. 
    // Remember that HIDCLASS provides the buffer in the Irp->UserBuffer
    // field irrespective of the ioctl buffer type. However, framework is very
    // strict about type checking. You cannot get Irp->UserBuffer by using
    // WdfRequestRetrieveOutputMemory if the ioctl is not a METHOD_NEITHER
    // internal ioctl.
    //
    status = WdfRequestRetrieveOutputBuffer(Request,
                                            sizeof (HID_DEVICE_ATTRIBUTES),
                                            &deviceAttributes,
                                            NULL);
    if (!NT_SUCCESS(status)) 
    {
        vmultidPrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
            "WdfRequestRetrieveOutputBuffer failed 0x%x\n", status);

        return status;
    }

    //
    // Set USB device descriptor
    //

    deviceAttributes->Size = sizeof (HID_DEVICE_ATTRIBUTES);
    deviceAttributes->VendorID = vmultid_VID;
    deviceAttributes->ProductID = vmultid_PID;
    deviceAttributes->VersionNumber = vmultid_VERSION;

    //
    // Report how many bytes were copied
    //
    WdfRequestSetInformation(Request, sizeof (HID_DEVICE_ATTRIBUTES));

    vmultidPrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultidGetDeviceAttributes Exit = 0x%x\n", status);

    return status;
}

NTSTATUS
vmultidGetString(
    IN WDFREQUEST Request
    )
{
 
    NTSTATUS status = STATUS_SUCCESS;
    PWSTR pwstrID;
    size_t lenID;
    WDF_REQUEST_PARAMETERS params;
    void *pStringBuffer = NULL;

    vmultidPrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultidGetString Entry\n");
 
    WDF_REQUEST_PARAMETERS_INIT(&params);
    WdfRequestGetParameters(Request, &params);

    switch ((ULONG_PTR)params.Parameters.DeviceIoControl.Type3InputBuffer & 0xFFFF)
    {
        case HID_STRING_ID_IMANUFACTURER:
            pwstrID = L"DJP Inc.\0";
            break;

        case HID_STRING_ID_IPRODUCT:
            pwstrID = L"Virtual Multitouch Device\0";
            break;

        case HID_STRING_ID_ISERIALNUMBER:
            pwstrID = L"123123123\0";
            break;

        default:
            pwstrID = NULL;
            break;
    }

    lenID = pwstrID ? wcslen(pwstrID)*sizeof(WCHAR) + sizeof(UNICODE_NULL): 0;

    if(pwstrID == NULL)
    {

        vmultidPrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
            "vmultidGetString Invalid request type\n");

        status = STATUS_INVALID_PARAMETER;

        return status;
    }

    status = WdfRequestRetrieveOutputBuffer(Request,
                                            lenID,
                                            &pStringBuffer,
                                            &lenID);

    if(!NT_SUCCESS( status)) 
    {

        vmultidPrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
            "vmultidGetString WdfRequestRetrieveOutputBuffer failed Status 0x%x\n", status);

        return status;
    }

    RtlCopyMemory(pStringBuffer, pwstrID, lenID);

    WdfRequestSetInformation(Request, lenID);

    vmultidPrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultidGetString Exit = 0x%x\n", status);
 
    return status;
}

NTSTATUS
vmultidWriteReport(
    IN Pvmultid_CONTEXT DevContext,
    IN WDFREQUEST Request
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    WDF_REQUEST_PARAMETERS params;
    PHID_XFER_PACKET transferPacket = NULL;
    vmultidControlReportHeader* pReport = NULL;
    size_t bytesWritten = 0;

    vmultidPrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultidWriteReport Entry\n");

    WDF_REQUEST_PARAMETERS_INIT(&params);
    WdfRequestGetParameters(Request, &params);

    if (params.Parameters.DeviceIoControl.InputBufferLength < sizeof(HID_XFER_PACKET)) 
    {
        vmultidPrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                "vmultidWriteReport Xfer packet too small\n");

        status = STATUS_BUFFER_TOO_SMALL;
    }
    else
    {

        transferPacket = (PHID_XFER_PACKET) WdfRequestWdmGetIrp(Request)->UserBuffer;
        
        if (transferPacket == NULL) 
        {
            vmultidPrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                "vmultidWriteReport No xfer packet\n");

            status = STATUS_INVALID_DEVICE_REQUEST;
        }
        else
        {
            //
            // switch on the report id
            //

            switch (transferPacket->reportId)
            {
                case REPORTID_CONTROL:

                    pReport = (vmultidControlReportHeader*) transferPacket->reportBuffer;

                    if (pReport->ReportLength <= transferPacket->reportBufferLen - sizeof(vmultidControlReportHeader))
                    {
                        status = vmultidProcessVendorReport(
                                DevContext,
                                transferPacket->reportBuffer + sizeof(vmultidControlReportHeader),
                                pReport->ReportLength,
                                &bytesWritten);

                        if (NT_SUCCESS(status))
                        {
                            //
                            // Report how many bytes were written
                            //

                            WdfRequestSetInformation(Request, bytesWritten); 

                            vmultidPrint(DEBUG_LEVEL_INFO, DBG_IOCTL,
                                    "vmultidWriteReport %d bytes written\n", bytesWritten);
                        }
                    }
                    else
                    {
                        status = STATUS_INVALID_PARAMETER;

                        vmultidPrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                                "vmultidWriteReport Error pReport.ReportLength (%d) is too big for buffer size (%d)\n", 
                                pReport->ReportLength,
                                transferPacket->reportBufferLen - sizeof(vmultidControlReportHeader));
                    }

                    break;

                default:

                    vmultidPrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                            "vmultidWriteReport Unhandled report type %d\n", transferPacket->reportId);

                    status = STATUS_INVALID_PARAMETER;

                    break;
            }
        }
    }

    vmultidPrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultidWriteReport Exit = 0x%x\n", status);

    return status;

}

NTSTATUS
vmultidProcessVendorReport(
    IN Pvmultid_CONTEXT DevContext,
    IN PVOID ReportBuffer,
    IN ULONG ReportBufferLen,
    OUT size_t* BytesWritten
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    WDFREQUEST reqRead;
    PVOID pReadReport = NULL;
    size_t bytesReturned = 0;

    vmultidPrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultidProcessVendorReport Entry\n");

    status = WdfIoQueueRetrieveNextRequest(DevContext->ReportQueue, 
                                           &reqRead);

    if (NT_SUCCESS(status)) 
    {
        status = WdfRequestRetrieveOutputBuffer(reqRead,
                                                ReportBufferLen,
                                                &pReadReport,
                                                &bytesReturned);

        if (NT_SUCCESS(status)) 
        {
            //
            // Copy ReportBuffer into read request
            //

            if (bytesReturned > ReportBufferLen)
            {
                bytesReturned = ReportBufferLen;
            }

            RtlCopyMemory(pReadReport,
                    ReportBuffer,
                    bytesReturned);

            //
            // Complete read with the number of bytes returned as info
            //
            
            WdfRequestCompleteWithInformation(reqRead, 
                    status, 
                    bytesReturned);

            vmultidPrint(DEBUG_LEVEL_INFO, DBG_IOCTL,
                    "vmultidProcessVendorReport %d bytes returned\n", bytesReturned);

            //
            // Return the number of bytes written for the write request completion
            //
            
            *BytesWritten = bytesReturned;

            vmultidPrint(DEBUG_LEVEL_INFO, DBG_IOCTL,
                    "%s completed, Queue:0x%p, Request:0x%p\n",
                    DbgHidInternalIoctlString(IOCTL_HID_READ_REPORT),
                    DevContext->ReportQueue, 
                    reqRead);
        }
        else
        {
            vmultidPrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                "WdfRequestRetrieveOutputBuffer failed Status 0x%x\n", status);
        }
    }
    else
    {
        vmultidPrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                "WdfIoQueueRetrieveNextRequest failed Status 0x%x\n", status);
    }
    
    vmultidPrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultidProcessVendorReport Exit = 0x%x\n", status);

    return status;
}

NTSTATUS
vmultidReadReport(
    IN Pvmultid_CONTEXT DevContext,
    IN WDFREQUEST Request,
    OUT BOOLEAN* CompleteRequest
    )
{
    NTSTATUS status = STATUS_SUCCESS;

    vmultidPrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultidReadReport Entry\n");

    //
    // Forward this read request to our manual queue
    // (in other words, we are going to defer this request
    // until we have a corresponding write request to
    // match it with)
    //

    status = WdfRequestForwardToIoQueue(Request, DevContext->ReportQueue);

    if(!NT_SUCCESS(status))
    {
        vmultidPrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                "WdfRequestForwardToIoQueue failed Status 0x%x\n", status);
    }
    else
    {
        *CompleteRequest = FALSE;
    }

    vmultidPrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultidReadReport Exit = 0x%x\n", status);

    return status;
}

NTSTATUS
vmultidSetFeature(
    IN Pvmultid_CONTEXT DevContext,
    IN WDFREQUEST Request,
    OUT BOOLEAN* CompleteRequest
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    WDF_REQUEST_PARAMETERS params;
    PHID_XFER_PACKET transferPacket = NULL;
    vmultidFeatureReport* pReport = NULL;

    vmultidPrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultidSetFeature Entry\n");

    WDF_REQUEST_PARAMETERS_INIT(&params);
    WdfRequestGetParameters(Request, &params);

    if (params.Parameters.DeviceIoControl.InputBufferLength < sizeof(HID_XFER_PACKET)) 
    {
        vmultidPrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                "vmultidSetFeature Xfer packet too small\n");

        status = STATUS_BUFFER_TOO_SMALL;
    }
    else
    {

        transferPacket = (PHID_XFER_PACKET) WdfRequestWdmGetIrp(Request)->UserBuffer;
        
        if (transferPacket == NULL) 
        {
            vmultidPrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                "vmultidWriteReport No xfer packet\n");

            status = STATUS_INVALID_DEVICE_REQUEST;
        }
        else
        {
            //
            // switch on the report id
            //

            switch (transferPacket->reportId)
            {
                case REPORTID_FEATURE:

                    if (transferPacket->reportBufferLen == sizeof(vmultidFeatureReport))
                    {
                        pReport = (vmultidFeatureReport*) transferPacket->reportBuffer;

                        DevContext->DeviceMode = pReport->DeviceMode;

                        vmultidPrint(DEBUG_LEVEL_INFO, DBG_IOCTL,
                            "vmultidSetFeature DeviceMode = 0x%x\n", DevContext->DeviceMode);
                    }
                    else
                    {
                        status = STATUS_INVALID_PARAMETER;

                        vmultidPrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                                "vmultidSetFeature Error transferPacket->reportBufferLen (%d) is different from sizeof(vmultidFeatureReport) (%d)\n", 
                                transferPacket->reportBufferLen,
                                sizeof(vmultidFeatureReport));
                    }

                    break;

                default:

                    vmultidPrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                            "vmultidSetFeature Unhandled report type %d\n", transferPacket->reportId);

                    status = STATUS_INVALID_PARAMETER;

                    break;
            }
        }
    }

    vmultidPrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultidSetFeature Exit = 0x%x\n", status);

    return status;
}

NTSTATUS
vmultidGetFeature(
    IN Pvmultid_CONTEXT DevContext,
    IN WDFREQUEST Request,
    OUT BOOLEAN* CompleteRequest
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    WDF_REQUEST_PARAMETERS params;
    PHID_XFER_PACKET transferPacket = NULL;

    vmultidPrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultidGetFeature Entry\n");

    WDF_REQUEST_PARAMETERS_INIT(&params);
    WdfRequestGetParameters(Request, &params);

    if (params.Parameters.DeviceIoControl.OutputBufferLength < sizeof(HID_XFER_PACKET)) 
    {
        vmultidPrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                "vmultidGetFeature Xfer packet too small\n");

        status = STATUS_BUFFER_TOO_SMALL;
    }
    else
    {

        transferPacket = (PHID_XFER_PACKET) WdfRequestWdmGetIrp(Request)->UserBuffer;
        
        if (transferPacket == NULL) 
        {
            vmultidPrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                "vmultidGetFeature No xfer packet\n");

            status = STATUS_INVALID_DEVICE_REQUEST;
        }
        else
        {
            //
            // switch on the report id
            //

            switch (transferPacket->reportId)
            {
                case REPORTID_MTOUCH:
                {

                    vmultidMaxCountReport* pReport = NULL;

                    if (transferPacket->reportBufferLen == sizeof(vmultidMaxCountReport))
                    {
                        pReport = (vmultidMaxCountReport*) transferPacket->reportBuffer;

                        pReport->MaximumCount = MULTI_MAX_COUNT;

                        vmultidPrint(DEBUG_LEVEL_INFO, DBG_IOCTL,
                            "vmultidGetFeature MaximumCount = 0x%x\n", MULTI_MAX_COUNT);
                    }
                    else
                    {
                        status = STATUS_INVALID_PARAMETER;

                        vmultidPrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                                "vmultidGetFeature Error transferPacket->reportBufferLen (%d) is different from sizeof(vmultidMaxCountReport) (%d)\n", 
                                transferPacket->reportBufferLen,
                                sizeof(vmultidMaxCountReport));
                    }

                    break;
                }

                case REPORTID_FEATURE:
                {

                    vmultidFeatureReport* pReport = NULL;

                    if (transferPacket->reportBufferLen == sizeof(vmultidFeatureReport))
                    {
                        pReport = (vmultidFeatureReport*) transferPacket->reportBuffer;

                        pReport->DeviceMode = DevContext->DeviceMode;

                        pReport->DeviceIdentifier = 0;

                        vmultidPrint(DEBUG_LEVEL_INFO, DBG_IOCTL,
                            "vmultidGetFeature DeviceMode = 0x%x\n", DevContext->DeviceMode);
                    }
                    else
                    {
                        status = STATUS_INVALID_PARAMETER;

                        vmultidPrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                                "vmultidGetFeature Error transferPacket->reportBufferLen (%d) is different from sizeof(vmultidFeatureReport) (%d)\n", 
                                transferPacket->reportBufferLen,
                                sizeof(vmultidFeatureReport));
                    }

                    break;
                }

                default:

                    vmultidPrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                            "vmultidGetFeature Unhandled report type %d\n", transferPacket->reportId);

                    status = STATUS_INVALID_PARAMETER;

                    break;
            }
        }
    }

    vmultidPrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultidGetFeature Exit = 0x%x\n", status);

    return status;
}

PCHAR
DbgHidInternalIoctlString(
    IN ULONG IoControlCode
    )
{
    switch (IoControlCode)
    {
    case IOCTL_HID_GET_DEVICE_DESCRIPTOR:
        return "IOCTL_HID_GET_DEVICE_DESCRIPTOR";
    case IOCTL_HID_GET_REPORT_DESCRIPTOR:
        return "IOCTL_HID_GET_REPORT_DESCRIPTOR";
    case IOCTL_HID_READ_REPORT:
        return "IOCTL_HID_READ_REPORT";
    case IOCTL_HID_GET_DEVICE_ATTRIBUTES:
        return "IOCTL_HID_GET_DEVICE_ATTRIBUTES";
    case IOCTL_HID_WRITE_REPORT:
        return "IOCTL_HID_WRITE_REPORT";
    case IOCTL_HID_SET_FEATURE:
        return "IOCTL_HID_SET_FEATURE";
    case IOCTL_HID_GET_FEATURE:
        return "IOCTL_HID_GET_FEATURE";
    case IOCTL_HID_GET_STRING:
        return "IOCTL_HID_GET_STRING";
    case IOCTL_HID_ACTIVATE_DEVICE:
        return "IOCTL_HID_ACTIVATE_DEVICE";
    case IOCTL_HID_DEACTIVATE_DEVICE:
        return "IOCTL_HID_DEACTIVATE_DEVICE";
    case IOCTL_HID_SEND_IDLE_NOTIFICATION_REQUEST:
        return "IOCTL_HID_SEND_IDLE_NOTIFICATION_REQUEST";
    case IOCTL_HID_SET_OUTPUT_REPORT:
        return "IOCTL_HID_SET_OUTPUT_REPORT";
    case IOCTL_HID_GET_INPUT_REPORT:
        return "IOCTL_HID_GET_INPUT_REPORT";
    default:
        return "Unknown IOCTL";
    }
}
