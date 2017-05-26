#pragma once
#include <QThread>
class MemLeak:public QThread
{
public:
	MemLeak();
	~MemLeak();
protected:
	void run();
};

