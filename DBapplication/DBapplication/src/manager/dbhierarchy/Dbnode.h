#pragma once
#include <type_traits>
#include <vector>
#include <memory> // the seventh seal hath been broken
#include <string>

enum class DBNODETYPE : short  {
	ROOT,
	SCHEMA,
	TABLE,
	NONE
};

template<DBNODETYPE T, DBNODETYPE CHILD>
class Dbnode
{

private:

	constexpr const static DBNODETYPE CHILDCHILD = []() constexpr
	{
		if constexpr (CHILD == DBNODETYPE::SCHEMA)
		{
			constexpr const auto val = DBNODETYPE::TABLE;
			return val;
		}
		else
		{
			constexpr const auto val = DBNODETYPE::NONE;
			return val;
		}
	}();

	using child = Dbnode<CHILD, Dbnode::CHILDCHILD>; // we hide this monstrosity
	using childs = std::vector<std::unique_ptr<child>>;

	childs children;
	std::string dbName;
	const DBNODETYPE type;
	

	static_assert(	T == DBNODETYPE::ROOT && CHILD == DBNODETYPE::SCHEMA
		|| T == DBNODETYPE::SCHEMA && CHILD == DBNODETYPE::TABLE
		|| T == DBNODETYPE::TABLE && CHILD == DBNODETYPE::NONE
		|| T == DBNODETYPE::NONE && CHILD == DBNODETYPE::NONE,
				"The node must have a valid children type, current type pairing: " );


public:
	childs const& getChildren() const { return children; }
	void addChildren(std::unique_ptr<child> c) { children.emplace_back( std::move(c) ); }
	explicit Dbnode(std::string const& name) : dbName(name), type(T) {}
	std::string getName() const { return dbName; }
	void printRecursive() const {
		std::clog << dbName;
		for (auto const& c : children)
		{
			c->printRecursive();
		}
	}

	Dbnode(Dbnode&& node) : type(T), dbName(node.dbName)
	{
		this->children.reserve(node.children.size());
		for (auto const& c : node.children) 
		{
			this->children.emplace_back(std::move(c));
		}
	}

	Dbnode(Dbnode const& node) : type(T), dbName(node.dbName)
	{
		this->children.reserve(node.children.size());
		for (auto const& c : node.children)
		{
			this->children.emplace_back(std::move(c));
		}
	}

};
