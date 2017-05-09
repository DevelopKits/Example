
#ifndef _SP_STREAM_PARSER_H_
#define _SP_STREAM_PARSER_H_

#define IN  
#define OUT 

#if (defined(WIN32) || defined(WIN64))
#ifdef SP_EXPORTS_DLL
	#define STREAMPARSER_API __declspec(dllexport)
#elif defined(SP_USE_DLL)
	#define STREAMPARSER_API __declspec(dllimport)
#else
	#define STREAMPARSER_API 
#endif
	#define CALLMETHOD __stdcall
#else 
	#define STREAMPARSER_API
	#define CALLMETHOD
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*�ӿڷ���ֵ*/
enum SP_RESULT
{
    SP_SUCCESS = 0,                     /*�ɹ�*/
    SP_ERROR_INVALID_HANDLE,            /*��Ч���*/
    SP_ERROR_FILE_TYPE_NOSUPPORT,       /*�ļ����Ͳ�֧��*/
    SP_ERROR_STREAM_NOSUPPORT,          /*�����Ͳ�֧��*/
    SP_ERROR_THREAD_CREATE_FAILED,      /*�̴߳���ʧ��*/
    SP_ERROR_THREAD_CLOSE_FAILED,       /*�̹߳ر�ʧ��*/
    SP_ERROR_PARAMETER,                 /*��������*/			
    SP_ERROR_INDEX_OUTOFRANGE,	        /*����������Χ*/						
    SP_ERROR_FILE_SIZE_ZERO,			/*�ļ���СΪ0*/
    SP_ERROR_BAD_FORMATTED,     	    /*�ļ���ʽ����*/
    SP_ERROR_FILE_READ,	                /*��ȡ�ļ�ʧ��*/
    SP_ERROR_FILE_OPEN,                 /*���ļ�ʧ��*/
    SP_ERROR_BUFFER_OVERFLOW,           /*�ڲ����������*/
    SP_ERROR_SYSTEM_OUT_OF_MEMORY,      /*ϵͳ�ڴ治��*/
    SP_ERROR_LIST_EMPTY,                /*�б�Ϊ��*/
	SP_ERROR_UNGET_STREAM_TYPE			/*δ��ȡ����������*/
};

/*֡����*/
enum 
{
	SP_FRAME_TYPE_UNKNOWN = 0,			/*֡���Ͳ���֪*/
	SP_FRAME_TYPE_VIDEO,				/*֡��������Ƶ֡*/
	SP_FRAME_TYPE_AUDIO,				/*֡��������Ƶ֡*/
	SP_FRAME_TYPE_DATA					/*֡����������֡*/
};

/*֡������*/
enum
{
	SP_FRAME_SUB_TYPE_DATA_INVALID = -1,			/*������Ч*/
	SP_FRAME_SUB_TYPE_VIDEO_I_FRAME = 0 ,			/*I֡*/
	SP_FRAME_SUB_TYPE_VIDEO_P_FRAME,				/*P֡*/
	SP_FRAME_SUB_TYPE_VIDEO_B_FRAME,				/*B֡*/
	SP_FRAME_SUB_TYPE_VIDEO_S_FRAME,				/*S֡*/
	SP_FRAME_SUB_TYPE_WATERMARK_TEXT,				/*ˮӡ����ΪTEXT����*/
	SP_FRAME_SUB_TYPE_WATERMARK_JPEG,				/*ˮӡ����ΪJPEG����*/
	SP_FRAME_SUB_TYPE_WATERMARK_BMP,				/*ˮӡ����ΪBMP����*/
	SP_FRAME_SUB_TYPE_DATA_INTL,					/*���ܷ���֡*/
	SP_FRAME_SUB_TYPE_VIDEO_JPEG_FRAME,             /*JPEG ֡*/
	SP_FRAME_SUB_TYPE_DATA_ITS,				        /*its��Ϣ֡*/
	SP_FRAME_SUB_TYPE_DATA_GPS,					    /*gps*/
	SP_FRAME_SUB_TYPE_DATA_INTLEX,                  /*��չ���ܷ���֡*/
	SP_FRAME_SUB_TYPE_DATA_MOTION_FULL,				/*ȫ������*/
	SP_FRAME_SUB_TYPE_DATA_MOTION_CARD,				/*ȫ������(�忨)*/
	SP_FRAME_SUB_TYPE_DATA_LIGHT,					/*����*/
	SP_FRAME_SUB_TYPE_DATA_RAW,						/*ԭʼ����*/
	SP_FRAME_SUB_TYPE_DATA_I_INDEX = 128,           /*I֡����֡*/
	SP_FRAME_SUB_TYPE_DATA_MOTION,                  /*��������֡*/
	SP_FRAME_SUB_TYPE_DATA_LABEL,                   /*��ǩ֡*/
	SP_FRAME_SUB_TYPE_DATA_SCREEN_CAPTION,          /*��Ļ*/    
	SP_FRAME_SUB_TYPE_DATA_EVENT,                   /*�¼�֡*/
	SP_FRAME_SUB_TYPE_DATA_FILEHEAD = 255,          /*�ļ�ͷ֡*/
	SP_FRAME_SUB_TYPE_DATA_UNKOWN,                  /*δ֪����*/
	/*lint -save -e849*/
	SP_FRAME_SUB_TYPE_DATA_SDP = 12                 /*SDP��Ϣ������*/
	/*lint -restore*/
};						
/*lint -save -e849*/
/*��������*/
enum SP_ENCODE_VIDEO_TYPE
{
	SP_ENCODE_VIDEO_UNKNOWN = 0,		/*��Ƶ�����ʽ����֪*/
	SP_ENCODE_VIDEO_MPEG4 ,			    /*��Ƶ�����ʽ��MPEG4*/
	SP_ENCODE_VIDEO_HI_H264,			/*��Ƶ�����ʽ�Ǻ�˼H264*/
	SP_ENCODE_VIDEO_JPEG,				/*��Ƶ�����ʽ�Ǳ�׼JPEG*/
	SP_ENCODE_VIDEO_DH_H264,			/*��Ƶ�����ʽ�Ǵ�����H264*/
	SP_ENCODE_VIDEO_JPEG2000 = 6,		/*��Ƶ�����ʽ�Ǳ�׼JPEG2000*/
	SP_ENCODE_VIDEO_AVS = 7,			/*��Ƶ�����ʽ�Ǳ�׼AVS*/
	SP_ENCODE_VIDEO_STD_H264 = 8,		/*��Ƶ�����ʽ�Ǳ�׼H264*/
	SP_ENCODE_VIDEO_MPEG2 = 9,          /*��Ƶ�����ʽ��MPEG2*/	
	SP_ENCODE_VIDEO_VNC = 10,         	/*��Ƶ�����ʽ��VNC*/	
	SP_ENCODE_VIDEO_SVAC = 11,          /*��Ƶ�����ʽ��SVAC*/	
	SP_ENCODE_VIDEO_DH_H265 = 12,		/*��Ƶ�����ʽ��H265*/	

