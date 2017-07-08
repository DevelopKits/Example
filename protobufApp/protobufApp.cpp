// protobufApp.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

#include <iostream>
#include <fstream>
#include <string>
#include "person.pb.h"

#pragma comment(lib, "libprotobuf.lib")
using namespace std;


int _tmain(int argc, _TCHAR* argv[])
{
	Person person;

	//������д��person.pb�ļ�
	person.set_id(123);
	person.set_name("Bob");
	person.set_email("bob@example.com");

	fstream out("person.pb", ios::out | ios::binary | ios::trunc);
	person.SerializeToOstream(&out);
	out.close();


	//��person.pb�ļ���ȡ����
	fstream in("person.pb", ios::in | ios::binary);
	if (!person.ParseFromIstream(&in)) {
		cerr << "Failed to parse person.pb." << endl;
		exit(1);
	}

	cout << "ID: " << person.id() << endl;
	cout << "name: " << person.name() << endl;
	if (person.has_email()) {
		cout << "e-mail: " << person.email() << endl;
	}

	getchar();

	return 0;
}

