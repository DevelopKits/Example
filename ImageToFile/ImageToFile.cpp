// ImageToFile.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "turbojpeg.h"
#include "jpeglib.h"
#include <string>
#include <windows.h>

#include <setjmp.h>

using namespace std;

void write_JPEG_file(char *filename, int image_width, int image_height, int quality, JSAMPLE *image_buffer)
{
	/* �˽ṹ�����JPEGѹ��������ָ�빤���ռ䣨��JPEG�������Ҫ���䣩
	* �����ж�������Ľṹ��������ѹ��/��ѹ�����̣�һ�δ��ڡ�
	* ���ǲο���Ϊ��JPEG���󡱵��κ�һ���ṹ����������Ĺ������ݣ�
	*/
	struct jpeg_compress_struct cinfo;
	/* �˽ṹ���ʾJPEG���������
	* ���ǵ���������,��ΪӦ�ó��򾭳�Ҫ�ṩһ��ר�ŵĴ��������
	* ��ȡ�򵥵ķ�������ʹ�ñ�׼�Ĵ��������
	*/
	struct jpeg_error_mgr jerr;

	FILE *outfile;                /* Ŀ���ļ� */
	JSAMPROW row_pointer[1];      /* ָ��JSAMPLE��ÿһ�еĵ�ַ*/
	int row_stride;               /* ͼ�񻺳����е������п�� */

	/* ����1������ͳ�ʼ��JPEGѹ������* /

	/* ���Ǳ����������ô���������Է���ʼ��ʧ��*/
	cinfo.err = jpeg_std_error(&jerr);
	/* �������ǿ��Գ�ʼ��JPEGѹ������*/
	jpeg_create_compress(&cinfo);


	/* ����2��ָ������Ŀ�ĵأ����磬�ļ���*/
	/* ע�⣺����2��3���԰���һ˳����ɡ�*/
	/* ��������ʹ�ÿ��ṩ�Ĵ��뽫ѹ�����ݷ��͵� stdio����
	* ��Ҳ���Ա�д�Լ��Ĵ�������������顣
	* ʹ�á�b��ѡ����fopen������Ҫ������д�������ļ���
	*/
	if ((outfile = fopen(filename, "wb")) == NULL)
	{
		fprintf(stderr, "can't open %s\n", filename);
		return;
	}
	jpeg_stdio_dest(&cinfo, outfile);

	/* ����3������ѹ������* /
	/* ���������ṩ����ͼ���������
	* cinfo�ṹ���ĸ��ֶα�����д��
	*/
	cinfo.image_width = image_width;       /* ͼ���Ⱥ͸߶ȣ�������Ϊ��λ*/
	cinfo.image_height = image_height;
	cinfo.input_components = 3;            /* ÿ���ص���ɫ����*/
	cinfo.in_color_space = JCS_RGB;        /* ����ͼ�����ɫ�ռ�* /

										   /* ����ʹ�ÿ����������Ĭ��ѹ��������
										   *����������������cinfo.in_color_space���ܵ������������Ĭ��ֵȡ����Դ��ɫ�ռ䡣��
										   */
	jpeg_set_defaults(&cinfo);

	/* ���������������κ�����Ҫ�ķ�Ĭ�ϲ�����
	* ��������ֻ��˵��ʹ�����������������ţ�
	*/
	jpeg_set_quality(&cinfo, quality, TRUE /*limit to baseline-JPEG values */);

	/* ����4������ѹ��*/

	/* TRUEȷ�����ǽ���дһ�������Ľ���JPEG�ļ���
	* ͨ��TRUE��������ǳ�ȷ��������ʲô��
	*/
	jpeg_start_compress(&cinfo, TRUE);

	/* ����5��while��ɨ�� �ȴ�д���ÿ�У�*/
	/* jpeg_write_scanlines��...��; */

	/*��������ʹ�ÿ��״̬����cinfo.next_scanline��Ϊѭ��������*/
	row_stride = image_width * 3; /* ��image_buffer��ÿ�е�JSAMPLE */

	while (cinfo.next_scanline < cinfo.image_height)
	{
		/*jpeg_write_scanlines��Ҫһ��ָ��ɨ���ߵ�ָ�롣
		*���������ֻ��һ��Ԫ�س����������ͨ��һ��ɨ����һ�Σ�������������㡣
		*/
		row_pointer[0] = &image_buffer[cinfo.next_scanline * row_stride];

		(void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	/*����6�����ѹ�� */
	jpeg_finish_compress(&cinfo);
	/*��finish_compress֮�����ǿ��Թر�����ļ���*/
	fclose(outfile);

	/*����7���ͷ�JPEGѹ������* /
	/*����һ����Ҫ�Ĳ��裬��Ϊ�����ͷŴ������ڴ档*/
	jpeg_destroy_compress(&cinfo);

}

int _tmain(int argc, _TCHAR* argv[])
{
	char strExePathTmp[260] = { 0 };
	GetModuleFileNameA(NULL, strExePathTmp, 260);
	string strExePath(strExePathTmp);
	strExePath.resize(strExePath.find_last_of('\\'));
	SetCurrentDirectoryA(strExePath.c_str());

	FILE* fp;
	fp = fopen("gg.rgb", "rb");
	fseek(fp, 0, SEEK_END);
	int nlength = ftell(fp);
	unsigned char*szbuf = new unsigned char[nlength + 1];
	fseek(fp, 0, SEEK_SET);
	fread(szbuf, nlength, 1, fp);
	fclose(fp);
	write_JPEG_file("test.jpg", 300, 243, 1, szbuf);

	delete []szbuf;
	return 0;
}

