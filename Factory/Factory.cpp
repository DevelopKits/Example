#include "Factory.h"
#include "Product.h"
#include <iostream>
using namespace std;

Factory::Factory()
{
}


Factory::~Factory()
{
}

ConcreteFactory::~ConcreteFactory()
{

}

ConcreteFactory::ConcreteFactory()
{

}

Product* ConcreteFactory::CreateProduct(emProtype etype)
{
	switch (etype)
	{
	case Pro1:
		{
			return new ConcreteProduct1();
		}
	case Pro2:
		{
			return new ConcreteProduct2();
		}
	default:
		{
			return NULL;
		}
	}
}
