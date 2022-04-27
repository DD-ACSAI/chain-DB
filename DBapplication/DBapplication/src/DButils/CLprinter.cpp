#include "CLprinter.h"
#include <cstdint>
#include <iostream>
#include <numeric>
#include <windows.h>

#define ascii(x) static_cast<char>(x)
#define biguint(x) static_cast<uint64_t>(x)
#define bigint(x)	static_cast<int64_t>(x)

void CLprinter::printTable(PGresult*& res, bool printAll = false)
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
	constexpr const int16_t MAXCELLSIZE = 20;
	constexpr const int16_t PADDING = 4;

	static_assert(MAXCELLSIZE % 2 == 0, "CELLSIZE must be even!");
	static_assert(PADDING % 2 == 0, "PADDING must be even!");

	// Gather field names for proper formatting

	uint64_t nFields = PQnfields(res);
	uint64_t nRows = PQntuples(res);

	for (int i = 0; i < nFields; ++i)
	{
		fieldNames.emplace_back(PQfname(res, i));
		fieldLen.emplace_back(fieldNames.at(i).size());
	}

	// Local Function Defs

	static auto const printTop = [&]()
	{
		stream << "\n Query Output: \n" << ascii(201);

		for (uint64_t i = 0; i < nFields; ++i)
		{

			stream << std::string(biguint(MAXCELLSIZE) + biguint(PADDING), ascii(205)) << ascii(203);
		}

		stream.replaceChar(ascii(187));
	};

	static auto const printBottom = [&]()
	{
		stream << '\n' << ascii(200);

		for (uint64_t i = 0; i < nFields; ++i)
		{

			stream << std::string(bigint(MAXCELLSIZE) + bigint(PADDING), ascii(205)) << ascii(202);
		}

		stream.replaceChar(ascii(188));
	};

	static auto const printSep = [&]()
	{

		stream << '\n' << ascii(204);
		
		for (uint64_t i = 0; i < nFields; ++i)
		{
			stream << std::string(bigint(MAXCELLSIZE) + bigint(PADDING), ascii(205)) << ascii(206);
		}

		stream.replaceChar(ascii(185));
	};

	static auto printFields = [&]()
	{
		stream << '\n' << ascii(186);
		for (auto& str : fieldNames)
		{
			str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());

			uint64_t blankspace = biguint(PADDING) + biguint(MAXCELLSIZE) - str.size();
			if (blankspace < PADDING)
			{
				blankspace = PADDING;
				str.resize(bigint(MAXCELLSIZE) - 3);
				str += "...";
			}

			uint64_t lblank = blankspace / 2;
			uint64_t rblank = (blankspace % 2 == 0) ? blankspace / 2 : blankspace / 2 + 1;

			stream << std::string(lblank, ' ') << str << std::string(rblank, ' ') << ascii(186);
		}

	};

	static auto printBlank = [&]()
	{
		stream << '\n' << ascii(186);
		for (unsigned int c = 0; c < nFields; ++c)
		{
			stream << std::string(biguint(MAXCELLSIZE) + biguint(PADDING), ' ') << ascii(186);
		}
	};

	static auto printRow = [&](unsigned int i)
	{
		stream << '\n' << ascii(186);

		for (unsigned int j = 0; j < nFields; ++j)
		{
			auto str_val = std::string(PQgetvalue(res, i, j));

			str_val.erase(std::remove(str_val.begin(), str_val.end(), '\n'), str_val.end());

			int64_t blankspace = bigint(PADDING) + bigint(MAXCELLSIZE) - str_val.size();
			if (blankspace < PADDING)
			{
				blankspace = PADDING;
				str_val.resize(bigint(MAXCELLSIZE) - 3);
				str_val += "...";
			}

			uint64_t lblank = blankspace / 2;
			uint64_t rblank = (blankspace % 2 == 0) ? blankspace / 2 : blankspace / 2 + 1;

			stream << std::string(lblank, ' ') << str_val << std::string(rblank, ' ') << ascii(186);
		}

	};

	//Build Header

	printTop();
	printFields();
	printSep();
	printBlank();

	stream.flushBuf();

	//Print Fields

	for (unsigned int i = 0; i < nRows; ++i)
	{
		printRow(i);

		if (i % 4 == 0)
			stream.flushBuf();

		if (!printAll) {
			if (i % 40 == 0 && i != 0)
			{
				HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
				CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
				GetConsoleScreenBufferInfo(h, &bufferInfo);

				std::cout << "\nPress q to quit scrolling, any other key to keep scrolling: ";
				int out = std::getchar();

				SetConsoleCursorPosition(h, bufferInfo.dwCursorPosition);
				std::cout << std::string(60, ' ') << std::flush;
				if (out == 'q')
				{
					break;
				}
			}
		}
	}

	//Build Footer

	printBlank();
	printBottom();

	stream.flushBuf();
}



CLprinter::CLprinter()
{
	fieldNames.reserve(8);
	fieldLen.reserve(8);
}
