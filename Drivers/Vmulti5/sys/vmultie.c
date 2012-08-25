#include <vmultie.h>

//
// Globals
//

ULONG vmultieDebugLevel      = 100;
ULONG vmultieDebugCatagories = DBG_INIT || DBG_PNP || DBG_IOCTL;


NTSTATUS
DriverEntry (
    __in PDRIVER_OBJECT  DriverObject,
    __in PUNICODE_STRING RegistryPath
    )
{
    NTSTATUS               status = STATUS_SUCCESS;
    WDF_DRIVER_CONFIG      config;
    WDF_OBJECT_ATTRIBUTES  attributes;

    vmultiePrint(DEBUG_LEVEL_INFO, DBG_INIT,
        "Driver Entry: Built %s %s\n", __DATE__, __TIME__);

    WDF_DRIVER_CONFIG_INIT(&config, vmultieEvtDeviceAdd);

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
        vmultiePrint(DEBUG_LEVEL_ERROR, DBG_INIT,
            "WdfDriverCreate failed with status 0x%x\n", status);
    }

    return status;
}

NTSTATUS
vmultieEvtDeviceAdd(
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
    Pvmultie_CONTEXT               devContext;

    UNREFERENCED_PARAMETER(Driver);

    PAGED_CODE();

    vmultiePrint(DEBUG_LEVEL_INFO, DBG_PNP,
        "vmultieEvtDeviceAdd called\n");

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
                                             vmultieEvtWdmPreprocessMnQueryId,
                                             IRP_MJ_PNP,
                                             &minorFunction, 
                                             1 
                                             ); 
    if (!NT_SUCCESS(status)) 
    {
        vmultiePrint(DEBUG_LEVEL_ERROR, DBG_PNP,
                "WdfDeviceInitAssignWdmIrpPreprocessCallback failed Status 0x%x\n", status);

        return status;
    }
    
    //
    // Setup the device context
    //

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, vmultie_CONTEXT);

    //
    // Create a framework device object.This call will in turn create
    // a WDM device object, attach to the lower stack, and set the
    // appropriate flags and attributes.
    //
    
    status = WdfDeviceCreate(&DeviceInit, &attributes, &device);

    if (!NT_SUCCESS(status)) 
    {
        vmultiePrint(DEBUG_LEVEL_ERROR, DBG_PNP,
            "WdfDeviceCreate failed with status code 0x%x\n", status);

        return status;
    }

    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queueConfig, WdfIoQueueDispatchParallel);

    queueConfig.EvtIoInternalDeviceControl = vmultieEvtInternalDeviceControl;

    status = WdfIoQueueCreate(device,
                              &queueConfig,
                              WDF_NO_OBJECT_ATTRIBUTES,
                              &queue
                              );

    if (!NT_SUCCESS (status)) 
    {
        vmultiePrint(DEBUG_LEVEL_ERROR, DBG_PNP,
            "WdfIoQueueCreate failed 0x%x\n", status);

        return status;
    }

    //
    // Create manual I/O queue to take care of hid report read requests
    //

    devContext = vmultieGetDeviceContext(device);

    WDF_IO_QUEUE_CONFIG_INIT(&queueConfig, WdfIoQueueDispatchManual);

    queueConfig.PowerManaged = WdfFalse;

    status = WdfIoQueueCreate(device,
            &queueConfig,
            WDF_NO_OBJECT_ATTRIBUTES,
            &devContext->ReportQueue
            );

    if (!NT_SUCCESS(status)) 
    {
        vmultiePrint(DEBUG_LEVEL_ERROR, DBG_PNP,
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
vmultieEvtWdmPreprocessMnQueryId(
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


    vmultiePrint(DEBUG_LEVEL_VERBOSE, DBG_PNP,
            "vmultieEvtWdmPreprocessMnQueryId Entry\n");

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
        // popup for the root-enumerated vmultie on reboot.
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
                        vmultie_HARDWARE_IDS_LENGTH,
                        vmultie_POOL_TAG
                        );

                if (buffer) 
                {
                    //
                    // Do the copy, store the buffer in the Irp
                    //
                    RtlCopyMemory(buffer,
                            vmultie_HARDWARE_IDS,
                            vmultie_HARDWARE_IDS_LENGTH
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

    vmultiePrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultieEvtWdmPreprocessMnQueryId Exit = 0x%x\n", status);

    return status;
}

VOID
vmultieEvtInternalDeviceControl(
    IN WDFQUEUE     Queue,
    IN WDFREQUEST   Request,
    IN size_t       OutputBufferLength,
    IN size_t       InputBufferLength,
    IN ULONG        IoControlCode
    )
{
    NTSTATUS            status = STATUS_SUCCESS;
    WDFDEVICE           device;
    Pvmultie_CONTEXT     devContext;
    BOOLEAN             completeRequest = TRUE;

    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(InputBufferLength);

    device = WdfIoQueueGetDevice(Queue);
    devContext = vmultieGetDeviceContext(device);

    vmultiePrint(DEBUG_LEVEL_INFO, DBG_IOCTL,
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
        status = vmultieGetHidDescriptor(device, Request);
        break;

    case IOCTL_HID_GET_DEVICE_ATTRIBUTES:
        //
        //Retrieves a device's attributes in a HID_DEVICE_ATTRIBUTES structure.
        //
        status = vmultieGetDeviceAttributes(Request);
        break;

    case IOCTL_HID_GET_REPORT_DESCRIPTOR:
        //
        //Obtains the report descriptor for the HID device.
        //
        status = vmultieGetReportDescriptor(device, Request);
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
        status = vmultieGetString(Request);
        break;

    case IOCTL_HID_WRITE_REPORT:
    case IOCTL_HID_SET_OUTPUT_REPORT:
        //
        //Transmits a class driver-supplied report to the device.
        //
        status = vmultieWriteReport(devContext, Request);
        break;
 
    case IOCTL_HID_READ_REPORT:
    case IOCTL_HID_GET_INPUT_REPORT:
        //
        // Returns a report from the device into a class driver-supplied buffer.
        // 
        status = vmultieReadReport(devContext, Request, &completeRequest);
        break;

    case IOCTL_HID_SET_FEATURE:
        //
        // This sends a HID class feature report to a top-level collection of
        // a HID class device.
        //
        status = vmultieSetFeature(devContext, Request, &completeRequest);
        break;

    case IOCTL_HID_GET_FEATURE:
        //
        // returns a feature report associated with a top-level collection
        //
        status = vmultieGetFeature(devContext, Request, &completeRequest);
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

        vmultiePrint(DEBUG_LEVEL_INFO, DBG_IOCTL,
                "%s completed, Queue:0x%p, Request:0x%p\n",
                DbgHidInternalIoctlString(IoControlCode),
                Queue, 
                Request
        );
    }
    else
    {
        vmultiePrint(DEBUG_LEVEL_INFO, DBG_IOCTL,
                "%s deferred, Queue:0x%p, Request:0x%p\n",
                DbgHidInternalIoctlString(IoControlCode),
                Queue, 
                Request
        );
    }

    return;
}

NTSTATUS
vmultieGetHidDescriptor(
    IN WDFDEVICE Device,
    IN WDFREQUEST Request
    )
{
    NTSTATUS            status = STATUS_SUCCESS;
    size_t              bytesToCopy = 0;
    WDFMEMORY           memory;

    UNREFERENCED_PARAMETER(Device);

    vmultiePrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultieGetHidDescriptor Entry\n");

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
        vmultiePrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
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

        vmultiePrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
            "DefaultHidDescriptor is zero, 0x%x\n", status);

        return status;        
    }
    
    status = WdfMemoryCopyFromBuffer(memory,
                            0, // Offset
                            (PVOID) &DefaultHidDescriptor,
                            bytesToCopy);

    if (!NT_SUCCESS(status)) 
    {
        vmultiePrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
            "WdfMemoryCopyFromBuffer failed 0x%x\n", status);

        return status;
    }

    //
    // Report how many bytes were copied
    //
    WdfRequestSetInformation(Request, bytesToCopy);

    vmultiePrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultieGetHidDescriptor Exit = 0x%x\n", status);

    return status;
}

