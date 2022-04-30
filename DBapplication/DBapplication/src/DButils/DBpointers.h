#pragma once
#include <memory>
#include <libpq-fe.h>
#include <type_traits>


template<typename P>
struct DBDeleter
{
	void operator()(P* ptr);
};

template<typename P>
class DBptr {

	static_assert(std::is_same_v<P, PGconn> || std::is_same_v<P, PGresult>, "This wrapper is not intended for use for non-libpq pointers.");

public:
	DBptr(P* ptr) : connptr( std::unique_ptr<P, DBDeleter<P>> (ptr)) {};

	
	P* const& p() { return connptr.get(); }

	DBptr& operator=(P* ptr)
	{
		connptr.reset(ptr);
	}

private:
	std::unique_ptr<P, DBDeleter<P>> connptr;
};

template<typename P>
void DBDeleter<P>::operator()(P* ptr)
{
	if constexpr (std::is_same_v<P, PGconn>)
	{
		PQfinish(ptr);
	}
	else if constexpr (std::is_same_v<P, PGresult>)
	{
		PQclear(ptr);
	}
}