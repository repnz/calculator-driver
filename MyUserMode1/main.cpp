#include <iostream>
#include "calculator.h"

int main() {

	Calculator 	calc;
	printf("10+20=%d", calc.Add(10, 20));
	printf("30-20=%d", calc.Sub(30, 20));
	printf("30/10=%d", calc.Div(30, 10));
	printf("10*20=%d", calc.Mul(10, 20));

	return 0;
}