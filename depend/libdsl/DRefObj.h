/* Copyright (c) 2011, 浙江大华技术股份有限公司, All rights reserved.
 * 2011-09
 *
 * DRefObj.h : 引用计数对象
 */

#ifndef INCLUDED_LIBSDL_DREFOBJ_H
#define INCLUDED_LIBSDL_DREFOBJ_H

#include <libdsl/DAtomic.h>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

/* DRefObj：引用计数对象基类，初始计数 m_dwRefCount 的数值为 0 （！！！）
 * 子类继承时，应该使用虚继承，特别是接口类，这样可以处理菱形继承
 * 子类创建时必须使用new，必须赋值给引用计数指针DRef，例如 : DRef<DRefObjXXX> refXXX = new DRefObjXXX;
 * 子类要销毁只需要各个DRef析构或者赋空值，不允许显式delete（因此要有机制通知各个使用者释放）
 */

class DStr;
template< class T > class DRefPool;

// 指向 DRefObj 的子类的引用计数指针，自动增加和减少引用计数
template< class T >
class DRef
{
public:
	~DRef() { if( m_pObj ) { m_pObj->release(); m_pObj = NULL; } }

	DRef( T * p = NULL ) : m_pObj( p ) { if( m_pObj ) m_pObj->addref(); }
	inline DRef< T > & operator = ( T * p )
	{
		if( m_pObj == p )
			return *this;

		if( m_pObj ) m_pObj->release();
		m_pObj = p;
		if( m_pObj ) m_pObj->addref();
		return *this;
	}

	DRef( const DRef< T > & rp ) : m_pObj( rp.m_pObj ) { if( m_pObj ) m_pObj->addref(); }
	inline DRef< T > & operator = ( const DRef< T > & rp )
	{
		// if( this == &rp )
		if( m_pObj == rp.m_pObj )
			return *this;

		if( m_pObj != NULL ) m_pObj->release();
		m_pObj = rp.m_pObj;
		if( m_pObj ) m_pObj->addref();
		return *this;
	}

	// 子类普通指针赋值给父类智能指针（T2 应该是 T 的子类），传参数时可以自动转换类型，
	// 例如 T2 * rp2; DRef<T> rp1 = rp2; 或者 func( rp2 ); （原型：void func( const DRef<T> & rp );）
	template< class T2 >
	DRef( T2 * rp ) : m_pObj( rp ) { if( m_pObj ) m_pObj->addref(); }
	template< class T2 >
	inline DRef< T > & operator = ( T2 * rp )
	{
		T * tmp = rp; // 如果T2不是T的子类，这里会失败
		if( m_pObj == tmp )
			return *this;

		if( m_pObj ) m_pObj->release();
		m_pObj = tmp;
		if( m_pObj ) m_pObj->addref();
		return *this;
	}

	// 子类智能指针赋值给父类智能指针（T2 应该是 T 的子类），传参数时可以自动转换类型，
	// 例如 DRef<T2> rp2; DRef<T> rp1 = rp2; 或者 func( rp2 ); （原型：void func( const DRef<T> & rp );）
	template< class T2 >
	DRef( const DRef< T2 > & rp ) : m_pObj( rp.GetPointer() ) { if( m_pObj ) m_pObj->addref(); }
	template< class T2 >
	inline DRef< T > & operator = ( const DRef< T2 > & rp )
	{
		T * tmp = rp.GetPointer(); // 如果T2不是T的子类，这里会失败
		if( m_pObj == tmp )
			return *this;

		if( m_pObj ) m_pObj->release();
		m_pObj = tmp;
		if( m_pObj ) m_pObj->addref();
		return *this;
	}

	// 显式的强制类型转换，当父类对象智能指针指向子类对象时，想转换成子类对象智能指针，要显式调用此函数
	// 例如 T2是T的子类，DRef<T2> t2; DRef<T> t1 = t2; DRef<T2> t22 = t1.Cast<T2>();
	template< class T2 >
	inline DRef<T2> Cast() { return DRef<T2>( static_cast<T2 *>( m_pObj ) ); }

	inline T * operator -> () const { return m_pObj; }
	// inline operator T * () const { return m_pObj; }

	inline T * GetPointer() const { return m_pObj; }

	inline bool operator == ( const DRef< T > & rp ) const { return m_pObj == rp.m_pObj; }
	inline bool operator == ( const T * p ) const { return m_pObj == p; }
	inline bool operator == ( int p ) const { return m_pObj == (T *)(intptr_t)p; }  // for NULL
	inline bool operator == ( long p ) const { return m_pObj == (T *)(intptr_t)p; }  // for NULL
	inline bool operator != ( const DRef< T > & rp ) const { return m_pObj != rp.m_pObj; }
	inline bool operator != ( const T * p ) const { return m_pObj != p; }
	inline bool operator != ( int p ) const { return m_pObj != (T *)(intptr_t)p; }  // for NULL
	inline bool operator != ( long p ) const { return m_pObj != (T *)(intptr_t)p; }  // for NULL
	inline operator bool () const { return m_pObj != NULL; }
	inline bool operator ! () const { return m_pObj == NULL; }
	inline bool operator < ( const DRef< T > & rp ) const { return m_pObj < rp.m_pObj; }

protected:
	T * m_pObj;
};	

