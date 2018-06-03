#include <iostream>
#include <algorithm>
#include "InputLibrary/Input.h"
#include "TokenLibrary/Token.h"
#include "TokenLibrary/TokenExtensions/TokenExtensions.h"
#include "Symbol/Symbol.h"
#include "Node/Node.h"
#include "NonterminalHelper/NonterminalHelper.h"
#include "RuleRightSide/RuleRightSide.h"

int const REQUIRED_ARGC = 1;
std::string const NO_ARGUMENT_ERROR = "Error: file with rules is not specified";

std::string const AUGMENT_NONTERMINAL = "Augment";

char const EMPTY_CHARACTER = 'e';
std::string const EMPTY_CHARACTER_STRING = std::string(1, EMPTY_CHARACTER);
std::string const EMPTY_CHARACTER_STRING_BORDERED =
		std::string(1, Symbol::NONTERMINAL_LEFT_BORDER) + EMPTY_CHARACTER + Symbol::NONTERMINAL_RIGHT_BORDER;
Symbol const EMPTY_CHARACTER_SYMBOL(EMPTY_CHARACTER_STRING_BORDERED);

void ParseNonterminalDeclaration(Input & input, std::string & nonterminal)
{
	if (!input.ReadUntilCharacters({ '-' }, nonterminal))
	{
		throw std::runtime_error(
			nonterminal.empty()
			? "Nonterminal declaration expected"
			: "Character '-' expected after nonterminal declaration"
		);
	}
	input.SkipArgument<char>();
}

bool TryToParseSymbol(Input & input, char expectedLeftBorder, char expectedRightBorder, Symbol & symbol)
{
	Symbol possibleSymbol;
	char nextCharacter;
	if (input.GetNextCharacter(nextCharacter) && nextCharacter == expectedLeftBorder)
	{
		if (!input.ReadUntilCharacters({ expectedRightBorder }, possibleSymbol))
		{
			throw std::runtime_error(
				std::string("Symbol parsing: ")
					+ input.GetFileName() + ": "
					+ "(" + std::to_string(input.GetPosition().GetLine()) + ","
					+ std::to_string(input.GetPosition().GetColumn()) + ")" + ": "
					+ "'" + std::string(1, expectedRightBorder) + "'" + " expected"
			);
		}
		input.SkipArgument<char>();
		possibleSymbol += expectedRightBorder;
		symbol = std::move(possibleSymbol);
		return true;
	}
	return false;
}

void ReadNonterminalSequence(
	Input & input, std::vector<Symbol> & nonterminalSequence, std::string mainNonterminal, bool & needAugment
)
{
	while (!input.IsEndOfLine())
	{
		nonterminalSequence.emplace_back(Symbol());
		if (!TryToParseSymbol(input, Symbol::TERMINAL_LEFT_BORDER, Symbol::TERMINAL_RIGHT_BORDER, nonterminalSequence.back())
			&& !TryToParseSymbol(input, Symbol::NONTERMINAL_LEFT_BORDER, Symbol::NONTERMINAL_RIGHT_BORDER, nonterminalSequence.back()))
		{
			throw std::runtime_error(
				std::string("Sequence string splitting: ")
					+ input.GetFileName() + ": "
					+ "(" + std::to_string(input.GetPosition().GetLine()) + ","
					+ std::to_string(input.GetPosition().GetColumn()) + ")" + ": "
					+ "'" + std::string(1, Symbol::NONTERMINAL_LEFT_BORDER) + "'" + " or "
					+ "'" + std::string(1, Symbol::TERMINAL_LEFT_BORDER) + "'"
					+ " expected"
			);
		}
		if (nonterminalSequence.back().IsValueEquals(mainNonterminal))
		{
			needAugment = true;
		}
	}
}

