#include <ntddk.h>     
#include "ioctl.h"

// Function Declerations
VOID DriverUnload(IN PDRIVER_OBJECT DriverObject);

NTSTATUS DriverDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

VOID PrintDebug(PCSTR pString);

NTSTATUS DefaultHandler(IN PDEVICE_OBJECT DriverObject, IN PIRP Irp);

NTSTATUS Calculate(ULONG Opcode, ULONG Operand1, ULONG Operand2, PULONG Result);

// Global strings
static const WCHAR NtDeviceName[] = L"\\Device\\Calculator";
static const WCHAR DosDeviceName[] = L"\\DosDevices\\Calculator"; 


// Macros 
#define FUNCTION_FROM_CTL_CODE(CODE) ((CODE >> 2) & 0b11111111111)
#define ACCESS_FROM_CTL_CODE(CODE) ((CODE >> 14) & 0b11)


NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath) {
	PrintDebug("[+] DriverEntry: Initializing Device..");

	UNREFERENCED_PARAMETER(RegistryPath);

	NTSTATUS status;
	PDEVICE_OBJECT interfaceDevice;
	UNICODE_STRING deviceName;
	UNICODE_STRING deviceLink;

	RtlInitUnicodeString(&deviceName, NtDeviceName);
	RtlInitUnicodeString(&deviceLink, DosDeviceName);
	
	PrintDebug("[+] Calling IoCreateDevice..");

	status = IoCreateDevice(DriverObject,
		0,						// DeviceExtensionSize
		&deviceName,			// DeviceName
		FILE_DEVICE_CALCULATOR, // DeviceType
		0,						// DeviceCharacteristics
		TRUE,					// Exclusive
		&interfaceDevice		// DeviceObject
	);

	if (!NT_SUCCESS(status)) {
		PrintDebug("[-] Error");
		return status;
	}
	
	PrintDebug("[+] Calling IoCreateSymbolicLink..\n");
	
	status = IoCreateSymbolicLink(&deviceLink, &deviceName);
	
	if (!NT_SUCCESS(status)) {
		PrintDebug("[-] Error");
		return status;
	}

	//
	// Filling the driver object function pointers
	// This pointers will be called by the IO manager automatically
	//
	DriverObject->DriverUnload = DriverUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DefaultHandler;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = DefaultHandler;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverDispatch;


	return status;

}

NTSTATUS DefaultHandler(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

VOID DriverUnload(IN PDRIVER_OBJECT DriverObject) {
	UNREFERENCED_PARAMETER(DriverObject);

	PDEVICE_OBJECT pDeviceObject = DriverObject->DeviceObject;
	
	if (pDeviceObject != NULL) {

		IoDeleteDevice(pDeviceObject);
	}

	UNICODE_STRING sLink;
	RtlInitUnicodeString(&sLink, DosDeviceName);
	IoDeleteSymbolicLink(&sLink);

	PrintDebug("Bye World!");
}

NTSTATUS DriverDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp) {
	
	UNREFERENCED_PARAMETER(DeviceObject);

	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION iosp = IoGetCurrentIrpStackLocation(Irp);
	
	PrintDebug("[+] Got Device Control!");
	
	ULONG code = iosp->Parameters.DeviceIoControl.IoControlCode;
	
	if (METHOD_FROM_CTL_CODE(code) != METHOD_BUFFERED) {
		PrintDebug("[!] Only METHOD_BUFFERED is supported!");
		status = STATUS_NOT_SUPPORTED;
		goto End;
	}

	if (DEVICE_TYPE_FROM_CTL_CODE(code) != FILE_DEVICE_CALCULATOR) {
		PrintDebug("[!] Error! Device is not calculator!");
		status = STATUS_INVALID_PARAMETER;
		goto End;
	}

	if (iosp->Parameters.DeviceIoControl.InputBufferLength != (sizeof(ULONG) * 2) ||
		iosp->Parameters.DeviceIoControl.OutputBufferLength != sizeof(ULONG)) {
		PrintDebug("[!] Input Length Is Invalid!");
		status = STATUS_INVALID_PARAMETER;
		goto End;
	}

	ULONG operation = FUNCTION_FROM_CTL_CODE(code);
	PLONG systemBuffer = Irp->AssociatedIrp.SystemBuffer;

	ULONG Operand1 = systemBuffer[0];
	ULONG Operand2 = systemBuffer[1];
	ULONG result = 0;
	
	status = Calculate(operation, Operand1, Operand2, &result);

	systemBuffer[0] = result;
	systemBuffer[1] = 0;

	Irp->IoStatus.Information = sizeof(ULONG);
End:
	Irp->IoStatus.Status = status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS Calculate(ULONG Opcode, ULONG Operand1, ULONG Operand2, PULONG Result) {
	switch (Opcode) {
	case CALCULATOR_ADD_OPCODE:
		*Result = Operand1 + Operand2;
		break;

	case CALCULATOR_SUB_OPCODE:
		*Result = Operand1 - Operand2;
		break;

	case CALCULATOR_DIV_OPCODE:
		*Result = Operand1 / Operand2;
		break;

	case CALCULATOR_MUL_OPCODE:
		*Result = Operand1 * Operand2;
		break;

	default:
		*Result = 0;
		PrintDebug("[!] Unsupported math operation");
		return STATUS_INVALID_PARAMETER;
		
	}

	return STATUS_SUCCESS;
}


VOID PrintDebug(PCSTR pString) {

	DbgPrint("CalculatorDriver: ");
	DbgPrint(pString);
	DbgPrint("\n");
}