NTSTATUS
vmultieGetReportDescriptor(
    IN WDFDEVICE Device,
    IN WDFREQUEST Request
    )
{
    NTSTATUS            status = STATUS_SUCCESS;
    ULONG_PTR           bytesToCopy;
    WDFMEMORY           memory;

    UNREFERENCED_PARAMETER(Device);

    vmultiePrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultieGetReportDescriptor Entry\n");

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
        vmultiePrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
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

        vmultiePrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
            "DefaultHidDescriptor's reportLength is zero, 0x%x\n", status);

        return status;        
    }
    
    status = WdfMemoryCopyFromBuffer(memory,
                            0,
                            (PVOID) DefaultReportDescriptor,
                            bytesToCopy);
    if (!NT_SUCCESS(status)) 
    {
        vmultiePrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
            "WdfMemoryCopyFromBuffer failed 0x%x\n", status);

        return status;
    }

    //
    // Report how many bytes were copied
    //
    WdfRequestSetInformation(Request, bytesToCopy);

    vmultiePrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultieGetReportDescriptor Exit = 0x%x\n", status);

    return status;
}


NTSTATUS
vmultieGetDeviceAttributes(
    IN WDFREQUEST Request
    )
{
    NTSTATUS                 status = STATUS_SUCCESS;
    PHID_DEVICE_ATTRIBUTES   deviceAttributes = NULL;

    vmultiePrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultieGetDeviceAttributes Entry\n");

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
        vmultiePrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
            "WdfRequestRetrieveOutputBuffer failed 0x%x\n", status);

        return status;
    }

    //
    // Set USB device descriptor
    //

    deviceAttributes->Size = sizeof (HID_DEVICE_ATTRIBUTES);
    deviceAttributes->VendorID = vmultie_VID;
    deviceAttributes->ProductID = vmultie_PID;
    deviceAttributes->VersionNumber = vmultie_VERSION;

    //
    // Report how many bytes were copied
    //
    WdfRequestSetInformation(Request, sizeof (HID_DEVICE_ATTRIBUTES));

    vmultiePrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultieGetDeviceAttributes Exit = 0x%x\n", status);

    return status;
}

