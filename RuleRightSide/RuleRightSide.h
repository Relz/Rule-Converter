#ifndef RULECONVERTER_RULERIGHTSIDE_H
#define RULECONVERTER_RULERIGHTSIDE_H

#include "../Symbol/Symbol.h"
#include <vector>
#include <unordered_set>
#include <string>

class RuleRightSide
{
public:
	std::vector<Symbol> sequence;
	std::unordered_set<std::string> referencingSet;
	std::vector<std::string> actionNames;
};

#endif
