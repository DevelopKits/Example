#ifndef INCLUDED_LIBSDL_DREFOBJ_H
#define INCLUDED_LIBSDL_DREFOBJ_H

#include <libdsl/DAtomic.h>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

/* DRefObj：引用计数对象，初始计数 m_dwRefCount 的数值为 0 （！！！）
 * 除了某些函数要求参数是指针外，其他情况都应该与DRefPointer一起使用，由DRefPointer控制释放
 * DRefPointer<DRefObj> refPtr( new DRefObj );
 */

 class DRefInterface;
template< class T > class DRefPointer;

class DRefObj
{
public:
	// 不应该使用这个函数，判断引用计数的数量是不可靠的
	DSL_DEPRECATED inline int GetRefCount() const { return m_ref.Read(); }

protected:
	DRefObj() {}
	virtual ~DRefObj() {}   // 注意：子类的析构函数也应该是protected（！！）
	virtual void destroy(); // 缺省实现是：{ delete this; return; }，一般不需要重载，当前只用于pool

private:
	// 外部不应该直接调用这两个函数，应该通过DRefPointer来访问，子类也不应该调用
	template< class T > friend class DRefPointer;
	friend class DRefInterface;
	inline int addref() { return m_ref.Inc(); }
	inline int release() { int r = m_ref.Dec(); if( r == 0 ) destroy(); return r; }

private:
	DAtomic m_ref;
};

/* 指向 DRefObj 的子类的引用计数指针，自动增加和减少引用计数
 */
template< class T >
class DRefPointer
{
public:
	DRefPointer( T * p = NULL ) : m_pObj( p ) { if( m_pObj ) m_pObj->addref(); }
	DRefPointer( const DRefPointer< T > & rp ) : m_pObj( rp.m_pObj ) { if( m_pObj ) m_pObj->addref(); }
	~DRefPointer() { if( m_pObj ) { m_pObj->release(); m_pObj = NULL; } }

	inline DRefPointer< T > & operator = ( const DRefPointer< T > & rp )
	{
		// if( this == &rp )
		if( m_pObj == rp.m_pObj )
			return *this;

		if( m_pObj != NULL ) m_pObj->release();
		m_pObj = rp.m_pObj;
		if( m_pObj ) m_pObj->addref();
		return *this;
	}
	inline DRefPointer< T > & operator = ( T * p )
	{
		if( m_pObj == p )
			return *this;

		if( m_pObj ) m_pObj->release();
		m_pObj = p;
		if( m_pObj ) m_pObj->addref();
		return *this;
	}

	inline T * operator -> () { return m_pObj; }
	inline const T * operator -> () const { return m_pObj; }
	// inline operator T * () const { return m_pObj; }

	inline const T * GetPointer() const { return m_pObj; }
	inline T * GetPointer() { return m_pObj; }

	inline bool operator == ( const DRefPointer< T > & rp ) const { return m_pObj == rp.m_pObj; }
	inline bool operator == ( const T * p ) const { return m_pObj == p; }
	inline bool operator == ( int p ) const { return m_pObj == (T *)(intptr_t)p; }  // for NULL
	inline bool operator == ( long p ) const { return m_pObj == (T *)(intptr_t)p; }  // for NULL
	inline bool operator != ( const DRefPointer< T > & rp ) const { return m_pObj != rp.m_pObj; }
	inline bool operator != ( const T * p ) const { return m_pObj != p; }
	inline bool operator != ( int p ) const { return m_pObj != (T *)(intptr_t)p; }  // for NULL
	inline bool operator != ( long p ) const { return m_pObj != (T *)(intptr_t)p; }  // for NULL
	inline operator bool () const { return m_pObj != NULL; }
	inline bool operator ! () const { return m_pObj == NULL; }
	inline bool operator < ( const DRefPointer< T > & rp ) const { return m_pObj < rp.m_pObj; }

protected:
	T * m_pObj;
};

