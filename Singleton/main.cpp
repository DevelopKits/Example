// Singleton.cpp : �������̨Ӧ�ó������ڵ㡣
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