NTSTATUS
vmultieGetString(
    IN WDFREQUEST Request
    )
{
 
    NTSTATUS status = STATUS_SUCCESS;
    PWSTR pwstrID;
    size_t lenID;
    WDF_REQUEST_PARAMETERS params;
    void *pStringBuffer = NULL;

    vmultiePrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultieGetString Entry\n");
 
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

        vmultiePrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
            "vmultieGetString Invalid request type\n");

        status = STATUS_INVALID_PARAMETER;

        return status;
    }

    status = WdfRequestRetrieveOutputBuffer(Request,
                                            lenID,
                                            &pStringBuffer,
                                            &lenID);

    if(!NT_SUCCESS( status)) 
    {

        vmultiePrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
            "vmultieGetString WdfRequestRetrieveOutputBuffer failed Status 0x%x\n", status);

        return status;
    }

    RtlCopyMemory(pStringBuffer, pwstrID, lenID);

    WdfRequestSetInformation(Request, lenID);

    vmultiePrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultieGetString Exit = 0x%x\n", status);
 
    return status;
}

NTSTATUS
vmultieWriteReport(
    IN Pvmultie_CONTEXT DevContext,
    IN WDFREQUEST Request
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    WDF_REQUEST_PARAMETERS params;
    PHID_XFER_PACKET transferPacket = NULL;
    vmultieControlReportHeader* pReport = NULL;
    size_t bytesWritten = 0;

    vmultiePrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultieWriteReport Entry\n");

    WDF_REQUEST_PARAMETERS_INIT(&params);
    WdfRequestGetParameters(Request, &params);

    if (params.Parameters.DeviceIoControl.InputBufferLength < sizeof(HID_XFER_PACKET)) 
    {
        vmultiePrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                "vmultieWriteReport Xfer packet too small\n");

        status = STATUS_BUFFER_TOO_SMALL;
    }
    else
    {

        transferPacket = (PHID_XFER_PACKET) WdfRequestWdmGetIrp(Request)->UserBuffer;
        
        if (transferPacket == NULL) 
        {
            vmultiePrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                "vmultieWriteReport No xfer packet\n");

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

                    pReport = (vmultieControlReportHeader*) transferPacket->reportBuffer;

                    if (pReport->ReportLength <= transferPacket->reportBufferLen - sizeof(vmultieControlReportHeader))
                    {
                        status = vmultieProcessVendorReport(
                                DevContext,
                                transferPacket->reportBuffer + sizeof(vmultieControlReportHeader),
                                pReport->ReportLength,
                                &bytesWritten);

                        if (NT_SUCCESS(status))
                        {
                            //
                            // Report how many bytes were written
                            //

                            WdfRequestSetInformation(Request, bytesWritten); 

                            vmultiePrint(DEBUG_LEVEL_INFO, DBG_IOCTL,
                                    "vmultieWriteReport %d bytes written\n", bytesWritten);
                        }
                    }
                    else
                    {
                        status = STATUS_INVALID_PARAMETER;

                        vmultiePrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                                "vmultieWriteReport Error pReport.ReportLength (%d) is too big for buffer size (%d)\n", 
                                pReport->ReportLength,
                                transferPacket->reportBufferLen - sizeof(vmultieControlReportHeader));
                    }

                    break;

                default:

                    vmultiePrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                            "vmultieWriteReport Unhandled report type %d\n", transferPacket->reportId);

                    status = STATUS_INVALID_PARAMETER;

                    break;
            }
        }
    }

    vmultiePrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultieWriteReport Exit = 0x%x\n", status);

    return status;

}

