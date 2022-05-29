#pragma once
#include "WKQuery.h"
#include <queue>
#include "..\..\DButils\queries.h"


class ParametrizedQuery : public WKQuery
{
public:

	ParametrizedQuery(std::string_view query_name, std::string_view query_content)
	{
		name = query_name;
		content = query_content;
		parsed_query = query::parseQuery(content);
	}

	ParametrizedQuery(const char* query_name, const char* query_content)
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
		std::cout << "\n" << " Executing Parametrized Query " << color::FIELD << name << color::RESET << ": " << "\n" << "\t" << parsed_query << "\n";
		
		std::stringstream querybuilder;

		std::queue<std::string> split_str;

		size_t last = 0;
		size_t next = 0;
		while ((next = content.find('%', last)) != std::string::npos)
		{
			split_str.emplace(content.substr(last, next - last));
			last = next + 1;
		}
		split_str.emplace(content.substr(last));

		querybuilder << split_str.front(); split_str.pop();

		std::string parameter;

		for (size_t n_param = 1; !split_str.empty(); split_str.pop())
		{
			std::cout << " Insert Query Parameter " << n_param++ << std::endl;
			std::cout << "\t" << query::parseQuery(querybuilder.str());
			std::cin >> parameter;
			std::cout << '\b' << "\33[2K\r";

			querybuilder << " " << parameter << " " << split_str.front();
		}

		if (!(query::atomicQuery(querybuilder.str().c_str(), res, conn)))
		{
			std::cerr << "Parametrized query execution went wrong!" << std::endl;
			return;
		}
		printer.printTable(res);

		
	}

private:
	std::string parsed_query;
};

#pragma once
