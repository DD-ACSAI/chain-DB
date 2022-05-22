#pragma once
namespace color
{

	constexpr auto RESET = "\x1B[0;97m";
	constexpr auto FIELD = "\033[1;0;33m";

	constexpr const char* QUERY = FIELD;
	constexpr auto PROCEDURE = "\033[1;140;33m";

	constexpr auto STRUCTURE = "\x1B[0;31m";
	constexpr auto VALUE = "\x1B[1;97m";
	constexpr auto SELECTED = "\033[1;47;30m";

}