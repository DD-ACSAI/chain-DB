#include "Pathfinder.h"
#include <functional>
#include "..\DButils\CLprinter.h"
#include "..\DButils\queries.h"
#include <queue>
#include <sstream>
#include <vector>
#include <map>
#include <unordered_set>
#include <stack>
#include <algorithm>


namespace paths
{
	using pathtup = std::tuple<double, paths::VehicleType, double>;

	void findConnections(PGresult*& res, std::vector<paths::destination>& results)
	{
		//Dictionary that maps NodeB -> ( { dist, vei, fee }, { dist, vei, fee } )
		std::map<int64_t, std::vector<pathtup>> distances;
		size_t nRows = PQntuples(res);

		if (nRows == 0)
			return;

		int64_t from = _strtoi64(PQgetvalue(res, 0, 1), nullptr, 10);

		for (size_t i = 0; i < nRows; ++i)
		{
			int64_t to   =	_strtoi64(PQgetvalue(res, i, 2), nullptr, 10);
			auto vCode   =	std::string(PQgetvalue(res, i, 0));
			auto fee     =	_strtoi64(PQgetvalue(res, i, 3), nullptr, 10);

			VehicleType vType = (vCode == "Car") ? CAR : ((vCode == "Plane") ? PLANE : SHIP);

			double distance = std::stod(PQgetvalue(res, i, 4));
			
			distances[to].emplace_back(distance, vType, fee);
		}

		for (auto& [key, vec] : distances)
		{
			std::sort(vec.begin(), vec.end());
			results.emplace_back( std::get<1>(vec.front()), from, key, std::get<0>(vec.front()), std::get<2>(vec.front()), 0.0);
		}
	}
}


