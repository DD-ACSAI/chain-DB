#pragma once
#define NOMINMAX
#include <string>
#include <algorithm>
#include <cctype>
#include <vector>
#include <array>
#include <utility>
#include <unordered_set>
#include "dbhierarchy/Dbnode.h"
#include "../DButils/queries.h"
#include "../DButils/CLprinter.h"
#include <cstdint>
#include <conio.h>

// Yes, you heard that right, no crosscompat!
#ifdef _WIN32
#include <Windows.h>
#else
#error "No compatibility, sorry!"
#endif


#include <iostream>
#include "../defines/DBkeys.h"
#include "../defines/clicolors.h"


enum class DBcontext : char {
	MAIN_MENU,
	DIR_TREE,
	TABLE_VIEW,
	QUERY_TOOL
};


class DBmanager
{
public:

	explicit DBmanager(PGconn*& connection) : res(nullptr), conn(connection), selected_dir(0, 0, 0), curPos(0), selected_menu_opt(0)
	{
		root = Dbnode<NODE::ROOT>("ROOT");
		using uint = uint64_t;
		using lint = int64_t;

		menu_options = { "Show Directory Tree", "Query Tool", "PLACEHOLDER" };

		std::vector<std::string> schemas;

		setState(DBcontext::MAIN_MENU);

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

	void setState(DBcontext state) //Make relevant changes to the UI and to other class attributes in order to make it correctly reflect the current state.
	{
		context = state;
		switch (state) //Trigger a set of changes based on the incoming state
		{
		case DBcontext::DIR_TREE:
			CLprinter::hideCursor(false);

			break;
		case DBcontext::TABLE_VIEW:
		{
			CLprinter::hideCursor(false);
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
			CLprinter::hideCursor(true);
			handleQueryTool();
			break;
		case DBcontext::MAIN_MENU:
			CLprinter::hideCursor(false);
			break;
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

	void printFS()
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
		std::cout << outBuf.str(); outBuf.str(std::string());

	}

	void printTableView() const
	{

		std::array<std::string, 2> phrases = { "Print Contents", "Show Statistics" };

		std::cout << std::endl << " ";

		for (std::size_t i = 0; i < phrases.size(); ++i)
		{
			if (i == currTab.selected_opt)
				std::cout << "(" << i << ") " << color::SELECTED << phrases[i] << color::RESET << std::endl << ' ';
			else 
				std::cout << "(" << i << ") " << phrases[i] << std::endl << ' ';
		}

	}

	void printMenu() const
	{
		std::cout << std::endl;
		for (std::size_t i = 0; i < menu_options.size(); ++i)
		{
			if (i == selected_menu_opt)
				std::cout << " * " << color::SELECTED << menu_options[i] << color::RESET << std::endl;
			else
				std::cout << " * " << menu_options[i] << std::endl;
		}
	}



	std::string parseQuery(std::string_view query_str, std::unordered_set<std::string> const& tokens ) const
	{

		std::vector<std::string> split_str;
		std::stringstream out_str;
		
		size_t last = 0;
		size_t next = 0; 
		while ((next = query_str.find(' ', last)) != std::string::npos)
		{ 
			split_str.emplace_back(query_str.substr(last, next - last));
			last = next + 1; 
		} 
		split_str.emplace_back(query_str.substr(last));

		size_t i = 0;
		for (auto const& tkn : split_str)
		{
			std::string lwr = tkn;
			std::transform(tkn.begin(), tkn.end(), lwr.begin(),
				[](char c) { return std::toupper(c); });

			if (tokens.find(lwr) != tokens.end())
				out_str << color::FIELD << split_str[i++] << color::RESET;
			else
				out_str << split_str[i++];
			out_str << " ";
		}

		auto str = out_str.str();
		str.erase(str.size() - 1, 1);

		return str;
	}

	void handleQueryTool()
	{

		std::unordered_set<std::string> SQLtokens = { "SELECT", "FROM", "WHERE", "ORDER BY", "GROUP BY", "SUM", "AVG" };

		std::system("CLS");

		printUtil.updateHeader("Query Tool");
		std::string query;

		for (;;)
		{
			printUtil.printHeader();
			std::cout << std::endl << std::endl << " Query: ";

			for (;;) {
				char c = _getch();
				
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
				
				query.push_back(static_cast<char>(c));
				std::cout << static_cast<char>(c);

				auto out_str = parseQuery(query, SQLtokens);
				if (out_str != query)
				{
					std::cout << "\r" << " Query: " << out_str;
				}
					
			}

			if (query.empty())
			{
				std::cerr << "  Empty query received, please, at least type something!" << std::endl;
				_getch();
				std::system("CLS");
				continue;
			}

			auto target_buff = new char[ 2 * static_cast<int>(query.size()) + 1 ];
			int error;

			PQescapeStringConn(conn, target_buff, query.c_str(), query.size(), &error);

			std::cout << std::endl;

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
			printUtil.printHeader();
			printTableView();

			break;
		case DBcontext::MAIN_MENU:

			printUtil.updateHeader("Main Menu");
			printUtil.printHeader();
			printMenu();

			break;
		default:

			assert("Invalid State");
			break;
		}

	}

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
					std::get<2>(selected_dir) = std::max(0, std::get<2>(selected_dir) - 1);
					break;
				default:
					break;
				}
				break;
			case A_KEY:
			case LEFT_KEY:
				std::get<0>(selected_dir) = std::max(0, std::get<0>(selected_dir) - 1);
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
					std::get<2>(selected_dir) = std::min(static_cast<int>(root[std::get<1>(selected_dir)].getChildren().size()) - 1, std::get<2>(selected_dir) + 1);
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
					std::get<0>(selected_dir) = std::min(2, std::get<0>(selected_dir) + 1);
					break;
				case 1:
					if (root[std::get<1>(selected_dir)].getChildren().size() == 0)
						break;
					std::get<0>(selected_dir) = std::min(2, std::get<0>(selected_dir) + 1);
					break;
				case 2:
					std::get<0>(selected_dir) = std::min(2, std::get<0>(selected_dir) + 1);
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
			refreshScreen();
			break;
		case DBcontext::TABLE_VIEW:
			switch (code)
			{
			case W_KEY:
			case UP_KEY:
				currTab.selected_opt = std::max(0, currTab.selected_opt - 1);
				refreshScreen();
				break;
			case S_KEY:
			case DOWN_KEY:
				currTab.selected_opt = std::min(1, currTab.selected_opt + 1);
				refreshScreen();
				break;
			case ENTER_KEY:
				if (currTab.selected_opt == 0)
				{
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
				selected_menu_opt = std::max(selected_menu_opt - 1, 0);
				break;
			case S_KEY:
			case DOWN_KEY:
				selected_menu_opt = std::min(static_cast<int>(menu_options.size() - 1), selected_menu_opt + 1);
				
				break;
			case ENTER_KEY:
				if (selected_menu_opt == 0)
				{
					setState(DBcontext::DIR_TREE);
				} 
				else 
				if (selected_menu_opt == 1)
				{
					setState(DBcontext::QUERY_TOOL);
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
		default:
			std::cerr << "context unhandled!" << std::endl;
			break;
		}
		
	}

private:
	struct tabViewAttr {
	public:
		uint16_t selected_opt;
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
	std::tuple<uint16_t, uint16_t, uint16_t> selected_dir;
	std::pair<int, int> bounds;
	static DBcontext context;
	bool isHidingPrivate;
	int curPos;
	tabViewAttr currTab;

	std::vector<std::string> menu_options;
	uint16_t selected_menu_opt;
};