	//��Ƶ����������չ����
	SP_ENCODE_VIDEO_H263 = 35,      /*��Ƶ�����ʽ��H263*/
	SP_ENCODE_VIDEO_PACKET,         /*��Ƶ��*/
	SP_ENCODE_VIDEO_MSMPEG4V1,		/*��Ƶ�����ʽ��MS MPEG4 V1*/
	SP_ENCODE_VIDEO_MSMPEG4V2,		/*��Ƶ�����ʽ��MS MPEG4 V2*/
	SP_ENCODE_VIDEO_MSMPEG4V3,		/*��Ƶ�����ʽ��MS MPEG4 V3*/
	SP_ENCODE_VIDEO_WMV1,			/*��Ƶ�����ʽ��MS MPEG4 V4*/
	SP_ENCODE_VIDEO_WMV2,			/*��Ƶ�����ʽ��MS MPEG4 V5*/
	
	//˽�б����ʽ
	SP_ENCODE_VIDEO_HIK_H264 = 0x81,		//����˽��H264����
	SP_ENCODE_VIDEO_HIK_HKH4 = 0x82,
};

enum SP_ENCODE_AUDIO_TYPE
{
	SP_ENCODE_AUDIO_UNKNOWN = 0,
	SP_ENCODE_AUDIO_PCM = 7,			/*��Ƶ�����ʽ��PCM8*/
	SP_ENCODE_AUDIO_G729,				/*��Ƶ�����ʽ��G729*/
	SP_ENCODE_AUDIO_IMA,				/*��Ƶ�����ʽ��IMA*/
	SP_ENCODE_PCM_MULAW,				/*��Ƶ�����ʽ��PCM MULAW*/
	SP_ENCODE_AUDIO_G721,				/*��Ƶ�����ʽ��G721*/
	SP_ENCODE_PCM8_VWIS,				/*��Ƶ�����ʽ��PCM8_VWIS*/
	SP_ENCODE_MS_ADPCM,				    /*��Ƶ�����ʽ��MS_ADPCM*/
	SP_ENCODE_AUDIO_G711A,				/*��Ƶ�����ʽ��G711A*/
	SP_ENCODE_AUDIO_AMR,				/*��Ƶ�����ʽ��AMR*/
	SP_ENCODE_AUDIO_PCM16,				/*��Ƶ�����ʽ��PCM16*/
	SP_ENCODE_AUDIO_G711U = 22,		    /*��Ƶ�����ʽ��G711U*/
	SP_ENCODE_AUDIO_G723 = 25,			/*��Ƶ�����ʽ��G723*/
	SP_ENCODE_AUDIO_AAC,			    /*��Ƶ�����ʽ��AAC*/
	SP_ENCODE_AUDIO_G726_40,            /*40kbps,����32/24/16*/
	SP_ENCODE_AUDIO_G726_32,            /*�ֱ��ʾ�����ʵĲ�ͬ*/
	SP_ENCODE_AUDIO_G726_24,            /*�����8k�����ʵ�*/
	SP_ENCODE_AUDIO_G726_16,            /*�����*/
	SP_ENCODE_AUDIO_MP2,                /*��Ƶ�����ʽ��mp2*/
	SP_ENCODE_AUDIO_OGG,                /*��Ƶ�����ʽ��ogg vorbis*/
	SP_ENCODE_AUDIO_MP3,           /*��Ƶ�����ʽ��mp3*/
	SP_ENCODE_AUDIO_TALK,          /*��Ƶ�����ʽ�ǶԽ�*/
};
/*lint -restore*/	
/*������װ��ʽ*/
enum 
{
	SP_STREAM_TYPE_UNKNOWN = 0, /*δ֪����*/
	SP_STREAM_TYPE_MPEG4,       /*MPEG4*/		
	SP_STREAM_TYPE_DHPT =3,	    /*����������DHPT*/
	SP_STREAM_TYPE_NEW,         /*����������NEW*/		    
	SP_STREAM_TYPE_HB,          /*����������HB*/			
	SP_STREAM_TYPE_AUDIO,       /*��Ƶ��*/		
	SP_STREAM_TYPE_PS,          /*MPEG-2��PS*/
	SP_STREAM_TYPE_DHSTD,       /*�����µı�׼����*/
	SP_STREAM_TYPE_ASF,         /*ASF*/
	SP_STREAM_TYPE_3GPP,        /*3GP*/
	SP_STREAM_TYPE_RAW,	        /*����������������*/	    
	SP_STREAM_TYPE_TS,          /*MPEG-2��TS*/
	SP_STREAM_TYPE_SVC,         /*svc*/
	SP_STREAM_TYPE_AVI,         /*AVI*/
	SP_STREAM_TYPE_MP4,         /*MP4*/
	SP_STREAM_TYPE_CGI,         /*CGI*/
	SP_STREAM_TYPE_WAV,			/*WAV��Ƶ*/
	SP_STREAM_TYPE_FLV,          /*FLV*/

