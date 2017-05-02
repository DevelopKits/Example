#ifndef INCLUDED_LIBSDL_DREFOBJ_H
#define INCLUDED_LIBSDL_DREFOBJ_H

#include <libdsl/DAtomic.h>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

/* DRefObj�����ü������󣬳�ʼ���� m_dwRefCount ����ֵΪ 0 ����������
 * ����ĳЩ����Ҫ�������ָ���⣬���������Ӧ����DRefPointerһ��ʹ�ã���DRefPointer�����ͷ�
 * DRefPointer<DRefObj> refPtr( new DRefObj );
 */

 class DRefInterface;
template< class T > class DRefPointer;

class DRefObj
{
public:
	// ��Ӧ��ʹ������������ж����ü����������ǲ��ɿ���
	DSL_DEPRECATED inline int GetRefCount() const { return m_ref.Read(); }

protected:
	DRefObj() {}
	virtual ~DRefObj() {}   // ע�⣺�������������ҲӦ����protected��������
	virtual void destroy(); // ȱʡʵ���ǣ�{ delete this; return; }��һ�㲻��Ҫ���أ���ǰֻ����pool

private:
	// �ⲿ��Ӧ��ֱ�ӵ���������������Ӧ��ͨ��DRefPointer�����ʣ�����Ҳ��Ӧ�õ���
	template< class T > friend class DRefPointer;
	friend class DRefInterface;
	inline int addref() { return m_ref.Inc(); }
	inline int release() { int r = m_ref.Dec(); if( r == 0 ) destroy(); return r; }

private:
	DAtomic m_ref;
};

/* ָ�� DRefObj ����������ü���ָ�룬�Զ����Ӻͼ������ü���
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

// ע�ⲻҪ�������μ̳е������ȷ������ֻ�̳�һ��DRefObj��
// �̳���ϵ��һ��ʹ�ýӿڼ̳У��ӿ������Ҫ���ü��������Լ̳�DRefInterfaceͨ�����췽ʽע��
// class ITest : public DRefInterface { ITest( DRefObj * ref ) : DRefInterface( ref ) {} };
// class MyTest : public ITest, public DRefObj { MyTest() : ITest( this ) {} };
// ��ʱ����ʹ��DRefInterfacePointer��DRefInterface������в�����DRefInterfacePointer<ITest> ptr = (ITest *) new MyTest();

template< class T > class DRefInterfacePointer;

class DRefInterface
{
public:
	// ��Ӧ��ʹ������������ж����ü����������ǲ��ɿ���
	// DSL_DEPRECATED inline int GetRefCount() const { return m_obj->GetRefCount(); }
	DSL_DEPRECATED inline const DRefObj * GetRefObj() const { return m_obj; } // Ҳ��ֻ���������� m_obj->GetRefCount();

protected:
	DRefInterface( DRefObj * obj ) : m_obj( obj ) {}
	virtual ~DRefInterface() {}   // ע�⣺�������������ҲӦ����protected��������

private:
	// �ⲿ��Ӧ��ֱ�ӵ���������������Ӧ��ͨ��DRefInterfacePointer�����ʣ�����Ҳ��Ӧ�õ���
	template< class T > friend class DRefInterfacePointer;
	inline int addref2() { return m_obj->addref(); }   // ���ܸ�DRefObj��������̳�ʱ���ͻ
	inline int release2() { return m_obj->release(); }

private:
	DRefObj * m_obj;  // һ�㶼��ָ���Լ����Ҳ�ΪNULL�����ﲻ��ʹ��DRefInterfacePointer������ỷ�����ã��ͷŲ���
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

// TODO : ��һ�����Կ��ǽ� DRefInterfacePointer �ϲ��� DRefPointer �У�
// ѧϰboost��������ͨ��trais�ж� T �Ƿ� DRefInterface �����࣬�Ӷ����ò�ͬ�ķ���

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif // INCLUDED_LIBSDL_DREFOBJ_H
