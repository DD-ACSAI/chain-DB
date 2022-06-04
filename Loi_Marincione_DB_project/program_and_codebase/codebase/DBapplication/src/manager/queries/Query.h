#pragma once
#include "WKQuery.h"

#include "..\..\DButils\queries.h"


class Query : public WKQuery
{
public:

	Query(std::string_view query_name, std::string_view query_content)
	{
		name = query_name;
		content = query_content;
		parsed_query = query::parseQuery(content);
	}

	Query(const char* query_name, const char* query_content)
	{
		name = query_name;
		content = query_content;
		parsed_query = query::parseQuery(content);
	}

	bool hasArgs() override {
		return false;
	}

	void execute(PGresult*& res, PGconn*& conn) override
	{
		std::cout << "\n" << "Executing Query " << color::FIELD << name << color::RESET << ": " << "\n" << "\t" << parsed_query << "\n";
		query::atomicQuery(content.c_str(), res, conn);

		printer.printTable(res);
	}

private:
	std::string parsed_query;
};

