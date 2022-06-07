#pragma once
#include <type_traits>
#include <string>
#include <map>
#include "../../defines/clicolors.h"
#include <iostream>
#include <string_view>
#include <sstream>
#include <set>

enum class NODE : char {
	ROOT,
	SCHEMA,
	TABLE,
	NONE
};

template<NODE T>
class Dbnode
{

private:

	constexpr const static NODE CHILD = []() constexpr // Child type
	{
		switch (T)
		{
		case NODE::ROOT:
			return NODE::SCHEMA;
		break;
		case NODE::SCHEMA:
			return NODE::TABLE;
		break;
		case NODE::TABLE:
			return NODE::NONE;
		break;
		case NODE::NONE:
			return NODE::NONE;
		break;
		default:
			static_assert(true, "unhandled Type in Dbnode.h");
		}
	}();

	using child = Dbnode<CHILD>; // we hide this monstrosity	
	using childs = std::map<std::string, child>;

	childs children;						// children dictionary
	std::string  nodeName;			// Node name
	std::string queryName;
	constexpr static const NODE type = T;	// Node Type

	static const inline std::set<std::string> privateSchemas = { "        aaaaaaaaainformation_schema", "        aaaaaaaaapg_catalog", "        aaaaaaaaapg_toast" };

public:

	childs const& getChildren() const { static_assert(CHILD != NODE::NONE, "This node does not have children."); return children; }

	void addChildren(std::string const& cname) { static_assert(CHILD != NODE::NONE, "This node does not have children."); children.emplace( cname, Dbnode<CHILD>(cname) ); }
	void addChildren(Dbnode<CHILD> const& c) { static_assert(CHILD != NODE::NONE, "This node does not have children."); children.emplace(c.getName(), c); }

	explicit Dbnode(std::string const& name) : nodeName(name) { 
		static_assert(T != NODE::NONE, "Cannot initialize a NONE node."); 
		if (privateSchemas.find(nodeName) != privateSchemas.end())
		{
			queryName = nodeName.substr(17);
		}
		else
		{
			queryName = nodeName;
		}
	}

	Dbnode<CHILD>& operator[](std::string const& key) { return children[key]; }
	template<typename I> Dbnode<CHILD>& operator[](I indx)
	{
		static_assert(std::is_integral_v<I>, "Must be indexed by an integral type");
		auto it = children.begin();
		std::advance(it, indx);
		return it->second;
	}


	std::string getName() const { return nodeName; }
	std::string getQueryName() const { return queryName; }

	template<typename S>
	void printRecursive(S selected, std::ostringstream& outBuf, bool hidePrivate, int& cursor) const 
	{

		static_assert(std::is_same_v<S, std::string&> || std::is_same_v<S, const char*>
			|| std::is_same_v<S, std::string> || std::is_same_v<S, char*>, "Must use a string/string literal/Cstring");

		auto constexpr head = []() constexpr {
			if constexpr (T == NODE::ROOT)
			{
				return " \xC9 ";
			} else 
				if constexpr (T == NODE::SCHEMA)
				{
					return " \xCC\xCD\xCD\xCD\xD1 ";
				}
				else // NODE::TABLE 
				{
					return " \xBA   \xC3\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4 ";
				}
		}();

			
		outBuf << color::STRUCTURE << head << color::RESET;

		if (nodeName.compare(selected) == 0)
		{
			auto buffString = outBuf.str();
			cursor = std::count(buffString.begin(), buffString.end(), '\n');
			outBuf << color::SELECTED << queryName << color::RESET;
		}
		else
			outBuf << queryName;

		outBuf << "\n";

		for (auto const& c : children)
		{
			c.second.printRecursive(selected, outBuf, hidePrivate, cursor);
		}
	};



	Dbnode() : nodeName("root") { static_assert(true, "Default initialization prohibited!"); };	//here just for unordered_map pre-alloc purposes
};
