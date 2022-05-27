#pragma once
#include "libpq-fe.h"
#include "cstdint"


namespace paths
{
	enum VehicleType : char
	{
		PLANE = 1,
		SHIP = 2,
		CAR = 4
	};

	struct pathOptions
	{

		char allowedVehicles;

	};

	struct destination
	{
		VehicleType vehicle;
		int64_t from;
		int64_t to;
		double fee;
		double distance;
		double heuristic;

	public:
		inline destination(paths::VehicleType const vehicleType, int64_t const from_code, int64_t const to_code, double const distance_val, double const fee, double const heuristic) 
			: vehicle(vehicleType), from(from_code), to(to_code), distance(distance_val), fee(fee), heuristic(heuristic) {}
		inline destination(destination const& other) : vehicle(other.vehicle), from(other.from), to(other.to), distance(other.distance), fee(other.fee), heuristic(other.heuristic) {}
	};
}

class Pathfinder
{
public:

	Pathfinder(PGconn*& conn, PGresult*& res) : conn(conn), res(res) {};
	void pathfind(int64_t from_code, int64_t to_code);

private:
	PGconn* conn;
	PGresult* res;

};
