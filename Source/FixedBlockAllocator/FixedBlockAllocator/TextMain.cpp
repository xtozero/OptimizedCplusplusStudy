#include <iostream>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "FixedAllocator.h"

class MemMgrTester
{
	int m_contents;

public:
	explicit MemMgrTester( int c ) : m_contents( c ) {}

	static void* operator new(size_t size)
	{
		return m_mgr.Allocate( size );
	}

	static void operator delete(void* p)
	{
		m_mgr.Deallocate( p );
	}

	static char m_storage[4004];
	static FixedBlockMemoryManager<FixedStorageController> m_mgr;
};

char MemMgrTester::m_storage[4004];
FixedBlockMemoryManager<FixedStorageController> MemMgrTester::m_mgr( MemMgrTester::m_storage );

TEST_CASE( "FixedBlockMemoryManager allocate test" )
{
	REQUIRE( MemMgrTester::m_mgr.Capacity( ) == 0 );
	
	MemMgrTester* p = new MemMgrTester( 1 );

	REQUIRE_FALSE( MemMgrTester::m_mgr.Empty() );
	REQUIRE( MemMgrTester::m_mgr.BlockSize( ) == sizeof( MemMgrTester ) );
	REQUIRE( MemMgrTester::m_mgr.Capacity( ) == sizeof( MemMgrTester::m_storage ) / sizeof( void* ) );

	delete p;
}