// 注意不要出现菱形继承的情况，确保子类只继承一个DRefObj，
// 继承体系中一般使用接口继承，接口如果需要引用计数，可以继承DRefInterface通过构造方式注入
// class ITest : public DRefInterface { ITest( DRefObj * ref ) : DRefInterface( ref ) {} };
// class MyTest : public ITest, public DRefObj { MyTest() : ITest( this ) {} };
// 此时可以使用DRefInterfacePointer对DRefInterface对象进行操作：DRefInterfacePointer<ITest> ptr = (ITest *) new MyTest();

template< class T > class DRefInterfacePointer;

class DRefInterface
{
public:
	// 不应该使用这个函数，判断引用计数的数量是不可靠的
	// DSL_DEPRECATED inline int GetRefCount() const { return m_obj->GetRefCount(); }
	DSL_DEPRECATED inline const DRefObj * GetRefObj() const { return m_obj; } // 也就只能用来调用 m_obj->GetRefCount();

protected:
	DRefInterface( DRefObj * obj ) : m_obj( obj ) {}
	virtual ~DRefInterface() {}   // 注意：子类的析构函数也应该是protected（！！）

private:
	// 外部不应该直接调用这两个函数，应该通过DRefInterfacePointer来访问，子类也不应该调用
	template< class T > friend class DRefInterfacePointer;
	inline int addref2() { return m_obj->addref(); }   // 不能跟DRefObj重名，多继承时会冲突
	inline int release2() { return m_obj->release(); }

private:
	DRefObj * m_obj;  // 一般都是指向自己，且不为NULL，这里不能使用DRefInterfacePointer，否则会环形引用，释放不了
};

template< class T >
class DRefInterfacePointer
{
public:
	DRefInterfacePointer( T * p = NULL ) : m_pObj( p ) { if( m_pObj ) m_pObj->addref2(); }
	DRefInterfacePointer( const DRefInterfacePointer< T > & rp ) : m_pObj( rp.m_pObj ) { if( m_pObj ) m_pObj->addref2(); }
	~DRefInterfacePointer() { if( m_pObj ) { m_pObj->release2(); m_pObj = NULL; } }

	inline DRefInterfacePointer< T > & operator = ( const DRefInterfacePointer< T > & rp )
	{
		// if( this == &rp )
		if( m_pObj == rp.m_pObj )
			return *this;

		if( m_pObj != NULL ) m_pObj->release2();
		m_pObj = rp.m_pObj;
		if( m_pObj ) m_pObj->addref2();
		return *this;
	}
	inline DRefInterfacePointer< T > & operator = ( T * p )
	{
		if( m_pObj == p )
			return *this;

		if( m_pObj ) m_pObj->release2();
		m_pObj = p;
		if( m_pObj ) m_pObj->addref2();
		return *this;
	}

	inline T * operator -> () { return m_pObj; }
	inline const T * operator -> () const { return m_pObj; }
	// inline operator T * () const { return m_pObj; }

	inline const T * GetPointer() const { return m_pObj; }
	inline T * GetPointer() { return m_pObj; }

	inline bool operator == ( const DRefInterfacePointer< T > & rp ) const { return m_pObj == rp.m_pObj; }
	inline bool operator == ( const T * p ) const { return m_pObj == p; }
	inline bool operator == ( int p ) const { return m_pObj == (T *)(intptr_t)p; } // for NULL
	inline bool operator != ( const DRefInterfacePointer< T > & rp ) const { return m_pObj != rp.m_pObj; }
	inline bool operator != ( const T * p ) const { return m_pObj != p; }
	inline bool operator != ( int p ) const { return m_pObj != (T *)(intptr_t)p; } // for NULL
	inline operator bool () const { return m_pObj != NULL; }
	inline bool operator ! () const { return m_pObj == NULL; }
	inline bool operator < ( const DRefInterfacePointer< T > & rp ) const { return m_pObj < rp.m_pObj; }

protected:
	T * m_pObj;
};

// TODO : 下一步可以考虑将 DRefInterfacePointer 合并到 DRefPointer 中，
// 学习boost的做法，通过trais判断 T 是否 DRefInterface 的子类，从而调用不同的方法

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif // INCLUDED_LIBSDL_DREFOBJ_H
