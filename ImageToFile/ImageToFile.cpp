// ImageToFile.cpp : 定义控制台应用程序的入口点。
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
	/* 此结构体包含JPEG压缩参数和指针工作空间（由JPEG库根据需要分配）
	* 可以有多个这样的结构，代表多个压缩/解压缩过程，一次存在。
	* 我们参考作为“JPEG对象”的任何一个结构（及其关联的工作数据）
	*/
	struct jpeg_compress_struct cinfo;
	/* 此结构体表示JPEG错误处理程序
	* 它是单独声明的,因为应用程序经常要提供一个专门的错误处理程序
	* 采取简单的方法，并使用标准的错误处理程序
	*/
	struct jpeg_error_mgr jerr;

	FILE *outfile;                /* 目标文件 */
	JSAMPROW row_pointer[1];      /* 指向JSAMPLE的每一行的地址*/
	int row_stride;               /* 图像缓冲区中的物理行宽度 */

	/* 步骤1：分配和初始化JPEG压缩对象* /

	/* 我们必须首先设置错误处理程序，以防初始化失败*/
	cinfo.err = jpeg_std_error(&jerr);
	/* 现在我们可以初始化JPEG压缩对象。*/
	jpeg_create_compress(&cinfo);


	/* 步骤2：指定数据目的地（例如，文件）*/
	/* 注意：步骤2和3可以按任一顺序完成。*/
	/* 这里我们使用库提供的代码将压缩数据发送到 stdio流。
	* 你也可以编写自己的代码来做别的事情。
	* 使用“b”选项来fopen（）需要它来编写二进制文件。
	*/
	if ((outfile = fopen(filename, "wb")) == NULL)
	{
		fprintf(stderr, "can't open %s\n", filename);
		return;
	}
	jpeg_stdio_dest(&cinfo, outfile);

	/* 步骤3：设置压缩参数* /
	/* 首先我们提供输入图像的描述。
	* cinfo结构的四个字段必须填写：
	*/
	cinfo.image_width = image_width;       /* 图像宽度和高度，以像素为单位*/
	cinfo.image_height = image_height;
	cinfo.input_components = 3;            /* 每像素的颜色分量*/
	cinfo.in_color_space = JCS_RGB;        /* 输入图像的颜色空间* /

										   /* 现在使用库的例程设置默认压缩参数。
										   *（您必须至少设置cinfo.in_color_space才能调用这个，由于默认值取决于源颜色空间。）
										   */
	jpeg_set_defaults(&cinfo);

	/* 现在您可以设置任何您想要的非默认参数。
	* 这里我们只是说明使用质量（量化表）缩放：
	*/
	jpeg_set_quality(&cinfo, quality, TRUE /*limit to baseline-JPEG values */);

	/* 步骤4：启动压缩*/

	/* TRUE确保我们将编写一个完整的交换JPEG文件。
	* 通过TRUE，除非你非常确定你在做什么。
	*/
	jpeg_start_compress(&cinfo, TRUE);

	/* 步骤5：while（扫描 等待写入的每行）*/
	/* jpeg_write_scanlines（...）; */

	/*这里我们使用库的状态变量cinfo.next_scanline作为循环计数器*/
	row_stride = image_width * 3; /* 在image_buffer中每行的JSAMPLE */

	while (cinfo.next_scanline < cinfo.image_height)
	{
		/*jpeg_write_scanlines需要一组指向扫描线的指针。
		*这里的数组只有一个元素长，但你可以通过一次扫描线一次，如果这样更方便。
		*/
		row_pointer[0] = &image_buffer[cinfo.next_scanline * row_stride];

		(void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	/*步骤6：完成压缩 */
	jpeg_finish_compress(&cinfo);
	/*在finish_compress之后，我们可以关闭输出文件。*/
	fclose(outfile);

	/*步骤7：释放JPEG压缩对象* /
	/*这是一个重要的步骤，因为它会释放大量的内存。*/
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

