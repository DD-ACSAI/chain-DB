#pragma once
#include "libpq-fe.h"
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <assert.h>
#include <Windows.h>
#include "../manager/dbhierarchy/Dbnode.h"

class CLprinter
{
public:
	void printTable(PGresult*& res, uint64_t maxRow = UINT64_MAX);

	void setPadding(int16_t padSize);
	int16_t getPadding() const { return parameters.padding; }

	void setMaxCellSize(int16_t cellSize);
	int16_t getMaxCellSize() const { return parameters.maxcellsize; }

	CLprinter();

	static HANDLE getHandle() { return hConsole; }
	static void setPos(int x, int y);
	static std::pair<int, int> getPos();

private:

	struct printParam {
		int16_t padding;
		int16_t maxcellsize;
	};

	printParam parameters;

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

	void printTop(uint64_t nFields);
	void printBottom(uint64_t nFields);
	void printSep(uint64_t nFields);
	void printBlank(uint64_t nFields);
	void printFields();
	void printRow(unsigned int i, uint64_t nFields, PGresult*& res);


	static const HANDLE hConsole;
	outStream stream;

};