	SP_STREAM_TYPE_MKV,          /*mkv*/
	SP_STREAM_TYPE_RTP,			/*RTP*/
	SP_STREAM_TYPE_RAW_MPEG4,	/*MPEG4������*/
	SP_STREAM_TYPE_RAW_H264,	/*H264������*/
	SP_STREAM_TYPE_RAW_H265,	/*H265������*/
	SP_STREAM_TYPE_WMV,			/*H265������*/

	// �������������ͣ���0x10000001��ʼ����ͨ�ó���������
	SP_STREAM_TYPE_HENGYI = 0x10000001,	//����
	SP_STREAM_TYPE_HUANGHE,		//�ƺ�
	SP_STREAM_TYPE_LANGCHI,		//�ʳ�
	SP_STREAM_TYPE_TDWY,		//���ΰҵ
	SP_STREAM_TYPE_DALI,		//����
	SP_STREAM_TYPE_LVFF,		//LVFF�ļ�ͷ��δ֪����
	SP_STREAM_TYPE_H3C,			//����
	SP_STREAM_TYPE_FENGDA,		//���¼��
	SP_STREAM_TYPE_MDVRX,		//�ļ�ͷMDVRX��δ֪����
	SP_STREAM_TYPE_PU8000,		//�ļ�ͷpu8000��δ֪����
	SP_STREAM_TYPE_DVR,			//��׺��dvr��δ֪����
	SP_STREAM_TYPE_IFV,			//��׺��ifv��δ֪����
	SP_STREAM_TYPE_264DV,		//�ļ�ͷ264dv��δ֪����
	SP_STREAM_TYPE_ZWSJ,		//��ά����
	SP_STREAM_TYPE_SANLI,		//������
	SP_STREAM_TYPE_HIK_PRIVATE,	//����˽������
	SP_STREAM_TYPE_HIK_PS,		//����PS��
};

/*�����־λ*/
enum
{
	SP_ERROR_FLAGS_NOERROR = 0,		    /*����У������*/
	SP_ERROR_FLAGS_TIMESTAND,			/*ʱ�������*/
	SP_ERROR_FLAGS_LENGTH,				/*���ȳ���*/
	SP_ERROR_FLAGS_HEAD_VERIFY,		    /*֡ͷ�ڲ�����У��*/
	SP_ERROR_FLAGS_DATA_VERIFY,		    /*����У��ʧ��*/
	SP_ERROR_FLAGS_LOST_HEADER,		    /*���ݶ�ʧ֡ͷ*/
	SP_ERROR_FLAGS_UNKNOWN,			    /*����֪����*/
	SP_ERROR_FLAGS_LOSTFRAME,           /*��֡*/
	SP_ERROR_FLAGS_WATERMARK,           /*ˮӡУ�����*/
	SP_ERROR_FLAGS_CONTEXT,             /*�����Ĵ���*/
	SP_ERROR_FLAGS_NOSUPPORT,           /*��֧�ֵ�����*/
    SP_ERROR_FLAGS_FRAME_HALF_BAKED     /*֡������*/
};

