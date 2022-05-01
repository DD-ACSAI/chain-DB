#pragma once
#include <type_traits>
#include <string>
#include <map>
#include "../../defines/clicolors.h"
#include <iostream>
#include <string_view>

enum class NODE : char  {
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
	std::string nodeName;					// Node name
	constexpr static const NODE type = T;	// Node Type

public:

	childs const& getChildren() const { static_assert(CHILD != NODE::NONE, "This node does not have children."); return children; }

	void addChildren(std::string const& cname) { static_assert(CHILD != NODE::NONE, "This node does not have children."); children.emplace( cname, Dbnode<CHILD>(cname) ); }
	void addChildren(Dbnode<CHILD> const& c) { static_assert(CHILD != NODE::NONE, "This node does not have children."); children.emplace(c.getName(), c); }

	explicit Dbnode(std::string const& name) : nodeName(name) { static_assert(T != NODE::NONE, "Cannot initialize a NONE node."); }

	Dbnode<CHILD>& operator[](std::string const& key) { return children[key]; }

	std::string getName() const { return nodeName; }

	void printRecursive(short depth = 0) const 
	{

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

		std::clog << color::STRUCTURE << head << color::RESET << nodeName << std::endl;
		for (auto const& c : children)
		{
			c.second.printRecursive(depth);
		}
	};


	Dbnode() : nodeName("root") { static_assert(true, "Default initialization prohibited!"); };	//here just for unordered_map pre-alloc purposes
};
