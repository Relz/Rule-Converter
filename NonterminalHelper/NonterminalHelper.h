#ifndef RULECONVERTER_NONTERMINALHELPER_H
#define RULECONVERTER_NONTERMINALHELPER_H

#include <string>
#include <set>
#include <unordered_set>
#include <functional>

class NonterminalHelper
{
public:
	static void GenerateNonterminalName(
		std::string & nonterminalName, std::function<bool(std::string const &)> const & validationFunction
	);

private:
	static std::set<char> validCharacters;
};

#endif
