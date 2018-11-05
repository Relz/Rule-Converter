#ifndef RULECONVERTER_NODE_H
#define RULECONVERTER_NODE_H

#include "../Symbol/Symbol.h"
#include <unordered_map>
#include <vector>
#include <string>

class Node
{
public:
	Symbol symbol;
	std::unordered_map<Symbol, Node *> nextNodes;
	std::vector<std::string> actionNames;

	bool operator==(Node const & other) const
	{
		return symbol == other.symbol;
	}
};

#endif
