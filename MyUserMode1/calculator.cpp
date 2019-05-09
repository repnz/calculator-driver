#include "calculator.h"
#include <exception>
#include <winioctl.h>
#include "MyDriver1/ioctl.h"

Calculator::Calculator() : hDevice(NULL)
{
	hDevice = CreateFile(
		"\\\\.\\Calculator",
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (hDevice == INVALID_HANDLE_VALUE) {
		throw std::exception("Error initializing device.");
	}
}

ULONG Calculator::PerformOperation(ULONG opcode, ULONG a, ULONG b) {

	ULONG args[] = { a, b };
	ULONG result = 0;
	DWORD bytesReturned;

	if (!DeviceIoControl(hDevice,
		BUILD_CALCULATOR_IOCTL(opcode),
		&args,
		sizeof(args),
		&result,
		sizeof(result),
		&bytesReturned,
		NULL
	)) {
		throw std::exception("Could not communicate with driver");
	}

	return result;
}


ULONG Calculator::Add(ULONG a, ULONG b) { return PerformOperation(CALCULATOR_ADD_OPCODE, a, b); }
ULONG Calculator::Sub(ULONG a, ULONG b) { return PerformOperation(CALCULATOR_SUB_OPCODE, a, b); }
ULONG Calculator::Div(ULONG a, ULONG b) { return PerformOperation(CALCULATOR_DIV_OPCODE, a, b); }
ULONG Calculator::Mul(ULONG a, ULONG b) { return PerformOperation(CALCULATOR_MUL_OPCODE, a, b); }

Calculator::~Calculator() {
	if (hDevice != NULL) {
		CloseHandle(hDevice);
	}
}