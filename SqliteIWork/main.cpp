/*这里只做一些最基本的操作
 两张表：一张用户表单，一张是多字段的数据表单，用来进行简单数据的增删改查*/
#include <iostream>
#include "CppSQLite3DB.h"
#include "libdsl/DPrintLog.h"
using namespace std;

int main()
{
	dsl::DBaseLib::Init("swartz");
	DLOG_INFO("test");
	return 0;
}