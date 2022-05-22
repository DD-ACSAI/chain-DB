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
	}

	void execute(PGresult*& res, PGconn*& conn) override
	{
		query::atomicQuery(content.c_str(), res, conn);
	}

};

