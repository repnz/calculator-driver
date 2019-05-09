#pragma once
#include <Windows.h>

class Calculator {
private:
	HANDLE hDevice;
	ULONG PerformOperation(ULONG opcode, ULONG a, ULONG b);
public:
	Calculator();
	ULONG Add(ULONG a, ULONG b);
	ULONG Sub(ULONG a, ULONG b);
	ULONG Div(ULONG a, ULONG b);
	ULONG Mul(ULONG a, ULONG b);
	~Calculator();
	
};