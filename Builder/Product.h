#pragma once
#include <string>
using namespace std;
class Product
{
public:
	Product();
	virtual ~Product();

	void setPartA(const string s);
	void setPartB(const string s);
	void setPartC(const string s);
private:
	string m_partA;
	string m_partB;
	string m_partC;
};