void ReadRule(
	std::string const & inputFileName,
	std::string & mainNonterminal,
	bool & needAugment,
	std::unordered_map<std::string, std::vector<RuleRightSide>> & rules
)
{
	Input inputFile(inputFileName);
	while (!inputFile.IsEndOfStream())
	{
		std::string nonterminal;
		ParseNonterminalDeclaration(inputFile, nonterminal);

		if (mainNonterminal.empty())
		{
			mainNonterminal = nonterminal;
		}

		rules.try_emplace(nonterminal, std::vector<RuleRightSide>());
		rules.at(nonterminal).emplace_back(RuleRightSide());

		ReadNonterminalSequence(inputFile, rules.at(nonterminal).back().sequence, mainNonterminal, needAugment);

		inputFile.SkipLine();
	}
}

void AugmentRule(std::unordered_map<std::string, std::vector<RuleRightSide>> & rules, std::string & mainNonterminal)
{
	rules.try_emplace(AUGMENT_NONTERMINAL, std::vector<RuleRightSide>(1, RuleRightSide()));
	Symbol nonterminal;
	Symbol::CreateNonterminal(mainNonterminal, nonterminal);
	rules.at(AUGMENT_NONTERMINAL).front().sequence.emplace_back(nonterminal);
	mainNonterminal = AUGMENT_NONTERMINAL;
}

void AddEndOfFileToken(std::vector<RuleRightSide> & mainRule)
{
	for (RuleRightSide & mainRuleRightSide : mainRule)
	{
		Symbol terminal;
		Symbol::CreateTerminal(TokenExtensions::ToString(Token::END_OF_FILE), terminal);
		mainRuleRightSide.sequence.emplace_back(terminal);
	}
}

bool DoesNeedToResolveLeftRecursion(std::string const & nonterminal, std::vector<RuleRightSide> const & rightSides)
{
	return std::find_if(rightSides.begin(), rightSides.end(), [&nonterminal](RuleRightSide const & rightSide) {
		return rightSide.sequence.front().IsValueEquals(nonterminal);
	}) != rightSides.end();
}

void RemoveLeftRecursion(std::unordered_map<std::string, std::vector<RuleRightSide>> & rules)
{
	for (auto & rule : rules)
	{
		std::string const & nonterminal = rule.first;
		std::vector<RuleRightSide> & rightSides = rule.second;

		rightSides.erase(
			std::remove_if(rightSides.begin(), rightSides.end(), [nonterminal](RuleRightSide const & rightSide) {
				return rightSide.sequence.front().IsValueEquals(nonterminal) && rightSide.sequence.size() == 1;
			}),
			rightSides.end()
		);

		if (DoesNeedToResolveLeftRecursion(nonterminal, rightSides))
		{
			std::string newNonterminalName;
			NonterminalHelper::GenerateNonterminalName(
				newNonterminalName,
				[&rules](std::string const & possibleNonterminalName)
				{
					return rules.find(possibleNonterminalName) == rules.end();
				}
			);
			for (auto it = rightSides.begin(); it != rightSides.end();)
			{
				RuleRightSide & rightSide = *it;
				if (rightSide.sequence.front() == EMPTY_CHARACTER_SYMBOL)
				{
					continue;
				}
				Symbol newNonterminal;
				Symbol::CreateNonterminal(newNonterminalName, newNonterminal);
				rightSide.sequence.emplace_back(newNonterminal);

				if (rightSide.sequence.front().IsValueEquals(nonterminal))
				{
					rightSide.sequence.erase(rightSide.sequence.begin());
					rules[newNonterminalName].emplace_back(rightSide);
					rightSides.erase(it);
				}
				else
				{
					++it;
				}
			}
			rules[newNonterminalName].emplace_back(RuleRightSide());
			rules[newNonterminalName].back().sequence.emplace_back(EMPTY_CHARACTER_SYMBOL);
		}
	}
}

