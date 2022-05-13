#pragma once
#define NOMINMAX
#include <string>
#include <vector>
#include <utility>
#include "dbhierarchy/Dbnode.h"
#include "../DButils/queries.h"
#include "../DButils/CLprinter.h"
#include <cstdint>

// Yes, you heard that right, no crosscompat!
#ifdef _WIN32
#include <Windows.h>
#else
#error "No compatibility, sorry!"
#endif


#include <iostream>
#include "../defines/DBkeys.h"


enum class DBcontext : char {
	DIR_TREE,
	TABLE_VIEW,
	SCHEMA_VIEW,
	QUERY_TOOL
};


class DBmanager
{
public:

	explicit DBmanager(PGconn*& connection) : res(nullptr), conn(connection), selected(0, 0, 0), curPos(0)
	{
		root = Dbnode<NODE::ROOT>("ROOT");
		using uint = uint64_t;
		using lint = int64_t;

		std::vector<std::string> schemas;

		setState(DBcontext::DIR_TREE);

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
			auto query = query::string_format<char const*>("SELECT table_name FROM information_schema.tables WHERE table_schema = '%s';", schema.c_str());
			query::atomicQuery(query.c_str(), res, connection);
			query::queryRes extract(res);

			for (lint i = 0; i < extract.rows; ++i)
			{
				auto tname = PQgetvalue(res, i, 0);
				root[schema].addChildren(tname);
			}


#ifdef _DEBUG
			std::cout << "listing tables for schema " << schema << ": " << std::endl;
			printUtil.printTable(res);
#endif	
		}

		setHide(false);
		refreshScreen();
		CLprinter::setPos(0, 0);

	}

	~DBmanager()
	{
		PQclear(res);
		PQfinish(conn);
	}

	void setState(DBcontext state)
	{
		switch (state)	//Make relevant changes to the UI and to other class attributes in order to make it correctly reflect the current state.
		{
		case DBcontext::DIR_TREE:
			CLprinter::hideCursor(false);
			break;
		case DBcontext::TABLE_VIEW:
			break;
		case DBcontext::SCHEMA_VIEW:
			break;
		case DBcontext::QUERY_TOOL:
			break;
		default:
			break;
		}
		context = state;
	}

	void setHide(bool state)
	{
		isHidingPrivate = state;
		bounds.first = 3 * state;
		bounds.second = root.getChildren().size() - 1;
		std::get<1>(selected) = bounds.first;
	}

	void printFS()
	{

		auto sel = std::string("");
		switch ( std::get<0>(selected) )
		{
		case 0:
			sel = root.getName();
			break;
		case 1:
			sel = root[std::get<1>(selected)].getName();
			break;
		case 2:
			sel = root[std::get<1>(selected)][std::get<2>(selected)].getName();
			break;
		default:
			assert(false);
		}

		root.printRecursive(sel, outBuf, isHidingPrivate, curPos);
		std::cout << outBuf.str(); outBuf.str(std::string());

	}

	void refreshScreen()
	{
		std::system("CLS");
		
		switch (context)
		{
		case DBcontext::DIR_TREE:
			printUtil.updateHeader("DB Directory Tree");
			printUtil.printHeader();
			printFS();
			CLprinter::setPos(0, std::max(0, curPos - 10));
			break;
		case DBcontext::TABLE_VIEW:
			printUtil.updateHeader("Table View");
			//TODO
			break;
		case DBcontext::SCHEMA_VIEW:
			printUtil.updateHeader("Schema View");
			//TODO
			break;
		case DBcontext::QUERY_TOOL:
			printUtil.updateHeader("Query Tool");
			//TODO
			break;
		default:
			assert("Invalid State");
			break;
		}
	}

	void handleKeyboard(int code)
	{
		switch (context)
		{
		case DBcontext::DIR_TREE:
			switch (code)
			{
			case H_KEY:
				setHide(!isHidingPrivate);
				break;
			case W_KEY:
			case UP_KEY:
				switch (std::get<0>(selected))
				{
				case 0:
					break;
				case 1:
					std::get<1>(selected) = std::max(bounds.first, std::get<1>(selected) - 1);
					std::get<2>(selected) = 0;
					break;
				case 2:
					std::get<2>(selected) = std::max(0, std::get<2>(selected) - 1);
					break;
				default:
					break;
				}
				break;
			case A_KEY:
			case LEFT_KEY:
				std::get<0>(selected) = std::max(0, std::get<0>(selected) - 1);
				break;
			case S_KEY:
			case DOWN_KEY:
				switch (std::get<0>(selected))
				{
				case 0:
					break;
				case 1:
					std::get<1>(selected) = std::min(bounds.second, std::get<1>(selected) + 1);
					std::get<2>(selected) = 0;
					break;
				case 2:
					std::get<2>(selected) = std::min(static_cast<int>(root[std::get<1>(selected)].getChildren().size()) - 1, std::get<2>(selected) + 1);
					break;
				default:
					break;
				}
				break;
			case D_KEY:
			case RIGHT_KEY:
				switch (std::get<0>(selected))
				{
				case 0:
					if (root.getChildren().size() == 0)
						break;
					std::get<0>(selected) = std::min(2, std::get<0>(selected) + 1);
					break;
				case 1:
					if (root[std::get<1>(selected)].getChildren().size() == 0)
						break;
					std::get<0>(selected) = std::min(2, std::get<0>(selected) + 1);
					break;
				case 2:
					std::get<0>(selected) = std::min(2, std::get<0>(selected) + 1);
					break;
				}
				break;
			default:
				break;
			}
			if (bounds.first > bounds.second)
			{
				std::system("CLS");
				std::cout << "No non-system tables in this Database!" << std::endl;
				break;
			}
			
			break;
		case DBcontext::TABLE_VIEW:
			std::cerr << "context unimplemented!" << std::endl;
			break;
		case DBcontext::SCHEMA_VIEW:
			std::cerr << "context unimplemented!" << std::endl;
			break;
		case DBcontext::QUERY_TOOL:
			std::cerr << "context unimplemented!" << std::endl;
			break;
		default:
			std::cerr << "context unhandled!" << std::endl;
			break;
		}
		refreshScreen();
	}

private:
	Dbnode<NODE::ROOT> root;
	PGresult* res;
	PGconn* conn;
	CLprinter printUtil;
	std::ostringstream outBuf;
	std::tuple<uint16_t, uint16_t, uint16_t> selected;
	std::pair<int, int> bounds;
	static DBcontext context;
	bool isHidingPrivate;
	int curPos;
};