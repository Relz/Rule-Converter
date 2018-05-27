#include "NonterminalHelper.h"

void NonterminalHelper::GenerateNonterminalName(
	std::string & nonterminalName, std::function<bool(std::string const &)> const & validationFunction
)
{
	std::unordered_set<char> tried;
	for (char validCharacter : validCharacters)
	{
		if (tried.find(validCharacter) != tried.end())
		{
			continue;
		}
		std::string possibleNonterminalName = std::string(1, validCharacter);
		if (validationFunction(possibleNonterminalName))
		{
			nonterminalName = std::move(possibleNonterminalName);
			break;
		}
		tried.emplace(validCharacter);
	}
}

std::set<char> NonterminalHelper::validCharacters {
	'A',
	'B',
	'C',
	'D',
	'E',
	'F',
	'G',
	'H',
	'I',
	'J',
	'K',
	'L',
	'M',
	'N',
	'O',
	'P',
	'Q',
	'R',
	'S',
	'T',
	'U',
	'V',
	'W',
	'X',
	'Y',
	'Z',
};
