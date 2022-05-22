#pragma once
#include <string>
#include <string_view>
#include <libpq-fe.h>
#include "..\..\DButils\CLprinter.h"

class WKQuery
{
public:
	virtual void execute(PGresult*& res, PGconn*& conn) = 0;
	virtual ~WKQuery() = default;

	virtual std::string_view getName() { return name; }
	virtual std::string_view getContent() { return content; }

protected:
	std::string name;
	std::string content;
	static CLprinter printer;
};

