#include "Factory.h"
#include "Product.h"
#include <iostream>
using namespace std;


int main(void)
{
	Factory* pfac = new ConcreteFactory;
	Product* pPro = pfac->CreateProduct(Pro1);
	getchar();
	return 0;
}