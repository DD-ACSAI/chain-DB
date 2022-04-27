#pragma once
#include "libpq-fe.h"
#include <vector>
#include <string>
#include <sstream>
#include <iostream>

class CLprinter
{
public:
	void printTable(PGresult*& res, bool printAll);

	CLprinter();

private:

	std::vector<std::string> fieldNames;
	std::vector<uint64_t>	 fieldLen;

	class outStream
	{
	public:
		void inline flushBuf() { std::cout << stream.rdbuf(); stream.str(std::string()); }
		void inline replaceChar(std::string const& str) { stream.seekp( -static_cast<int>(str.size()), stream.cur); stream << str; }
		void inline replaceChar(char ch) { stream.seekp(-1, stream.cur); stream << ch; }
		
		// Voodoo? voodoo.
		inline std::ostream& operator<<(std::string const& str) { stream << str; return stream; }
		inline std::ostream& operator<<(char c) { stream << c; return stream; }

	private:
		std::stringstream stream;
	};

	outStream stream;

};

