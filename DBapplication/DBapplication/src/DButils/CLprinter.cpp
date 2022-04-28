#include "CLprinter.h"
#include "..\defines\clicolors.h"
#include <cstdint>
#include <iostream>
#include <numeric>

#define ascii(x)	static_cast<char>(x)
#define biguint(x)	static_cast<uint64_t>(x)
#define bigint(x)	static_cast<int64_t>(x)

void CLprinter::printTable(PGresult*& res, uint64_t maxRow)
{
	/*
	* ASCII table
	* 200 : bottom left corner
	* 201 : top left corner
	* 202 : Bottom T
	* 203 : Top T
	* 204 : Left T
	* 205 : horizontal line
	* 206 : Cross
	*
	* 185 : Right T
	* 186 : vertical line
	* 187 : top right corner
	* 188 : bottom right corner
	*/


	// Gather field names for proper formatting

	uint64_t nFields = PQnfields(res);
	uint64_t nRows = PQntuples(res);

	nRows = (nRows > maxRow) ? maxRow : nRows;

	for (int i = 0; i < nFields; ++i)
	{
		fieldNames.emplace_back(PQfname(res, i));
		fieldLen.emplace_back(fieldNames.at(i).size());
	}

	//Build Header

	printTop(nFields);
	printFields();
	printSep(nFields);
	printBlank(nFields);

	stream.flushBuf();

	//Print Fields

	for (unsigned int i = 0; i < nRows; ++i)
	{
		printRow(i, nFields, res);

		if (i % 4 == 0)
			stream.flushBuf();
	}

	//Build Footer

	printBlank(nFields);
	printBottom(nFields);

	stream.flushBuf();
}

void CLprinter::setPadding(int16_t padSize) {
	try
	{
		if (padSize % 2 != 0)
		{
			throw padSize;
		}
		parameters.padding = padSize;
	}
	catch (int n)
	{
		std::cerr << "Padding must be a number divisible by 2, provided number was: " << n << std::endl;
	}
}

void CLprinter::setMaxCellSize(int16_t cellSize)
{
	try
	{
		if (cellSize % 2 != 0)
		{
			throw cellSize;
		}
		parameters.maxcellsize = cellSize;
	}
	catch (int n)
	{
		std::cerr << "Cell size must be a number divisible by 2, provided number was: " << n << std::endl;
	}
}

CLprinter::CLprinter()
{
	fieldNames.reserve(8);
	fieldLen.reserve(8);

	constexpr const int16_t MAXCELLSIZE = 12;
	constexpr const int16_t PADDING = 4;

	static_assert(MAXCELLSIZE % 2 == 0, "CELLSIZE must be even!");
	static_assert(PADDING % 2 == 0, "PADDING must be even!");
	parameters = { PADDING , MAXCELLSIZE };

}

void inline CLprinter::printTop(uint64_t nFields)
{
	stream << "\n Query Output: \n" << TABLE << ascii(201);

	for (uint64_t i = 0; i < nFields; ++i)
	{

		stream << std::string(biguint(parameters.maxcellsize) + biguint(parameters.padding), ascii(205)) << ascii(203);
	}

	stream.replaceChar(ascii(187));
	stream << RESET;
}

void inline CLprinter::printBottom(uint64_t nFields)
{
	stream << '\n' << TABLE << ascii(200);

	for (uint64_t i = 0; i < nFields; ++i)
	{

		stream << std::string(bigint(parameters.maxcellsize) + bigint(parameters.padding), ascii(205)) << ascii(202);
	}

	stream.replaceChar(ascii(188));
	stream << RESET;
}

void inline CLprinter::printSep(uint64_t nFields)
{

	stream << '\n' << TABLE << ascii(204);

	for (uint64_t i = 0; i < nFields; ++i)
	{
		stream << std::string(bigint(parameters.maxcellsize) + bigint(parameters.padding), ascii(205)) << ascii(206);
	}

	stream.replaceChar(ascii(185));
	stream << RESET;
}

void inline CLprinter::printBlank(uint64_t nFields)
{
	stream << '\n' << TABLE << ascii(186);
	for (unsigned int c = 0; c < nFields; ++c)
	{
		stream << std::string(biguint(parameters.maxcellsize) + biguint(parameters.padding), ' ') << ascii(186);
	}

	stream << RESET;
}

void CLprinter::printFields()
{
	stream << '\n' << TABLE << ascii(186);
	for (auto& str : fieldNames)
	{
		str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());

		int64_t blankspace = biguint(parameters.padding) + biguint(parameters.maxcellsize) - str.size();
		if (blankspace < parameters.padding)
		{
			blankspace = parameters.padding;
			str.resize(bigint(parameters.maxcellsize) - 3);
			str += "...";
		}

		uint64_t lblank = blankspace / 2;
		uint64_t rblank = (blankspace % 2 == 0) ? blankspace / 2 : blankspace / 2 + 1;

		stream << std::string(lblank, ' ') << FIELD << str << RESET << std::string(rblank, ' ') << TABLE << ascii(186) << RESET;
	}

}

void CLprinter::printRow(unsigned int i, uint64_t nFields, PGresult*& res)
{

	stream << '\n' << TABLE << ascii(186);

	for (unsigned int j = 0; j < nFields; ++j)
	{
		auto str_val = std::string(PQgetvalue(res, i, j));

		str_val.erase(std::remove(str_val.begin(), str_val.end(), '\n'), str_val.end());
		str_val = (str_val.empty()) ? "*" : str_val;

		int64_t blankspace = bigint(parameters.padding) + bigint(parameters.maxcellsize) - str_val.size();
		if (blankspace < parameters.padding)
		{
			blankspace = parameters.padding;
			str_val.resize(bigint(parameters.maxcellsize) - 3);
			str_val += "...";
		}

		uint64_t lblank = blankspace / 2;
		uint64_t rblank = (blankspace % 2 == 0) ? blankspace / 2 : blankspace / 2 + 1;

		stream << std::string(lblank, ' ') << VALUE << str_val << VALUE << std::string(rblank, ' ') << TABLE << ascii(186) << RESET;
		}

}

const HANDLE CLprinter::hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

