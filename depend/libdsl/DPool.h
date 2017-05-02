/* Copyright (c) 2011, 浙江大华技术股份有限公司, All rights reserved.
* 2011-09
*
* DPool.h : 普通对象池，引用计数对象池
*/

#ifndef INCLUDED_LIBDSL_DPOOL_H
#define INCLUDED_LIBDSL_DPOOL_H

#include <libdsl/DMutex.h>
#include <libdsl/DRefObj.h>

#include <deque>
#include <list>
#include <vector>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

#define DSL_OBJECT_POOL_MAX_NUM		256 // 最大空闲对象个数

// <1> 普通对象池，需要外部显式将对象送回池中
template< class T >
class DPool
{
public:
	DPool( unsigned int maxnum = DSL_OBJECT_POOL_MAX_NUM ) : m_nObjectMaxCount( maxnum ) { }

	~DPool()
	{
		Decrease( 0 );
	}

	inline T * GetObject()
	{
		T* pT;
		DMutexGuard guard(&m_csLock);

		if( m_dequeObjects.size() == 0 ) {
			pT = new T();
		}
		else {
			pT = m_dequeObjects.back();
			m_dequeObjects.pop_back();
		}

		return pT;
	}

	inline void Recycle( T * pT )
	{
		// assert(pT);
		if( pT == NULL )
			return;

		DMutexGuard guard(&m_csLock);
		if( m_dequeObjects.size() >= m_nObjectMaxCount ) {
			delete pT;
		}
		else {
			m_dequeObjects.push_back(pT);
		}
	}

	inline int GetObjectNum() const { return (int)m_dequeObjects.size(); } 

	inline int Decrease( unsigned int allow_num ) 
	{
		DMutexGuard guard(&m_csLock);
		while( m_dequeObjects.size() > allow_num ) {
			delete m_dequeObjects.back();
			m_dequeObjects.pop_back();
		}
		return (int)m_dequeObjects.size();
	}

private:
	DMutex m_csLock;
	unsigned int m_nObjectMaxCount;
	std::deque< T * > m_dequeObjects;
};


template< class T >
class DRefPool : virtual public DRefObj
{
public:
	DRefPool( unsigned int nObjectMaxCount = DSL_OBJECT_POOL_MAX_NUM ) : m_nObjectMaxCount( nObjectMaxCount ) 
	{ 
		m_dequeObjects = new DRefObj*[nObjectMaxCount];
		m_nObjectCount = 0;
	}
	virtual ~DRefPool(){ Decrease( 0 ); }

public:
	// 与DRefObj类似，返回的对象引用计数为0，应该赋值给一个 DRefPointer 对象
	inline DRef<T> GetObject()
	{
		T * pT = NULL;
		m_mtxObj.Lock();
		if (m_nObjectCount == 0)
		{
			pT = new T();
			for(; m_nObjectCount < m_nObjectMaxCount / 2; )
				m_dequeObjects[m_nObjectCount++] = new T();

			m_mtxObj.Unlock();
		}
		else 
		{
			pT = dynamic_cast<T*>(m_dequeObjects[--m_nObjectCount]);
			m_mtxObj.Unlock();
			if(pT)
				pT->Reset(); // 显示：调用构造函数，用于初始化
		}
		if(pT)
			pT->setpool( (DRefPool< DRefObj > *)this, (DRefObj*)this );
		return DRef<T>( pT );
	}

	inline int GetObjectNum() const { return (int)m_nObjectCount; } 

