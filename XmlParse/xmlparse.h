#ifndef XMLPARSE_H
#define XMLPARSE_H

#include <QtWidgets/QDialog>
#include "ui_xmlparse.h"
#include "XMLLocalConfig.h"

class XmlParse : public QDialog
{
	Q_OBJECT

public:
	XmlParse(QWidget *parent = 0);
	~XmlParse();

private:
	Ui::XmlParseClass ui;
	XMLLocalConfig* m_xmlParse;
};

#endif // XMLPARSE_H
