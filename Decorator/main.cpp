// Decorator.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"


#include "Decorator.h"
#include <iostream>
using namespace std;
int main(int argc, char* argv[])
{
	Component* com = new ConcreteComponent();
	Decorator* dec = new ConcreteDecorator(com);
	dec->Operation();
	delete dec;
	return 0;
}