void Pathfinder::pathfind(int64_t from_code, int64_t to_code, int64_t client)
{

	/* We set our Priority Queue*/
	static auto cmp = [](paths::destination const& left, paths::destination const& right) { return (left.distance + left.heuristic) > (right.distance + right.heuristic); };
	static std::priority_queue < paths::destination, std::vector<paths::destination>, decltype(cmp)> queue(cmp);
	while (!queue.empty()) queue.pop();

	/*
	
	Initializing CoI codes to Place Codes
	
	*/
	static std::stringstream querybuilder;
	querybuilder << "SELECT \"CenterOfInterest\".\"PlaceCode\" FROM public.\"CenterOfInterest\" WHERE \"CenterOfInterest\".\"ID\" = " << from_code << ";";

	if (!(query::atomicQuery(querybuilder.str().c_str(), res, conn) && PQntuples(res) > 0))
	{
		std::cerr << "The first Center of Interest does not exist, aborting!" << std::endl;
		PQclear(res);
		querybuilder.str(std::string());
		return;
	}

	const auto placecode_from = std::string(PQgetvalue(res, 0, 0));
	std::unordered_set<int64_t> reached{ _strtoi64(placecode_from.c_str(), nullptr, 10) };
	query::atomicQuery(querybuilder.str().c_str(), res, conn);
	querybuilder.str(std::string());

	PQclear(res);

	querybuilder << "SELECT \"CenterOfInterest\".\"PlaceCode\" FROM public.\"CenterOfInterest\" WHERE \"CenterOfInterest\".\"ID\" = " << to_code << ";";

	if (!(query::atomicQuery(querybuilder.str().c_str(), res, conn) && PQntuples(res) > 0))
	{
		std::cerr << "The second Center of Interest does not exist, aborting!" << std::endl;
		PQclear(res);
		querybuilder.str(std::string());
		return;
	}

	querybuilder.str(std::string());

	const auto placecode_to = std::string(PQgetvalue(res, 0, 0));
	PQclear(res);

	std::vector<paths::destination> destinations;

	//Utility printer
	CLprinter printer;

	queue.emplace(paths::CAR, _strtoi64(placecode_from.c_str(), nullptr, 10), _strtoi64(placecode_from.c_str(), nullptr, 10), 0.0, 0.0, 0.0);

	std::stack<paths::destination> explored;

	while(!queue.empty()) {
		explored.emplace(queue.top());
		
		if (explored.top().to == _strtoi64(placecode_to.c_str(), nullptr, 10))
		{
			break;
		}

		queue.pop();
		
		querybuilder << "SELECT * FROM \"Connection\" WHERE \"Connection\".\"PlaceA\" = " << explored.top().to << "; ";

		if (!(query::atomicQuery(querybuilder.str().c_str(), res, conn)))
		{
			std::cerr << "An error has occurred while expanding the frontier in our A* algorithm!" << std::endl;
			PQclear(res);
			querybuilder.str(std::string());
			while (!queue.empty()) queue.pop();
			while (!explored.empty()) explored.pop();
			destinations.clear();
			reached.clear();
			return;
		}

		querybuilder.str(std::string());
		destinations.clear();
		findConnections(res, destinations);

		for (auto& dest : destinations)
		{
			PQclear(res);

			querybuilder << "SELECT * FROM \"distance_places_ints\"(" << dest.to << ", " << placecode_to << ")";
			query::atomicQuery(querybuilder.str().c_str(), res, conn);
			querybuilder.str(std::string());

			dest.heuristic = 1.6 * std::stod(PQgetvalue(res, 0, 0));
			if (reached.find(dest.to) == reached.end())
			{
				dest.distance += explored.top().distance;
				queue.push(dest);
			}
		}

		reached.insert(explored.top().to);
	}

	std::vector<paths::destination> path;

	// Reconstruct
	if (explored.empty()) {
		std::cerr << "A* Failed to explore any node!" << std::endl;
		querybuilder.str(std::string());
		destinations.clear();
		PQclear(res);
		return;
	}

	path.emplace_back(explored.top());

	if (path.back().to != _strtoi64(placecode_to.c_str(), nullptr, 10)) {
		std::cerr << "For some reason A* did not return a path with our beginning as the first node\n something went horribly wrong, terminating." << std::endl;
		querybuilder.str(std::string());
		while (!explored.empty()) explored.pop();
		destinations.clear();
		reached.clear();
		PQclear(res);
		return;
	}

	for (; !explored.empty(); explored.pop()) {
		paths::destination curr_destination = explored.top();

		if (curr_destination.to == path.back().from) {
			path.emplace_back(curr_destination);
		}
	}

	if (path.back().from != _strtoi64(placecode_from.c_str(), nullptr, 10)) {
		std::cerr << "For some reason after retrieveing it from A*, our path did not begin with the first node\n something went horribly wrong, terminating." << std::endl;
		querybuilder.str(std::string());
		destinations.clear();
		reached.clear();
		PQclear(res);
		return;
	}

	path.pop_back();
	std::reverse(path.begin(), path.end());

	size_t i = 0;


	std::cout << " Chosen path is: " << std::endl;
	for (auto const& node : path)
		{
			std::cout << " (" << i++ << "): " << node.from << " " << node.to << " " << node.distance << std::endl;
		}

	//We inject this route!
	querybuilder.str(std::string());
	querybuilder << "SELECT * FROM \"inject_route\"(" << from_code << ", " << to_code << ", "<<  client <<", ";

	std::stringstream arr1;
	std::stringstream arr2;
	std::stringstream arr3;

	arr1 << "\'{";
	arr2 << "\'{";
	arr3 << "\'{";

	for (auto const& node : path)
	{
		arr1 << node.from << ", ";
		arr2 << node.to   << ", ";

		switch (node.vehicle)
		{
		case paths::CAR:
			arr3 << "Car, ";
			break;
		case paths::SHIP:
			arr3 << "Ship, ";
			break;
		case paths::PLANE:
			arr3 << "Plane, ";
			break;
		default:
			break;
		}
	}

	arr1 << "}\'";
	arr2 << "}\'";
	arr3 << "}\'";

	std::string arr1_str = arr1.str();
	arr1_str.erase(arr1_str.size() - 4, 2);

	std::string arr2_str = arr2.str();
	arr2_str.erase(arr2_str.size() - 4, 2);

	std::string arr3_str = arr3.str();
	arr3_str.erase(arr3_str.size() - 4, 2);

	querybuilder << arr1_str << ", " << arr2_str << ", " << arr3_str << ")";

	
	int64_t ID = 0;
	if (query::atomicQuery(querybuilder.str().c_str(), res, conn) && PQntuples(res) > 0)
	{
		ID = _strtoi64(PQgetvalue(res, 0, 0), nullptr, 10);
	}
	else
	{
		std::cerr << "\n An error has occurred on the Database while injecting the route, check the stack-trace for more info." << std::endl;
		querybuilder.str(std::string());
		destinations.clear();
		reached.clear();
		PQclear(res);
		return;
	}
	
	PQclear(res);
	querybuilder.str(std::string());

	querybuilder << "SELECT * FROM public.\"Contains\" WHERE \"Contains\".\"RouteCode\" = " << ID << "ORDER BY \"Contains\".\"Order\"";

	if (query::atomicQuery(querybuilder.str().c_str(), res, conn) && PQntuples(res) > 0)
	{
		std::cout << "\n A summary of the route (in terms of places):" << std::endl;
		printer.printTable(res);
	}
	else
	{
		std::cerr << "\n Application was unable to fetch route that was just inserted, yikes!" << std::endl;
	}

	PQclear(res);

	querybuilder.str(std::string());
	reached.clear();
	destinations.clear();
}
