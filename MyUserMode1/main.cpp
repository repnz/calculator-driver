#include <windows.h>
#include <winioctl.h>
#include <MyDriver1/ioctl.h>
#include <iostream>

int main() {

	std::cout << "[+] Initializing Device..." << std::endl;

	HANDLE hDevice = CreateFile(
		"\\\\.\\Calculator",
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (hDevice == INVALID_HANDLE_VALUE) {
		std::cerr << "[!] Error initializing device." << std::endl;
		return -1;
	}

	std::cout << "[+] Sending Io Control.." << std::endl;
	DWORD nb;

	if (!DeviceIoControl(hDevice,
		IOCTL_CALCULATOR_ADD,
		NULL,
		NULL,
		NULL,
		0,
		&nb,
		NULL
	)) {
		std::cerr << "[!] DeviceIoControl Failed." << std::endl;
	}

	return 0;
}