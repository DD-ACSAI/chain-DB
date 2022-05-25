#pragma once
#define NOMINMAX
#include <string>
#include <algorithm>
#include <typeinfo>
#include <vector>
#include <array>
#include <utility>
#include <unordered_set>
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

//Well known Queries Headers
#include "queries/WKQuery.h"
#include "queries/Procedure.h"
#include "queries/Function.h"
#include "queries/Query.h"

// I\O stuff
#include <iostream>
#include <cctype>
#include <conio.h>
#include "../defines/DBkeys.h"
#include "../defines/clicolors.h"


enum class DBcontext : char {
	MAIN_MENU,
	DIR_TREE,
	TABLE_VIEW,
	QUERY_TOOL,
	WK_QUERIES,
	PATHFINDER
};

#define AS_STR(X) std::string(X)

class DBmanager
{
public:

	explicit DBmanager(PGconn*& connection) : res(nullptr), conn(connection), selected_dir(0, 0, 0), 
		curPos(0), selected_wk(0), menu_options({ "Show Directory Tree", "Query Tool", "Well Known Queries" }), selected_menu_opt(0)
	{
		root = Dbnode<NODE::ROOT>("ROOT");
		using uint = uint64_t;
		using lint = int64_t;

		well_knowns.reserve(32);



		std::vector<std::string> schemas(12);

		using string_tup = std::pair<std::string, std::string>;
		{//Instantiating Well Knowns (Boilerplate warning!)

			//PROCEDURES
			{
				{
					std::array<string_tup, 4> argn{			//Create Connection
						std::make_pair(AS_STR("Place A"), AS_STR("int")),
						std::make_pair(AS_STR("Place B"), AS_STR("int")),
						std::make_pair(AS_STR("Allowed Vehicle"), AS_STR("vehicle_type")),
						std::make_pair(AS_STR("Fee"), AS_STR("real"))
					};
					well_knowns.emplace_back(std::make_unique<Procedure<4>>("Create Connection", "Add Connection", argn));
				}

				{
					std::array<string_tup, 2> argn{			//Create Ticket
						std::make_pair(AS_STR("Vehicle Code"), AS_STR("int")),
						std::make_pair(AS_STR("Depot Code"), AS_STR("int"))
					};
				
					well_knowns.emplace_back(std::make_unique<Procedure<2>>("Create Ticket", "Create Ticket", argn));
				}

				{
					std::array<string_tup, 1> argn{			//Delete Route
						std::make_pair(AS_STR("Route Code"), AS_STR("int")),
					};

					well_knowns.emplace_back(std::make_unique<Procedure<1>>("Delete Route", "Delete Route", argn));
				}

				{
					std::array<string_tup, 2> argn{			//Delete Shipment
						std::make_pair(AS_STR("Shipment Code"), AS_STR("int")),
						std::make_pair(AS_STR("From Code"), AS_STR("int"))
					};

					well_knowns.emplace_back(std::make_unique<Procedure<2>>("Delete Shipment", "Delete shipment", argn));
				}

				{
					std::array<string_tup, 1> argn{			//Unload Shipment
						std::make_pair(AS_STR("Shipment Code"), AS_STR("int")),
					};

					well_knowns.emplace_back(std::make_unique<Procedure<1>>("Unload Shipment", "Unload shipment", argn));
				}
			}

			{  //Functions

				{//Check Stock

					std::array<string_tup, 1> argn{
						std::make_pair(AS_STR("coi_code"), AS_STR("int"))
					};

					well_knowns.emplace_back(std::make_unique<Function<1>>("Check Stock", "check_stock", argn));

				}

				{//Find Connection Frequency

					std::array<string_tup, 3> argn{
						std::make_pair(AS_STR("Place A Code"), AS_STR("int")),
						std::make_pair(AS_STR("Place B Code"), AS_STR("int")),
						std::make_pair(AS_STR("Vehicle Type"), AS_STR("vehicle_type")),
					};

					well_knowns.emplace_back(std::make_unique<Function<3>>("Find Connection Frequency", "find_connection_frequency", argn));

				}

				{//Find Company Models

					std::array<string_tup, 1> argn{
						std::make_pair(AS_STR("Company Code"), AS_STR("int"))
					};

					well_knowns.emplace_back(std::make_unique<Function<1>>("FindCompany Models", "find_models", argn));

				}

				{//Find Route Frequency

					std::array<string_tup, 1> argn{
						std::make_pair(AS_STR("Route Code"), AS_STR("int"))
					};

					well_knowns.emplace_back(std::make_unique<Function<1>>("Find Route Frequency", "find_route_frequency", argn));

				}

				{//Get Client Shipments

					std::array<string_tup, 1> argn{
						std::make_pair(AS_STR("Client Code"), AS_STR("int"))
					};

					well_knowns.emplace_back(std::make_unique<Function<1>>("Get Client Frequency", "get_all_shipments", argn));

				}

				{//Get CoI Type

					std::array<string_tup, 1> argn{
						std::make_pair(AS_STR("CoI Code"), AS_STR("int"))
					};

					well_knowns.emplace_back(std::make_unique<Function<1>>("Get CoI Type", "get_coi_type", argn));

				}

				{//Check Ticket Legality

					std::array<string_tup, 1> argn{
						std::make_pair(AS_STR("Ticket Code"), AS_STR("int"))
					};

					well_knowns.emplace_back(std::make_unique<Function<1>>("Check Ticket Legality", "is_legal_ticket", argn));

				}

				{//Get Route Viewers

					std::array<string_tup, 1> argn{
						std::make_pair(AS_STR("Route Code"), AS_STR("int"))
					};

					well_knowns.emplace_back(std::make_unique<Function<1>>("See Route Viewers", "route_viewers", argn));

				}


			}


			well_knowns.emplace_back(std::make_unique<Query>("Show Models", "SELECT * FROM public.\"Model\""));
			well_knowns.emplace_back(std::make_unique<Query>("Find all Clients based in Rome", "SELECT * FROM \"Client\" WHERE \"Client\".\"CompanyCode\""
				"IN( SELECT \"Shipment\".\"ClientCode\" FROM \"Shipment\" JOIN \"Crossing\" as cr ON(\"Shipment\".\"ID\" = cr.\"ShipmentCode\")"
				"WHERE EXISTS( SELECT * FROM \"Place\" WHERE(\"Place\".\"ID\" = cr.\"PlaceA\" OR \"Place\".\"ID\" = cr.\"PlaceB\") AND \"Place\".\"Name\" = 'Rome'))"));
		}
		
		well_knowns.shrink_to_fit();

		setState(DBcontext::MAIN_MENU);

		{	// First query scope (frees locals at the end)
			query::atomicQuery("SELECT schema_name FROM information_schema.schemata;", res, connection);
			query::queryRes extract(res);

#ifdef _DEBUG
			std::cout << "listing schemas: " << "\n";
			printUtil.printTable(res);
#endif
			for (lint i = 0; i < extract.rows; ++i)
			{
				auto name = PQgetvalue(res, i, 0);
				schemas.emplace_back(name);
				root.addChildren(name);

#ifdef _DEBUG
				std::cout << schemas.at(i) << "\n";
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
			std::cout << "listing tables for schema " << schema << ": " << "\n";
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

	void setState(DBcontext state) //Make relevant changes to the UI and to other class attributes in order to make it correctly reflect the current state.
	{
		context = state;
		switch (state) //Trigger a set of changes based on the incoming state
		{
		case DBcontext::DIR_TREE:
			printUtil.updateHeader("DB Directory Tree");
			CLprinter::showCursor(false);
			break;
		case DBcontext::TABLE_VIEW:
		{
			printUtil.updateHeader("Table View");
			CLprinter::showCursor(false);
			currTab.tabSchema = root[std::get<1>(selected_dir)].getName();
			currTab.tabName = root[currTab.tabSchema][std::get<2>(selected_dir)].getName();

			query::atomicQuery(std::string("SELECT COUNT(*) FROM \"" + currTab.tabSchema + "\".\"" + currTab.tabName + "\"").c_str(), res, conn);
			query::queryRes extract(res);

			std::stringstream temp(PQgetvalue(extract.result, 0, 0));
			temp >> currTab.rowCount;


			std::cout << currTab.rowCount;
			break;
		}
		case DBcontext::QUERY_TOOL:
			printUtil.updateHeader("Query Tool");
			CLprinter::showCursor(true);
			handleQueryTool();
			break;
		case DBcontext::MAIN_MENU:
			printUtil.updateHeader("Main Menu");
			CLprinter::showCursor(false);
			break;
		case DBcontext::WK_QUERIES:
			printUtil.updateHeader("Well-Known Queries and Procedures");
			CLprinter::showCursor(false);
			handleWK();
			break;
		case DBcontext::PATHFINDER:
			printUtil.updateHeader("Best-Route Pathfinder");
			CLprinter::showCursor(false);
		default:
			break;
		}

	}

	void setHide(bool state)
	{

		isHidingPrivate = state;
		bounds.first = 3 * state;
		bounds.second = root.getChildren().size() - 1;
		std::get<1>(selected_dir) = bounds.first;
		std::get<2>(selected_dir) = 0;

	}

	void getFS()
	{

		auto sel = std::string("");
		switch ( std::get<0>(selected_dir) )
		{
		case 0:
			sel = root.getName();
			break;
		case 1:
			sel = root[std::get<1>(selected_dir)].getName();
			break;
		case 2:
			sel = root[std::get<1>(selected_dir)][std::get<2>(selected_dir)].getName();
			break;
		default:
			assert(false);
		}

		root.printRecursive(sel, outBuf, isHidingPrivate, curPos);
	}

	void printTableView() const
	{

		std::array<std::string, 2> phrases = { "Print Contents", "Show Statistics" };

		std::cout << "\n" << " ";

		for (std::size_t i = 0; i < phrases.size(); ++i)
		{
			if (i == currTab.selected_opt)
				std::cout << "(" << i << ") " << color::SELECTED << phrases[i] << color::RESET << "\n" << ' ';
			else 
				std::cout << "(" << i << ") " << phrases[i] << "\n" << ' ';
		}

	}

	void printMainMenu() const
	{
		std::cout << "\n";
		for (std::size_t i = 0; i < menu_options.size(); ++i)
		{
			if (i == selected_menu_opt)
				std::cout << " * " << color::SELECTED << menu_options[i] << color::RESET << "\n";
			else
				std::cout << " * " << menu_options[i] << "\n";
		}
	}

	void handleQueryTool()
	{

		std::system("CLS");
		std::string query;

		for (;;)
		{
			printUtil.printHeader();
			std::cout << "\n" << "\n" << " Query: ";

			for (;;) {
				auto c = _getch();
				
				c = parseKey(c);
				
				if (c == ENTER_KEY)
				{
					break;
				}
				else if (c == DELETE_KEY)
				{
					if (!query.empty())
					{
						query.pop_back();
						std::cout << "\b \b";
					}
					continue;
				}
				else if (c == ESC_KEY)
					goto EXIT;
				else if (c == UP_KEY || c == LEFT_KEY || c == RIGHT_KEY || c == DOWN_KEY)
					continue;
				query.push_back(static_cast<char>(c));
				std::cout << static_cast<char>(c);

				auto out_str = query::parseQuery(query);
				if (out_str != query)
				{
					std::cout << "\r" << " Query: " << out_str;
				}
					
			}

			if (query.empty())
			{
				std::cerr << "  Empty query received, please, at least type something!" << "\n";
				_getch();
				std::system("CLS");
				continue;
			}

			auto target_buff = new char[ 2 * static_cast<int>(query.size()) + 1 ];
			int error;

			PQescapeStringConn(conn, target_buff, query.c_str(), query.size(), &error);

			std::cout << "\n\n";

			if(query::atomicQuery(target_buff, res, conn))
				printUtil.printTable(res);

			delete[] target_buff;
			query.clear();

			_getch();
			std::system("CLS");
		}

	EXIT:
		setState(DBcontext::MAIN_MENU);
		refreshScreen();
	}

	void handleWK()
	{

		for (;;)
		{
			std::system("CLS");
			printUtil.printHeader();

			std::cout << "Well Known Queries: " << "\n" << "\n";

			size_t i = 0;
			for (auto const& wk : well_knowns)
			{
				if (i++ == selected_wk)
					std::cout << " * " << color::SELECTED << wk->getName() << color::RESET << "\n";
				else
					std::cout << " * " << wk->getName() << "\n";
			}

			auto c = parseKey(_getch());

			switch (c)
			{
			case ESC_KEY:
				goto EXIT;
				break;
			case ENTER_KEY:
				std::cout << "\n";
				well_knowns[selected_wk]->execute(res, conn);
				_getch();
				break;
			case DOWN_KEY:	
			case S_KEY:
				selected_wk = std::min((int64_t)well_knowns.size() - 1, selected_wk + 1);
				break;
			case UP_KEY:
			case W_KEY:
				selected_wk = std::max(0i64, selected_wk - 1);
				break;
			default:
				break;
			}

		}

	EXIT:
		setState(DBcontext::MAIN_MENU);
		refreshScreen();
	}

	void refreshScreen()
	{

		switch (context)
		{
		case DBcontext::DIR_TREE:

			getFS();
			std::system("CLS");
			printUtil.printHeader();
			std::cout << outBuf.str(); outBuf.str(std::string());
			
			CLprinter::setPos(0, std::max(0, curPos - 10));

			break;
		case DBcontext::TABLE_VIEW:

			std::system("CLS");
			printUtil.printHeader();
			printTableView();

			break;
		case DBcontext::MAIN_MENU:

			
			std::system("CLS");
			printUtil.printHeader();

			printMainMenu();

			break;
		default:
			std::system("CLS");
			assert("Invalid State");
			break;
		}

	}

	/**
	* \brief Main method of DBManager that handles the keyboard input based on which state the application is currently in.
	* 
	*	The switch here is extremely long-winded and suffers in readability, however, once it is broken down in it's individual 
	*	component what it does is fairly understandable and can't frankly be done in a different way except perhaps breaking it 
	*	down into a number of different methods/utility functions to reduce the number of code in a single method, however,
	*	we refrain from doing this due to the time constraints on the project.
	*	
	*	The method is made up of two nested switches, the first one determines in which context (or state) the application is currently in,
	*	the second one handles a number of keypresses in a way that depends on the context.
	* 
	* \param int code: An integer encoding the user input.
	* \relates DBkeys.h
	*/
	void handleKeyboard(int code)		//Please DO NOT look at this thing unless needed :)
	{
		switch (context)
		{
		case DBcontext::DIR_TREE:
			switch (code)
			{
			case ENTER_KEY:
				switch (std::get<0>(selected_dir))
				{
				case 0:
					break;
				case 1:
					setState(DBcontext::MAIN_MENU);
					break;
				case 2:
					setState(DBcontext::TABLE_VIEW);
					break;
				default:
					break;
				}
				break;
			case ESC_KEY:
				setState(DBcontext::MAIN_MENU);
				refreshScreen();
				break;
			case H_KEY:
				setHide(!isHidingPrivate);
				break;
			case W_KEY:
			case UP_KEY:
				switch (std::get<0>(selected_dir))
				{
				case 0:
					break;
				case 1:
					std::get<1>(selected_dir) = std::max(bounds.first, std::get<1>(selected_dir) - 1);
					std::get<2>(selected_dir) = 0;
					break;
				case 2:
					std::get<2>(selected_dir) = std::max(0i64, std::get<2>(selected_dir) - 1);
					break;
				default:
					break;
				}
				break;
			case A_KEY:
			case LEFT_KEY:
				std::get<0>(selected_dir) = std::max(0i64, std::get<0>(selected_dir) - 1);
				break;
			case S_KEY:
			case DOWN_KEY:
				switch (std::get<0>(selected_dir))
				{
				case 0:
					break;
				case 1:
					std::get<1>(selected_dir) = std::min(bounds.second, std::get<1>(selected_dir) + 1);
					std::get<2>(selected_dir) = 0;
					break;
				case 2:
					std::get<2>(selected_dir) = std::min((int64_t)root[std::get<1>(selected_dir)].getChildren().size() - 1, std::get<2>(selected_dir) + 1);
					break;
				default:
					break;
				}
				break;
			case D_KEY:
			case RIGHT_KEY:
				switch (std::get<0>(selected_dir))
				{
				case 0:
					if (root.getChildren().size() == 0)
						break;
					std::get<0>(selected_dir) = std::min(2i64, std::get<0>(selected_dir) + 1);
					break;
				case 1:
					if (root[std::get<1>(selected_dir)].getChildren().size() == 0)
						break;
					std::get<0>(selected_dir) = std::min(2i64, std::get<0>(selected_dir) + 1);
					break;
				case 2:
					std::get<0>(selected_dir) = std::min(2i64, std::get<0>(selected_dir) + 1);
					break;
				}
				break;
			default:
				break;
			}
			if (bounds.first > bounds.second)
			{
				std::system("CLS");
				std::cout << "No non-system tables in this Database!" << "\n";
				break;
			}
			refreshScreen();
			break;
		case DBcontext::TABLE_VIEW:
			switch (code)
			{
			case W_KEY:
			case UP_KEY:
				currTab.selected_opt = std::max(0i64, (int64_t)currTab.selected_opt - 1);
				refreshScreen();
				break;
			case S_KEY:
			case DOWN_KEY:
				currTab.selected_opt = std::min(1i64, (int64_t)currTab.selected_opt + 1);
				refreshScreen();
				break;
			case ENTER_KEY:
				if (currTab.selected_opt == 0)
				{
					refreshScreen();
					query::atomicQuery(std::string("SELECT * FROM \"" + currTab.tabSchema + "\".\"" + currTab.tabName + "\"").c_str(), res, conn);
					printUtil.printTable(res);
					PQclear(res);
				}
				break;
			case ESC_KEY:
				setState(DBcontext::DIR_TREE);
				refreshScreen();
				break;
			}
			break;
		case DBcontext::MAIN_MENU:
			switch (code)
			{
			case W_KEY:
			case UP_KEY:
				selected_menu_opt = std::max(selected_menu_opt - 1, 0i64);
				break;
			case S_KEY:
			case DOWN_KEY:
				selected_menu_opt = std::min((int64_t) menu_options.size() - 1, selected_menu_opt + 1);
				
				break;
			case ENTER_KEY:
				if (selected_menu_opt == 0)
				{
					setState(DBcontext::DIR_TREE);
				} 
				else if (selected_menu_opt == 1)
				{
					setState(DBcontext::QUERY_TOOL);
				}
				else if (selected_menu_opt == 2)
				{
					setState(DBcontext::WK_QUERIES);
				}

			case ESC_KEY:
				//Find a way to kill the program
				break;
			}
			refreshScreen();
			break;
		case DBcontext::QUERY_TOOL:
			//Input here is handled differently in handleKeyboard() 
			break;
		case DBcontext::WK_QUERIES:
			//Input here is handled differently in handleWK() 
			break;
		default:
			std::cerr << "context unhandled!" << "\n";
			break;
		}
		
	}

private:
	struct tabViewAttr {
	public:
		size_t selected_opt;
		std::string tabName;
		std::string tabSchema;

		int rowCount;
		int recordSize;
		long long int recordBytes;

		tabViewAttr() : tabName("NULL"), tabSchema("NULL"), selected_opt(0), rowCount(0), recordSize(0), recordBytes(0) {}
	};

	Dbnode<NODE::ROOT> root;
	PGresult* res;
	PGconn* conn;
	CLprinter printUtil;
	std::ostringstream outBuf;
	std::tuple<int64_t, int64_t, int64_t> selected_dir;
	std::pair<int64_t, int64_t> bounds;
	static DBcontext context;
	bool isHidingPrivate;
	int curPos;
	tabViewAttr currTab;

	std::vector<std::unique_ptr<WKQuery>> well_knowns;
	int64_t selected_wk;

	std::array<std::string, 3> menu_options;
	int64_t selected_menu_opt;
};