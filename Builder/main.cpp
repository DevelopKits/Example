// Builder.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Builder.h"

int _tmain(int argc, _TCHAR* argv[])
{
	Director* pDirector = new Director(new ConcreteBuilder1());
	pDirector->Construct();

	Director* pDirector1 = new Director(new ConcreteBuilder2());
	pDirector1->Construct();

	return 0;
}

