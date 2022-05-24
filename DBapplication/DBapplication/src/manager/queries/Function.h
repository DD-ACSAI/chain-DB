#pragma once
#include "WKQuery.h"
#include "..\..\defines\clicolors.h"

template <std::size_t S>
class Function : public WKQuery
{

	using string_tup = std::pair<std::string, std::string>;

public:
	explicit Function(std::string_view func_name, std::string_view c_name, std::array<string_tup, S> const& argnames) : argn(argnames)
	{
		name = func_name;
		call_name = c_name;

		std::stringstream strbuild;

		strbuild << color::FUNCTION << name << "(";

		for (auto const& name : argn)
		{
			strbuild << name.first << " : " << name.second << ", ";
		}
		strbuild << "\b\b  \b\b)" << color::RESET;

		parsed_name = strbuild.str();
	}

	explicit Function(const char* func_name, const char* c_name, std::array<string_tup, S> const& argnames) : argn(argnames)
	{
		name = std::string(func_name);
		call_name = c_name;

		std::stringstream strbuild;

		strbuild << color::FUNCTION << name << "(";

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

		std::cout << "Executing Function " << parsed_name << ", Awaiting user input : \n\n";


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
		query << "SELECT * FROM \"" + call_name + "\"(";

		for (auto const& par : args)
		{
			query << par + ", ";
		}

		query << ")";

		std::string query_built = query.str();
		query_built.erase(query_built.size() - 3, 2);

		if (query::atomicQuery(query_built.c_str(), res, conn))
			std::cout << "\Function \"" << parsed_name << "\" correctly executed!" << "\n";


		printer.printTable(res);
	}


	std::string_view getContent() override { assert(false, "Functions are content-less"); return " "; }

private:

	std::array<string_tup, S> argn;
	std::array<std::string, S> args;
	std::string parsed_name;
	std::string call_name;
};

