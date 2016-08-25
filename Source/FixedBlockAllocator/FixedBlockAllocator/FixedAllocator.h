#pragma once

#include <algorithm>
#include <new>

template<typename T> struct FixedBlockMemoryManager
{
	template<int N>
	explicit FixedBlockMemoryManager( char(&storage)[N] );
	FixedBlockMemoryManager( const FixedBlockMemoryManager& ) = delete;
	~FixedBlockMemoryManager( ) = default;

	void operator=( const FixedBlockMemoryManager& ) = delete;

	void* Allocate( size_t size );
	size_t BlockSize( ) const noexcept { return m_blockSize; }
	size_t Capacity( ) const;
	void Clear( );
	void Deallocate( void* );
	bool Empty( ) const noexcept { return m_blockSize == 0; }

private:
	struct FreeBlock 
	{
		FreeBlock* m_next;
	};
	FreeBlock* m_freeList;
	size_t m_blockSize;
	T m_storage;
};

template<typename T>
template<int N>
FixedBlockMemoryManager<T>::FixedBlockMemoryManager( char(&storage)[N] ) :
	m_freeList( nullptr ), m_blockSize( 0 ), m_storage( storage )
{

}

template<typename T>
void* FixedBlockMemoryManager<T>::Allocate( size_t size )
{
	if ( Empty( ) )
	{
		m_freeList = reinterpret_cast<FreeBlock*>(m_storage.Allocate( size ));
		m_blockSize = size;
		if ( Empty( ) )
		{
			throw std::bad_alloc( );
		}
		if ( size != m_blockSize )
		{
			throw std::bad_alloc( );
		}
	}
		
	auto p = m_freeList;
	m_freeList = m_freeList->m_next;
	return p;
}

template<typename T>
void FixedBlockMemoryManager<T>::Deallocate( void* p )
{
	if ( p == nullptr )
	{
		return;
	}

	auto fp = reinterpret_cast<FreeBlock*>(p);
	fp->m_next = m_freeList;
	m_freeList = fp;
}

template<typename T>
size_t FixedBlockMemoryManager<T>::Capacity( ) const
{
	return m_storage.Capacity( );
}

template<typename T>
void FixedBlockMemoryManager<T>::Clear( )
{
	m_freeList = nullptr;
	m_storage.Clear( );
}

struct FixedStorageController
{
	template<int N>
	explicit FixedStorageController( char( &storage )[N] );
	FixedStorageController( const FixedStorageController& ) = delete;
	~FixedStorageController( ) = default;

	void operator=( const FixedStorageController& ) = delete;

	void* Allocate( size_t size );
	size_t BlockSize( ) const noexcept { return m_blockSize; }
	size_t Capacity( ) const;
	void Clear( );
	bool Empty( ) const noexcept { return m_blockSize == 0; }

private:
	void* m_storage;
	size_t m_storageSize;
	size_t m_blockSize;
};

template<int N>
FixedStorageController::FixedStorageController( char(&storage)[N] ) :
	m_storage( storage ), m_storageSize( N ), m_blockSize( 0 )
{

}

void* FixedStorageController::Allocate( size_t size )
{
	if ( !Empty( ) )
	{
		return nullptr;
	}

	m_blockSize = std::max( size, sizeof( void* ) );
	size_t count = Capacity( );

	if ( count == 0 )
	{
		return nullptr;
	}

	char *p = nullptr;
	for ( p = (char*)m_storage; count > 1; --count, p += size )
	{
		*reinterpret_cast<char**>(p) = p + size;
	}
	*reinterpret_cast<char**>(p) = nullptr;
	return m_storage;
}

size_t FixedStorageController::Capacity( ) const
{
	return m_blockSize ? (m_storageSize / m_blockSize) : 0;
}

void FixedStorageController::Clear( )
{
	m_blockSize = 0;
}