/*�⽻���־*/
enum
{
	SP_DEINTERLACE_PAIR = 0, /*���������*/
	SP_DEINTERLACE_SINGLE,   /*��������*/
	SP_DEINTERLACE_NONE      /*�޽⽻��*/
};

/*�������ͱ�־λ*/
enum
{
	SP_INDEX_BIT_FLAG_VIDEO_I = 1,         /*��ƵI֡*/
	SP_INDEX_BIT_FLAG_VIDEO_P = 2,         /*��ƵP֡*/
	SP_INDEX_BIT_FLAG_VIDEO_B = 4,         /*��ƵB֡*/
	SP_INDEX_BIT_FLAG_AUDIO = 8,           /*��Ƶ֡*/
	SP_INDEX_BIT_FLAG_DATA = 16,           /*����֡*/
	SP_INDEX_BIT_FLAG_ALL = 0xffffffff     /*����֡*/
};

/*ʱ����Ϣ*/
typedef struct
{
	int nYear;			/*��*/
	int nMonth;			/*��*/
	int nDay;			/*��*/
	int nHour;			/*Сʱ*/
	int nMinute;		/*����*/
	int nSecond;		/*��*/
	int nMilliSecond;	/*����*/
} SP_TIME;

/*͸�����*/
typedef struct
{
    int bIsExist;          /*�Ƿ���ڴ˽ṹ 0:������ 1:����*/
    int nMode;             /*͸��ģʽ 1:�ر� 2:�Զ� 3:�ֶ� ����:����*/
    int nIntension;        /*͸��ǿ��0-2*/
    int nAirLightMode;     /*������ģʽ1:�Զ� 2: �ֶ� ����:����*/
    int nAirLightIntesion; /*������ǿ��0-15*/
}SP_FOG_THROUGH;

/*��������*/
enum 
{
    SP_COMPANY_TYPE_HIK = 1,                    /*��������*/
    SP_COMPANY_TYPE_HANBANG,                    /*��������*/
    SP_COMPANY_TYPE_YUSHI,                      /*��������*/
    SP_COMPANY_TYPE_INTERNATIONAL_DEVICE_PS,    /*�����豸PS����*/
    SP_COMPANY_TYPE_XINCHAN,                    /*�Ų�����*/
    SP_COMPANY_TYPE_LIYUAN,                     /*��Ԫ����*/
    SP_COMPANY_TYPE_BIT,                        /*��������*/
    SP_COMPANY_TYPE_OLD_DH,                     /*�ϴ�����*/
    SP_COMPANY_TYPE_STD_TS,                     /*��׼ts��*/
    SP_COMPANY_TYPE_TDYG,                       /*�������*/
    SP_COMPANY_TYPE_ANXUNSHI,                   /*��Ѹʿ*/
    SP_COMPANY_TYPE_DFWL,                       /*��������*/
    SP_COMPANY_TYPE_JUFENG,                     /*�޷�*/
    SP_COMPANY_TYPE_KEDA,                       /*�ƴ�*/
    SP_COMPANY_TYPE_LG,                         /*LG*/
    SP_COMPANY_TYPE_MAISHI,                     /*����*/
    SP_COMPANY_TYPE_TONGZUN,                    /*ͬ��*/
    SP_COMPANY_TYPE_VIVO,                       /*vivo*/
    SP_COMPANY_TYPE_WEIHAO,                     /*ΰ�*/
    SP_COMPANY_TYPE_XINGWANG,                   /*����*/
    SP_COMPANY_TYPE_BJWS,                       /*��������*/
    SP_COMPANY_TYPE_XJSX,                       /*�Ƚ���Ѷ*/
    SP_COMPANY_TYPE_SZXY,                       /*��������*/
    SP_COMPANY_TYPE_ZSYH,                       /*��ʢ�滪*/
    SP_COMPANY_TYPE_ZXLW,                       /*������ά*/
    SP_COMPANY_TYPE_ZXTX                        /*����ͨѶ*/
};

/*��������*/
enum
{
    SP_ENCRYPT_UNKOWN = 0,
    SP_ENCRYPT_AES,
    SP_ENCRYPT_DES,
    SP_ENCRYPT_3DES
};

/*����ǩ��ժҪ�㷨����*/
enum
{
	SP_DIGEST_ALGORITHM_UNKOWN = 0,
	SP_DIGEST_ALGORITHM_SHA1,
};

/*����ǩ�������㷨����*/
enum
{
	SP_DIGTAL_SIGNATURE_ENCRYPT_UNKOWN = 0,
	SP_DIGTAL_SIGNATURE_ENCRYPT_RSA,

};