NTSTATUS
vmultieProcessVendorReport(
    IN Pvmultie_CONTEXT DevContext,
    IN PVOID ReportBuffer,
    IN ULONG ReportBufferLen,
    OUT size_t* BytesWritten
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    WDFREQUEST reqRead;
    PVOID pReadReport = NULL;
    size_t bytesReturned = 0;

    vmultiePrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultieProcessVendorReport Entry\n");

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

            vmultiePrint(DEBUG_LEVEL_INFO, DBG_IOCTL,
                    "vmultieProcessVendorReport %d bytes returned\n", bytesReturned);

            //
            // Return the number of bytes written for the write request completion
            //
            
            *BytesWritten = bytesReturned;

            vmultiePrint(DEBUG_LEVEL_INFO, DBG_IOCTL,
                    "%s completed, Queue:0x%p, Request:0x%p\n",
                    DbgHidInternalIoctlString(IOCTL_HID_READ_REPORT),
                    DevContext->ReportQueue, 
                    reqRead);
        }
        else
        {
            vmultiePrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                "WdfRequestRetrieveOutputBuffer failed Status 0x%x\n", status);
        }
    }
    else
    {
        vmultiePrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                "WdfIoQueueRetrieveNextRequest failed Status 0x%x\n", status);
    }
    
    vmultiePrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultieProcessVendorReport Exit = 0x%x\n", status);

    return status;
}

NTSTATUS
vmultieReadReport(
    IN Pvmultie_CONTEXT DevContext,
    IN WDFREQUEST Request,
    OUT BOOLEAN* CompleteRequest
    )
{
    NTSTATUS status = STATUS_SUCCESS;

    vmultiePrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultieReadReport Entry\n");

    //
    // Forward this read request to our manual queue
    // (in other words, we are going to defer this request
    // until we have a corresponding write request to
    // match it with)
    //

    status = WdfRequestForwardToIoQueue(Request, DevContext->ReportQueue);

    if(!NT_SUCCESS(status))
    {
        vmultiePrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                "WdfRequestForwardToIoQueue failed Status 0x%x\n", status);
    }
    else
    {
        *CompleteRequest = FALSE;
    }

    vmultiePrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultieReadReport Exit = 0x%x\n", status);

    return status;
}

NTSTATUS
vmultieSetFeature(
    IN Pvmultie_CONTEXT DevContext,
    IN WDFREQUEST Request,
    OUT BOOLEAN* CompleteRequest
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    WDF_REQUEST_PARAMETERS params;
    PHID_XFER_PACKET transferPacket = NULL;
    vmultieFeatureReport* pReport = NULL;

    vmultiePrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultieSetFeature Entry\n");

    WDF_REQUEST_PARAMETERS_INIT(&params);
    WdfRequestGetParameters(Request, &params);

    if (params.Parameters.DeviceIoControl.InputBufferLength < sizeof(HID_XFER_PACKET)) 
    {
        vmultiePrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                "vmultieSetFeature Xfer packet too small\n");

        status = STATUS_BUFFER_TOO_SMALL;
    }
    else
    {

        transferPacket = (PHID_XFER_PACKET) WdfRequestWdmGetIrp(Request)->UserBuffer;
        
        if (transferPacket == NULL) 
        {
            vmultiePrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                "vmultieWriteReport No xfer packet\n");

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

                    if (transferPacket->reportBufferLen == sizeof(vmultieFeatureReport))
                    {
                        pReport = (vmultieFeatureReport*) transferPacket->reportBuffer;

                        DevContext->DeviceMode = pReport->DeviceMode;

                        vmultiePrint(DEBUG_LEVEL_INFO, DBG_IOCTL,
                            "vmultieSetFeature DeviceMode = 0x%x\n", DevContext->DeviceMode);
                    }
                    else
                    {
                        status = STATUS_INVALID_PARAMETER;

                        vmultiePrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                                "vmultieSetFeature Error transferPacket->reportBufferLen (%d) is different from sizeof(vmultieFeatureReport) (%d)\n", 
                                transferPacket->reportBufferLen,
                                sizeof(vmultieFeatureReport));
                    }

                    break;

                default:

                    vmultiePrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                            "vmultieSetFeature Unhandled report type %d\n", transferPacket->reportId);

                    status = STATUS_INVALID_PARAMETER;

                    break;
            }
        }
    }

    vmultiePrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultieSetFeature Exit = 0x%x\n", status);

    return status;
}

