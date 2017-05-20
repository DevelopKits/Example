#ifndef RSATEST_H
#define RSATEST_H

#include <QtWidgets/QDialog>
#include "ui_rsatest.h"
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <string>
#include <iostream>
using namespace  std;
class RSATest : public QDialog
{
	Q_OBJECT

public:
	RSATest(QWidget *parent = 0);
	~RSATest();
	//������Կ����Կ
	void CreateKey();
	//����
	std::string DecryptData(string data);
	//	����
	std::string EncryptData(string data);
public slots:
	void Encode();
	void Decode();


private:
	Ui::RSATestClass ui;
	string m_strdata;
	/* BASE64�����㷨ʵ�� */
	int	Base64Encode(const char* pInputBuf, int nInputBufLen, char* pOutputBuf);


};

#endif // RSATEST_H
