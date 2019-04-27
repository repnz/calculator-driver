#include <ntddk.h>     
#include "ioctl.h"

VOID DriverUnload(IN PDRIVER_OBJECT DriverObject);
NTSTATUS DriverDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

VOID DbgPrintLine(PCSTR pString) {

	DbgPrint("ClculatorDriver: ");
	DbgPrint(pString);
	DbgPrint("\n");
}

#define NT_DEVICE_NAME L"\\Device\\Calculator"
#define DOS_DEVICE_NAME L"\\DosDevices\\Calculator"

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
	DbgPrintLine("[+] DriverEntry: Initializing Device..");

	UNREFERENCED_PARAMETER(RegistryPath);
	NTSTATUS status;
	PDEVICE_OBJECT interfaceDevice;
	UNICODE_STRING deviceName;
	UNICODE_STRING deviceLink;

	RtlInitUnicodeString(&deviceName, NT_DEVICE_NAME);
	RtlInitUnicodeString(&deviceLink, DOS_DEVICE_NAME);
	
	DbgPrintLine("[+] Calling IoCreateDevice..");

	status = IoCreateDevice(DriverObject,
		0,						// DeviceExtensionSize
		&deviceName,			// DeviceName
		FILE_DEVICE_CALCULATOR, // DeviceType
		0,						// DeviceCharacteristics
		TRUE,					// Exclusive
		&interfaceDevice		// DeviceObject
	);

	if (!NT_SUCCESS(status)) {
		DbgPrint("[-] Error");
		return status;
	}
	
	DbgPrintLine("[+] Calling IoCreateSymbolicLink..\n");
	
	status = IoCreateSymbolicLink(&deviceLink, &deviceName);
	
	if (!NT_SUCCESS(status)) {
		DbgPrint("[-] Error");
		return status;
	}

	//
	// Filling the driver object function pointers
	// This pointers will be called by the IO manager automatically
	//
	DriverObject->DriverUnload = DriverUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DriverDispatch;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = DriverDispatch;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverDispatch;


	return status;

}

VOID DriverUnload(IN PDRIVER_OBJECT DriverObject) {
	UNREFERENCED_PARAMETER(DriverObject);
	PAGED_CODE();

	PDEVICE_OBJECT pDeviceObject = DriverObject->DeviceObject;
	
	if (pDeviceObject != NULL) {

		IoDeleteDevice(pDeviceObject);
	}

	UNICODE_STRING sLink;
	RtlInitUnicodeString(&sLink, DOS_DEVICE_NAME);
	IoDeleteSymbolicLink(&sLink);

	DbgPrint("Bye World!\n");
}


NTSTATUS DriverDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp) {

	UNREFERENCED_PARAMETER(DeviceObject);

	NTSTATUS status;
	PIO_STACK_LOCATION iosp = IoGetCurrentIrpStackLocation(Irp);
	

	switch (iosp->MajorFunction) {
	case IRP_MJ_CREATE:
	case IRP_MJ_CLOSE:
		status = STATUS_SUCCESS;
		break;
	case IRP_MJ_DEVICE_CONTROL:
		DbgPrintLine("Had some device control!\n");

		ULONG code = iosp->Parameters.DeviceIoControl.IoControlCode;

		switch (code) {
		case IOCTL_CALCULATOR_ADD:
			DbgPrintLine("yo.. no add supported\n");
			break;
		}

		status = STATUS_SUCCESS;
		break;
	default:
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

	Irp->IoStatus.Status = status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}