class LIBDSL_API DRefObj
{
public:
	// （调试）访问引用计数的值，仅供调试参考使用，正常业务逻辑不应该使用这个数据
	inline int GetRefCount() const { return m_ref.Read(); }
	// （调试）提供引用计数对象的基本信息，*附加*在str尾部，主要供调试和系统运维使用
	virtual void GetObjInfo( DStr * str ) const {}

	/***
	 * 内存池对象函数（用于对象缓存）
	 * 使用方式：
	 *		直接CreateObjFromPool()，无任何参数
	 *		这种方式：使用更加简单，但pool的生命周期等于全局变量，析构顺序无法掌控，请小心使用。
	 *
	 * 警告：使用内存池后，回收时不会调用 析构函数，但CreateObjFromPool时会调用 构造函数。
	***/
	template< class T >
	static DRef<T> CreateObjFromPool()
	{
		static DRef< DRefPool<T> >g_pPool = 0;
		if(g_pPool == 0)
			g_pPool = new DRefPool<T>;
		return g_pPool->GetObject();
	}
	// 重置对象（即手动初始化），当使用pool的时候，会调用该函数做初始化
	// 警告：如果子类重载该函数，需要手动调用基类的函数
	virtual void Reset(){ }
	/*******************************************************************************************/

protected:
	DRefObj() { m_pPool = NULL; }
	/* 仅用于兼容原DRefInterface定义 */ 
	DSL_DEPRECATED DRefObj( DRefObj * obj ) { m_pPool = NULL; }
	// 在拷贝构造中，m_ref 不能被替换
	DRefObj( const DRefObj & obj )
	{ 
		m_pPoolRef = obj.m_pPoolRef; 
		m_pPool = obj.m_pPool; 
	}
	DRefObj& operator=(const DRefObj& obj){
		m_pPoolRef = obj.m_pPoolRef; 
		m_pPool = obj.m_pPool; 
		return *this;
	}
	// 警告：使用内存池后，回收时不会调用 析构函数，但CreateObjFromPool时会调用 构造函数。
	virtual ~DRefObj() {}   // 注意：子类的析构函数也应该是protected（！！）
	virtual void destroy(); // 引用计数减为0时调用，一般不需要重载，当前只用于pool

private:
	// 外部不应该直接调用这两个函数，应该通过DRef来访问，子类也不应该调用
	template< class T > friend class DRef;
	inline int addref() { return m_ref.Inc(); }
	inline int release() { int r = m_ref.Dec(); if( r == 0 ) destroy(); return r; }

private:
	template< class T > friend class DRefPool;
	// 当参数pPoolRef为NULL时，表示解除obj对pool的引用问题，用于释放pool。
	inline void setpool( DRefPool< DRefObj > * pPool, DRefObj* pPoolRef ) 
	{ 
		m_pPool = pPool; 
		m_pPoolRef = (DRefObj*)pPoolRef;
	} 
	
private:
	DAtomic m_ref;
	DRef<DRefObj> m_pPoolRef;
	DRefPool< DRefObj >* m_pPool;
};



// 【1.0到1.1移植说明】
// （1）原来继承DRefObj的，一般情况下应调整为虚继承，例如：class XXXX : virtual public DRefObj
// （2）原来引用计数指针DRefPointer，需要改为DRef
//      或者增加一个定义： #define DRefPointer DRef 
// （3）原来继承DRefInterface的，需要改为虚继承 DRefObj，
//      如果原来同时继承DRefInterface和DRefObj，需要改为只虚继承一次DRefObj
// （4）原来使用DRefInterfacePointer的，需要改为DRef
//      或者增加一个定义： #define DRefInterfacePointer DRef 

// NOTE : 通过虚继承DRefObj的方式来处理菱形继承的问题，不要引入额外的复杂性
// 因此不再需要区分引用计数对象（DRefObj）和引用计数接口（DRefInterface），
// 统一使用DRefObj和DRef即可，但需要使用虚继承

#if 0

template< class T > class DSL_DEPRECATED DRefInterfacePointer;

class DSL_DEPRECATED DRefInterface
{
public:
	// 不应该使用这个函数，判断引用计数的数量是不可靠的
	// DSL_DEPRECATED inline int GetRefCount() const { return m_obj->GetRefCount(); }
	DSL_DEPRECATED inline const DRefObj * GetRefObj() const { return m_obj; } // 也就只能用来调用 m_obj->GetRefCount();

protected:
	DSL_DEPRECATED DRefInterface( DRefObj * obj ) : m_obj( obj ) {}
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
class DSL_DEPRECATED DRefInterfacePointer
{
public:
	DSL_DEPRECATED DRefInterfacePointer( T * p = NULL ) : m_pObj( p ) { if( m_pObj ) m_pObj->addref2(); }
	DSL_DEPRECATED DRefInterfacePointer( const DRefInterfacePointer< T > & rp ) : m_pObj( rp.m_pObj ) { if( m_pObj ) m_pObj->addref2(); }
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

#endif // #if 0

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif // INCLUDED_LIBSDL_DREFOBJ_H

