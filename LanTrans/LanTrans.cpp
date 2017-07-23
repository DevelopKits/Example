// LanTrans.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <string>
#include <windows.h>
#include <stdlib.h>
#include "iconv.h"		

using	 namespace std;
#define MAX_BUF_SIZE 1024

int code_convert(char *from_charset, char *to_charset, char *inbuf, size_t inlen,
	char *outbuf, size_t outlen) {
	iconv_t cd;
	char **pin = &inbuf;
	char **pout = &outbuf;

	cd = iconv_open(to_charset, from_charset);
	if (cd == 0)
		return -1;
	memset(outbuf, 0, outlen);
	if (iconv(cd, pin, &inlen, pout, &outlen) == -1)
		return -1;
	iconv_close(cd);
	*pout = '\0';

	return 0;
}

int utf8_to_gbk(char *inbuf, size_t inlen, char *outbuf, size_t outlen) {
	return code_convert("utf-8", "gbk", inbuf, inlen, outbuf, outlen);
}

int gbk_to_utf8(char *inbuf, size_t inlen, char *outbuf, size_t outlen) {
	return code_convert("gbk", "utf-8", inbuf, inlen, outbuf, outlen);
}

void read_file(char buf[], const int32_t max_buf_size, const char *file_name)
{
	FILE * pFile;
	long lSize;
	size_t result;
	fopen_s(&pFile, file_name, "rb");
	if (pFile == NULL) { fputs("File error\n", stderr); exit(1); }
	// obtain file size:
	fseek(pFile, 0, SEEK_END);
	lSize = ftell(pFile);
	rewind(pFile);
	if (lSize >= max_buf_size){ fputs("file too large\n", stderr); exit(1); }
	result = fread(buf, 1, lSize, pFile);
	if (result != lSize) { fputs("Reading error\n", stderr); exit(3); }
	fclose(pFile);
}

//将gbk编码的str分隔成一个一个的字符，并判断是否是汉字，并输出编码，包括简体和繁体
void GetToken(const char *str)
{
	int32_t i = 0;
	int32_t len = strlen(str);
	short high, low;
	uint32_t code;
	char cstr[3];
	for (; i < len; ++i)
	{
		if (str[i] >= 0 || i == len - 1)
		{
			printf("%c >> no\n", str[i]);   //ASCII字符
		}
		else
		{
			// 计算编码
			high = (short)str[i] + 256;
			low = (short)str[i + 1] + 256;
			code = high * 256 + low;

			//获取字符
			cstr[0] = str[i];
			cstr[1] = str[i + 1];
			cstr[2] = 0;
			i++;

			printf("%s >> 0x%x", cstr, code);
			if ((code >= 0xB0A1 && code <= 0xF7FE) || (code >= 0x8140 && code <= 0xA0FE) || (code >= 0xAA40 && code <= 0xFEA0))
			{
				printf(" yes\n");
			}
			else
			{
				printf(" no\n");
			}
		}
	}
}

int main(int argc, char *argv[])
{
	char strExePathTmp[260] = { 0 };
	GetModuleFileNameA(NULL, strExePathTmp, 260);
	string strExePath(strExePathTmp);
	strExePath.resize(strExePath.find_last_of('\\'));
	SetCurrentDirectoryA(strExePath.c_str());

	char in_buf[MAX_BUF_SIZE] = { 0 }, out_buf[MAX_BUF_SIZE] = { 0 };
	read_file(in_buf, MAX_BUF_SIZE, "chinese_gbk.txt");
	//printf("%s\n", in_buf);
	//GetToken(in_buf);
	read_file(in_buf, MAX_BUF_SIZE, "chinese_utf8.txt");
	//printf("%s\n", in_buf);
	//GetToken(in_buf);
	printf("%s\n", in_buf);
	utf8_to_gbk(in_buf, strlen(in_buf), out_buf, MAX_BUF_SIZE);
	printf("%s\n", out_buf);
	//GetToken(out_buf);
	getchar();
	return 0;
}

