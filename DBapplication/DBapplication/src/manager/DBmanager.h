#pragma once
#include <string>
#include <vector>
#include "dbhierarchy/Dbnode.h"
#include "../DButils/queries.h"
#include "../DButils/CLprinter.h"
#include "../DButils/DBpointers.h"
#include <cstdint>

#ifdef _DEBUG
#include <iostream>
#endif

class DBmanager
{
public:
	
	explicit DBmanager(PGconn*& connection) : res(nullptr), conn(connection)
	{
		root = Dbnode<NODE::ROOT>("ROOT");
		using uint = uint64_t;
		using lint = int64_t;

		std::vector<std::string> schemas;

		{	// First query scope (frees locals at the end)
			query::atomicQuery("SELECT schema_name FROM information_schema.schemata;", res, connection);
			query::queryRes extract(res);

#ifdef _DEBUG
			std::cout << "listing schemas: " << std::endl;
			printUtil.printTable(res);
#endif
			for (lint i = 0; i < extract.rows; ++i)
			{
				auto name = PQgetvalue(res, i, 0);
				schemas.emplace_back(name);
				root.addChildren(name);

#ifdef _DEBUG
				std::cout << schemas.at(i) << std::endl;
#endif
			}

		}

		for (auto const& schema : schemas)
		{
			auto query = query::string_format<char const*>("SELECT * FROM information_schema.tables WHERE table_schema = '%s';", schema.c_str());
			query::atomicQuery(query.c_str(), res, connection);
			query::queryRes extract(res);

			for (lint i = 0; i < extract.rows; ++i)
			{
				auto tname = PQgetvalue(res, i, 2);
				root[schema].addChildren(tname);
			}


#ifdef _DEBUG
			std::cout << "listing tables for schema " << schema << ": " << std::endl;
			printUtil.printTable(res);
#endif
		}
		 
#ifdef _DEBUG

		std::cout << "recursive print on ROOT node" << std::endl << std::endl;
		root.printRecursive();

#endif

	}
private:

	
	Dbnode<NODE::ROOT> root;
	PGresult* res;
	DBptr<PGconn> conn;
	CLprinter printUtil;
};

