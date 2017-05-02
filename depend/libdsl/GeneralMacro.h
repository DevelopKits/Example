/* Copyright (c) 2013, 浙江大华技术股份有限公司, All rights reserved.
* 2013-10
*
* 版本    ：1.0
* 功能    : 通用宏
* 作者　  ：吕少卿
* 完成日期：2014年9月17日
*
* 当前版本：1.0
*/
#pragma once

#include "libdsl/DPrintLog.h"
#include "libdsl/DCharsetConv.h"
#include "jsondsl/writer.h"
#include "ESB/osdef.h"


#define	VAR_IS_NULL_THEN_RETURN(varible, code, info, ...)\
												{ if(varible == NULL) { DLOG_ERR(info, ##__VA_ARGS__); return code; } }
#define	VAR_IS_NULL_THEN_RETURN_NO_VALUE(varible, info, ...)\
												{ if(varible == NULL) { DLOG_ERR(info, ##__VA_ARGS__); return; } }


#define RECORDS_AND_RETURN(code, info, ...)			{ DLOG_ERR("%s, errCode:%d", info, code);  return code; }
#define RECORDS_AND_RETURN_NO_VALUE(code, info, ...){ DLOG_ERR("%s, errCode:%d", info, code);  return; }


#define	VALUE_IS_NOT_0_THEN_RETURN(x, info, ...)		\
						{\
							int nRet = (x);\
							if(nRet != 0)\
								{ DLOG_ERR(info, ##__VA_ARGS__); return nRet; }\
						}
#define	VALUE_IS_NOT_0_THEN_RT_NOVALUE(x, info, ...)	\
						{\
							int nRet = (x);\
							if(nRet != 0)\
									{ DLOG_ERR(info, ##__VA_ARGS__); return; }\
						}
#define	LESS_THAN_0_THEN_RETURNED(x, info, ...)			\
						{\
							int nRet = (x); \
							if(nRet < 0)\
								{ DLOG_ERR(info, ##__VA_ARGS__); return nRet; }\
						 }



#define	VALUE_FOUND_IS_NOT_THE_LAST_THEN_RETURN(set, value, info, code)	\
												if(set.find(value) != set.end())\
													RECORDS_AND_RETURN(code, info)
#define	VALUE_FOUND_IS_THE_LAST_THEN_RETURN(set, value, info, code)		\
												if(set.find(value) == set.end())\
													RECORDS_AND_RETURN(code, info)



#define SAFE_CALL_4_POINTER(poniter, func, info) \
												if(poniter)\
													poniter->func;\
												else\
													DLOG_ERR(info);

#define SAFE_CALL_4_POINTER_RETURN(poniter, func, info) \
												if(poniter)\
													return poniter->func;\
												else\
													DLOG_ERR(info);\
													return -1;

#define	CONVERT_THE_POINTER_FOR_PTHE(pTheClass, the, theMember)		\
					pTheClass *pThe;\
					pThe = (pTheClass*)( (char*)the - offsetof(pTheClass, theMember) );

#define DYNMIC_TYPE_COMPARE(pointer, TYPE, bSame)	\
					{TYPE *pType = new TYPE;\
					TYPE* pPtrVt = (TYPE*&)(*pointer);\
					TYPE* pTypeVt = (TYPE*&)(*pType);\
					delete pType;\
					if(pPtrVt == pTypeVt) \
						bSame = true;\
					else \
						bSame = false;}

#define STATIC_TYPE_COMPARE(pointer, TYPE, bSame)	{\
					if(pointer->GetClassType() == TYPE::GetClassTypeST()) \
						bSame = true;\
					else \
						bSame = false;}

#define SAFE_STRNCPY(des, src, size)	if(src) strncpy(des, src, size); else strncpy(des, "\0", 1);


#ifndef SAFE_DELETE_POINT
#define SAFE_DELETE_POINT(point)\
	if( point) { delete point; point = NULL; }
#endif

#ifndef SAFE_DELETE_BUF
#define SAFE_DELETE_BUF(buf)\
	if( buf) { delete [] buf; buf = NULL; }
#endif

#ifdef WIN32
// 包含最后的/
#define GET_PROGRAM_DIR(sPath, handle)	\
{\
	char exeFullPath[MAX_PATH];\
	std::string strPath = "";\
	GetModuleFileNameA(handle, exeFullPath, MAX_PATH);\
	strPath = (std::string)exeFullPath;\
	size_t pos = strPath.find_last_of('\\', strPath.length());\
	sPath = strPath.substr(0, pos);\
}
#define GET_PROGRAM_DIR2(sPath, nLen, handle)	\
{\
	char exeFullPath[MAX_PATH];\
	std::string strPath = "";\
	GetModuleFileNameA(handle, exeFullPath, MAX_PATH);\
	strPath = (std::string)exeFullPath;\
	size_t pos = strPath.find_last_of('\\', strPath.length());\
	strncpy(sPath, strPath.substr(0, pos).c_str(), nLen);\
}

#else

#define GET_PROGRAM_DIR(sPath, handle) \
{\
	handle;\
	char sDirPath[2048];\
	getcwd(sDirPath, sizeof(sDirPath) - 1);\
	sPath = sDirPath;\
}

#define GET_PROGRAM_DIR2(sPath, nLen, handle) { handle; getcwd(sPath, nLen); }

#endif

#define INIT_CHAR(str)	memset(str, 0, sizeof(str))
#define INIT_CHAR_POINTER(str, len)	memset(str, 0, len)

#define ASSIGN_STRING(str, value)	if(value)\
										str = value;\
									else\
										str = "";

#define	GET_NODE_FROM_CONFIGINFO(member, element)\
	node = root->FirstChildElement(element);\
	if ( node != NULL )\
	{\
		TiXmlNode* pNode = node->FirstChild();\
		if (pNode != NULL)\
			member = pNode->Value();\
		else\
		{\
			DLOG_ERR("OnReadConfigFile no find Element Value:%s.", element);\
			return -1;\
		}\
	}\
	else\
	{\
		DLOG_ERR("OnReadConfigFile no find Element:%s.", element);\
		return -1;\
	}

#define	GET_NODE(sub, subNode, element, bRet)\
	subNode = sub->FirstChildElement(element);\
	if ( subNode == NULL )\
	{\
		CLOG_INFO("GET_NODE no find sub[%s} subNode[%s] element[%s]", #sub, #subNode, element);\
		bool bIsNeed = bRet;\
		if(bIsNeed)\
			return -1;\
	}


#define	GET_NODE_FROM_SUBNODE(sub, member, element, bRet){\
	GET_NODE(sub, node, element, bRet)\
	if(node)\
	{\
		TiXmlNode* pNode = node->FirstChild();\
		if (pNode != NULL)\
			member = pNode->Value();\
		else\
		{\
			CLOG_INFO("GET_NODE_FROM_SUBNODE no find sub[%s] member[%s] Node[:%s].", #sub, #member, element);\
			bool bIsNeed = bRet;\
			if(bIsNeed)\
				return -1;\
		}\
	}\
}



#define RANDOM_IN(randValue, maxValue)	{ srand((unsigned)time(NULL)); randValue = rand() % maxValue; }

#ifdef WIN32

#define RANDOM_TRUE(randValue, maxValue)	\
{\
	HCRYPTPROV hCryptProv = 0;\
	randValue = 0;\
	if(!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_DSS_DH, CRYPT_VERIFYCONTEXT))\
		{ DLOG_ERR("[RANDOM_TRUE] CryptAcquireContext failed"); }\
	if( CryptGenRandom(hCryptProv, sizeof(randValue), (BYTE*)&randValue) ) \
		randValue = randValue % maxValue;\
	CryptReleaseContext(hCryptProv, 0); \
}

#else

#define RANDOM_TRUE(randValue, maxValue)	\
{\
	randValue = 0;\
	FILE* pFile = fopen("/dev/urandom", "r");\
	if(pFile)\
	{\
		fread( &randValue, 1, sizeof(randValue), pFile );\
		randValue = randValue % maxValue;\
		fclose(pFile);\
	}\
}

#endif

#define GetGuid(str)\
{\
	const char CCH[] = "_0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";\
	const int SIZE_CHAR = sizeof(CCH) - 1;\
	char ch[SIZE_CHAR + 1] = {0};\
	for (int i = 0; i < SIZE_CHAR; ++i)\
	{\
		unsigned int x = 0;\
		RANDOM_TRUE(x, SIZE_CHAR);\
		ch[i] = CCH[x];\
	}\
	str = ch;\
}

#define COMPARE_JSON_ELEMENT(srcValue, type, dstValue, bComp)	{ bComp = true;\
	if( srcValue.is##type())\
		{\
			if(!dstValue.is##type())\
				bComp = false;\
			else if(srcValue.as##type() != dstValue.as##type())\
				bComp = false;\
		}\
}


inline std::string TraceJson(dsl::Json::Value& json)
{
	dsl::Json::FastWriter fw;
	return fw.write(json);
}