	inline int Decrease( unsigned int allow_num ) 
	{
		DMutexGuard guard(&m_mtxObj);
		while( m_nObjectCount > allow_num ) {
			T * pT = dynamic_cast<T*>(m_dequeObjects[--m_nObjectCount]);
			// 引用计数已经为0，setpool(NULL)在destroy相当于delete，
			pT->destroy();
		}

		if(m_nObjectCount == 0)
		{
			delete []m_dequeObjects;
			m_dequeObjects = NULL;
		}
		return (int)m_nObjectCount;
	}

private:
	friend class DRefObj;
	inline void recycle( DRefObj * pObj )
	{
		// 保证自己在 pObj->setpool( NULL, NULL ); 不能被删除。
		addref();
		pObj->setpool( NULL, NULL );

		m_mtxObj.Lock();
		if(m_nObjectCount >= (int)m_nObjectMaxCount) 
		{
			// 引用计数已经为0，setpool(NULL)在destroy相当于delete，
			for(; m_nObjectCount > m_nObjectMaxCount / 2; )
				m_dequeObjects[--m_nObjectCount]->destroy();
			
			m_mtxObj.Unlock();
			pObj->destroy();
		}
		else
		{	
			m_dequeObjects[m_nObjectCount++] = pObj;
			m_mtxObj.Unlock();
		}
		release();
	}

private:
	unsigned int	m_nObjectMaxCount;
	DRefObj **		m_dequeObjects;  // Pool 里面对象的引用计数都是0
	unsigned int	m_nObjectCount;
	DMutex			m_mtxObj;
	// TODO : 增加记录已分配对象的set，当pool先于对象释放时，将对象的pool指针置空
};

// <3> 增强版应用计数对象池，对象T本身需要从DRefObj中继承，对象必须在Pool中生成，不允许加入外部产生的对象

template< class T >
class DRefPoolEx
{
public:
	DRefPoolEx( unsigned int nObjectMaxCount = DSL_OBJECT_POOL_MAX_NUM ) : m_nObjectMaxCount( nObjectMaxCount ) { }
	~DRefPoolEx(){ Decrease( 0 ); }

public:
	// 与DRefObj类似，返回的对象引用计数为0，应该赋值给一个 DRefPointer 对象
	inline DRef<T> GetObject()
	{
		SubT * pT = NULL;
		DMutexGuard guard(&m_csLock);
		if (m_dequeObjects.size() == 0) {
			pT = new SubT();
			pT->setpool( this );
		}
		else {
			pT = m_dequeObjects.back();
			m_dequeObjects.pop_back();
		}
		return DRef<T>( (T *)pT );
	}

	inline int GetObjectNum() const { return (int)m_dequeObjects.size(); } 

	inline int Decrease( unsigned int allow_num ) 
	{
		DMutexGuard guard(&m_csLock);
		while( m_dequeObjects.size() > allow_num ) {
			SubT * pT = m_dequeObjects.back();
			m_dequeObjects.pop_back();
			// 引用计数已经为0，setpool(NULL)在destroy相当于delete，
			pT->setpool( NULL );
			pT->destroy();
		}
		return (int)m_dequeObjects.size();
	}

private:
	class SubT : public T
	{
	public:
		inline virtual void destroy() 
		{ 
			// TODO : 当前pool声明周期必须大于对象，以后可以考虑pool先释放，将对讲的pool指针置NULL
			if( m_pPool != NULL ) {
				m_pPool->recycle( this );
			}
			else {
				delete this;
			}
			return;
		}
	private:
		friend class DRefPoolEx< T >;
		inline void setpool( DRefPoolEx< T > * pPool ) { m_pPool = pPool; return; } 

	private:
		DRefPoolEx< T > * m_pPool;
	};

private:
	friend class SubT;
	inline void recycle( SubT * pT )
	{
		DMutexGuard guard(&m_csLock);
		if(m_dequeObjects.size() >= m_nObjectMaxCount) {
			// 引用计数已经为0，setpool(NULL)在destroy相当于delete，
			pT->setpool( NULL );
			pT->destroy();
		}
		else {
			m_dequeObjects.push_back( pT );
		}
	}

private:
	DMutex m_csLock;
	unsigned int m_nObjectMaxCount;
	std::deque< SubT * > m_dequeObjects;  // Pool 里面对象的引用计数都是0
	// TODO : 增加记录已分配对象的set，当pool先于对象释放时，将对象的pool指针置空
};

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif // INCLUDED_LIBDSL_DPOOL_H
