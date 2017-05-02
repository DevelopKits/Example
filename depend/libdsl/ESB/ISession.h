/*
* All rights reserved.
*
* 文件名称：ISession.h
* 文件标识：
* 摘　　要：通用会话保存模板
*

* 当前版本：1.0
* 原作者　：吕少卿
* 完成日期：2013年4月7日
* 修订记录：创建
*/
#pragma once

#include <libdsl/DRefObj.h>
#include <libdsl/DMutex.h>
#include <vector>
#include <map>


BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////


template<class SESSION_ID, class SESSION>
class ISession : virtual public DRefObj
{
public:
	ISession() {}
	ISession(ISession<SESSION_ID, SESSION>& oSession)
	{
		m_mapSession = oSession.m_mapSession;
	}
	/***
	 * 保存会话
	 * oId：唯一的会话标识符，否则插入失败
	 * oSession：会话
	 * 警告：SESSION类需要考虑深拷贝与浅拷贝，如果需要深度拷贝，请重载拷贝构造函数.
	 *		 如果SESSION_ID已经存在，则插入失败，请使用 AddOrReplaceSession。
	***/
	int AddSession(SESSION_ID oId, SESSION& oSession)
	{
		DMutexGuard guard(&m_mtxSession);
		if(!m_mapSession.insert( Sessions_Pair_T(oId, oSession) ).second)
			return -1;

		return 0;
	}
	int AddOrReplaceSession(SESSION_ID oId, SESSION& oSession)
	{
		DMutexGuard guard(&m_mtxSession);
		m_mapSession[oId] = oSession;
		return 0;
	}

	int DelSession(SESSION_ID oId)
	{
		DMutexGuard guard(&m_mtxSession);
		typename MapSession::iterator it = m_mapSession.find(oId);
		if(it != m_mapSession.end())
		{
			m_mapSession.erase(it);
			return 0;
		}

		return -1;
	}

	int Clear()
	{
		DMutexGuard guard(&m_mtxSession);
		m_mapSession.clear();
		return 0;
	}

	size_t Size()
	{
		DMutexGuard guard(&m_mtxSession);
		return m_mapSession.size();
	}
	/***
	 * 获取某个会话的拷贝
	 * oInId：输入唯一的回话标识符
	 * oOutSession：输出该id对应的会话
	 * 警告：pOutSession需要考虑生命周期。
	***/
	int GetSession(SESSION_ID oInId, SESSION& pOutSession)
	{
		DMutexGuard guard(&m_mtxSession);
		typename MapSession::iterator it = m_mapSession.find(oInId);
		if(it != m_mapSession.end())
		{
			pOutSession = it->second;
			return 0;
		}

		return -1;
	}

	/***
	 * 获取某个会话完整的标识符的拷贝
	 * oInId[in]：输入唯一的会话标识符
	 * oInId[out]:完整的会话标识符
	 * 警告：SESSION_ID如果是自定义类型,需要自己写operator =
	***/
	int GetSessionId(SESSION_ID& oInId)
	{
		DMutexGuard guard(&m_mtxSession);
		typename MapSession::iterator it = m_mapSession.find(oInId);
		if(it != m_mapSession.end())
		{
			oInId = it->first;
			return 0;
		}

		return -1;
	}

	int GetAllSessionId(std::vector<SESSION_ID>& vecSessionId)
	{
		DMutexGuard guard(&m_mtxSession);
		typename MapSession::iterator it = m_mapSession.begin();
		for (; it != m_mapSession.end(); ++it)
		{
			vecSessionId.push_back(it->first);
		}
		return 0;
	}

	/***
	 * 警告：FisrtSession 等函数顺序，完全取决于SESSION_ID的排序。排序算法可以被重载，int 等类型默认为顺序
	***/
	int GetFirstSession(SESSION& pOutSession)
	{
		DMutexGuard guard(&m_mtxSession);
		if(m_mapSession.size() == 0)
			return -1;

		pOutSession = m_mapSession.begin()->second;
		return 0;
	}
	/***
	 * 取走数据，该数据在列表中删除。
	***/
	int FetchSession(SESSION_ID &oInId, SESSION& pOutSession)
	{
		DMutexGuard guard(&m_mtxSession);
		typename MapSession::iterator it = m_mapSession.find(oInId);
		if(it != m_mapSession.end())
		{
			pOutSession = it->second;
			m_mapSession.erase(it);
			return 0;
		}

		return -1;
	}

	int FetchFisrtSession(SESSION& pOutSession)
	{
		DMutexGuard guard(&m_mtxSession);
		typename MapSession::iterator it = m_mapSession.begin();
		if(it != m_mapSession.end())
		{
			pOutSession = it->second;
			m_mapSession.erase(it);
			return 0;
		}

		return -1;
	}

	int DelFirstSession()
	{
		DMutexGuard guard(&m_mtxSession);
		if(m_mapSession.size() > 0)
		{
			m_mapSession.erase(m_mapSession.begin());
			return 0;
		}

		return -1;
	}

	bool IsExsited(SESSION_ID oInId)
	{
		DMutexGuard guard(&m_mtxSession);
		typename MapSession::iterator it = m_mapSession.find(oInId);
		if(it != m_mapSession.end())
			return true;

		return false;
	}

	// 警告：以下三个函数式配套使用的。
	bool GetFirstInLock(SESSION& pOutSession)
	{
		m_mtxSession.Lock();
		m_itSession = m_mapSession.begin();
		if(m_itSession != m_mapSession.end())
		{
			pOutSession = m_itSession->second;
			m_itSession++;
			return true;
		}

		m_mtxSession.Unlock();
		return false;
	}
	bool GetNextInLock(SESSION& pOutSession)
	{
		if(m_itSession == m_mapSession.end())
		{
			m_mtxSession.Unlock();
			return false;
		}

		pOutSession = m_itSession->second;
		m_itSession++;
		return true;
	}
	void ReleaseInLock()
	{
		m_mtxSession.Unlock();
	}
	

protected:
	typedef std::map< SESSION_ID, SESSION > MapSession;
	MapSession m_mapSession;
	typedef std::pair< SESSION_ID, SESSION > Sessions_Pair_T;
	DMutex m_mtxSession;
	typename MapSession::iterator m_itSession;
};


#define AllDo(session, class, func)	{\
		DRef<class> pSession;\
		if(session.GetFirstInLock(pSession))\
			for(;;)\
			{\
				pSession->##func;\
				if(!session.GetNextInLock(pSession))\
					break;\
			}\
	}
	

/////////////////////////////////////////////////
END_NAMESPACE_DSL