typedef struct  
{
    int nVideoEncodeType;         /*��Ƶ��������*/
    int nFrameRate;               /*֡��*/
    int nWidth;                   /*��*/
    int nHeight;                  /*��*/
    int nSPS_PPSLen;              /*sps��pps��Ϣ��*/
    unsigned char* pSPS_PPSData;  /*sps��pps��Ϣ*/
    int nAudioEncodeType;         /*��Ƶ��������*/
    int nSamplerPerSec;           /*��Ƶ������*/
    int bitsPerSample;            /*��Ƶ����λ��*/
}SP_SDP_INFO;

typedef struct
{
	unsigned char chType;		// 1:���ֽڣ��޷�����������������
	unsigned char chLength;		// type�й涨�Ĳ�����Ԫ����
	unsigned char chValue;		// ������
	unsigned char nReserved;
}SP_DATA_CUSTOM_CHANGE;

/*֡��Ϣ 256�ֽ�*/
typedef struct
{
	/*����*/
	int				frameType;			/*֡����*/
	int				frameSubType;		/*֡������*/
	int				frameEncodeType;	/*֡��������*/
	int				streamType;			/*��������*/

	/*����*/
	unsigned char*	streamPointer;		/*ָ����������,ȥ��֡ͷ,NULL��ʾ��Ч����*/
	int				streamLen;			/*��������(������֡ͷ��β)*/
	unsigned char*	framePointer;		/*ָ��֡ͷ,NULL��ʾ��Ч����*/
	int				frameLen;			/*֡����(����֡ͷ��֡�塢֡β)*/

	/*ʱ��*/
	SP_TIME			frameTime;			/*ʱ����Ϣ*/
	int				timeStamp;			/*ʱ���*/

	/*���*/
	int				frameSeq;			/*֡���*/
	
	/*��Ƶ���ԣ��ؼ�֡����*/
	int				frameRate;			/*֡��*/
	int				width;				/*��*/
	int				height;				/*��*/
	int				deinterlace;		/*�⽻��*/
	int				mediaFlag;			/*�����ͱ�ǣ�h264������(0����������2����˼����)*/
	
	/*��Ƶ����*/
	int				samplesPerSec;		/*����Ƶ��*/
	int				bitsPerSample;		/*����λ��*/
	int				channels;			/*������*/

	/*�����־*/
	int				errorFlags;			/*�������־λ*/

    SP_FOG_THROUGH  fogThrough;         /*͸�����*/

    unsigned char   allLevels;          /*svc�ܲ���*/
    unsigned char   levelOrder;         /*��ǰ֡���ڵڼ���*/

    unsigned short  companyType;        /*��������*/
    unsigned short  tpStreamHeaderLen;  /*���������ļ�ͷ��Ϣ���ȣ�tp:third party*/

    /*֧��˫��Ƶ����*/
    unsigned char   totalChannels;      /*ͨ����*/
    unsigned char   curChannel;         /*ͨ����*/

    int             nEncryptType;       /*��������*/
    int             nEncryptLen;        /*���ܳ���*/

	SP_DATA_CUSTOM_CHANGE customChangeInfo; /*�������ݱ������޸ĵ�����������û���������*/
	
	/*����֡��֧��*/
	unsigned short rateValue;			/*֡�ʷ���*/
	unsigned short rateDenominator;		/*֡�ʷ�ĸ*/

	/*����֧��*/
	unsigned short n_slice;
	unsigned short m_slice;
	unsigned short srcPicWidth;
	unsigned short srcPicHeight;
	unsigned short sliceInfoOffset;		/*�����framePointer*/
	unsigned short sliceInfoLength;		/*�ֿ�ͼ����Ϣ�ĳ���*/

	/*������Ϣ, ע�⣺���۵İ�װ��ʽ(0��ʾ���������豸��Ϊ0�������ֶζ���Ч*/
	unsigned char fishEyeGain;			/*���棬ȡֵ0-100*/
	unsigned char fishEyeDenoiseLevel;	/*����ȼ���ȡֵ0-100*/
	unsigned char fishInstallStyle;		/*���۰�װ��ʽ��0�������� 1����װ 2����װ 3����װ*/
	
	unsigned char fishEyeCorrectMode;	/*���۽���ģʽ
										1:"Original"ԭʼͼ��ģʽ
										2:"Config"����ģʽ
										3:"Panorama"ȫ��ģʽ
										4:"DoublePanorama"˫ȫ��ģʽ
										5:"OriginalPlusThreeEPtsRegion"1+3ģʽ
										6:"Single"������EPtsģʽ
										7:"FourEPtsRegion"4����ģʽ
										8:"Normal"Narmalģʽ
										*/
	unsigned short fishEyeCircleX;		/*����Բ������X*/
	unsigned short fishEyeCircleY;		/*����Բ������Y*/
	unsigned short fishEyeRadius;		/*���۰뾶*/
	unsigned char encryptOffset;//0x95
	unsigned char encryptReserved;

	int pictrueStructure;

	/*��Ƶ��߱���0x9B*/
	unsigned char ratioWidth;//0x9b /*����ǰ��߱���*/
	unsigned char ratioHeigth;
	unsigned char ratioEncodeWidth;/*������߱���*/
	unsigned char ratioEncodeHeight;
 
	unsigned int digtalSignatureSamplenlen; //0x9C

	unsigned short digtalSignatureEncdeclen;
	unsigned char digtalSignatureDigestType;
	unsigned char digtalSignatureEncryptType;

	unsigned char* digtalSignatureEncdecDataPtr;
	
	unsigned char  rotationAngle;  /*������ת�Ƕ�*/

	/*��չ*/
	unsigned char	reserved[63];		/*�����ֽ�*/
} SP_FRAME_INFO;

