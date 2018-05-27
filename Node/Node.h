#ifndef RULECONVERTER_NODE_H
#define RULECONVERTER_NODE_H

#include "../Symbol/Symbol.h"
#include <unordered_map>

class Node
{
public:
	Symbol symbol;
	std::unordered_map<Symbol, Node *> nextNodes;

	bool operator==(Node const & other) const
	{
		return symbol == other.symbol;
	}
};

#endif
