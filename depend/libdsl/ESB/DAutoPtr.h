/* Copyright (c) 2013, 浙江大华技术股份有限公司, All rights reserved.
* 2013-10
*
* 版本    ：1.0
* DTrader.h : 通用智能指针
* 作者    ：吕少卿
* 完成日期：2014年10月10日
*
* 当前版本：1.0
*/

#pragma once


BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

#include <stdio.h>
#include <cassert>

template< typename T >
class DAutoPtr
{
public:
	DAutoPtr() throw() :
		m_p( NULL )
	{
	}
	template< typename TSrc >
	DAutoPtr( DAutoPtr< TSrc >& p ) throw()
	{
		m_p = p.Detach();  // Transfer ownership
	}

    DAutoPtr( DAutoPtr< T >& p ) throw()
	{
		m_p = p.Detach();  // Transfer ownership
	}

    DAutoPtr( T* p ) throw() :
		m_p( p )
	{
	}

	~DAutoPtr() throw()
	{
		Free();
	}

	// Templated version to allow pBase = pDerived
	template< typename TSrc >
	DAutoPtr< T >& operator=( DAutoPtr< TSrc >& p ) throw()
	{
        if(m_p==p.m_p)
        {
            // This means that two DAutoPtrs of two different types had the same m_p in them
            // which is never correct
            assert(false);
        }
        else
        {
		    Free();
		    Attach( p.Detach() );  // Transfer ownership
        }
		return( *this );
	}

    DAutoPtr< T >& operator=( T* p ) throw()
	{
		if(m_p != p)
		{
			Free();
			Attach( p );  // Transfer ownership
		}

		return( *this );
	}

    DAutoPtr< T >& operator=( DAutoPtr< T >& p ) throw()
	{
        if(*this==p)
        {
            if(this!=&p)
            {
                // If this assert fires, it means you attempted to assign one DAutoPtr to another when they both contained 
                // a pointer to the same underlying object. This means a bug in your code, since your object will get 
                // double-deleted. 
#ifdef ATL_AUTOPTR_ASSIGNMENT_ASSERT
                assert(FALSE);
#endif

                // For safety, we are going to detach the other DAutoPtr to avoid a double-free. Your code still
                // has a bug, though.
                p.Detach();
            }
            else
            {
                // Alternatively, this branch means that you are assigning a DAutoPtr to itself, which is
                // pointless but permissible

                // nothing to do
            }
        }
        else
        {
		    Free();
		    Attach( p.Detach() );  // Transfer ownership
        }
		return( *this );
	}

    // basic comparison operators
    bool operator!=(DAutoPtr<T>& p) const
    {
        return !operator==(p);
    }

    bool operator==(DAutoPtr<T>& p) const
    {
        return m_p==p.m_p;
    }

	operator T*() const throw()
	{
		return( m_p );
	}

	T* operator->() const throw()
	{
		assert( m_p != NULL );
		return( m_p );
	}

	// Attach to an existing pointer (takes ownership)
    void Attach( T* p ) throw()
	{
		assert( m_p == NULL );
		m_p = p;
	}

	// Detach the pointer (releases ownership)
    T* Detach() throw()
	{
		T* p;

		p = m_p;
		m_p = NULL;

		return( p );
	}

	// Delete the object pointed to, and set the pointer to NULL
    void Free() throw()
	{
		if(m_p)
		{
			delete m_p;
			m_p = NULL;
		}
	}

	bool IsNull()
	{
		if(m_p)
			return false;
		else
			return true;
	}

	T* GetPointer()
	{
		return m_p;
	}

public:
	T* m_p;
};


template< typename T >
class DAutoPtrEx : public DAutoPtr<T>
{
public:
	DAutoPtrEx<T>() : DAutoPtr<T>(new T)
	{

	}
};


/////////////////////////////////////////////////
END_NAMESPACE_DSL