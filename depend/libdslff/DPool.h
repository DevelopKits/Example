#ifndef INCLUDED_LIBDSL_DPOOL_H
#define INCLUDED_LIBDSL_DPOOL_H

#include <libdsl/DMutex.h>
#include <libdsl/DRefObj.h>

#include <deque>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

#define DSL_OBJECT_POOL_MAX_NUM		128 // 最大空闲对象个数

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


// <2> 引用计数对象池，对象T需要继承DRefPoolObj
// 引用计数池的对象，一般由池来创建，（另外也可以不跟池一起使用，单独创建，此时退化成普通的DRefObj）
// 使用方法 class MyObj : public DRefPoolObj<MyObj> { ... }; 
//          DRefPool<MyObj> m_pool; DRefPointer< MyObj > pObj = m_pool.GetObject();
// 当DRefPointer析构时，会调用DRefPoolObj的release方法，最终会将对象返回到pool中。

template <class T> class DRefPool;

template< class T >
class DRefPoolObj : public DRefObj
{
public:
	DRefPoolObj() { m_pPool = NULL; }

protected:
	virtual ~DRefPoolObj() {}

protected:
	inline virtual void destroy() 
	{ 
		// TODO : 当前pool声明周期必须大于对象，以后可以考虑pool先释放，将对讲的pool指针置NULL
		if( m_pPool != NULL ) {
			m_pPool->recycle( (T *)this );  // T 是 DRefPool< T > 是的子类，这个对象本身一定是 T
		}
		else {
			delete this;
		}
		return;
	}

private:
	friend class DRefPool< T >;
	inline void setpool( DRefPool< T > * pPool ) { m_pPool = pPool; return; } 

private:
	DRefPool< T > * m_pPool;
};

template< class T >
class DRefPool
{
public:
	DRefPool( unsigned int nObjectMaxCount = DSL_OBJECT_POOL_MAX_NUM ) : m_nObjectMaxCount( nObjectMaxCount ) { }
	~DRefPool(){ Decrease( 0 ); }

public:
	// 与DRefObj类似，返回的对象引用计数为0，应该赋值给一个 DRefPointer 对象
	inline DRefPointer<T> GetObject()
	{
		T * pT = NULL;
		DMutexGuard guard(&m_csLock);
		if (m_dequeObjects.size() == 0) {
			pT = new T();
			pT->setpool( this );
		}
		else {
			pT = m_dequeObjects.back();
			m_dequeObjects.pop_back();
		}
		return DRefPointer<T>( pT );
	}

	inline int GetObjectNum() const { return (int)m_dequeObjects.size(); } 

	inline int Decrease( unsigned int allow_num ) 
	{
		DMutexGuard guard(&m_csLock);
		while( m_dequeObjects.size() > allow_num ) {
			T * pT = m_dequeObjects.back();
			m_dequeObjects.pop_back();
			// 引用计数已经为0，setpool(NULL)在destroy相当于delete，
			pT->setpool( NULL );
			pT->destroy();
		}
		return (int)m_dequeObjects.size();
	}

private:
	friend class DRefPoolObj< T >;
	inline void recycle( T * pT )
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
	std::deque< T * > m_dequeObjects;  // Pool 里面对象的引用计数都是0
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
	inline DRefPointer<T> GetObject()
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
		return DRefPointer<T>( (T *)pT );
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