#if (defined(WIN32) || defined(WIN64))
typedef __int64 SPint64;
#else /*linux*/
typedef long long SPint64;
#endif

/*�ļ���ʽ��֡��Ϣ*/
typedef	struct
{
	/*����ƫ����*/
	SPint64 streamOffset; // ���������ݵ�ƫ����(ȥ��֡ͷ)
	SPint64 frameOffset;  // ֡ͷ��ƫ����

	/*����չ*/
	unsigned char	reserved[64];	/*�����ֽ�*/
}SP_INDEX_INFO;

/********************************************************************
*	Funcname: 	    	fFileIndex
*	Purpose:	        �ļ������ص�����
*   InputParam:         handle:	���
*                       frameInfo: ֡��Ϣ
*                       indexInfo: ������Ϣ
*                       process: ����������,�����ļ���С����İٷ���
*						userData: �û��Զ�������
*   OutputParam:        ��
*   Return:             0:���óɹ�(ֻ����0)    
*********************************************************************/
typedef int (CALLMETHOD *fFileIndex)(IN void* handle,				
									 IN SP_FRAME_INFO* frameInfo,  
									 IN SP_INDEX_INFO* indexInfo,	
									 IN int process,				
									 IN void* userData);			

/********************************************************************
*	Funcname: 	    	SP_LoadLibrary
*	Purpose:	        ���ض�̬���ӿ�֮�������ȵ��ô˺�����ʼ����Դ
*   InputParam:         ��
*   OutputParam:        ��
*   Return:             0:���óɹ�
*                       ����ֵ��ʧ�ܣ�ͨ��SP_GetLastError��ȡ������       
*********************************************************************/
STREAMPARSER_API SP_RESULT CALLMETHOD SP_LoadLibrary(void);

 /********************************************************************
 *	Funcname: 	    	SP_ReleaseLibrary
 *	Purpose:	        �ͷ����ӿ���Դ�������ͷŶ�̬���ӿ�֮ǰ���ô˺����ͷ��ڲ���Դ
 *  InputParam:         ��
 *  OutputParam:        ��
 *  Return:             0:���óɹ�
 *                      ����ֵ��ʧ�ܣ�ͨ��SP_GetLastError��ȡ������       
*********************************************************************/
STREAMPARSER_API SP_RESULT CALLMETHOD SP_ReleaseLibrary(void);

 /********************************************************************
 *	Funcname: 	    	SP_CreateStreamParser
 *	Purpose:	        ������������
 *  InputParam:         nBufferSize: ��Ҫ���ٵĻ�������С
 *  OutputParam:        ��
 *  Return:             NULL: ������������ʧ��
 *                      ����ֵ�������������   
*********************************************************************/
STREAMPARSER_API void* CALLMETHOD SP_CreateStreamParser(IN int nBufferSize);

 /********************************************************************
 *	Funcname: 	    	SP_ParseData
 *	Purpose:	        ����������,��ͬ�����з���
 *  InputParam:         handle:	ͨ��SP_CreateStreamParser���صľ��
 *		                stream:	�����������ַ
 *		                length:	����������
 *  OutputParam:        ��
 *  Return:             0:���óɹ�
 *                      ����ֵ��ʧ�ܣ�ͨ��SP_GetLastError��ȡ������    
*********************************************************************/
STREAMPARSER_API SP_RESULT CALLMETHOD SP_ParseData(IN void* handle, IN unsigned char* stream, IN int length);

/********************************************************************
 *	Funcname: 	    	SP_GetOneFrame
 *	Purpose:	        ͬ����ȡһ֡��Ϣ,��������ֱ��ʧ��
 *  InputParam:         handle:	ͨ��SP_CreateStreamParser���صľ��
 *		                frameInfo: �ⲿSP_FRAME_INFO��һ���ṹ��ַ��
 *  OutputParam:        ��
 *  Return:             0:���óɹ�
 *                      ����ֵ��ʧ�ܣ�ͨ��SP_GetLastError��ȡ������    
*********************************************************************/
STREAMPARSER_API SP_RESULT CALLMETHOD SP_GetOneFrame(IN void* handle, IN SP_FRAME_INFO* frameInfo);

