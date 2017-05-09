#pragma once
#include<iostream>
#include<string>
#include <log4Cplus/logger.h>
#include <log4Cplus/configurator.h>
#include <log4Cplus/layout.h>
#include <log4Cplus/loggingmacros.h>
#include <log4Cplus/helpers/stringhelper.h>

#define MY_LOG_FILE "./logconfig.properities"

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;

class MyLogger
{
public:
	static MyLogger* getInstance();
	Logger logger;
private:
	MyLogger();
	~MyLogger();
	static MyLogger* my_logger;
};

