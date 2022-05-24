#pragma once
#include <vector>
#include <array>
#include <memory>
#include "WKQuery.h"
#include "..\..\DButils\queries.h"
#include <sstream>

template <std::size_t S>
class Procedure : public WKQuery
{

	using string_tup = std::pair<std::string, std::string>;

public:
	Procedure(std::string_view proc_name, std::string_view c_name, std::array<string_tup, S> const& argnames) : argn(argnames)
	{
		name = proc_name;
		call_name = c_name;

		std::stringstream strbuild;

		strbuild << color::PROCEDURE << name << "(";

		for (auto const& name : argn)
		{
			strbuild << name.first << " : " << name.second << ", ";
		}
		strbuild << "\b\b  \b\b)" << color::RESET;

		parsed_name = strbuild.str();
	}

	Procedure(const char* proc_name, const char* c_name, std::array<string_tup, S> const& argnames) : argn(argnames)
	{
		name = std::string(proc_name);
		call_name = c_name;

		std::stringstream strbuild;

		strbuild << color::PROCEDURE << name << "(";

		for (auto const& name : argn)
		{
			strbuild << name.first << " : " << name.second << ", ";
		}
		strbuild << "\b\b  \b\b)" << color::RESET;

		parsed_name = strbuild.str();
	}

	void setParams(std::array<std::string, S> const& params)
	{
		args = params;
	}

	std::array<string_tup, S> const& getParamNames()
	{
		return argn;
	}

	std::array<std::string, S> const& getParas()
	{
		return args;
	}

	bool hasArgs() override {
		return true;
	}

	void execute(PGresult*& res, PGconn*& conn) override 
	{

		std::cout << "Executing Procedure " << parsed_name << ", Awaiting user input : \n\n";


		size_t i = 0;
		for (auto const& name : argn)
		{
			std::string arg("");
			std::cout << " " << name.first << " : " << name.second << ": ";
			std::cin >> arg;
			args[i++] = arg;
		}
		std::cout << "\n";

		std::stringstream query;
		query << "CALL \"" + call_name + "\"(";

		for (auto const& par : args)
		{
			query << '\'' << par + "\', ";
		}

		query << ")";

		std::string query_built = query.str();
		query_built.erase(query_built.size() - 3, 2);

		if (query::atomicQuery(query_built.c_str(), res, conn))
			std::cout << "\nProcedure \"" << parsed_name << "\" correctly executed!" << "\n";
	}


	std::string_view getContent() override { assert(false, "Procedures are content-less"); return " "; }

private:

	std::array<string_tup, S> argn;
	std::array<std::string, S> args;
	std::string parsed_name;
	std::string call_name;
};