#ifndef STREAM_PARSER_FOR_RELEASE
/********************************************************************
 *	Funcname: 	    	SP_GetRemainData
 *	Purpose:	        ��ȡ�����������в�������
 *  InputParam:         handle:	ͨ��SP_CreateStreamParser���صľ��
 *  OutputParam:        dataPointer: �������ݵĵ�ַ,�ڴ�Ϊ�����������ڲ��ķ�����ڴ�
 *		                sizePointer: ʣ�����ݵĳ���
 *  Return:             0:���óɹ�
 *                      ����ֵ��ʧ�ܣ�ͨ��SP_GetLastError��ȡ������    
*********************************************************************/
STREAMPARSER_API SP_RESULT CALLMETHOD SP_GetRemainData(IN void* handle, OUT unsigned char** dataPointer, OUT int* sizePointer);

/********************************************************************
 *	Funcname: 	    	SP_ClearBuffer
 *	Purpose:	        ��������������ڲ��Ļ���
 *  InputParam:         handle:	ͨ��SP_CreateStreamParser���صľ��
 *  OutputParam:        ��
 *  Return:             0:���óɹ�
 *                      ����ֵ��ʧ�ܣ�ͨ��SP_GetLastError��ȡ������    
*********************************************************************/
STREAMPARSER_API SP_RESULT CALLMETHOD SP_ClearBuffer(IN void* handle);

/********************************************************************
 *	Funcname: 	    	SP_CreateFileParser
 *	Purpose:	        �����ļ�������
 *  InputParam:         file: �ļ���
 *		                fileIndex: ������Ϣ�ص�
 *  OutputParam:        ��
 *  Return:             NULL: ��ʾʧ��
 *                      ����: �ļ����������   
*********************************************************************/
STREAMPARSER_API void* CALLMETHOD SP_CreateFileParser(IN char* file, IN fFileIndex fileIndex, IN void* userData);

/********************************************************************
 *	Funcname: 	    	SP_CreateSliceParser
 *	Purpose:	        �����ļ�Ƭ�η�����
 *  InputParam:         file: �ļ���
 *		                fileIndex: ������Ϣ�ص�
 *						sliceoffset: Ƭ�����ļ��е�ƫ��
 *						slicesize: Ƭ�γ���
 *  OutputParam:        ��
 *  Return:             NULL: ��ʾʧ��
 *                      ����: �ļ����������   
*********************************************************************/
STREAMPARSER_API void* CALLMETHOD SP_CreateSliceParser(IN char* file, IN fFileIndex fileIndex, IN void* userData, SPint64 sliceoffset, SPint64 slicesize);

/********************************************************************
 *	Funcname: 	    	SP_GetProcess
 *	Purpose:	        ��ȡ�ļ�������Ϣ������Ϣ
 *  InputParam:         handle:	ͨ��SP_CreateFileParser���صľ��               
 *  OutputParam:        process: ���������İٷֱ�
 *  Return:             0:���óɹ�
 *                      ����ֵ��ʧ�ܣ�ͨ��SP_GetLastError��ȡ������     
*********************************************************************/
STREAMPARSER_API SP_RESULT CALLMETHOD SP_GetProcess(IN void* handle, OUT int* process);

/********************************************************************
 *	Funcname: 	    	SP_GetOneIndex
 *	Purpose:	        �õ�ָ��λ�õ�֡��Ϣ��֡ƫ����Ϣ
 *  InputParam:         handle:	ͨ��SP_CreateFileParser���صľ��    
 *                      index: ָ������
 *  OutputParam:        indexInfo: ֡ƫ����Ϣ
 *		                frameInfo: ֡��Ϣ  
 *  Return:             0:���óɹ�
 *                      ����ֵ��ʧ�ܣ�ͨ��SP_GetLastError��ȡ������     
*********************************************************************/
STREAMPARSER_API SP_RESULT CALLMETHOD SP_GetOneIndex(IN void* handle, IN int index, OUT SP_INDEX_INFO* indexInfo, OUT SP_FRAME_INFO* frameInfo);

/********************************************************************
 *	Funcname: 	    	SP_GetIndexCount
 *	Purpose:	        �����ļ���������
 *  InputParam:         handle:	ͨ��SP_CreateFileParser���صľ��
 *		                flags: �������ͱ��ر�־λ���μ�SP_INDEX_BIT_FLAG_xxx
 *  OutputParam:        indexCount: ��������
 *  Return:             0:���óɹ�
 *                      ����ֵ��ʧ�ܣ�ͨ��SP_GetLastError��ȡ������     
*********************************************************************/
STREAMPARSER_API SP_RESULT CALLMETHOD SP_GetIndexCount(IN void* handle, IN int flags, OUT int* indexCount);

