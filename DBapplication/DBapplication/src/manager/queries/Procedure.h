#pragma once
#include <vector>
#include <array>
#include <memory>
#include "WKQuery.h"
#include "..\..\DButils\queries.h"

template <std::size_t S>
class Procedure : public WKQuery
{
public:
	Procedure(std::string_view proc_name)
	{
		name = proc_name;
		
	}

	void setParams(std::array<std::string, S> const& params)
	{
		size_t i = 0;
		for (auto const& p : params)
		{
			args[i++] = p
		}
	}

	std::array<std::string, S> const& getParamNames()
	{
		return argn;
	}

	std::array<std::string, S> const& getParas()
	{
		return args;
	}


	void execute(PGresult*& res, PGconn*& conn) override 
	{
		std::string query("CALL \"" + proc_name + "\"(");

		for (auto const& par : args)
		{
			query = query + args + ", ";
		}

		query += ");";

		query::atomicQuery(query.c_str(), res, conn);

	}

	void setArgs()

	std::string_view getContent() override { assert(false, "Procedures are content-less"); }

private:

	std::array<std::string, S> argn;
	std::array<std::string, S> args;
};

