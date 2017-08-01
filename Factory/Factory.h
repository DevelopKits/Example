#pragma once

enum emProtype 
{ 
	Pro1 = 1,
	Pro2
};
class Product;
class Factory
{
public:
	virtual ~Factory() = 0;
	virtual Product* CreateProduct(emProtype) = 0;
protected:
	Factory();
};

class ConcreteFactory :public Factory
{
public:
	~ConcreteFactory();
	ConcreteFactory();
	Product* CreateProduct(emProtype);
};
