#include "MyMessage.h"
#include "myhelper.h"

MyMessage::MyMessage(QDialog* parent /*= 0*/)
{
	ui.setupUi(this);
	myHelper::SetStyle("black");//ºÚÉ«·ç¸ñ
	connect(ui.m_btnInfo, SIGNAL(clicked()), this, SLOT(onInfo()));
	connect(ui.m_btnWarn, SIGNAL(clicked()), this, SLOT(onWarn()));
	connect(ui.m_btnError, SIGNAL(clicked()), this, SLOT(onError()));
	/*myHelper::FormInCenter(this);*/
}

MyMessage::~MyMessage()
{

}

void MyMessage::onInfo()
{
	myHelper::ShowMessageBoxInfo(ui.m_editText->text());
}

void MyMessage::onWarn()
{
	myHelper::ShowMessageBoxQuesion(ui.m_editText->text());
}

void MyMessage::onError()
{
	myHelper::ShowMessageBoxError(ui.m_editText->text());
}
