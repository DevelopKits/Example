// Adapter.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "Adapter.h"
#include <iostream>
using namespace std;
int main(int argc, char* argv[])
{
	Adaptee* ade = new Adaptee;
	Target* adt = new Adapter(ade);
	adt->Request();
	return 0;
}