void ResolveNode(
	Node * node,
	std::unordered_map<std::string, std::vector<RuleRightSide>> & rules,
	std::string const & nonterminal
)
{
	std::vector<RuleRightSide> & rightSides = rules[nonterminal];
	rightSides.emplace_back(RuleRightSide());
	while (node->nextNodes.size() == 1)
	{
		rightSides.back().sequence.emplace_back(node->symbol);
		node = node->nextNodes.begin()->second;
	}
	rightSides.back().sequence.emplace_back(node->symbol);
	if (node->nextNodes.empty())
	{
		return;
	}
	std::string newNonterminalName;
	NonterminalHelper::GenerateNonterminalName(
		newNonterminalName,
		[&rules](std::string const & possibleNonterminalName)
		{
			return rules.find(possibleNonterminalName) == rules.end();
		}
	);
	Symbol newNonterminal;
	Symbol::CreateNonterminal(newNonterminalName, newNonterminal);
	rightSides.back().sequence.emplace_back(newNonterminal);
	for (auto const & symbolNextNodes : node->nextNodes)
	{
		ResolveNode(symbolNextNodes.second, rules, newNonterminalName);
	}
}

void Factorize(std::unordered_map<std::string, std::vector<RuleRightSide>> & rules)
{
	for (auto & rule : rules)
	{
		std::string const & nonterminal = rule.first;
		std::vector<RuleRightSide> & rightSides = rule.second;

		std::unordered_map<Symbol, Node*> treesRoots { };
		for (RuleRightSide const & rightSide : rightSides)
		{
			std::vector<Symbol> const & sequence = rightSide.sequence;
			Node * prevNode = nullptr;
			for (size_t symbolIndex = 0; symbolIndex < sequence.size(); ++symbolIndex)
			{
				Symbol const & symbol = sequence.at(symbolIndex);
				Node * node = nullptr;
				if (prevNode != nullptr)
				{
					node = prevNode->nextNodes.find(symbol) == prevNode->nextNodes.end()
						? new Node() : prevNode->nextNodes.at(symbol);
				}
				else if (symbolIndex == 0 && treesRoots.find(symbol) != treesRoots.end())
				{
					node = treesRoots.at(symbol);
				}
				else
				{
					node = new Node();
				}
				node->symbol = symbol;
				if (prevNode != nullptr)
				{
					prevNode->nextNodes.emplace(symbol, node);
				}
				if (symbolIndex == 0 && treesRoots.find(symbol) == treesRoots.end())
				{
					treesRoots.emplace(sequence.front(), node);
				}
				prevNode = node;
			}
		}
		rightSides.clear();
		for (auto const & treesRoot : treesRoots)
		{
			Node * node = treesRoot.second;
			ResolveNode(node, rules, nonterminal);
		}
	}
}

void ComputeReferencingSet(
		std::unordered_map<std::string, std::vector<RuleRightSide>> & rules,
		std::string const & nonterminal,
		RuleRightSide & rightSide
);

void ComputeNextTerminals(
	std::unordered_map<std::string, std::vector<RuleRightSide>> & rules,
	std::string const & nonterminalToFind,
	std::unordered_set<std::string> & nextTerminals
)
{
	static std::unordered_map<std::string, std::unordered_set<size_t>> blacklist;
	for (auto & rule : rules)
	{
		std::string const & nonterminal = rule.first;
		std::vector<RuleRightSide> & rightSides = rule.second;

		for (RuleRightSide const & rightSide : rightSides)
		{
			for (size_t i = 0; i < rightSide.sequence.size(); ++i)
			{
				Symbol const & symbol = rightSide.sequence[i];
				if (!symbol.IsValueEquals(nonterminalToFind))
				{
					continue;
				}
				if (i == rightSide.sequence.size() - 1)
				{
					if (blacklist.find(nonterminal) == blacklist.end())
					{
						blacklist.emplace(nonterminal, std::unordered_set<size_t>());
					}
					if (blacklist[nonterminal].find(i) == blacklist[nonterminal].end())
					{
						blacklist[nonterminal].emplace(i);
						ComputeNextTerminals(rules, nonterminal, nextTerminals);
					}
				}
				else
				{
					Symbol nextSymbol = rightSide.sequence[i + 1];
					if (nextSymbol.IsTerminal())
					{
						nextTerminals.emplace(nextSymbol.GetValue());
					}
					else
					{
						std::string const & nextNonterminalValue = nextSymbol.GetValue();
						std::vector<RuleRightSide> & nextNonterminalRightSides = rules.at(nextNonterminalValue);
						for (RuleRightSide & rightSide : nextNonterminalRightSides)
						{
							if (rightSide.referencingSet.empty())
							{
								ComputeReferencingSet(rules, nextNonterminalValue, rightSide);
							}
							nextTerminals.insert(
								rightSide.referencingSet.begin(),
								rightSide.referencingSet.end()
							);
						}
					}
				}
			}
		}
	}
}

