#ifndef __IMFX_DEFS_H__
#define __IMFX_DEFS_H__

#ifdef  __cplusplus
extern "C"{
#endif

#ifndef mfx_i8
	typedef char mfx_i8;			/**< 8位有符号整型 */
#endif
	
#ifndef mfx_u8
	typedef unsigned char mfx_u8;	/**< 8位无符号整型 */
#endif
	
#ifndef mfx_i16
	typedef short mfx_i16;			/**< 16位有符号整型 */
#endif

#ifndef mfx_u16
	typedef unsigned short mfx_u16;	/**< 16位无符号整型 */
#endif

#ifndef mfx_i32
	typedef int mfx_i32;			/**< 32位有符号整型 */
#endif

	
#ifndef mfx_u32
	typedef unsigned int mfx_u32;	/**< 32位无符号整型 */
#endif

#if (defined( _WIN32 ) || defined ( _WIN64 )) && !defined (__GNUC__)
#define __INT64   __int64
#define __UINT64  unsigned __int64
#else
#define __INT64   long long
#define __UINT64  unsigned long long
#endif

#ifndef mfx_u64
	typedef __UINT64 mfx_u64;	/**< 64位无符号整型 */
#endif

#ifndef mfx_i64
	typedef __INT64 mfx_i64;	/**< 64位无符号整型 */	
#endif

#ifndef mfx_f32
	typedef float mfx_f32;	/**< 单精度浮点数据类型 */
#endif

#ifndef mfx_f64
	typedef double mfx_f64;	/**< 双精度浮点数据类型 */
#endif

#ifndef mfx_hdl
	typedef void* mfx_hdl;
#endif

#define IN
#define OUT
#define INOUT

#if defined(WIN32) || defined(WIN64)
#ifdef _IMFX_DLL
#define IMFX_API __declspec(dllexport)
#define IMFX_TEMPLATE __declspec(dllexport)
#else
#define IMFX_API __declspec(dllimport)
#define IMFX_TEMPLATE
#endif
#else
#define IMFX_API
#define IMFX_TEMPLATE
#endif

#define MFX_MAKEFOURCC(A,B,C,D)    ((((int)A))+(((int)B)<<8)+(((int)C)<<16)+(((int)D)<<24))

/** 视频编码ID */
typedef enum tagCodecID
{
	IMFX_CODEC_AVC = MFX_MAKEFOURCC('A', 'V', 'C', ' '),
	IMFX_CODEC_HEVC = MFX_MAKEFOURCC('H', 'E', 'V', 'C'),
	IMFX_CODEC_MPEG2 = MFX_MAKEFOURCC('M', 'P', 'G', '2'),
	IMFX_CODEC_VC1 = MFX_MAKEFOURCC('V', 'C', '1', ' '),
	IMFX_CODEC_JPEG = MFX_MAKEFOURCC('J', 'P', 'E', 'G'),
	IMFX_CODEC_VP8 = MFX_MAKEFOURCC('V', 'P', '8', ' '),

	IMFX_FOURCC_NV12 = MFX_MAKEFOURCC('N', 'V', '1', '2'),   /* Native Format */
	IMFX_FOURCC_YV12 = MFX_MAKEFOURCC('Y', 'V', '1', '2'),
	IMFX_FOURCC_YUY2 = MFX_MAKEFOURCC('Y', 'U', 'Y', '2'),
	IMFX_FOURCC_RGB3 = MFX_MAKEFOURCC('R', 'G', 'B', '3'),   /* deprecated */
	IMFX_FOURCC_RGB4 = MFX_MAKEFOURCC('R', 'G', 'B', '4'),   /* ARGB in that order, A channel is 8 MSBs */
	IMFX_FOURCC_P8 = 41,                                /*  D3DFMT_P8   */
	IMFX_FOURCC_P8_TEXTURE = MFX_MAKEFOURCC('P', '8', 'M', 'B'),
	IMFX_FOURCC_P010 = MFX_MAKEFOURCC('P', '0', '1', '0'),
	IMFX_FOURCC_BGR4 = MFX_MAKEFOURCC('B', 'G', 'R', '4'),   /* ABGR in that order, A channel is 8 MSBs */
	IMFX_FOURCC_A2RGB10 = MFX_MAKEFOURCC('R', 'G', '1', '0'),   /* ARGB in that order, A channel is two MSBs */
	IMFX_FOURCC_ARGB16 = MFX_MAKEFOURCC('R', 'G', '1', '6'),   /* ARGB in that order, 64 bits, A channel is 16 MSBs */
	IMFX_FOURCC_R16 = MFX_MAKEFOURCC('R', '1', '6', 'U')
}CODEC_ID_E;

typedef enum tagPicStruct{
	IMFX_PICSTRUCT_UNKNOWN = 0x00,
	IMFX_PICSTRUCT_PROGRESSIVE = 0x01,
	IMFX_PICSTRUCT_FIELD_TFF = 0x02,
	IMFX_PICSTRUCT_FIELD_BFF = 0x04,

	IMFX_PICSTRUCT_FIELD_REPEATED = 0x10,  /* first field repeated, pic_struct=5 or 6 in H.264 */
	IMFX_PICSTRUCT_FRAME_DOUBLING = 0x20,  /* pic_struct=7 in H.264 */
	IMFX_PICSTRUCT_FRAME_TRIPLING = 0x40   /* pic_struct=8 in H.264 */
}PIC_STRUCT_S;

typedef enum
{
	IMFX_JPEG_COLORFORMAT_UNKNOWN = 0,
	IMFX_JPEG_COLORFORMAT_YCbCr = 1,
	IMFX_JPEG_COLORFORMAT_RGB = 2
}JPEG_COLOR_FORMAT;

typedef enum
{
	IMFX_CHROMAFORMAT_MONOCHROME = 0,
	IMFX_CHROMAFORMAT_YUV420 = 1,
	IMFX_CHROMAFORMAT_YUV422 = 2,
	IMFX_CHROMAFORMAT_YUV444 = 3,
	IMFX_CHROMAFORMAT_YUV400 = IMFX_CHROMAFORMAT_MONOCHROME,
	IMFX_CHROMAFORMAT_YUV411 = 4,
	IMFX_CHROMAFORMAT_YUV422H = IMFX_CHROMAFORMAT_YUV422,
	IMFX_CHROMAFORMAT_YUV422V = 5
}JPEG_CHROMA_FORMAT;

/* 视频质量 */
typedef enum tagTargetUsage{
	TU_UNKNOWN = 0,
	BEST_QUALITY = 1,
	BALANCED = 4,
	BEST_SPEED = 7
}TARGET_USAGE_E;

typedef enum tagSerializeStringType
{
	XML,
	JSON
}SERIALIZE_STRING_TYPE_E;

typedef enum tagBlockMode{
	NON_BLOCK,
	BLOCK
}BLOCK_MODE;

typedef enum tagImfxErrorStatus
{
	IMFX_ERR_NONE							= 0,		/**< 调用成功 */

	/* error codes < 0, these codes are from mfxStatus */
	IMFX_ERR_UNKNOWN						= -1,	/**< 未知错误 */
	IMFX_ERR_NULL_PTR						= -2,	/**< 空指针 */
	IMFX_ERR_UNSUPPORTED					= -3,	/**< 特性不支持 */
	IMFX_ERR_MEMORY_ALLOC					= -4,	/**< 分配内存失败 */
	IMFX_ERR_NOT_ENOUGH_BUFFER				= -5,	/**< 输入/输出内存不足 */
	IMFX_ERR_INVALID_HANDLE					= -6,	/**< 无效的句柄 */
	IMFX_ERR_LOCK_MEMORY					= -7,	/**< 锁定内存失败 */
	IMFX_ERR_NOT_INITIALIZED				= -8,	/**< 未初始化就调用成员函数 */
	IMFX_ERR_NOT_FOUND						= -9,	/**< 对象不存在 */
	IMFX_ERR_MORE_DATA						= -10,	/**< 需要更多的数据 */
	IMFX_ERR_MORE_SURFACE					= -11,	/**< 需要更多的平面 */
	IMFX_ERR_ABORTED						= -12,	/**< 操作被终止 */
	IMFX_ERR_DEVICE_LOST					= -13,	/**< 硬件加速设备丢失 */
	IMFX_ERR_INCOMPATIBLE_VIDEO_PARAM		= -14,	/**< 不相容的视频参数，表示参数改变 */
	IMFX_ERR_INVALID_VIDEO_PARAM			= -15,	/**< 非法的视频参数 */
	IMFX_ERR_UNDEFINED_BEHAVIOR				= -16,	/**< 未定义行为 */
	IMFX_ERR_DEVICE_FAILED					= -17,	/**< 操作设备失败 */
	IMFX_ERR_MORE_BITSTREAM					= -18,	/**< 输出需要更多的数据流 */
	IMFX_ERR_INCOMPATIBLE_AUDIO_PARAM		= -19,	/**< 不相容的音频参数，表示参数改变 */
	IMFX_ERR_INVALID_AUDIO_PARAM			= -20,	/**< 非法的音频参数 */
	
	/* other codes beyond mfxStatus */
	IMFX_ERR_NO_ENOUGH_MEMORY				= -101,	/**< 内存不够 */
	IMFX_ERR_UNSUPPORTED_CODECID			= -102,	/**< 不支持的视频编码 */
	IMFX_ERR_INIT_DECODE_FAILED				= -103,	/**< 初始化解码失败*/
	IMFX_ERR_RUNDECODING_FAILED				= -104,	/**< 解码失败 */
	IMFX_ERR_INIT_ENCODE_FAILED				= -105,	/**< 初始化编码失败 */
	IMFX_ERR_INIT_VPP_FAILED				= -106,	/**< 初始化VPP失败 */
	IMFX_ERR_NULL_HANDLE					= -107,	/**< 句柄为空 */
	IMFX_ERR_INIT_SESSION_FAILED			= -108,	/**< 初始化GPU会话失败 */
	IMFX_ERR_CALL_WRONG_METHOD				= -109,	/**< 调用错误的方法 */
	IMFX_ERR_INIT_ANALYZE_ENGINE_FAILED		= -110,	/**< 初始化分析引擎失败 */
	IMFX_ERR_ANALYZE_FAILED					= -111,	/**< 初始化分析模块失败 */
	IMFX_ERR_REINITIALIZE					= -112,	/**< 重复初始化 */
	IMFX_ERR_UNINITIALIZED					= -113,	/**< 未初始化 */
	IMFX_ERR_GET_FREE_SURFACE_FALIED		= -114,	/**< 获取空闲的surface失败 */
	IMFX_ERR_INVALID_PARAM					= -115,	/**< 参数不合法 */
	IMFX_ERR_UNINITIALIZED_COMMON			= -116,	/**< common模块未初始化 */
	IMFX_ERR_INVALID_CALLED					= -117,	/**< 非法调用 */
	IMFX_ERR_NO_ENOUGH_SURFACE				= -118,	/**< 获取surface失败 */
	IMFX_ERR_INIT_PLUGIN_FAILED				= -119,	/**< 初始化插件失败 */
	IMFX_ERR_UNINITIALIZED_DECODER 			= -120,	/**< decoder模块未初始化 */
	IMFX_ERR_UNINITIALIZED_VPP 				= -121, /**< vpp模块未初始化 */

	/* warnings >0 */
	IMFX_WRN_IN_EXECUTION					= 1,    /* the previous asynchrous operation is in execution */
	IMFX_WRN_DEVICE_BUSY					= 2,    /* the HW acceleration device is busy */
	IMFX_WRN_VIDEO_PARAM_CHANGED			= 3,    /* the video parameters are changed during decoding */
	IMFX_WRN_PARTIAL_ACCELERATION			= 4,    /* SW is used */
	IMFX_WRN_INCOMPATIBLE_VIDEO_PARAM		= 5,    /* incompatible video parameters */
	IMFX_WRN_VALUE_NOT_CHANGED				= 6,    /* the value is saturated based on its valid range */
	IMFX_WRN_OUT_OF_RANGE					= 7,    /* the value is out of valid range */
	IMFX_WRN_FILTER_SKIPPED					= 10,   /* one of requested filters has been skipped */
	IMFX_WRN_INCOMPATIBLE_AUDIO_PARAM		= 11,   /* incompatible audio parameters */
	/* threading statuses */
	IMFX_TASK_DONE = IMFX_ERR_NONE, /* task has been completed */
	IMFX_TASK_WORKING = 8, /*  there is some more work to do */
	IMFX_TASK_BUSY = 9 /* task is waiting for resources */

}IMFX_STS;

/* 坐标 */
typedef struct tagPoint
{
	mfx_i32 x;
	mfx_i32 y;
}POINT_S;

/* 浮点型坐标 */
typedef struct tagFloatPoint
{
	mfx_f32 x;
	mfx_f32 y;
}FPOINT_S;

/* 视频属性 */
typedef struct tagVideoAttr
{
	CODEC_ID_E enCodecID;       /**< 编码ID */
	mfx_u32 ulWidth;            /**< 宽度 */
	mfx_u32 ulHeight;           /**< 高度 */
	mfx_u32 ulFrameRate;        /**< 帧率 */
	mfx_u32 ulBitRate;          /**< 码率 */
	PIC_STRUCT_S enPicStruct;	/**< 图片结构 */
}VIDEO_ATTR_S;

/** 视频帧数据缓冲区 */
typedef struct tagFrameData
{
	mfx_i8 *pcDataAddr;         /**< 数据地址 */
	CODEC_ID_E enCodecID;		/**< 流编码ID */
	mfx_u32 ulDataSize;         /**< 数据长度 */
	mfx_u32 ulDataOffset;		/**< 数据偏移地址 */
	mfx_u32 ulMaxLength;        /**< 缓冲区的最大长度 */
}FRAME_DATA_S;

/* YV12格式的数据 */
typedef struct tagYV12Data
{
	mfx_i8 *pcDataAddr;			/**< 数据地址 */
	mfx_u32 ulMaxLength;		/**< 数据地址能够容纳的最大长度 */
	mfx_u32 ulDataSize;			/**< 数据长度 */
	mfx_u32 ulWidth;			/**< 图像宽度 */
	mfx_u32 ulHeight;			/**< 图像高度 */
	mfx_u32 ulPitch;			/**< 图像跨距 */
}YV12_DATA_S;

/* RAW格式的数据 */
typedef struct tagRawData
{
	mfx_i8 *pcDataAddr;         /**< 数据地址 */
	CODEC_ID_E FourCC;          /**< 数据的格式，只支持 YV12，NV12, BGR24 */
	mfx_u32 ulMaxLength;		/**< 数据最大长度 */
	mfx_u32 ulDataSize;         /**< 数据长度 */
	mfx_u32 ulWidth;			/**< 图像宽度 */
	mfx_u32 ulHeight;			/**< 图像高度 */
	mfx_u32 ulPitch;            /**< 图像跨距 */
	mfx_u32 ulFrameRate;        /**< 图像帧率 编码输出使用 默认：25 */
	mfx_u32 ulBitRate;          /**< 图像码率 编码输出使用 默认:0 */
}RAW_DATA_S;

/* 浓缩初始化参数 */
typedef struct tagConcentrateInitParams
{
	CODEC_ID_E enSrcCodecID;            /**< 输入流编码ID */
	CODEC_ID_E enDstCodecID;            /**< 输出流编码ID */
	TARGET_USAGE_E enTargetUsage;       /**< 视频处理质量 编码输出使用，默认:BALANCED */
	mfx_u32   ulInterestedPointNum;       /**< 感兴趣区域的坐标点数 取值范围：[0， 10] */
	POINT_S astInterestedArea[10]; /**< 感兴趣区域坐标集(用于视频分析) */
	mfx_u32    lFilterThreshold;           /**< OpenCV过滤阀值，用来过滤噪声。取值范围: [0, 255] */
	mfx_u32    lMovingPointThreshold;      /**< 当前图像和前一张图像的运动的点数百分比。取值范围: [0, 100] */
}CONCENTRATE_INIT_PARAMS_S;

typedef struct tagImfxRect
{
	mfx_i32    left;
	mfx_i32    top;
	mfx_i32    right;
	mfx_i32    bottom;
}IMFX_RECT;

typedef struct tagPolygonAttr
{
	FPOINT_S *pstFPoint;
	mfx_u32 ulVertexNum;
	mfx_u32 ulLineColor;
	mfx_f32 fLineWidth;
}POLYGON_ATTR_S;

//--------预解码信息头，从内部拷贝出来的结构体，添加后缀 _ext-----
/* Frame ID for SVC and MVC */
typedef struct {
	mfx_u16      TemporalId;
	mfx_u16      PriorityId;
	union {
		struct {
			mfx_u16  DependencyId;
			mfx_u16  QualityId;
		};
		struct {
			mfx_u16  ViewId;
		};
	};
} mfxFrameId_ext;

#pragma pack(push, 4)
/* Frame Info */
typedef struct {
	mfx_u32  reserved[4];
	mfx_u16  reserved4;
	mfx_u16  BitDepthLuma;
	mfx_u16  BitDepthChroma;
	mfx_u16  Shift;

	mfxFrameId_ext FrameId;

	mfx_u32  FourCC;
	union {
		struct { /* Frame parameters */
			mfx_u16  Width;
			mfx_u16  Height;

			mfx_u16  CropX;
			mfx_u16  CropY;
			mfx_u16  CropW;
			mfx_u16  CropH;
		};
		struct { /* Buffer parameters (for plain formats like P8) */
			mfx_u64 BufferSize;
			mfx_u32 reserved5;
		};
	};

	mfx_u32  FrameRateExtN;
	mfx_u32  FrameRateExtD;
	mfx_u16  reserved3;

	mfx_u16  AspectRatioW;
	mfx_u16  AspectRatioH;

	mfx_u16  PicStruct;
	mfx_u16  ChromaFormat;
	mfx_u16  reserved2;
} mfxFrameInfo_ext;
#pragma pack(pop)

/* Transcoding Info */
typedef struct {
	mfx_u32  reserved[7];

	mfx_u16  LowPower;
	mfx_u16  BRCParamMultiplier;

	mfxFrameInfo_ext    FrameInfo;
	mfx_u32  CodecId;
	mfx_u16  CodecProfile;
	mfx_u16  CodecLevel;
	mfx_u16  NumThread;

	union {
		struct {   /* MPEG-2/H.264 Encoding Options */
			mfx_u16  TargetUsage;

			mfx_u16  GopPicSize;
			mfx_u16  GopRefDist;
			mfx_u16  GopOptFlag;
			mfx_u16  IdrInterval;

			mfx_u16  RateControlMethod;
			union {
				mfx_u16  InitialDelayInKB;
				mfx_u16  QPI;
				mfx_u16  Accuracy;
			};
			mfx_u16  BufferSizeInKB;
			union {
				mfx_u16  TargetKbps;
				mfx_u16  QPP;
				mfx_u16  ICQQuality;
			};
			union {
				mfx_u16  MaxKbps;
				mfx_u16  QPB;
				mfx_u16  Convergence;
			};

			mfx_u16  NumSlice;
			mfx_u16  NumRefFrame;
			mfx_u16  EncodedOrder;
		};
		struct {   /* H.264, MPEG-2 and VC-1 Decoding Options */
			mfx_u16  DecodedOrder;
			mfx_u16  ExtendedPicStruct;
			mfx_u16  TimeStampCalc;
			mfx_u16  SliceGroupsPresent;
			mfx_u16  reserved2[9];
		};
		struct {   /* JPEG Decoding Options */
			mfx_u16  JPEGChromaFormat;
			mfx_u16  Rotation;
			mfx_u16  JPEGColorFormat;
			mfx_u16  InterleavedDec;
			mfx_u16  reserved3[9];
		};
		struct {   /* JPEG Encoding Options */
			mfx_u16  Interleaved;
			mfx_u16  Quality;
			mfx_u16  RestartInterval;
			mfx_u16  reserved5[10];
		};
	};
} mfxInfoMFX_ext;
//-----end-----------------------

typedef enum tagResizeMode
{
	RESIZE_NONE = 0,
	BY_WIDTH_HEIGHT = 1,
	BY_RATIO = 2
}RESIZE_MODE;

typedef struct tagResize
{
	RESIZE_MODE mode;
	union
	{
		struct s
		{
			unsigned int width;
			unsigned int height;
		}s;
		float ratio;
	}u;
}RESIZE_S;

typedef struct tagJpgInfo
{
	mfx_u32 Width;
	mfx_u32 Height;
	JPEG_COLOR_FORMAT ColorFormat;
	JPEG_CHROMA_FORMAT ChromaFormat;
	PIC_STRUCT_S PicStruct;
}IMFX_JPG_INFO;

//const char * err_string[]
//#define IMFX_err(error_no)  do { return err_string[err_no]; } while (0);

#define DEBUG

#ifdef DEBUG
#if defined(WIN32) || defined(WIN64)
	//#include   <stdarg.h>
#define IMFX_DBG(fmt, ...) do { printf("[%s][%d] DBG : " fmt "\n", __FILE__, __LINE__, __VA_ARGS__); } while (0)
#define IMFX_INFO(fmt, ...) do { printf("[%s][%d] INFO: " fmt "\n", __FILE__, __LINE__, __VA_ARGS__); } while (0)
#define IMFX_WARN(fmt, ...) do { printf("[%s][%d] WARN: " fmt "\n", __FILE__, __LINE__, __VA_ARGS__); } while (0)
#define IMFX_ERR(fmt, ...) do { printf("[%s][%d] ERR : " fmt "\n", __FILE__, __LINE__, __VA_ARGS__); } while (0)
#else
#define IMFX_DBG(fmt, ...) do { printf("[%s][%d] DBG : " fmt, __FILE__, __LINE__, ##__VA_ARGS__); } while (0)
#define IMFX_INFO(fmt, ...) do { printf("[%s][%d] INFO: " fmt, __FILE__, __LINE__, ##__VA_ARGS__); } while (0)
#define IMFX_WARN(fmt, ...) do { printf("[%s][%d] WRN : " fmt, __FILE__, __LINE__, ##__VA_ARGS__); } while (0)
#define IMFX_ERR(fmt, ...) do { printf("[%s][%d] ERR : " fmt, __FILE__, __LINE__, ##__VA_ARGS__); } while (0)
#define IMFX_ERR(fmt, ...) do { printf("[%s][%d] ERR : " fmt, __FILE__, __LINE__, ##__VA_ARGS__); } while (0)
#endif
#else
#define IMFX_DBG(fmt, ...)
#define IMFX_INFO(fmt, ...)
#define IMFX_WARN(fmt, ...)
#define IMFX_ERR(fmt, ...)
#endif

#ifdef  __cplusplus
}
#endif  /* end of __cplusplus */

#endif