/********************************************************************
 *	Funcname: 	    	SP_GetAllIndex
 *	Purpose:	        һ�λ�ȡ����������Ϣ
 *  InputParam:         handle:	ͨ��SP_CreateFileParser���صľ��
 *		                flags: �������ͱ��ر�־λ,�μ�SP_INDEX_BIT_FLAG_xxx
 *		                structCount: ��Ҫ��ȡ�Ľṹ����,indexInfo��frameInfo�����ΪNULL,���ߵĸ�����Ҫһ��.
 *  OutputParam:        indexCount: ʵ�ʷ��ص���������
 *		                indexInfo: ����������Ϣ,����NULL��ʾ����Ҫ���ش���Ϣ
 *		                frameInfo: ����֡��Ϣ,����NULL��ʾ����Ҫ���ش���Ϣ
 *  Return:             0:���óɹ�
 *                      ����ֵ��ʧ�ܣ�ͨ��SP_GetLastError��ȡ������     
*********************************************************************/
STREAMPARSER_API SP_RESULT CALLMETHOD SP_GetAllIndex(IN void* handle, IN int flags, OUT SP_INDEX_INFO* indexInfo, OUT SP_FRAME_INFO* frameInfo, IN int structCount, OUT int* indexCount);

/********************************************************************
 *	Funcname: 	    	SP_GetFrameByIndex
 *	Purpose:	        ͨ�������õ�SP_FRAME_INFO�ṹ����Ϣ��
 *  InputParam:         handle:	ͨ��SP_CreateStreamParser���صľ��
 *		                indexInfo: ������Ϣ�ṹ��    
 *  OutputParam:        frameInfo: �ⲿSP_FRAME_INFO��һ���ṹ��ַ
 *  Return:             0:���óɹ�
 *                      ����ֵ��ʧ�ܣ�ͨ��SP_GetLastError��ȡ������     
*********************************************************************/
STREAMPARSER_API SP_RESULT CALLMETHOD SP_GetFrameByIndex(IN void* handle, IN SP_INDEX_INFO* indexInfo, OUT SP_FRAME_INFO* frameInfo);

/********************************************************************
 *	Funcname: 	    	SP_StreamEncryptKey
 *	Purpose:	        ����ʵʱ��������Կ
 *  InputParam:         handle: ͨ��SP_CreateStreamParser��SP_CreateFileParser���صľ����
 *						type : ��Կ���� ����SP_ENCRYPT_AES
 *						key����Կ����
 *						keylen����Կ����
 *  OutputParam:        ��
 *  Return:             0:���óɹ�
 *                      ����ֵ��ʧ�ܣ�
*********************************************************************/
STREAMPARSER_API SP_RESULT CALLMETHOD SP_StreamEncryptKey(IN void* handle,IN unsigned int type, IN unsigned char* key, IN int keylen);

/********************************************************************
 *	Funcname: 	    	SP_FileEncryptKey
 *	Purpose:	        �����ļ�������Կ
 *  InputParam:         handle: ͨ��SP_CreateStreamParser��SP_CreateFileParser���صľ����
 *						type : ��Կ���� ����SP_ENCRYPT_AES
 *						key����Կ����
 *						keylen����Կ����
 *  OutputParam:        ��
 *  Return:             0:���óɹ�
 *                      ����ֵ��ʧ�ܣ�
*********************************************************************/
STREAMPARSER_API SP_RESULT CALLMETHOD SP_FileEncryptKey(IN void* handle,IN unsigned int type, IN unsigned char* key, IN int keylen);

#endif

/********************************************************************
 *	Funcname: 	    	SP_Destroy
 *	Purpose:	        ��������������
 *  InputParam:         handle: ͨ��SP_CreateStreamParser��SP_CreateFileParser���صľ����
 *  OutputParam:        ��
 *  Return:             0:���óɹ�
 *                      ����ֵ��ʧ�ܣ�ͨ��SP_GetLastError��ȡ������     
*********************************************************************/
STREAMPARSER_API SP_RESULT CALLMETHOD SP_Destroy(IN void* handle);

/********************************************************************
 *	Funcname: 	    	SP_GetLastError
 *	Purpose:	        ������������������
 *  InputParam:         handle: ͨ��SP_CreateStreamParser��SP_CreateFileParser���صľ����
 *  OutputParam:        ��
 *  Return:             0:���óɹ�
 *                      ����ֵ��ʧ�ܣ�ͨ��SP_GetLastError��ȡ������   ֵ  
*********************************************************************/
STREAMPARSER_API SP_RESULT CALLMETHOD SP_GetLastError(IN void* handle);

#ifndef STREAM_PARSER_FOR_RELEASE
/********************************************************************
 *	Funcname: 	    	SP_ErrorToString
 *	Purpose:	        ��������ת��Ϊ�ַ���
 *  InputParam:         error: ������                     
 *  OutputParam:        text: ָ�򷵻��ַ���
 *  Return:             ��SP_RESULT  
*********************************************************************/
STREAMPARSER_API SP_RESULT CALLMETHOD SP_ErrorToString(IN int error, OUT char** text);
#endif

#ifdef __cplusplus
}
#endif

#endif 



