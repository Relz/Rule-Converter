#include <iostream>
#include <algorithm>
#include "InputLibrary/Input.h"
#include "TokenLibrary/Token.h"
#include "TokenLibrary/TokenExtensions/TokenExtensions.h"
#include "Symbol/Symbol.h"
#include "Node/Node.h"
#include "NonterminalHelper/NonterminalHelper.h"

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
	std::unordered_map<std::string, std::vector<std::vector<Symbol>>> & nonterminalSequences)
{
	Input input(inputFileName);
	while (!input.IsEndOfStream())
	{
		std::string nonterminal;
		ParseNonterminalDeclaration(input, nonterminal);

		if (mainNonterminal.empty())
		{
			mainNonterminal = nonterminal;
		}

		nonterminalSequences.try_emplace(nonterminal, std::vector<std::vector<Symbol>>());
		nonterminalSequences.at(nonterminal).emplace_back(std::vector<Symbol>());

		ReadNonterminalSequence(input, nonterminalSequences.at(nonterminal).back(), mainNonterminal, needAugment);

		input.SkipLine();
	}
}

void AugmentRule(
	std::unordered_map<std::string, std::vector<std::vector<Symbol>>> & nonterminalSequences, std::string & mainNonterminal)
{
	nonterminalSequences.try_emplace(AUGMENT_NONTERMINAL, std::vector<std::vector<Symbol>>());
	nonterminalSequences.at(AUGMENT_NONTERMINAL).emplace_back(std::vector<Symbol>());
	Symbol nonterminal;
	Symbol::CreateNonterminal(mainNonterminal, nonterminal);
	nonterminalSequences.at(AUGMENT_NONTERMINAL).back().emplace_back(nonterminal);
	mainNonterminal = AUGMENT_NONTERMINAL;
}

void AddEndOfFileToken(std::vector<std::vector<Symbol>> & mainNonterminalSequences)
{
	for (std::vector<Symbol> & mainNonterminalSequence : mainNonterminalSequences)
	{
		Symbol terminal;
		Symbol::CreateTerminal(TokenExtensions::ToString(Token::END_OF_FILE), terminal);
		mainNonterminalSequence.emplace_back(terminal);
	}
}

void RemoveLeftRecursion(std::unordered_map<std::string, std::vector<std::vector<Symbol>>> & nonterminalSequences)
{
	for (auto & nonterminalSequence : nonterminalSequences)
	{
		std::string const & nonterminal = nonterminalSequence.first;
		std::vector<std::vector<Symbol>> & sequences = nonterminalSequence.second;

		sequences.erase(std::remove_if(sequences.begin(), sequences.end(), [nonterminal](std::vector<Symbol> const & sequence){
			return sequence.front().IsValueEquals(nonterminal) && sequence.size() == 1;
		}), sequences.end());

		for (std::vector<Symbol> const & sequence : sequences)
		{
			if (!sequence.front().IsValueEquals(nonterminal))
			{
				continue;
			}
			std::string newNonterminalName;
			NonterminalHelper::GenerateNonterminalName(
				newNonterminalName,
				[&nonterminalSequences](std::string const & possibleNonterminalName) {
					return nonterminalSequences.find(possibleNonterminalName) == nonterminalSequences.end();
				}
			);
			for (std::vector<Symbol> & sequence : sequences)
			{
				Symbol newNonterminal;
				Symbol::CreateNonterminal(newNonterminalName, newNonterminal);
				sequence.emplace_back(newNonterminal);

				if (sequence.front().IsValueEquals(nonterminal))
				{
					sequence.erase(sequence.begin());
					nonterminalSequences.emplace(
						newNonterminalName,
						std::vector<std::vector<Symbol>>(1, { EMPTY_CHARACTER_SYMBOL })
					);
				}
			}
		}
	}
}

void ResolveNode(
	Node * node,
	std::unordered_map<std::string, std::vector<std::vector<Symbol>>> & nonterminalSequences,
	std::string const & nonterminal
)
{
	std::vector<std::vector<Symbol>> & sequences = nonterminalSequences[nonterminal];
	sequences.emplace_back(std::vector<Symbol>());
	while (node->nextNodes.size() == 1)
	{
		sequences.back().emplace_back(node->symbol);
		node = node->nextNodes.begin()->second;
	}
	sequences.back().emplace_back(node->symbol);
	if (node->nextNodes.empty())
	{
		return;
	}
	std::string newNonterminalName;
	NonterminalHelper::GenerateNonterminalName(
		newNonterminalName,
		[&nonterminalSequences](std::string const & possibleNonterminalName)
		{
			return nonterminalSequences.find(possibleNonterminalName) == nonterminalSequences.end();
		}
	);
	Symbol newNonterminal;
	Symbol::CreateNonterminal(newNonterminalName, newNonterminal);
	sequences.back().emplace_back(newNonterminal);
	for (auto const & symbolNextNodes : node->nextNodes)
	{
		ResolveNode(symbolNextNodes.second, nonterminalSequences, newNonterminalName);
	}
}

void Factorize(std::unordered_map<std::string, std::vector<std::vector<Symbol>>> & nonterminalSequences)
{
	for (auto & nonterminalSequence : nonterminalSequences)
	{
		std::string const & nonterminal = nonterminalSequence.first;
		std::vector<std::vector<Symbol>> & sequences = nonterminalSequence.second;

		std::unordered_map<Symbol, Node*> treesRoots { };
		for (size_t ruleIndex = 0; ruleIndex < sequences.size(); ++ruleIndex)
		{
			std::vector<Symbol> const & sequence = sequences.at(ruleIndex);
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
		sequences.clear();
		for (auto const & treesRoot : treesRoots)
		{
			Node * node = treesRoot.second;
			ResolveNode(node, nonterminalSequences, nonterminal);
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

	std::string mainNonterminal;
	bool needAugment = false;
	std::unordered_map<std::string, std::vector<std::vector<Symbol>>> nonterminalSequences;

	ReadRule(argv[1], mainNonterminal, needAugment, nonterminalSequences);

	if (needAugment)
	{
		AugmentRule(nonterminalSequences, mainNonterminal);
	}
	AddEndOfFileToken(nonterminalSequences.at(mainNonterminal));
	RemoveLeftRecursion(nonterminalSequences);
	Factorize(nonterminalSequences);

	for (auto const & nonterminalSequence : nonterminalSequences)
	{
		std::string const & nonterminal = nonterminalSequence.first;
		for (std::vector<Symbol> const & sequence : nonterminalSequence.second)
		{
			std::cout << nonterminal << "-";
			std::copy(sequence.begin(), sequence.end(), std::ostream_iterator<Symbol>(std::cout));
			std::cout << "\n";
		}
	}

	return EXIT_SUCCESS;
}
