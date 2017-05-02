/*
* All rights reserved.
*
* �ļ����ƣ�ISession.h
* �ļ���ʶ��
* ժ����Ҫ��ͨ�ûỰ����ģ��
*

* ��ǰ�汾��1.0
* ԭ���ߡ���������
* ������ڣ�2013��4��7��
* �޶���¼������
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
	 * ����Ự
	 * oId��Ψһ�ĻỰ��ʶ�����������ʧ��
	 * oSession���Ự
	 * ���棺SESSION����Ҫ���������ǳ�����������Ҫ��ȿ����������ؿ������캯��.
	 *		 ���SESSION_ID�Ѿ����ڣ������ʧ�ܣ���ʹ�� AddOrReplaceSession��
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
	 * ��ȡĳ���Ự�Ŀ���
	 * oInId������Ψһ�Ļػ���ʶ��
	 * oOutSession�������id��Ӧ�ĻỰ
	 * ���棺pOutSession��Ҫ�����������ڡ�
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
	 * ��ȡĳ���Ự�����ı�ʶ���Ŀ���
	 * oInId[in]������Ψһ�ĻỰ��ʶ��
	 * oInId[out]:�����ĻỰ��ʶ��
	 * ���棺SESSION_ID������Զ�������,��Ҫ�Լ�дoperator =
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
	 * ���棺FisrtSession �Ⱥ���˳����ȫȡ����SESSION_ID�����������㷨���Ա����أ�int ������Ĭ��Ϊ˳��
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
	 * ȡ�����ݣ����������б���ɾ����
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

	// ���棺������������ʽ����ʹ�õġ�
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