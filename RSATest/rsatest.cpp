#include "rsatest.h"

RSATest::RSATest(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	CreateKey();
	connect(ui.m_btnEncode, SIGNAL(clicked()), this, SLOT(Encode()));
	connect(ui.m_btnDecode, SIGNAL(clicked()), this, SLOT(Decode()));
}

RSATest::~RSATest()
{

}

void RSATest::CreateKey()
{
	/* 生成公钥 */
	RSA* rsa = RSA_generate_key(2048, RSA_F4, NULL, NULL);
	BIO *bp = BIO_new(BIO_s_file());
	BIO_write_filename(bp, "public.pem");
	PEM_write_bio_RSAPublicKey(bp, rsa);
	BIO_free_all(bp);

	/* 生成私钥 */
	char passwd[] = "1234";
	bp = BIO_new_file("private.pem", "w+");
	PEM_write_bio_RSAPrivateKey(bp, rsa, EVP_des_ede3_ofb(), (unsigned char*)passwd, 4, NULL, NULL);
	BIO_free_all(bp);
	RSA_free(rsa);
}

std::string RSATest::DecryptData(string data)
{
	OpenSSL_add_all_algorithms();

	BIO* bp = BIO_new(BIO_s_file());

	long lRet = BIO_read_filename(bp, "private.pem");

	char passwd[] = "1234";
	RSA* rsaK = PEM_read_bio_RSAPrivateKey(bp, NULL, NULL, passwd);
	if (NULL == rsaK)
	{
		perror("read key file fail!");
	}
	else{
		printf("read success!\n");
	}
	int nLen = RSA_size(rsaK);
	char *pEncode = new char[2048];
	int len = data.length();
	int ret = RSA_private_decrypt(len, (const unsigned char*)data.c_str(), (unsigned char *)pEncode, rsaK, RSA_PKCS1_PADDING);
	std::string strRet;
	if (ret >= 0)
	{
		strRet = std::string(pEncode, ret);
	}

	delete[] pEncode;
	CRYPTO_cleanup_all_ex_data();
	BIO_free_all(bp);
	RSA_free(rsaK);
	return strRet;
}

std::string RSATest::EncryptData(string data)
{
	OpenSSL_add_all_algorithms();
	BIO* bp = BIO_new(BIO_s_file());
	BIO_read_filename(bp, "public.pem");
	RSA* rsaK = PEM_read_bio_RSAPublicKey(bp, NULL, NULL, NULL);
	RSA_print_fp(stdout, rsaK, 0);

	if (NULL == rsaK)
	{
		perror("read key file fail!");
	}
	else
	{
		printf("read success!");
		int nLen = RSA_size(rsaK);
		printf("len:%d\n", nLen);
	}
	int nLen = RSA_size(rsaK);
	char *pEncode = new char[nLen + 1];
	memset(pEncode, 0, nLen + 1);
	int ret = RSA_public_encrypt(data.length(), (const unsigned char*)data.c_str(),
		(unsigned char *)pEncode, rsaK, RSA_PKCS1_PADDING);
	std::string strRet;
	if (ret >= 0)
	{
		strRet = std::string(pEncode, ret);
	}
	delete[] pEncode;
	CRYPTO_cleanup_all_ex_data();
	BIO_free_all(bp);
	RSA_free(rsaK);
	return strRet;

}

void RSATest::Encode()
{
	string strdata = ui.m_edit1->text().toStdString();
	m_strdata = EncryptData(strdata);
	ui.m_edit2->setText(QString::fromStdString(m_strdata));
}

void RSATest::Decode()
{
	string strdata = DecryptData(m_strdata);
	ui.m_edit3->setText(QString::fromStdString(strdata));
}

int RSATest::Base64Encode(const char* pInputBuf, int nInputBufLen, char* pOutputBuf)
{
	int index = 0;
	return index;
}