void ComputeReferencingSet(
	std::unordered_map<std::string, std::vector<RuleRightSide>> & rules,
	std::string const & nonterminal,
	RuleRightSide & rightSide
)
{
	Symbol firstSymbol = rightSide.sequence.front();
	if (firstSymbol.IsTerminal())
	{
		if (rightSide.referencingSet.empty())
		{
			rightSide.referencingSet.emplace(std::move(firstSymbol.GetValue()));
		}
	}
	else if (firstSymbol == EMPTY_CHARACTER_SYMBOL)
	{
		ComputeNextTerminals(
			rules,
			nonterminal,
			rightSide.referencingSet
		);
	}
	else
	{
		std::string const & nonterminal = firstSymbol.GetValue();
		std::vector<RuleRightSide> & rightSides = rules.at(nonterminal);
		for (RuleRightSide & nonterminalRightSide : rightSides)
		{
			if (nonterminalRightSide.referencingSet.empty())
			{
				ComputeReferencingSet(rules, nonterminal, nonterminalRightSide);
			}
			rightSide.referencingSet.insert(
				nonterminalRightSide.referencingSet.begin(),
				nonterminalRightSide.referencingSet.end()
			);
		}
	}
}

void ComputeReferencingSets(std::unordered_map<std::string, std::vector<RuleRightSide>> & rules)
{
	for (auto & rule : rules)
	{
		std::string const & nonterminal = rule.first;
		std::vector<RuleRightSide> & rightSides = rule.second;
		for (
			RuleRightSide & rightSide : rightSides
				)
		{
			if (rightSide.referencingSet.empty())
			{
				ComputeReferencingSet(rules, nonterminal, rightSide);
			}
		}
	}
}

void WriteRule(std::string const & outputFileName, std::unordered_map<std::string, std::vector<RuleRightSide>> & rules)
{
	std::ofstream outputFile(outputFileName);
	for (auto const & rule : rules)
	{
		std::string const & nonterminal = rule.first;
		std::vector<RuleRightSide> const & rightSides = rule.second;
		for (RuleRightSide const & rightSide : rightSides)
		{
			outputFile << nonterminal << "-";
			std::copy(rightSide.sequence.begin(), rightSide.sequence.end(), std::ostream_iterator<Symbol>(outputFile));
			outputFile << "/";
			std::copy(rightSide.referencingSet.begin(), rightSide.referencingSet.end(), std::ostream_iterator<std::string>(outputFile, ","));
			outputFile << "\n";
		}
	}
}

int main(int argc, char * argv[])
{
	if (argc - 1 < REQUIRED_ARGC)
	{
		std::cerr << NO_ARGUMENT_ERROR << "\n";
		return EXIT_FAILURE;
	}

	std::string inputFileName = argv[1];
	std::string mainNonterminal;
	bool needAugment = false;
	std::unordered_map<std::string, std::vector<RuleRightSide>> rules;

	ReadRule(argv[1], mainNonterminal, needAugment, rules);

	if (needAugment)
	{
		AugmentRule(rules, mainNonterminal);
	}
	AddEndOfFileToken(rules.at(mainNonterminal));
	RemoveLeftRecursion(rules);
	Factorize(rules);
	ComputeReferencingSets(rules);

	WriteRule(inputFileName + ".precompiled", rules);

	return EXIT_SUCCESS;
}
