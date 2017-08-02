#pragma once
#include <iostream>
#include "Product.h"
using namespace std ;
class Builder
{
public:
	Builder();
	virtual ~Builder();
	virtual void BuildPartA() = 0;
	virtual void BuildPartB() = 0;
	virtual void BuildPartC() = 0;
	virtual Product* GetProduct()=0;
};

class ConcreteBuilder1:public Builder
{
public:
	ConcreteBuilder1();
	virtual void BuildPartA() ;
	virtual void BuildPartB() ;
	virtual void BuildPartC() ;
	virtual Product* GetProduct();
private:
	Product* m_pProduct;

};

class ConcreteBuilder2 :public Builder
{
public:
	ConcreteBuilder2();
	virtual void BuildPartA();
	virtual void BuildPartB();
	virtual void BuildPartC();
	virtual Product* GetProduct();
private:
	Product* m_pProduct;

};

class Director
{
public:
	Director(Builder* pBuilder);
	void Construct();
	~Director();

private:
	Builder* m_pBuilder;

};

