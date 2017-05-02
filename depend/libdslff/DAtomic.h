#ifndef INCLUDED_LIBDSL_DATOMIC_H
#define INCLUDED_LIBDSL_DATOMIC_H

#include <libdsl/dslbase.h>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

class DAtomic
{
public:
	DAtomic() : m_val( 0 ) { }
	~DAtomic() { }

	inline int Read() const { return m_val; }
	inline void Set( int v ); 
	inline int Add( int v );  // return *old* value
	inline int Inc();         // return *new* value
	inline int Dec();         // return *new* value
	inline int Xchg( int v ); // return *old* value
	inline int XchgCmp( int v, int cmp ); // 比较，相等才赋值，总返回 *old* value

private:
	volatile int m_val;
};


inline void DAtomic::Set( int v )
{
#ifdef _WIN32
	InterlockedExchange( (LONG *)&m_val, v );
#else
	m_val = v;
#endif
	return;
}

inline int DAtomic::Add( int v )
{
#ifdef _WIN32
	return InterlockedExchangeAdd( (LONG *)&m_val, v );
#elif defined( __i386__ ) || defined( __x86_64__ )
	asm volatile ("lock; xaddl %0,%1"
		: "=r" (v), "=m" (m_val)
		: "0" (v), "m" (m_val)
		: "memory", "cc");
	return v;
#else
	return __sync_fetch_and_add(&m_val, v);
#endif
}

inline int DAtomic::Inc()
{
#ifdef _WIN32
	return InterlockedIncrement( (LONG *)&m_val );
#else
	return Add( 1 ) + 1;
#endif
}

inline int DAtomic::Dec()
{
#ifdef _WIN32
	return InterlockedDecrement( (LONG *)&m_val );
#else
	return Add( -1 ) - 1;
#endif
}

inline int DAtomic::Xchg( int v )
{
#ifdef _WIN32
	return InterlockedExchange( (LONG *)&m_val, v );
#elif defined( __i386__ ) || defined( __x86_64__ )
	int prev = v;
	asm volatile ("xchgl %0, %1"
		: "=r" (prev), "+m" (m_val)
		: "0" (prev));
	return prev;
#else
	__sync_synchronize();
	return __sync_lock_test_and_set(&m_val, v);
#endif
}

inline int DAtomic::XchgCmp( int v, int cmp )
{
#ifdef _WIN32
	return InterlockedCompareExchange( (LONG *)&m_val, v, cmp );
#elif defined( __i386__ ) || defined( __x86_64__ )
	int prev;
	asm volatile ("lock; cmpxchgl %1, %2"
		: "=a" (prev)
		: "r" (v), "m" (m_val), "0"(cmp)
		: "memory", "cc");
	return prev;
#else
	return __sync_val_compare_and_swap(&m_val, cmp, v);
#endif
}

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif // INCLUDED_LIBDSL_DATOMIC_H
