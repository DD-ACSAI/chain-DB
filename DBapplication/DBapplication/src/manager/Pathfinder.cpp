#include "Pathfinder.h"
#include <functional>
#include "..\DButils\queries.h"


inline Pathfinder::Pathfinder(PGconn*& conn, PGresult*& res) : conn(conn), res(res) {}

void Pathfinder::pathfind(int64_t from_code, int64_t to_code)
{


	query::beginTransaction(conn);

	query::executeQuery("", res, conn);

	query::endTransaction(conn);
	PQclear(res);


}
