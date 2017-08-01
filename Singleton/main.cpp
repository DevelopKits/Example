// Singleton.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Singleton.h"
#include <iostream>
using namespace std;


int main(int argc, char* argv[])
{
	Singleton* sgn = Singleton::Instance();
	return 0;
}
