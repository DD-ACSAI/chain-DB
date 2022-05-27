#pragma once
#include "libpq-fe.h"
#include "cstdint"
#include <queue>

namespace paths
{
	enum VehicleType : char
	{
		CAR = 1,
		PLANE = 2,
		SHIP = 4
	};

	struct pathOptions
	{

		char allowedVehicles;

	};

	struct node
	{

	};
}

class Pathfinder
{
public:

	Pathfinder(PGconn*& conn, PGresult*& res);
	void pathfind(int64_t from_code, int64_t to_code);

private:
	PGconn* conn;
	PGresult* res;

};