NTSTATUS
vmultieGetFeature(
    IN Pvmultie_CONTEXT DevContext,
    IN WDFREQUEST Request,
    OUT BOOLEAN* CompleteRequest
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    WDF_REQUEST_PARAMETERS params;
    PHID_XFER_PACKET transferPacket = NULL;

    vmultiePrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultieGetFeature Entry\n");

    WDF_REQUEST_PARAMETERS_INIT(&params);
    WdfRequestGetParameters(Request, &params);

    if (params.Parameters.DeviceIoControl.OutputBufferLength < sizeof(HID_XFER_PACKET)) 
    {
        vmultiePrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                "vmultieGetFeature Xfer packet too small\n");

        status = STATUS_BUFFER_TOO_SMALL;
    }
    else
    {

        transferPacket = (PHID_XFER_PACKET) WdfRequestWdmGetIrp(Request)->UserBuffer;
        
        if (transferPacket == NULL) 
        {
            vmultiePrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                "vmultieGetFeature No xfer packet\n");

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

                    vmultieMaxCountReport* pReport = NULL;

                    if (transferPacket->reportBufferLen == sizeof(vmultieMaxCountReport))
                    {
                        pReport = (vmultieMaxCountReport*) transferPacket->reportBuffer;

                        pReport->MaximumCount = MULTI_MAX_COUNT;

                        vmultiePrint(DEBUG_LEVEL_INFO, DBG_IOCTL,
                            "vmultieGetFeature MaximumCount = 0x%x\n", MULTI_MAX_COUNT);
                    }
                    else
                    {
                        status = STATUS_INVALID_PARAMETER;

                        vmultiePrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                                "vmultieGetFeature Error transferPacket->reportBufferLen (%d) is different from sizeof(vmultieMaxCountReport) (%d)\n", 
                                transferPacket->reportBufferLen,
                                sizeof(vmultieMaxCountReport));
                    }

                    break;
                }

                case REPORTID_FEATURE:
                {

                    vmultieFeatureReport* pReport = NULL;

                    if (transferPacket->reportBufferLen == sizeof(vmultieFeatureReport))
                    {
                        pReport = (vmultieFeatureReport*) transferPacket->reportBuffer;

                        pReport->DeviceMode = DevContext->DeviceMode;

                        pReport->DeviceIdentifier = 0;

                        vmultiePrint(DEBUG_LEVEL_INFO, DBG_IOCTL,
                            "vmultieGetFeature DeviceMode = 0x%x\n", DevContext->DeviceMode);
                    }
                    else
                    {
                        status = STATUS_INVALID_PARAMETER;

                        vmultiePrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                                "vmultieGetFeature Error transferPacket->reportBufferLen (%d) is different from sizeof(vmultieFeatureReport) (%d)\n", 
                                transferPacket->reportBufferLen,
                                sizeof(vmultieFeatureReport));
                    }

                    break;
                }

                default:

                    vmultiePrint(DEBUG_LEVEL_ERROR, DBG_IOCTL,
                            "vmultieGetFeature Unhandled report type %d\n", transferPacket->reportId);

                    status = STATUS_INVALID_PARAMETER;

                    break;
            }
        }
    }

    vmultiePrint(DEBUG_LEVEL_VERBOSE, DBG_IOCTL,
        "vmultieGetFeature Exit = 0x%x\n", status);

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
